#include "crow.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <iostream>
#include "bcrypt/BCrypt.hpp"
#include "jwt-cpp/include/jwt-cpp/jwt.h"

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>

// Secret used for signing JWT tokens
const std::string JWT_SECRET = "yourSuperSecretKey123!";

// ----- Priority Scheduler ----- //
// Each task wraps a function that returns a crow::response.
// The task also carries a priority (lower numbers mean higher priority).
struct RequestTask {
    int priority;
    std::function<crow::response()> task;
    std::promise<crow::response> promise;
};

// Custom comparator for the priority queue.
struct TaskComparator {
    bool operator()(const std::shared_ptr<RequestTask>& a, const std::shared_ptr<RequestTask>& b) {
        return a->priority > b->priority;  // Lower number = higher priority.
    }
};

class PriorityScheduler {
public:
    PriorityScheduler(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                while (true) {
                    std::shared_ptr<RequestTask> currentTask;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !taskQueue.empty(); });
                        if (stop && taskQueue.empty())
                            return;
                        currentTask = taskQueue.top();
                        taskQueue.pop();
                    }
                    try {
                        crow::response res = currentTask->task();
                        currentTask->promise.set_value(std::move(res));
                    } catch (...) {
                        currentTask->promise.set_exception(std::current_exception());
                    }
                }
            });
        }
    }

    std::future<crow::response> scheduleTask(int priority, std::function<crow::response()> func) {
        auto task = std::make_shared<RequestTask>();
        task->priority = priority;
        task->task = func;
        std::future<crow::response> fut = task->promise.get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("Scheduler has been stopped.");
            taskQueue.push(task);
        }
        condition.notify_one();
        return fut;
    }

    ~PriorityScheduler() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }
private:
    std::vector<std::thread> workers;
    std::priority_queue<std::shared_ptr<RequestTask>,
                        std::vector<std::shared_ptr<RequestTask>>,
                        TaskComparator> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

// Create a global scheduler with 4 worker threads.
PriorityScheduler scheduler(4);

// ----- End Priority Scheduler ----- //

int main() {
    crow::SimpleApp app;

    // GET /api/users - Priority 3
    CROW_ROUTE(app, "/api/users").methods("GET"_method)([]() {
        auto futureResponse = scheduler.scheduleTask(3, []() -> crow::response {
            crow::response res;
            try {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "your_new_password"));
                con->setSchema("banking_system");

                std::unique_ptr<sql::Statement> stmt(con->createStatement());
                std::unique_ptr<sql::ResultSet> result(stmt->executeQuery("SELECT * FROM users"));

                crow::json::wvalue users;
                int i = 0;
                while (result->next()) {
                    users[i]["id"] = result->getInt("user_id");
                    users[i]["name"] = result->getString("username");
                    users[i]["email"] = result->getString("email");
                    i++;
                }
                res.code = 200;
                res.set_header("Content-Type", "application/json");
                res.write(users.dump());
            } catch (const sql::SQLException& e) {
                std::cerr << "❌ GET /api/users SQL Error: " << e.what() << std::endl;
                res.code = 500;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Database error\"}");
            }
            return res;
        });
        return futureResponse.get();
    });

    // POST /api/users/signup - Priority 2
    CROW_ROUTE(app, "/api/users/signup").methods("POST"_method)([](const crow::request& req) {
        auto futureResponse = scheduler.scheduleTask(2, [req]() -> crow::response {
            crow::response res;
            auto body = crow::json::load(req.body);
            if (!body) {
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Invalid JSON\"}");
                return res;
            }
            std::string email = body["email"].s();
            std::string password = body["password"].s();
            std::string nickname = body["nickname"].s();
            try {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "your_new_password"));
                con->setSchema("banking_system");
                std::unique_ptr<sql::PreparedStatement> checkStmt(
                    con->prepareStatement("SELECT email FROM users WHERE email = ?"));
                checkStmt->setString(1, email);
                std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());
                if (checkRes->next()) {
                    res.code = 400;
                    res.set_header("Content-Type", "application/json");
                    res.write("{\"error\": \"Email already exists.\"}");
                    return res;
                }
                std::string hashedPassword = generateHash(password);
                std::unique_ptr<sql::PreparedStatement> insertStmt(
                    con->prepareStatement("INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)"));
                insertStmt->setString(1, nickname);
                insertStmt->setString(2, email);
                insertStmt->setString(3, hashedPassword);
                insertStmt->execute();
                crow::json::wvalue resBody;
                resBody["message"] = "User created successfully.";
                res.code = 201;
                res.set_header("Content-Type", "application/json");
                res.write(resBody.dump());
            } catch (const sql::SQLException& e) {
                res.code = 500;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Database error\"}");
            }
            return res;
        });
        return futureResponse.get();
    });

    // POST /api/users/login - Priority 1
    CROW_ROUTE(app, "/api/users/login").methods("POST"_method)([](const crow::request& req) {
        auto futureResponse = scheduler.scheduleTask(1, [req]() -> crow::response {
            crow::response res;
            auto body = crow::json::load(req.body);
            if (!body) {
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Invalid JSON\"}");
                return res;
            }
            std::string email = body["email"].s();
            std::string password = body["password"].s();
            try {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "your_new_password"));
                con->setSchema("banking_system");
                std::unique_ptr<sql::PreparedStatement> stmt(
                    con->prepareStatement("SELECT * FROM users WHERE email = ?"));
                stmt->setString(1, email);
                std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
                if (!result->next()) {
                    res.code = 401;
                    res.set_header("Content-Type", "application/json");
                    res.write("{\"error\": \"Invalid email or password\"}");
                    return res;
                }
                std::string dbPasswordHash = result->getString("password_hash");
                if (!validatePassword(password, dbPasswordHash)) {
                    res.code = 401;
                    res.set_header("Content-Type", "application/json");
                    res.write("{\"error\": \"Invalid email or password\"}");
                    return res;
                }
                auto token = jwt::create()
                    .set_issuer("SquidBank")
                    .set_type("JWS")
                    .set_payload_claim("email", jwt::claim(email))
                    .set_payload_claim("nickname", jwt::claim(result->getString("username")))
                    .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
                    .sign(jwt::algorithm::hs256{JWT_SECRET});
                crow::json::wvalue resBody;
                resBody["message"] = "Login successful";
                resBody["token"] = token;
                resBody["user"]["email"] = email;
                resBody["user"]["nickname"] = result->getString("username");
                resBody["user"]["id"] = result->getInt("user_id");
                res.code = 200;
                res.set_header("Content-Type", "application/json");
                res.write(resBody.dump());
            } catch (const sql::SQLException& e) {
                res.code = 500;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Database error\"}");
            }
            return res;
        });
        return futureResponse.get();
    });

    // GET /api/users/<int>/accounts - Priority 2
    CROW_ROUTE(app, "/api/users/<int>/accounts").methods("GET"_method)([](int userId) {
        auto futureResponse = scheduler.scheduleTask(2, [userId]() -> crow::response {
            crow::response res;
            try {
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "your_new_password"));
                con->setSchema("banking_system");
                std::unique_ptr<sql::PreparedStatement> stmt(
                    con->prepareStatement("SELECT account_type, balance FROM accounts WHERE user_id = ?")
                );
                stmt->setInt(1, userId);
                std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
                crow::json::wvalue accounts;
                int i = 0;
                while (result->next()) {
                    accounts[i]["account_type"] = result->getString("account_type");
                    accounts[i]["balance"] = static_cast<double>(result->getDouble("balance"));
                    i++;
                }
                res.code = 200;
                res.set_header("Content-Type", "application/json");
                res.write(accounts.dump());
            } catch (const sql::SQLException& e) {
                std::cerr << "❌ GET /api/users/<id>/accounts SQL Error: " << e.what() << std::endl;
                res.code = 500;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Database error\"}");
            }
            return res;
        });
        return futureResponse.get();
    });

    // POST /api/transfer - Priority 1 (as before)
    CROW_ROUTE(app, "/api/transfer").methods("POST"_method)([](const crow::request& req) {
        int priority = 1;
        auto futureResponse = scheduler.scheduleTask(priority, [req]() -> crow::response {
            crow::response res;
            auto body = crow::json::load(req.body);
            if (!body) {
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Invalid JSON\"}");
                return res;
            }
            int userId = body["user_id"].i();
            std::string fromAccountType = body["from_account"].s();
            std::string toAccountType = body["to_account"].s();
            double amount = body["amount"].d();
            if (amount <= 0) {
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Transfer amount must be greater than zero.\"}");
                return res;
            }
            const int maxRetries = 3;
            int retryCount = 0;
            bool success = false;
            while (retryCount < maxRetries && !success) {
                try {
                    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                    std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "your_new_password"));
                    con->setSchema("banking_system");
                    con->setAutoCommit(false);
                    
                    std::unique_ptr<sql::PreparedStatement> selectFromStmt(
                        con->prepareStatement("SELECT account_id, balance FROM accounts WHERE user_id = ? AND account_type = ? FOR UPDATE"));
                    selectFromStmt->setInt(1, userId);
                    selectFromStmt->setString(2, fromAccountType);
                    std::unique_ptr<sql::ResultSet> fromResult(selectFromStmt->executeQuery());
                    if (!fromResult->next()) {
                        con->rollback();
                        res.code = 400;
                        res.set_header("Content-Type", "application/json");
                        res.write("{\"error\": \"Source account not found.\"}");
                        return res;
                    }
                    double fromBalance = fromResult->getDouble("balance");
                    int fromAccountId = fromResult->getInt("account_id");
                    if (fromBalance < amount) {
                        con->rollback();
                        res.code = 400;
                        res.set_header("Content-Type", "application/json");
                        res.write("{\"error\": \"Insufficient funds in source account.\"}");
                        return res;
                    }
                    std::unique_ptr<sql::PreparedStatement> selectToStmt(
                        con->prepareStatement("SELECT account_id, balance FROM accounts WHERE user_id = ? AND account_type = ? FOR UPDATE"));
                    selectToStmt->setInt(1, userId);
                    selectToStmt->setString(2, toAccountType);
                    std::unique_ptr<sql::ResultSet> toResult(selectToStmt->executeQuery());
                    if (!toResult->next()) {
                        con->rollback();
                        res.code = 400;
                        res.set_header("Content-Type", "application/json");
                        res.write("{\"error\": \"Destination account not found.\"}");
                        return res;
                    }
                    int toAccountId = toResult->getInt("account_id");
                    std::unique_ptr<sql::PreparedStatement> updateFromStmt(
                        con->prepareStatement("UPDATE accounts SET balance = balance - ? WHERE account_id = ?"));
                    updateFromStmt->setDouble(1, amount);
                    updateFromStmt->setInt(2, fromAccountId);
                    updateFromStmt->executeUpdate();
                    std::unique_ptr<sql::PreparedStatement> updateToStmt(
                        con->prepareStatement("UPDATE accounts SET balance = balance + ? WHERE account_id = ?"));
                    updateToStmt->setDouble(1, amount);
                    updateToStmt->setInt(2, toAccountId);
                    updateToStmt->executeUpdate();
                    con->commit();
                    success = true;
                    crow::json::wvalue resBody;
                    resBody["message"] = "Transfer successful.";
                    res.code = 200;
                    res.set_header("Content-Type", "application/json");
                    res.write(resBody.dump());
                    return res;
                } catch (const sql::SQLException &e) {
                    if (e.getErrorCode() == 1213) { // Deadlock error code in MySQL
                        ++retryCount;
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    } else {
                        try { /* attempt rollback if possible */ } catch (...) {}
                        res.code = 500;
                        res.set_header("Content-Type", "application/json");
                        res.write("{\"error\": \"Database error during transfer.\"}");
                        return res;
                    }
                }
            }
            if (!success) {
                res.code = 500;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Transfer failed due to database deadlock. Please try again later.\"}");
            }
            return res;
        });
        return futureResponse.get();
    });

    app.port(3000).multithreaded().run();
}

#include "crow.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <iostream>

int main() {
    crow::SimpleApp app;

    // Get all users
    CROW_ROUTE(app, "/api/users").methods("GET"_method)([]() {
        crow::response res;
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "Riadchahla13"));
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

    // Signup
    CROW_ROUTE(app, "/api/users/signup").methods("POST"_method)([](const crow::request& req) {
        crow::response res;
        auto body = crow::json::load(req.body);

        if (!body) {
            std::cerr << "❌ Invalid JSON in /signup" << std::endl;
            res.code = 400;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Invalid JSON\"}");
            res.end();
            return res;
        }

        std::string email = body["email"].s();
        std::string password = body["password"].s();
        std::string nickname = body["nickname"].s();

        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "Riadchahla13"));
            con->setSchema("banking_system");

            std::unique_ptr<sql::PreparedStatement> checkStmt(
                con->prepareStatement("SELECT email FROM users WHERE email = ?"));
            checkStmt->setString(1, email);
            std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());

            if (checkRes->next()) {
                std::cerr << "⚠️ Email already exists: " << email << std::endl;
                res.code = 400;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Email already exists.\"}");
                res.end();
                return res;
            }

            std::unique_ptr<sql::PreparedStatement> insertStmt(
                con->prepareStatement("INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)"));
            insertStmt->setString(1, nickname);
            insertStmt->setString(2, email);
            insertStmt->setString(3, password);
            insertStmt->execute();

            std::cerr << "✅ User created: " << email << std::endl;

            crow::json::wvalue resBody;
            resBody["message"] = "User created successfully.";
            res.code = 201;
            res.set_header("Content-Type", "application/json");
            res.write(resBody.dump());
            res.end();
            return res;

        } catch (const sql::SQLException& e) {
            std::cerr << "❌ SQL Error in /signup: " << e.what() << std::endl;
            res.code = 500;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Database error\"}");
            res.end();
            return res;
        }
    });

    // Login
    CROW_ROUTE(app, "/api/users/login").methods("POST"_method)([](const crow::request& req) {
        crow::response res;
        auto body = crow::json::load(req.body);

        if (!body) {
            std::cerr << "❌ Invalid JSON in /login" << std::endl;
            res.code = 400;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Invalid JSON\"}");
            res.end();
            return res;
        }

        std::string email = body["email"].s();
        std::string password = body["password"].s();

        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "ADD_YOUR_MYSQL_PASSWROD_HERE"));
            con->setSchema("banking_system");

            std::unique_ptr<sql::PreparedStatement> stmt(
                con->prepareStatement("SELECT * FROM users WHERE email = ?"));
            stmt->setString(1, email);
            std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());

            if (!result->next()) {
                std::cerr << "❌ Email not found: " << email << std::endl;
                res.code = 401;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Invalid email or password\"}");
                res.end();
                return res;
            }

            std::string dbPassword = result->getString("password_hash");

            if (password != dbPassword) {
                std::cerr << "❌ Incorrect password for: " << email << std::endl;
                res.code = 401;
                res.set_header("Content-Type", "application/json");
                res.write("{\"error\": \"Invalid email or password\"}");
                res.end();
                return res;
            }

            std::cerr << "✅ Login successful: " << email << std::endl;

            crow::json::wvalue resBody;
            resBody["message"] = "Login successful";
            resBody["user"]["email"] = result->getString("email");
            resBody["user"]["nickname"] = result->getString("username");

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(resBody.dump());
            res.end();
            return res;

        } catch (const sql::SQLException& e) {
            std::cerr << "❌ SQL Error in /login: " << e.what() << std::endl;
            res.code = 500;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Database error\"}");
            res.end();
            return res;
        }
    });

    app.port(3000).multithreaded().run();
}

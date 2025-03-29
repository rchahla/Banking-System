#include "crow.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <iostream>
#include "bcrypt/BCrypt.hpp"
#include "jwt-cpp/include/jwt-cpp/jwt.h"

const std::string JWT_SECRET = "yourSuperSecretKey123!";

int main() {
    crow::SimpleApp app;

    // Get all users (for testing, avoid exposing passwords!)
    CROW_ROUTE(app, "/api/users").methods("GET"_method)([]() {
        crow::response res;
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "ADD_YOUR_MYSQL_PASSWORD_HERE"));
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
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "ADD_YOUR_MYSQL_PASSWORD_HERE"));
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
            return res;

        } catch (const sql::SQLException& e) {
            res.code = 500;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Database error\"}");
            return res;
        }
    });

    // Login
    CROW_ROUTE(app, "/api/users/login").methods("POST"_method)([](const crow::request& req) {
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
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "ADD_YOUR_MYSQL_PASSWORD_HERE"));
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

            // ✅ Generate JWT token
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

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(resBody.dump());
            return res;

        } catch (const sql::SQLException& e) {
            res.code = 500;
            res.set_header("Content-Type", "application/json");
            res.write("{\"error\": \"Database error\"}");
            return res;
        }
    });

    app.port(3000).multithreaded().run();
}

#include "crow_all.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <iostream>

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/users").methods("GET"_method)([]() {
        crow::response res;
        try {
            sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "root", "Riadchahla13"));
            con->setSchema("banking_system");

            std::unique_ptr<sql::Statement> stmt(con->createStatement());
            std::unique_ptr<sql::ResultSet> result(stmt->executeQuery("SELECT * FROM users"));

            crow::json::wvalue users;
            int i = 0;
            while (result->next()) {
                users[i]["id"] = result->getInt("id");
                users[i]["name"] = result->getString("name");
                users[i]["email"] = result->getString("email");
                i++;
            }

            res.code = 200;
            res.set_header("Content-Type", "application/json");
            res.write(users.dump());
        } catch (const sql::SQLException& e) {
            res.code = 500;
            res.write("Database error: " + std::string(e.what()));
        }
        return res;
    });

    app.port(3000).multithreaded().run();
}

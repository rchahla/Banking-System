const express = require("express");
const mysql = require("mysql2");
const app = express();
const PORT = 3000;

//Parse data in body as JASON
app.use(express.json());

// Create a MySQL connection
const db = mysql.createConnection({
  host: "localhost", // Replace with your host
  user: "root", // Replace with your MySQL username
  password: "Riadchahla13", // Replace with your MySQL password
  database: "banking_system", // Database name
});

// Connect to the database
db.connect((err) => {
  if (err) {
    console.error("Error connecting to the database:", err.message);
    return;
  }
  console.log("Connected to the MySQL database!");
});

// Route to fetch all users
app.get("/api/users", (req, res) => {
  const query = "SELECT * FROM users"; // SQL query to fetch all users
  db.query(query, (err, results) => {
    if (err) {
      console.error("Error fetching users:", err.message);
      res.status(500).send("Error fetching users");
      return;
    }
    res.json(results); // Send the query results as JSON
  });
});

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});

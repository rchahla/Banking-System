import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import "./LoginSignup.css";

import SquidLogo from "../../assets/Images/SquidLogo.webp";
import user_icon from "../../assets/Images/person.png";
import email_icon from "../../assets/Images/email.png";
import password_icon from "../../assets/Images/password.png";

const LoginSignup = () => {
  const [action, setAction] = useState("Login");
  const [nickname, setNickname] = useState("");
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [emailError, setEmailError] = useState("");
  const [message, setMessage] = useState("");

  const navigate = useNavigate();

  const validateEmail = (email) => /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);

  const handleEmailChange = (e) => {
    const value = e.target.value;
    setEmail(value);
    setEmailError(validateEmail(value) ? "" : "Please enter a valid email !");
  };

  const handleSubmit = () => {
    if (!validateEmail(email)) {
      alert("Invalid email. Please correct it before submitting.");
      return;
    }

    const endpoint =
      action === "Login" ? "/api/users/login" : "/api/users/signup";
    const payload =
      action === "Login" ? { email, password } : { email, password, nickname };

    fetch(endpoint, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload),
    })
      .then((res) => res.json())
      .then((data) => {
        console.log(data);

        if (data.error) {
          alert(data.error);
          setMessage(data.error);
        } else {
          alert(data.message || "Success!");
          setMessage(data.message || "Success!");

          if (data.message === "Login successful") {
            // ✅ Store user session info
            localStorage.setItem("token", data.token);
            localStorage.setItem("nickname", data.user.nickname);
            localStorage.setItem("email", data.user.email);
            localStorage.setItem("user_id", data.user.id);

            navigate("/homepage");
          } else {
            setNickname("");
            setEmail("");
            setPassword("");
          }
        }
      })
      .catch((err) => {
        console.error("Error:", err);
        alert("Server error");
        setMessage("Server error.");
      });
  };

  const handleKeyPress = (e) => {
    if (e.key === "Enter") handleSubmit();
  };

  return (
    <div className="LoginSignup">
      <div className="title">
        <h1>Squid</h1>
        <img
          src={SquidLogo}
          alt="Squid Logo"
          style={{ width: "120px", height: "auto", margin: "0 10px" }}
        />
        <h1>Bank</h1>
      </div>

      <div
        className={`container ${
          action === "Login" ? "login-height" : "signup-height"
        }`}
      >
        <div className="header2">
          <div className="text">{action}</div>
          <div className="underline"></div>
        </div>

        <form onKeyPress={handleKeyPress}>
          <div className="inputs">
            {action === "Login" ? null : (
              <div className="input">
                <img src={user_icon} alt="User" />
                <input
                  type="text"
                  placeholder="First and Lastname"
                  value={nickname}
                  onChange={(e) => setNickname(e.target.value)}
                />
              </div>
            )}
            <div className="input">
              <img src={email_icon} alt="Email" />
              <input
                type="email"
                placeholder="Email"
                value={email}
                onChange={handleEmailChange}
              />
            </div>
            {emailError && <p className="error">{emailError}</p>}
            <div className="input">
              <img src={password_icon} alt="Password" />
              <input
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
              />
            </div>
          </div>

          {message && <p className="message">{message}</p>}

          <div className="submit-container">
            <div
              className={action === "Login" ? "submit gray" : "submit"}
              onClick={() => setAction("Sign Up")}
            >
              Sign Up
            </div>
            <div
              className={action === "Sign Up" ? "submit gray" : "submit"}
              onClick={() => setAction("Login")}
            >
              Login
            </div>
            <div className="submit" onClick={handleSubmit}>
              Submit
            </div>
          </div>
        </form>
      </div>
    </div>
  );
};

export default LoginSignup;

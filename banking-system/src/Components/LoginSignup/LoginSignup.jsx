import React, { useState } from "react";
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

      <div className="container">
        <div className="header2">
          <div className="text">{action}</div>
          <div className="underline"></div>
        </div>

        <div className="inputs">
          {action === "Login" ? null : (
            <div className="input">
              <img src={user_icon} alt="User" />
              <input
                type="text"
                placeholder="Nickname"
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
              onChange={(e) => setEmail(e.target.value)}
            />
          </div>
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
        </div>
      </div>
    </div>
  );
};

export default LoginSignup;

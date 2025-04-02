import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import "./HomePage.css";

import SquidLogo from "../../assets/Images/SquidLogo.webp";
import background from "../../assets/Images/Hp.jpg";
import user_icon from "../../assets/Images/person.png";
import logout_icon from "../../assets/Images/logout.png";

const HomePage = () => {
  const [accounts, setAccounts] = useState([]);

  const token = localStorage.getItem("token");
  const email = localStorage.getItem("email");
  const nickname = localStorage.getItem("nickname");
  const formattedNickname = nickname ? nickname.toUpperCase() : nickname;
  const user_id = localStorage.getItem("user_id");

  const navigate = useNavigate();

  const handleSignOut = () => {
    localStorage.clear();
    navigate("/");
  };

  useEffect(() => {
    if (!user_id) {
      console.error("No user ID found");
      return;
    }

    fetch(`/api/users/${user_id}/accounts`, {
      method: "GET",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${token}`,
      },
    })
      .then((res) => res.json())
      .then((data) => {
        console.log("Accounts data: ", data);
        setAccounts(data);
      })
      .catch((err) => {
        console.error("Failed to fetch account info:", err);
      });
  }, []);

  return (
    <div className="homepage-container">
      <div className="top-bar">
        <div className="logo-text-wrapper">
          <img src={SquidLogo} alt="Squid Logo" className="logo-img" />
          <h1 className="logo-text">Squid Bank</h1>
        </div>

        <div className="logo-user">
          <img src={user_icon} alt="User Logo" className="logo-img2" />
          <h1 className="logo-text2">{formattedNickname}</h1>
        </div>

        <div className="signout-wrapper" onClick={handleSignOut}>
          <img src={logout_icon} alt="Logout Logo" style={{ width: "30px" }} />
          <h1 style={{ fontSize: "18px", paddingRight: "10px" }}>Sign Out</h1>
        </div>
      </div>

      <div className="seperator">
        <h1 style={{ color: "white", fontSize: "18px" }}>My Account</h1>
      </div>

      <div className="account-container">
        <img src={background} alt="Mountains" className="homepage-bg" />
        <div className="account-summary">
          <h1 className="account-info">Account Summary</h1>
          <h2>GOOD MORNING, {formattedNickname}</h2>
        </div>
      </div>

      <div className="bottom-container">
        <div className="bank-container">
          <div className="bank-text">
            <h1>Bank Accounts</h1>
            <div className="black-underline"></div>
          </div>

          <div className="checking-account">
            {accounts ? (
              accounts.map((account, index) => (
                <div
                  key={index}
                  onClick={() => {
                    account.account_type === "Checking"
                      ? console.log("Checking account clicked")
                      : console.log("Savings account clicked");
                  }}
                >
                  <h3>SB {account.account_type}</h3>
                  <p>Balance: ${account.balance.toFixed(2)} CAD </p>
                </div>
              ))
            ) : (
              <p>Balance: </p>
            )}
          </div>

          <div className="bank-text">
            <h1>Credit Cards</h1>
            <div className="black-underline"></div>
          </div>
        </div>

        <div className="transfers-container"></div>
      </div>

      {/* You can add more sections below here and layout will expand properly */}
    </div>
  );
};

export default HomePage;

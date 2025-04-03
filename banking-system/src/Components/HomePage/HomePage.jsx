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
  const nickname = localStorage.getItem("nickname");
  const formattedNickname = nickname ? nickname.toUpperCase() : "";
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
  }, [user_id, token]);

  const handleTransfer = () => {
    const fromAccount = document.getElementById("fromAccountDropdown").value;
    const toAccount = document.getElementById("toAccountDropdown").value;
    const amountInput = document.querySelector(".amount-input").value;
    const amount = parseFloat(amountInput);

    if (!fromAccount || !toAccount || isNaN(amount)) {
      console.error("Invalid transfer details");
      return;
    }

    fetch("/api/transfer", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${token}`,
      },
      body: JSON.stringify({
        user_id: parseInt(user_id),
        from_account: fromAccount,
        to_account: toAccount,
        amount: amount,
      }),
    })
      .then((res) => res.json())
      .then((data) => {
        console.log("Transfer response: ", data);
        // Optionally refresh account balances or notify the user of success/failure
      })
      .catch((err) => {
        console.error("Transfer error:", err);
      });
  };

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
          <img
            src={logout_icon}
            alt="Logout Logo"
            style={{ width: "20px", paddingRight: "5px" }}
          />
          <h1
            style={{
              fontSize: "18px",
              paddingRight: "10px",
              fontWeight: "100",
            }}
          >
            Sign Out
          </h1>
        </div>
      </div>

      <div className="seperator">
        <h1 style={{ color: "white", fontSize: "18px" }}>My Account</h1>
      </div>

      <div className="account-container">
        <img src={background} alt="Background" className="homepage-bg" />
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
            {accounts && accounts.length > 0 ? (
              accounts.map((account, index) => (
                <div
                  key={index}
                  onClick={() => {
                    if (account.account_type === "Checking") {
                      console.log("Checking account clicked");
                    } else {
                      console.log("Savings account clicked");
                    }
                  }}
                >
                  <h3>SB {account.account_type}</h3>
                  <p>Balance: ${account.balance.toFixed(2)} CAD</p>
                </div>
              ))
            ) : (
              <p>No accounts found</p>
            )}
          </div>

          <div className="bank-text">
            <h1>Credit Cards</h1>
            <div className="black-underline"></div>
          </div>
        </div>

        <div className="transfers-container">
          <h1 className="transfers-title">Transfers and Payments</h1>
          <div className="transfers">
            <h1 className="transfers-text">From</h1>
            <select className="dropdown" id="fromAccountDropdown">
              <option value="">Select Account</option>
              <option value="Checking">Checking Account</option>
              <option value="Savings">Savings Account</option>
              <option value="Credit">Credit Card</option>
            </select>
            <h1 className="transfers-text">To</h1>
            <select className="dropdown" id="toAccountDropdown">
              <option value="">Select Account</option>
              <option value="Checking">Checking Account</option>
              <option value="Savings">Savings Account</option>
              <option value="Credit">Credit Card</option>
            </select>
            <h1 className="transfers-text">Amount</h1>
            <div className="amount-input-container">
              <div className="currency-box">$</div>
              <input type="text" className="amount-input" placeholder="0.00" />
            </div>
            <div className="transfer-submit" onClick={handleTransfer}>
              <h1 className="submit-text">Submit</h1>
            </div>
          </div>
        </div>
      </div>

      {/* Additional sections can be added here */}
    </div>
  );
};

export default HomePage;

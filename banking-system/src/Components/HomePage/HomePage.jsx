import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import "./HomePage.css";

import SquidLogo from "../../assets/Images/SquidLogo.webp";
import background from "../../assets/Images/Hp.jpg";
import user_icon from "../../assets/Images/person.png";
import logout_icon from "../../assets/Images/logout.png";
import add_circle from "../../assets/Images/add-circle.webp";
import red_x from "../../assets/Images/red-x.png";

const HomePage = () => {
  const [accounts, setAccounts] = useState([]);
  const [showPopup, setShowPopup] = useState(false);
  const [firstName, setFirstName] = useState("");
  const [lastName, setLastName] = useState("");
  const [phoneNumber, setPhoneNumber] = useState("");
  const [email, setEmail] = useState("");
  const [income, setIncome] = useState("");
  const [accountType, setAccountType] = useState("");
  const [recipientEmail, setRecipientEmail] = useState("");
  const [recipients, setRecipients] = useState([]);
  const token = localStorage.getItem("token");
  const nickname = localStorage.getItem("nickname");
  const formattedNickname = nickname ? nickname.toUpperCase() : "";
  const user_id = localStorage.getItem("user_id");

  const navigate = useNavigate();

  const fetchAccounts = () => {
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
  };

  useEffect(() => {
    if (!user_id) return;
    fetchAccounts();
  }, [user_id, token]);

  const handleSignOut = () => {
    localStorage.clear();
    navigate("/");
  };

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
      })
      .catch((err) => {
        console.error("Transfer error:", err);
      });
  };

  const openBankAccount = () => {
    setShowPopup(false);
    fetch(`/api/users/${user_id}/accounts`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${token}`,
      },
      body: JSON.stringify({
        account_type: accountType,
        balance: 0.0,
      }),
    })
      .then((res) => res.json())
      .then((data) => {
        console.log("Account created:", data);

        // ✅ Reset inputs
        setFirstName("");
        setLastName("");
        setPhoneNumber("");
        setEmail("");
        setIncome("");
        setAccountType("");

        // Delay fetch to let backend commit and respond
        fetchAccounts();
      })
      .catch((err) => {
        console.error("Account creation error:", err);
      });
  };

  const addRecipients = () => {
    if (recipientEmail.trim() === "") return;

    // Check if already added
    if (recipients.includes(recipientEmail.trim())) {
      alert("Recipient already added.");
      return;
    }

    setRecipients((prev) => [...prev, recipientEmail.trim()]);
    setRecipientEmail(""); // Clear input box after adding
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
            {accounts &&
              accounts
                .filter(
                  (account) =>
                    account.account_type === "Checking" ||
                    account.account_type === "Savings"
                )
                .map((account, index) => (
                  <div className="account-balance" key={index}>
                    <h3>SB {account.account_type}</h3>
                    <p>Balance: ${account.balance.toFixed(2)} CAD</p>
                  </div>
                ))}

            {accounts.filter(
              (account) =>
                account.account_type === "Checking" ||
                account.account_type === "Savings"
            ).length < 2 && (
              <div className="open-account">
                <img
                  src={add_circle}
                  alt="Add Circle"
                  className="add-circle-logo"
                />
                <p
                  style={{ color: "#006ac3" }}
                  onClick={() => setShowPopup(true)}
                >
                  Open a Bank Account
                </p>
              </div>
            )}

            {showPopup && (
              <>
                <div
                  className="popup-overlay"
                  onClick={() => setShowPopup(false)}
                ></div>
                <div className="account-creation-container">
                  <div className="account-creation-title">
                    <h1 style={{ marginTop: "0px", fontSize: "24px" }}>
                      Bank Account Application
                    </h1>
                    <img
                      src={red_x}
                      alt="Red X"
                      className="red-x"
                      onClick={() => setShowPopup(false)}
                    />
                  </div>
                  <input
                    type="text"
                    placeholder="First Name"
                    className="account-creation-text"
                    value={firstName}
                    onChange={(e) => setFirstName(e.target.value)}
                  />
                  <input
                    type="text"
                    placeholder="Last Name"
                    className="account-creation-text"
                    value={lastName}
                    onChange={(e) => setLastName(e.target.value)}
                  />
                  <input
                    type="text"
                    placeholder="Phone Number"
                    className="account-creation-text"
                    value={phoneNumber}
                    onChange={(e) => setPhoneNumber(e.target.value)}
                  />
                  <input
                    type="text"
                    placeholder="Email"
                    className="account-creation-text"
                    value={email}
                    onChange={(e) => setEmail(e.target.value)}
                  />
                  <input
                    type="text"
                    placeholder="Yearly Income"
                    className="account-creation-text"
                    value={income}
                    onChange={(e) => setIncome(e.target.value)}
                  />
                  <select
                    className="dropdown"
                    value={accountType}
                    onChange={(e) => setAccountType(e.target.value)}
                  >
                    <option value="">Type Of Account</option>
                    <option value="Checking">Checking</option>
                    <option value="Savings">Savings Account</option>
                    <option value="Credit">Credit Card</option>
                  </select>
                  <div className="transfer-submit" onClick={openBankAccount}>
                    <h1 className="submit-text">Submit</h1>
                  </div>
                </div>
              </>
            )}
          </div>

          <div className="bank-text">
            <h1>Credit Cards</h1>
            <div
              className="black-underline"
              style={{ marginBottom: "10px" }}
            ></div>
            <div className="credit-account-section">
              {/* Show Credit Card account(s) if they exist */}
              {accounts
                .filter((account) => account.account_type === "Credit")
                .map((account, index) => (
                  <div className="account-balance" key={`credit-${index}`}>
                    <h3>SB {account.account_type}</h3>
                    <p>Balance: ${account.balance.toFixed(2)} CAD</p>
                  </div>
                ))}

              {/* Show Open Credit Card option if user doesn't already have one */}
              {!accounts.some(
                (account) => account.account_type === "Credit"
              ) && (
                <div className="open-account">
                  <img
                    src={add_circle}
                    alt="Add Circle"
                    className="add-circle-logo"
                  />
                  <p
                    style={{ color: "#006ac3" }}
                    onClick={() => setShowPopup(true)}
                  >
                    Open a Credit Card
                  </p>
                </div>
              )}
            </div>
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
              {recipients.map((email, index) => (
                <option key={`recipient-${index}`} value={email}>
                  {email}
                </option>
              ))}
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

        <div className="add-people-container">
          <h1 className="add-people-title">Add Recipients To eTransfer</h1>
          <div className="add-people">
            <input
              type="text"
              className="people-email"
              placeholder="Recipients Email Address"
              value={recipientEmail}
              onChange={(e) => setRecipientEmail(e.target.value)}
            />
            <div className="transfer-submit" onClick={addRecipients}>
              <h1 className="submit-text">Add</h1>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default HomePage;

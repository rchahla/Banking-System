import React from 'react';
import { useNavigate } from "react-router-dom";
import "./HomePage.css";

import SquidLogo from "../../assets/Images/SquidLogo.webp";
import background from "../../assets/Images/Hp.jpg";
import user_icon from "../../assets/Images/person.png";
import logout_icon from "../../assets/Images/logout.png";


const HomePage = () => {

    const email = localStorage.getItem("email");
    const nickname = localStorage.getItem("nickname");
    const formattedNickname= nickname ? nickname.toUpperCase():nickname;

    const navigate = useNavigate();

    const handleSignOut = () => {
        localStorage.clear(); // or remove specific items
        navigate("/"); // or your login page route
    };

    return (

        <div className='homepage-container'>


        <div className='logo-text-wrapper'>
            <img src={SquidLogo} alt="Squid Logo" className='logo-img' />
            <h1 className='logo-text'>Squid Bank</h1>
        </div>

        <div className='logo-text-wrapper2'>
            <img src={user_icon} alt="User Logo" className='logo-img2' />
            <h1 className='logo-text2'>{formattedNickname}</h1>
        </div>

        <div className='signout-wrapper' onClick={()=>{handleSignOut();console.log("Signout Clicked")}}>
        <img src={logout_icon} alt="Logout Logo" style={{width: "30px"}}/>
            <h1 style={{fontSize:"18px", paddingRight:"10px"}}>Sign Out</h1>
        </div>

        <div className='seperator'>
            <h1 style={{color:"white",fontSize:"18px"}}>My Account</h1>
        </div>


         <img src={background} alt="Mountains" className="homepage-bg" />

         <div className='account-summary'>
         <h1 className='account-info'>Account Summary</h1>
         <h2>GOOD MORNING, {formattedNickname}</h2>
         </div>

        <h1>Welcome {formattedNickname} To Your Bank Account !</h1>



        </div>
    );
};

export default HomePage;



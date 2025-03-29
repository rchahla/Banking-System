import React from 'react';
import "./HomePage.css";


const HomePage = () => {

    const email = localStorage.getItem("email");
    const nickname = localStorage.getItem("nickname");
    const formattedNickname= nickname ? nickname.charAt(0).toUpperCase() + nickname.slice(1) :"";


    return (

        <div className='homepage-container'>
        <h1>Welcome {formattedNickname} To Your Bank Account !</h1>

        <div className='checking-container'>
            <h1>{email}</h1>
        </div>


        </div>
    );
};

export default HomePage;



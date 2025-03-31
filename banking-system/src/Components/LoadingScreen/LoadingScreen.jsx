import React from 'react';
import "./LoadingScreen.css";


const LoadingScreen = () => {

    const email = localStorage.getItem("email");
    const nickname = localStorage.getItem("nickname");
    const formattedNickname= nickname ? nickname.charAt(0).toUpperCase() + nickname.slice(1) :"";


    return (

        <>

        </>
    );
};

export default LoadingScreen;



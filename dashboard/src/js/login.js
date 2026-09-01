import { auth } from "./firebase.js";

import {
    signInWithEmailAndPassword,
    sendPasswordResetEmail
} from "firebase/auth";


// ============================================================
// ELEMENTS
// ============================================================

const loginForm =
    document.getElementById("loginForm");

const emailInput =
    document.getElementById("email");

const passwordInput =
    document.getElementById("password");

const loginButton =
    document.getElementById("loginButton");

const loginMessage =
    document.getElementById("loginMessage");

const forgotPassword =
    document.getElementById("forgotPassword");


// ============================================================
// MESSAGE
// ============================================================

function showMessage(message)
{
    loginMessage.textContent = message;
}


// ============================================================
// LOGIN BUTTON STATE
// ============================================================

function setLoading(isLoading)
{
    loginButton.disabled = isLoading;

    const text =
        loginButton.querySelector("span:first-child");

    const arrow =
        loginButton.querySelector(".arrow");

    if (isLoading)
    {
        text.textContent = "Signing in...";
        arrow.textContent = "•••";
    }
    else
    {
        text.textContent = "Sign in";
        arrow.textContent = "→";
    }
}


// ============================================================
// FIREBASE ERROR
// ============================================================

function getFirebaseErrorMessage(error)
{
    switch (error.code)
    {
        case "auth/invalid-email":
            return "Please enter a valid email address.";

        case "auth/invalid-credential":
            return "Incorrect email or password.";

        case "auth/user-not-found":
            return "Incorrect email or password.";

        case "auth/wrong-password":
            return "Incorrect email or password.";

        case "auth/user-disabled":
            return "This account has been disabled.";

        case "auth/too-many-requests":
            return "Too many attempts. Please try again later.";

        case "auth/network-request-failed":
            return "Network error. Check your connection.";

        default:
            console.error(
                "[AUTH] Firebase error:",
                error
            );

            return "Unable to sign in. Please try again.";
    }
}


// ============================================================
// LOGIN
// ============================================================

loginForm.addEventListener(
    "submit",
    async (event) =>
    {
        event.preventDefault();

        showMessage("");

        const email =
            emailInput.value.trim();

        const password =
            passwordInput.value;


        if (!email || !password)
        {
            showMessage(
                "Please enter your email and password."
            );

            return;
        }


        setLoading(true);


        try
        {
            await signInWithEmailAndPassword(
                auth,
                email,
                password
            );


            console.log(
                "[AUTH] Login successful."
            );


            /*
             * Firebase has authenticated the user.
             *
             * The next page will be our dashboard.
             */

            window.location.href =
                "/dashboard.html";
        }
        catch (error)
        {
            showMessage(
                getFirebaseErrorMessage(error)
            );

            setLoading(false);
        }
    }
);


// ============================================================
// FORGOT PASSWORD
// ============================================================

forgotPassword.addEventListener(
    "click",
    async () =>
    {
        const email =
            emailInput.value.trim();


        if (!email)
        {
            showMessage(
                "Enter your email first."
            );

            emailInput.focus();

            return;
        }


        try
        {
            await sendPasswordResetEmail(
                auth,
                email
            );


            showMessage(
                "Password reset email sent."
            );

            console.log(
                "[AUTH] Password reset email sent."
            );
        }
        catch (error)
        {
            showMessage(
                getFirebaseErrorMessage(error)
            );
        }
    }
);
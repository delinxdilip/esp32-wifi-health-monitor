// ============================================================
// AUTHENTICATION
// ============================================================

import {
    signInWithEmailAndPassword,
    signOut,
    onAuthStateChanged,
    sendPasswordResetEmail
} from
    "https://www.gstatic.com/firebasejs/11.10.0/firebase-auth.js";

import { auth } from "./firebase.js";


// ============================================================
// LOGIN
// ============================================================

async function login(
    email,
    password
) {
    return await signInWithEmailAndPassword(
        auth,
        email,
        password
    );
}


// ============================================================
// LOGOUT
// ============================================================

async function logout()
{
    return await signOut(auth);
}


// ============================================================
// PASSWORD RESET
// ============================================================

async function resetPassword(
    email
) {
    return await sendPasswordResetEmail(
        auth,
        email
    );
}


// ============================================================
// AUTH STATE
// ============================================================

function observeAuthState(
    callback
) {
    return onAuthStateChanged(
        auth,
        callback
    );
}


// ============================================================
// EXPORT
// ============================================================

export {
    login,
    logout,
    resetPassword,
    observeAuthState
};
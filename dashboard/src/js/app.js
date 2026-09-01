import { onAuthStateChanged } from "https://www.gstatic.com/firebasejs/11.10.0/firebase-auth.js";
import { auth } from "./firebase.js";


// ============================================================
// APPLICATION ENTRY
// ============================================================

console.log("[APP] Starting application...");


// ============================================================
// AUTHENTICATION STATE
// ============================================================

onAuthStateChanged(auth, (user) => {

    if (user)
    {
        console.log(
            "[APP] User authenticated."
        );

        window.location.href =
            "/dashboard.html";

        return;
    }


    console.log(
        "[APP] User is not authenticated."
    );

    window.location.href =
        "/login.html";
});
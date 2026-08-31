// ============================================================
// ESP32 HEALTH MONITOR
// Frontend JavaScript
// ============================================================


// ============================================================
// HELPER
// ============================================================

function getElement(id) {
    return document.getElementById(id);
}


// ============================================================
// SETUP PAGE
// ============================================================

function initializeSetupPage() {

    const form = getElement("wifiForm");

    if (!form) {
        return;
    }

    const password =
        getElement("password");

    const passwordToggle =
        getElement("passwordToggle");

    const connectButton =
        getElement("connectButton");

    const buttonText =
        getElement("buttonText");

    const status =
        getElement("status");


    // --------------------------------------------------------
    // SHOW / HIDE PASSWORD
    // --------------------------------------------------------

    passwordToggle.addEventListener(
        "click",
        function () {

            if (password.type === "password") {

                password.type = "text";

                passwordToggle.textContent =
                    "Hide";

            } else {

                password.type = "password";

                passwordToggle.textContent =
                    "Show";
            }

        }
    );


    // --------------------------------------------------------
    // FORM SUBMISSION
    // --------------------------------------------------------

    form.addEventListener(
        "submit",
        async function (event) {

            event.preventDefault();


            const ssid =
                getElement("ssid").value.trim();

            const passwordValue =
                password.value;


            if (!ssid) {

                showStatus(
                    status,
                    "Please enter a Wi-Fi network name."
                );

                return;
            }


            // Disable button.

            connectButton.disabled = true;

            buttonText.textContent =
                "Connecting...";


            showStatus(
                status,
                "Testing Wi-Fi connection..."
            );


            try {

                const formData =
                    new URLSearchParams();

                formData.append(
                    "ssid",
                    ssid
                );

                formData.append(
                    "password",
                    passwordValue
                );


                const response =
                    await fetch(
                        "/save",
                        {
                            method: "POST",

                            headers: {
                                "Content-Type":
                                    "application/x-www-form-urlencoded"
                            },

                            body:
                                formData.toString()
                        }
                    );


                const html =
                    await response.text();


                if (!response.ok) {

                    throw new Error(
                        "Connection request failed."
                    );
                }


                showStatus(
                    status,
                    "Wi-Fi configuration submitted. " +
                    "The ESP32 is connecting..."
                );


                // Give the ESP32 time to finish.

                setTimeout(
                    function () {

                        window.location.href =
                            "/";

                    },
                    5000
                );

            } catch (error) {

                console.error(error);

                showStatus(
                    status,
                    "Unable to communicate with the ESP32."
                );

                connectButton.disabled = false;

                buttonText.textContent =
                    "Connect to Wi-Fi";
            }

        }
    );
}


// ============================================================
// DASHBOARD PAGE
// ============================================================

function initializeDashboardPage() {

    const dashboardStatus =
        getElement("dashboardStatus");

    if (!dashboardStatus) {
        return;
    }


    // Initial status request.

    updateDashboard();


    // Refresh every 2 seconds.

    setInterval(
        updateDashboard,
        2000
    );


    // Reconfigure button.

    const reconfigureButton =
        getElement("reconfigureButton");


    reconfigureButton.addEventListener(
        "click",
        async function () {

            const confirmed =
                window.confirm(
                    "Are you sure you want to " +
                    "reconfigure Wi-Fi?\n\n" +
                    "The ESP32 will disconnect from " +
                    "the current network and enter " +
                    "Wi-Fi setup mode."
                );


            if (!confirmed) {
                return;
            }


            reconfigureButton.disabled =
                true;


            reconfigureButton.textContent =
                "Starting setup...";


            try {

                const response =
                    await fetch(
                        "/reconfigure",
                        {
                            method: "POST"
                        }
                    );


                if (!response.ok) {

                    throw new Error(
                        "Unable to start Wi-Fi setup."
                    );
                }


                showStatus(
                    dashboardStatus,
                    "Wi-Fi setup mode is starting. " +
                    "Connect your phone to " +
                    "ESP32-WiFi-Setup."
                );


                /*
                 * The ESP32 will shut down the normal
                 * Wi-Fi connection and create its AP.
                 *
                 * Give the browser a little time before
                 * navigating to the setup page.
                 */

                setTimeout(
                    function () {

                        window.location.href =
                            "http://192.168.4.1/";

                    },
                    2500
                );


            } catch (error) {

                console.error(error);

                showStatus(
                    dashboardStatus,
                    "Could not start Wi-Fi setup."
                );


                reconfigureButton.disabled =
                    false;

                reconfigureButton.textContent =
                    "Reconfigure Wi-Fi";
            }

        }
    );
}


// ============================================================
// DASHBOARD DATA
// ============================================================

async function updateDashboard() {

    try {

        const response =
            await fetch(
                "/status",
                {
                    cache: "no-store"
                }
            );


        if (!response.ok) {

            throw new Error(
                "Status request failed."
            );
        }


        const data =
            await response.json();


        updateElement(
            "ssid",
            data.ssid
        );

        updateElement(
            "deviceId",
            data.deviceId
        );

        updateElement(
            "edgeName",
            data.edgeName
        );

        updateElement(
            "model",
            data.model
        );

        updateElement(
            "firmware",
            data.firmware
        );


        updateElement(
            "rssi",
            data.rssi
        );


        updateElement(
            "quality",
            data.quality
        );

        updateElement(
            "temperature",
            data.temperature
        );


        updateElement(
            "ip",
            data.ip
        );


        updateElement(
            "uptime",
            data.uptime
        );


        updateElement(
            "heap",
            data.heap
        );


        updateElement(
            "flash",
            data.flash
        );


        updateElement(
            "firmware",
            data.firmware
        );


        // Device is online.

        setConnectionStatus(
            true
        );


        updateElement(
            "lastUpdated",
            new Date().toLocaleTimeString()
        );


    } catch (error) {

        console.error(
            "Dashboard update failed:",
            error
        );


        setConnectionStatus(
            false
        );

    }
}


// ============================================================
// UPDATE ELEMENT
// ============================================================

function updateElement(
    id,
    value
) {

    const element =
        getElement(id);


    if (!element) {
        return;
    }


    if (
        value === undefined ||
        value === null
    ) {

        element.textContent =
            "—";

        return;
    }


    element.textContent =
        value;
}


// ============================================================
// CONNECTION STATUS
// ============================================================

function setConnectionStatus(
    online
) {

    const status =
        getElement(
            "connectionStatus"
        );

    const indicator =
        getElement(
            "connectionIndicator"
        );


    if (!status || !indicator) {
        return;
    }


    if (online) {

        status.textContent =
            "Online";

        status.style.color =
            "var(--success)";

        indicator.style.background =
            "var(--success)";

        indicator.style.boxShadow =
            "0 0 10px var(--success)";

    } else {

        status.textContent =
            "Offline";

        status.style.color =
            "var(--danger)";

        indicator.style.background =
            "var(--danger)";

        indicator.style.boxShadow =
            "0 0 10px var(--danger)";
    }
}


// ============================================================
// STATUS MESSAGE
// ============================================================

function showStatus(
    element,
    message
) {

    if (!element) {
        return;
    }


    element.textContent =
        message;


    element.classList.remove(
        "hidden"
    );
}


// ============================================================
// INITIALIZE
// ============================================================

document.addEventListener(
    "DOMContentLoaded",
    function () {

        initializeSetupPage();

        initializeDashboardPage();

    }
);
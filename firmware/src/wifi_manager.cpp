#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include "led_manager.h"
#include "logger.h"


namespace
{

// ============================================================
// CONFIGURATION
// ============================================================

const char *AP_SSID = "ESP32-WiFi-Setup";

const unsigned long WIFI_CONNECT_TIMEOUT = 15000;


// ============================================================
// STATE
// ============================================================

Preferences preferences;

String savedSSID;
String savedPassword;

bool provisioningMode = false;


// ============================================================
// LOAD SAVED CREDENTIALS
// ============================================================

bool loadSavedCredentials()
{
    preferences.begin("wifi", true);

    savedSSID = preferences.getString(
        "ssid",
        ""
    );

    savedPassword = preferences.getString(
        "password",
        ""
    );

    preferences.end();


    if (savedSSID.length() == 0)
    {
        LOG_DEBUG(
            "WIFI",
            "No saved Wi-Fi credentials."
        );

        return false;
    }


    LOG_DEBUG(
        "WIFI",
        "Saved Wi-Fi credentials found."
    );

    LOG_DEBUG(
        "WIFI",
        "SSID: %s",
        savedSSID.c_str()
    );

    return true;
}


// ============================================================
// SAVE CREDENTIALS
// ============================================================

void saveCredentials(
    const String &ssid,
    const String &password
)
{
    preferences.begin(
        "wifi",
        false
    );

    preferences.putString(
        "ssid",
        ssid
    );

    preferences.putString(
        "password",
        password
    );

    preferences.end();


    savedSSID = ssid;
    savedPassword = password;


    LOG_DEBUG(
        "WIFI",
        "Wi-Fi credentials saved."
    );
}


// ============================================================
// CONNECT TO WI-FI
// ============================================================

bool connectToWiFi(
    const String &ssid,
    const String &password
)
{
    LOG_INFO(
        "WIFI",
        "Connecting to Wi-Fi..."
    );

    LOG_INFO(
        "WIFI",
        "SSID: %s",
        ssid.c_str()
    );

    ledBlinkBlue(2);

    // White = connecting.
    ledWhite();


    /*
     * Make sure we are in station mode.
     */

    WiFi.mode(
        WIFI_STA
    );


    /*
     * Disconnect any previous connection.
     */

    WiFi.disconnect(
        true
    );

    delay(200);


    WiFi.begin(
        ssid.c_str(),
        password.c_str()
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime <
            WIFI_CONNECT_TIMEOUT
    )
    {
        delay(500);
    }


    // ========================================================
    // SUCCESS
    // ========================================================

    if (WiFi.status() == WL_CONNECTED)
    {
        LOG_INFO(
            "WIFI",
            "Wi-Fi connected!"
        );

        LOG_INFO(
            "WIFI",
            "IP address: %s",
            WiFi.localIP().toString().c_str()
        );

        LOG_INFO(
            "WIFI",
            "RSSI: %d dBm",
            WiFi.RSSI()
        );


        // Green = connected.
        ledGreen();


        return true;
    }


    // ========================================================
    // FAILURE
    // ========================================================

    LOG_ERROR(
        "WIFI",
        "Wi-Fi connection failed."
    );


    WiFi.disconnect(
        true
    );


    // Red = failed.
    ledRed();


    return false;
}


// ============================================================
// START AP
// ============================================================

void startAccessPoint()
{
    LOG_INFO(
        "WIFI",
        "Starting provisioning mode..."
    );

    ledBlinkBlue(2);

    provisioningMode = true;


    /*
     * Stop station connection.
     */

    WiFi.disconnect(
        true
    );

    delay(200);


    /*
     * AP only.
     */

    WiFi.mode(
        WIFI_AP
    );


    bool success =
        WiFi.softAP(
            AP_SSID
        );


    if (!success)
    {
        LOG_ERROR(
            "WIFI",
            "Failed to start AP!"
        );

        ledRed();

        return;
    }


    LOG_INFO(
        "WIFI",
        "=============================="
    );

    LOG_INFO(
        "WIFI",
        "Wi-Fi provisioning mode started"
    );

    LOG_INFO(
        "WIFI",
        "=============================="
    );

    LOG_INFO(
        "WIFI",
        "Network: %s",
        AP_SSID
    );

    LOG_INFO(
        "WIFI",
        "Setup IP: http://%s",
        WiFi.softAPIP().toString().c_str()
    );


    /*
     * Blue = provisioning mode.
     */

    ledBlue();
}


// ============================================================
// PUBLIC FUNCTIONS
// ============================================================

} // namespace


// ============================================================
// BEGIN
// ============================================================

void wifiManagerBegin()
{
    LOG_INFO(
        "WIFI",
        "Starting Wi-Fi manager..."
    );

    ledBlinkBlue(2);

    provisioningMode = false;


    /*
     * Try previously saved credentials.
     */

    if (loadSavedCredentials())
    {
        if (
            connectToWiFi(
                savedSSID,
                savedPassword
            )
        )
        {
            return;
        }


        LOG_WARN(
            "WIFI",
            "Saved Wi-Fi unavailable."
        );
    }


    /*
     * No working credentials.
     *
     * Start provisioning AP.
     */

    LOG_INFO(
        "WIFI",
        "Entering provisioning mode."
    );


    startAccessPoint();
}


// ============================================================
// LOOP
// ============================================================

void wifiManagerLoop()
{
    /*
     * Reserved for future Wi-Fi monitoring.
     *
     * Examples:
     *
     * - automatic reconnection
     * - connection timeout
     * - Wi-Fi health monitoring
     * - LED status updates
     */
}


// ============================================================
// CONNECTION STATE
// ============================================================

bool isWifiConnected()
{
    return (
        WiFi.status() ==
        WL_CONNECTED
    );
}


// ============================================================
// PROVISIONING STATE
// ============================================================

bool isWifiProvisioning()
{
    return provisioningMode;
}


// ============================================================
// START PROVISIONING
// ============================================================

void wifiManagerStartProvisioning()
{
    LOG_INFO(
        "WIFI",
        "Reconfiguration requested."
    );


    if (provisioningMode)
    {
        LOG_DEBUG(
            "WIFI",
            "Already in provisioning mode."
        );

        return;
    }


    /*
     * IMPORTANT:
     *
     * We do NOT erase the current credentials.
     *
     * They remain stored until new credentials
     * successfully connect.
     */

    startAccessPoint();
}


// ============================================================
// CONFIGURE WI-FI
// ============================================================

bool wifiManagerConfigure(
    const String &ssid,
    const String &password
)
{
    if (ssid.length() == 0)
    {
        LOG_ERROR(
            "WIFI",
            "SSID is empty."
        );

        ledRed();

        return false;
    }


    LOG_INFO(
        "WIFI",
        "=============================="
    );

    LOG_INFO(
        "WIFI",
        "Testing new Wi-Fi configuration"
    );

    LOG_INFO(
        "WIFI",
        "=============================="
    );
    ledBlinkBlue(2);

    /*
     * Remember that the old credentials are still
     * stored in Preferences.
     *
     * We only overwrite them after successful
     * connection.
     */


    // --------------------------------------------------------
    // Try new network
    // --------------------------------------------------------

    bool connected =
        connectToWiFi(
            ssid,
            password
        );


    if (!connected)
    {
        LOG_ERROR(
            "WIFI",
            "New Wi-Fi configuration failed."
        );

        ledRed();

        /*
         * Return to provisioning mode so the user
         * can try again.
         */

        startAccessPoint();


        return false;
    }


    // --------------------------------------------------------
    // SUCCESS
    // --------------------------------------------------------

    LOG_INFO(
        "WIFI",
        "New Wi-Fi configuration successful."
    );


    /*
     * Now it is safe to replace the saved credentials.
     */

    saveCredentials(
        ssid,
        password
    );


    /*
     * Provisioning is complete.
     */

    provisioningMode = false;


    /*
     * AP should no longer be needed.
     */

    WiFi.softAPdisconnect(
        true
    );


    /*
     * Stay in station mode.
     */

    WiFi.mode(
        WIFI_STA
    );


    ledGreen();


    LOG_INFO(
        "WIFI",
        "Provisioning complete."
    );


    return true;
}
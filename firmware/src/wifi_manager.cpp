#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include "led_manager.h"


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
        Serial.println(
            "[WIFI] No saved Wi-Fi credentials."
        );

        return false;
    }


    Serial.println(
        "[WIFI] Saved Wi-Fi credentials found."
    );

    Serial.print(
        "[WIFI] SSID: "
    );

    Serial.println(
        savedSSID
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


    Serial.println(
        "[WIFI] Wi-Fi credentials saved."
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
    Serial.println();
    Serial.println(
        "[WIFI] Connecting to Wi-Fi..."
    );

    Serial.print(
        "[WIFI] SSID: "
    );

    Serial.println(
        ssid
    );


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

        Serial.print(".");
    }


    Serial.println();


    // ========================================================
    // SUCCESS
    // ========================================================

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(
            "[WIFI] Wi-Fi connected!"
        );

        Serial.print(
            "[WIFI] IP address: "
        );

        Serial.println(
            WiFi.localIP()
        );

        Serial.print(
            "[WIFI] RSSI: "
        );

        Serial.print(
            WiFi.RSSI()
        );

        Serial.println(
            " dBm"
        );


        // Green = connected.
        ledGreen();


        return true;
    }


    // ========================================================
    // FAILURE
    // ========================================================

    Serial.println(
        "[WIFI] Wi-Fi connection failed."
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
    Serial.println();
    Serial.println(
        "[WIFI] Starting provisioning mode..."
    );


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
        Serial.println(
            "[WIFI] ERROR: Failed to start AP!"
        );

        ledRed();

        return;
    }


    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "Wi-Fi provisioning mode started"
    );

    Serial.println(
        "================================"
    );


    Serial.print(
        "[WIFI] Network: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "[WIFI] Setup IP: http://"
    );

    Serial.println(
        WiFi.softAPIP()
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
    Serial.println();
    Serial.println(
        "[WIFI] Starting Wi-Fi manager..."
    );


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


        Serial.println(
            "[WIFI] Saved Wi-Fi unavailable."
        );
    }


    /*
     * No working credentials.
     *
     * Start provisioning AP.
     */

    Serial.println(
        "[WIFI] Entering provisioning mode."
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
    Serial.println();
    Serial.println(
        "[WIFI] Reconfiguration requested."
    );


    if (provisioningMode)
    {
        Serial.println(
            "[WIFI] Already in provisioning mode."
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
        Serial.println(
            "[WIFI] ERROR: SSID is empty."
        );

        return false;
    }


    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "Testing new Wi-Fi configuration"
    );

    Serial.println(
        "================================"
    );


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
        Serial.println(
            "[WIFI] New Wi-Fi configuration failed."
        );


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

    Serial.println(
        "[WIFI] New Wi-Fi configuration successful."
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


    Serial.println(
        "[WIFI] Provisioning complete."
    );


    return true;
}
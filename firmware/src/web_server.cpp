#include "web_server.h"

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "wifi_manager.h"


namespace
{

WebServer server(80);

// ============================================================
// ESP32 INTERNAL TEMPERATURE
// ============================================================

float getInternalTemperature()
{
    return temperatureRead();
}


// ============================================================
// MIME TYPE
// ============================================================

String getContentType(
    const String &path
)
{
    if (path.endsWith(".html"))
    {
        return "text/html";
    }

    if (path.endsWith(".css"))
    {
        return "text/css";
    }

    if (path.endsWith(".js"))
    {
        return "application/javascript";
    }

    if (path.endsWith(".json"))
    {
        return "application/json";
    }

    if (path.endsWith(".png"))
    {
        return "image/png";
    }

    if (
        path.endsWith(".jpg") ||
        path.endsWith(".jpeg")
    )
    {
        return "image/jpeg";
    }

    if (path.endsWith(".ico"))
    {
        return "image/x-icon";
    }

    return "text/plain";
}


// ============================================================
// SERVE FILE
// ============================================================

bool serveFile(
    const String &path
)
{
    if (!LittleFS.exists(path))
    {
        Serial.print(
            "[WEB] File not found: "
        );

        Serial.println(
            path
        );

        return false;
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        Serial.print(
            "[WEB] Failed to open: "
        );

        Serial.println(
            path
        );

        return false;
    }


    server.streamFile(
        file,
        getContentType(path)
    );


    file.close();


    return true;
}


// ============================================================
// ROOT
// ============================================================

void handleRoot()
{
    if (isWifiProvisioning())
    {
        if (!serveFile("/setup.html"))
        {
            server.send(
                500,
                "text/plain",
                "setup.html not found"
            );
        }

        return;
    }


    if (!serveFile("/dashboard.html"))
    {
        server.send(
            500,
            "text/plain",
            "dashboard.html not found"
        );
    }
}


// ============================================================
// DASHBOARD
// ============================================================

void handleDashboard()
{
    if (!serveFile("/dashboard.html"))
    {
        server.send(
            500,
            "text/plain",
            "dashboard.html not found"
        );
    }
}


// ============================================================
// SAVE WI-FI
// ============================================================

void handleSave()
{
    String ssid =
        server.arg("ssid");

    String password =
        server.arg("password");


    Serial.println();
    Serial.println(
        "[WEB] Received Wi-Fi configuration."
    );


    if (ssid.length() == 0)
    {
        server.send(
            400,
            "text/html",
            "<h1>Error</h1>"
            "<p>SSID cannot be empty.</p>"
        );

        return;
    }


    /*
     * Test the credentials.
     *
     * wifiManagerConfigure() will save them
     * only if the connection succeeds.
     */

    bool success =
        wifiManagerConfigure(
            ssid,
            password
        );


    if (success)
    {
        /*
         * Send response before the network changes
         * underneath the browser.
         */

        server.send(
            200,
            "text/html",
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<meta name='viewport' "
            "content='width=device-width,initial-scale=1'>"
            "<title>Connected</title>"
            "</head>"
            "<body>"
            "<h1>Wi-Fi Connected!</h1>"
            "<p>The ESP32 successfully connected "
            "to your Wi-Fi network.</p>"
            "<p>You can close this page.</p>"
            "</body>"
            "</html>"
        );


        /*
         * Give the browser time to receive the response.
         */

        delay(1500);


        return;
    }


    /*
     * Connection failed.
     *
     * The old credentials were NOT overwritten.
     *
     * AP mode is active again.
     */

    server.send(
        200,
        "text/html",
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta name='viewport' "
        "content='width=device-width,initial-scale=1'>"
        "<title>Connection Failed</title>"
        "</head>"
        "<body>"
        "<h1>Connection Failed</h1>"
        "<p>Could not connect to the Wi-Fi network.</p>"
        "<p>Please check the SSID and password.</p>"
        "<p><a href='/'>Try Again</a></p>"
        "</body>"
        "</html>"
    );
}


// ============================================================
// STATUS API
// ============================================================

void handleStatus()
{
    if (!isWifiConnected())
    {
        server.send(
            503,
            "application/json",
            "{\"connected\":false}"
        );

        return;
    }


    // --------------------------------------------------------
    // RSSI
    // --------------------------------------------------------

    int rssi =
        WiFi.RSSI();


    // --------------------------------------------------------
    // QUALITY
    // --------------------------------------------------------

    int quality =
        map(
            rssi,
            -100,
            -50,
            0,
            100
        );


    if (quality < 0)
    {
        quality = 0;
    }

    if (quality > 100)
    {
        quality = 100;
    }


    // --------------------------------------------------------
    // UPTIME
    // --------------------------------------------------------

    unsigned long uptimeSeconds =
        millis() / 1000;


    unsigned long days =
        uptimeSeconds / 86400;

    uptimeSeconds %= 86400;


    unsigned long hours =
        uptimeSeconds / 3600;

    uptimeSeconds %= 3600;


    unsigned long minutes =
        uptimeSeconds / 60;


    unsigned long seconds =
        uptimeSeconds % 60;


    char uptime[32];


    snprintf(
        uptime,
        sizeof(uptime),
        "%lud %02lu:%02lu:%02lu",
        days,
        hours,
        minutes,
        seconds
    );

    // --------------------------------------------------------
    // INTERNAL TEMPERATURE
    // --------------------------------------------------------

    float temperature =
        getInternalTemperature();
    Serial.print(
        "[WEB] Internal temperature: "
    );
    Serial.println(
        temperature
    );


    // --------------------------------------------------------
    // JSON
    // --------------------------------------------------------

    String json = "{";


    json += "\"connected\":true,";


    json += "\"ssid\":\"";
    json += WiFi.SSID();
    json += "\",";


    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",";


    json += "\"rssi\":";
    json += String(rssi);
    json += ",";


    json += "\"quality\":";
    json += String(quality);
    json += ",";

    json += "\"temperature\":";
    if (temperature > -100.0)
    {
        json += String(
            temperature,
            2
        );
    }
    else
    {
        json += "null";
    }

    json += ",";


    json += "\"uptime\":\"";
    json += uptime;
    json += "\",";


    json += "\"heap\":";
    json += String(
        ESP.getFreeHeap()
    );
    json += ",";


    json += "\"flash\":";
    json += String(
        ESP.getFlashChipSize()
    );
    json += ",";


    json += "\"firmware\":\"1.0.0\"";


    json += "}";


    server.send(
        200,
        "application/json",
        json
    );
}


// ============================================================
// RECONFIGURE WI-FI
// ============================================================

void handleReconfigure()
{
    Serial.println();
    Serial.println(
        "[WEB] Reconfigure Wi-Fi requested."
    );


    /*
     * Respond FIRST.
     *
     * The browser needs to receive this response
     * before the ESP32 changes network mode.
     */

    server.send(
        200,
        "application/json",
        "{\"success\":true,"
        "\"message\":\"Wi-Fi setup mode starting\"}"
    );


    delay(500);


    /*
     * Start the ESP32 AP.
     */

    wifiManagerStartProvisioning();
}


// ============================================================
// NOT FOUND
// ============================================================

void handleNotFound()
{
    Serial.print(
        "[WEB] 404: "
    );

    Serial.println(
        server.uri()
    );


    server.send(
        404,
        "text/plain",
        "404 - File Not Found"
    );
}


// ============================================================
// ROUTES
// ============================================================

void setupRoutes()
{
    // --------------------------------------------------------
    // Pages
    // --------------------------------------------------------

    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );


    server.on(
        "/dashboard",
        HTTP_GET,
        handleDashboard
    );


    // --------------------------------------------------------
    // Wi-Fi API
    // --------------------------------------------------------

    server.on(
        "/save",
        HTTP_POST,
        handleSave
    );


    server.on(
        "/reconfigure",
        HTTP_POST,
        handleReconfigure
    );


    // --------------------------------------------------------
    // Status API
    // --------------------------------------------------------

    server.on(
        "/status",
        HTTP_GET,
        handleStatus
    );


    // --------------------------------------------------------
    // Static files
    // --------------------------------------------------------

    server.on(
        "/setup.html",
        HTTP_GET,
        []()
        {
            if (!serveFile("/setup.html"))
            {
                server.send(
                    404,
                    "text/plain",
                    "setup.html not found"
                );
            }
        }
    );


    server.on(
        "/dashboard.html",
        HTTP_GET,
        []()
        {
            if (!serveFile("/dashboard.html"))
            {
                server.send(
                    404,
                    "text/plain",
                    "dashboard.html not found"
                );
            }
        }
    );


    server.on(
        "/style.css",
        HTTP_GET,
        []()
        {
            if (!serveFile("/style.css"))
            {
                server.send(
                    404,
                    "text/plain",
                    "style.css not found"
                );
            }
        }
    );


    server.on(
        "/app.js",
        HTTP_GET,
        []()
        {
            if (!serveFile("/app.js"))
            {
                server.send(
                    404,
                    "text/plain",
                    "app.js not found"
                );
            }
        }
    );


    server.onNotFound(
        handleNotFound
    );
}

} // namespace


// ============================================================
// WEB SERVER BEGIN
// ============================================================

void webServerBegin()
{
    Serial.println();
    Serial.println(
        "[WEB] Starting web server..."
    );


    // --------------------------------------------------------
    // LittleFS
    // --------------------------------------------------------

    if (!LittleFS.begin(true))
    {
        Serial.println(
            "[WEB] ERROR: LittleFS mount failed!"
        );

        return;
    }


    Serial.println(
        "[WEB] LittleFS mounted."
    );


    // --------------------------------------------------------
    // Routes
    // --------------------------------------------------------

    setupRoutes();


    // --------------------------------------------------------
    // Start HTTP server
    // --------------------------------------------------------

    server.begin();


    Serial.println(
        "[WEB] HTTP server started."
    );


    if (isWifiProvisioning())
    {
        Serial.print(
            "[WEB] Setup page: http://"
        );

        Serial.println(
            WiFi.softAPIP()
        );
    }
    else if (isWifiConnected())
    {
        Serial.print(
            "[WEB] Dashboard: http://"
        );

        Serial.println(
            WiFi.localIP()
        );
    }
}


// ============================================================
// WEB SERVER LOOP
// ============================================================

void webServerLoop()
{
    server.handleClient();
}
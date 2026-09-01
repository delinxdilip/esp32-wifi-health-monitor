#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include "firebase_manager.h"
#include "device_manager.h"
#include "device_config.h"
#include "time_manager.h"
#include "logger.h"
#include "led_manager.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>


// ============================================================
// FIREBASE OBJECTS
// ============================================================

UserAuth userAuth(
    FIREBASE_API_KEY,
    FIREBASE_USER_EMAIL,
    FIREBASE_USER_PASSWORD
);

FirebaseApp firebaseApp;

WiFiClientSecure sslClient;

using AsyncClient = AsyncClientClass;

AsyncClient firebaseClient(sslClient);

RealtimeDatabase database;


// ============================================================
// FIREBASE STATE
// ============================================================

bool firebaseConnected = false;

static bool deviceRegistered = false;


// ============================================================
// TELEMETRY CONFIGURATION
// ============================================================

static const unsigned long SAMPLE_INTERVAL = 1000;
static const unsigned long TELEMETRY_INTERVAL = 30000;


// ============================================================
// TELEMETRY STATE
// ============================================================

static unsigned long lastSampleMillis = 0;
static unsigned long telemetryStartMillis = 0;

static uint32_t sampleCount = 0;

static float temperatureSum = 0.0f;

static int32_t rssiSum = 0;

static uint32_t qualitySum = 0;

static uint32_t heapSum = 0;

static uint32_t minimumHeap = UINT32_MAX;


// ============================================================
// INTERNAL TEMPERATURE
// ============================================================

static float readInternalTemperature()
{
    return temperatureRead();
}


// ============================================================
// WIFI QUALITY
// ============================================================

static int calculateWifiQuality(int rssi)
{
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

    return quality;
}


// ============================================================
// FIREBASE AUTH CALLBACK
// ============================================================

void firebaseAuthCallback(
    AsyncResult &result
)
{
    if (result.isEvent())
    {
        LOG_INFO(
            "FIREBASE",
            "Event: %s - %s",
            result.uid().c_str(),
            result.eventLog().message().c_str()
        );
    }


    if (result.isError())
    {
        LOG_ERROR(
            "FIREBASE",
            "Error: %s (%d)",
            result.error().message().c_str(),
            result.error().code()
        );
    }
}


// ============================================================
// REGISTER DEVICE
// ============================================================

static void registerDevice()
{
    if (deviceRegistered)
    {
        return;
    }


    if (!firebaseApp.ready())
    {
        return;
    }


    LOG_INFO(
        "FIREBASE",
        "Registering device..."
    );


    String basePath =
        "/devices/" +
        String(getDeviceId());


    String deviceIdPath =
        basePath +
        "/info/deviceId";


    String namePath =
        basePath +
        "/info/name";


    String modelPath =
        basePath +
        "/info/model";


    String firmwarePath =
        basePath +
        "/info/firmware";


    // --------------------------------------------------------
    // Device ID
    // --------------------------------------------------------

    database.set(
        firebaseClient,
        deviceIdPath,
        getDeviceId()
    );


    // --------------------------------------------------------
    // Edge name
    // --------------------------------------------------------

    database.set(
        firebaseClient,
        namePath,
        EDGE_NAME
    );


    // --------------------------------------------------------
    // Device model
    // --------------------------------------------------------

    database.set(
        firebaseClient,
        modelPath,
        DEVICE_MODEL
    );


    // --------------------------------------------------------
    // Firmware version
    // --------------------------------------------------------

    database.set(
        firebaseClient,
        firmwarePath,
        FIRMWARE_VERSION
    );


    deviceRegistered = true;


    LOG_INFO(
        "FIREBASE",
        "Device registration requested."
    );


    LOG_INFO(
        "FIREBASE",
        "Device path: %s",
        basePath.c_str()
    );
}


// ============================================================
// RESET TELEMETRY BUFFER
// ============================================================

static void resetTelemetry()
{
    sampleCount = 0;

    temperatureSum = 0.0f;

    rssiSum = 0;

    qualitySum = 0;

    heapSum = 0;

    minimumHeap = UINT32_MAX;


    telemetryStartMillis =
        millis();


    lastSampleMillis =
        millis();
}


// ============================================================
// COLLECT TELEMETRY SAMPLE
// ============================================================

static void collectTelemetrySample()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return;
    }


    // --------------------------------------------------------
    // Wi-Fi
    // --------------------------------------------------------

    int rssi =
        WiFi.RSSI();


    int quality =
        calculateWifiQuality(rssi);


    // --------------------------------------------------------
    // Internal temperature
    // --------------------------------------------------------

    float temperature =
        readInternalTemperature();


    // --------------------------------------------------------
    // Memory
    // --------------------------------------------------------

    uint32_t heap =
        ESP.getFreeHeap();


    if (heap < minimumHeap)
    {
        minimumHeap = heap;
    }


    // --------------------------------------------------------
    // Accumulate values
    // --------------------------------------------------------

    temperatureSum +=
        temperature;


    rssiSum +=
        rssi;


    qualitySum +=
        quality;


    heapSum +=
        heap;


    sampleCount++;


    // --------------------------------------------------------
    // Serial debug
    // --------------------------------------------------------

    
    LOG_DEBUG(
        "FIREBASE",
        "Telemetry sample"
    );

    LOG_DEBUG(
        "FIREBASE",
        "Temperature: %.2f",
        temperature
    );

    LOG_DEBUG(
        "FIREBASE",
        "RSSI: %d",
        rssi
    );

    LOG_DEBUG(
        "FIREBASE",
        "Quality: %d",
        quality
    );

    LOG_DEBUG(
        "FIREBASE",
        "Free heap: %d",
        heap
    );

    LOG_DEBUG(
        "FIREBASE",
        "Minimum heap: %d",
        minimumHeap
    );

    LOG_DEBUG(
        "FIREBASE",
        "Sample count: %d",
        sampleCount
    );
}


// ============================================================
// SEND TELEMETRY
// ============================================================

static void sendTelemetry()
{
    if (!firebaseApp.ready())
    {
        return;
    }


    if (sampleCount == 0)
    {
        LOG_DEBUG(
            "FIREBASE",
            "No telemetry samples."
        );

        resetTelemetry();

        return;
    }


    // --------------------------------------------------------
    // Calculate averages
    // --------------------------------------------------------

    float averageTemperature =
        temperatureSum /
        sampleCount;


    float averageRssi =
        (float)rssiSum /
        sampleCount;


    float averageQuality =
        (float)qualitySum /
        sampleCount;


    float averageHeap =
        (float)heapSum /
        sampleCount;


    // --------------------------------------------------------
    // ESP32 timestamp
    // --------------------------------------------------------

    String espTimestamp =
        getTimestamp();


    if (espTimestamp.length() == 0)
    {
        LOG_DEBUG(
            "FIREBASE",
            "ESP32 timestamp unavailable."
        );

        espTimestamp =
            "unsynchronized";
    }


    // --------------------------------------------------------
    // Firebase path
    // --------------------------------------------------------

    String telemetryPath =
        "/devices/" +
        String(getDeviceId()) +
        "/telemetry";


    // --------------------------------------------------------
    // Current device health values
    // --------------------------------------------------------

    uint32_t currentHeap =
        ESP.getFreeHeap();


    uint32_t flashSize =
        ESP.getFlashChipSize();


    uint32_t sketchSize =
        ESP.getSketchSize();


    uint32_t uptime =
        millis() / 1000;


    // --------------------------------------------------------
    // Build telemetry JSON
    // --------------------------------------------------------

    object_t json;

    JsonWriter writer;


    object_t obj1;
    object_t obj2;
    object_t obj3;
    object_t obj4;
    object_t obj5;
    object_t obj6;
    object_t obj7;
    object_t obj8;
    object_t obj9;
    object_t obj10;
    object_t obj11;
    object_t obj12;


    // --------------------------------------------------------
    // ESP32 timestamp
    // --------------------------------------------------------

    writer.create(
        obj1,
        "espTimestamp",
        string_t(espTimestamp)
    );


    // --------------------------------------------------------
    // Firebase server timestamp
    // --------------------------------------------------------

    writer.create(
        obj2,
        "serverTimestamp",
        object_t("{\".sv\":\"timestamp\"}")
    );


    // --------------------------------------------------------
    // Average internal temperature
    // --------------------------------------------------------

    writer.create(
        obj3,
        "internalTemperature",
        number_t(
            averageTemperature,
            2
        )
    );


    // --------------------------------------------------------
    // Average RSSI
    // --------------------------------------------------------

    writer.create(
        obj4,
        "rssi",
        number_t(
            averageRssi,
            0
        )
    );


    // --------------------------------------------------------
    // Average Wi-Fi quality
    // --------------------------------------------------------

    writer.create(
        obj5,
        "wifiQuality",
        number_t(
            averageQuality,
            0
        )
    );


    // --------------------------------------------------------
    // Average free heap
    // --------------------------------------------------------

    writer.create(
        obj6,
        "freeHeap",
        number_t(
            averageHeap,
            0
        )
    );


    // --------------------------------------------------------
    // Minimum free heap during window
    // --------------------------------------------------------

    writer.create(
        obj7,
        "minimumFreeHeap",
        minimumHeap
    );


    // --------------------------------------------------------
    // Current free heap
    // --------------------------------------------------------

    writer.create(
        obj8,
        "currentFreeHeap",
        currentHeap
    );


    // --------------------------------------------------------
    // Flash size
    // --------------------------------------------------------

    writer.create(
        obj9,
        "flashSize",
        flashSize
    );


    // --------------------------------------------------------
    // Firmware sketch size
    // --------------------------------------------------------

    writer.create(
        obj10,
        "sketchSize",
        sketchSize
    );


    // --------------------------------------------------------
    // Uptime
    // --------------------------------------------------------

    writer.create(
        obj11,
        "uptime",
        uptime
    );


    // --------------------------------------------------------
    // Samples
    // --------------------------------------------------------

    writer.create(
        obj12,
        "sampleCount",
        sampleCount
    );


    // --------------------------------------------------------
    // Join JSON
    // --------------------------------------------------------

    writer.join(
        json,
        12,
        obj1,
        obj2,
        obj3,
        obj4,
        obj5,
        obj6,
        obj7,
        obj8,
        obj9,
        obj10,
        obj11,
        obj12
    );


    // --------------------------------------------------------
    // Push telemetry
    // --------------------------------------------------------

    LOG_INFO(
        "FIREBASE",
        "Sending telemetry..."
    );

    ledBlinkBlue(1);

    database.push<object_t>(
        firebaseClient,
        telemetryPath,
        json,
        firebaseTelemetryCallback,
        "telemetryPush"
    );


    LOG_DEBUG(
        "FIREBASE",
        "Telemetry push requested."
    );

    LOG_DEBUG(
        "FIREBASE",
        "Samples collected: %d",
        sampleCount
    );

    LOG_DEBUG(
        "FIREBASE",
        "Average temperature: %.2f",
        averageTemperature
    );

    LOG_DEBUG(
        "FIREBASE",
        "Average free heap: %.0f",
        averageHeap
    );

    LOG_DEBUG(
        "FIREBASE",
        "Minimum free heap: %d",
        minimumHeap
    );


    // --------------------------------------------------------
    // Prepare next 30-second window
    // --------------------------------------------------------

    resetTelemetry();
}


// ============================================================
// TELEMETRY CALLBACK
// ============================================================

void firebaseTelemetryCallback(
    AsyncResult &result
)
{
    if (!result.isResult())
    {
        return;
    }


    if (result.isError())
    {
        LOG_ERROR(
            "FIREBASE",
            "Telemetry error: %s (%d)",
            result.error().message().c_str(),
            result.error().code()
        );

        ledRed();

        return;
    }


    if (result.available())
    {
        LOG_DEBUG(
            "FIREBASE",
            "Telemetry write result: %s",
            result.c_str()
        );

        ledGreen();
    }
}


// ============================================================
// FIREBASE INITIALIZATION
// ============================================================

void firebaseManagerBegin()
{
    LOG_INFO(
        "FIREBASE",
        "=============================="
    );

    LOG_INFO(
        "FIREBASE",
        "Initializing..."
    );

    LOG_INFO(
        "FIREBASE",
        "=============================="
    );

    ledBlinkBlue(2);

    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_WARN(
            "FIREBASE",
            "Wi-Fi is not connected."
        );

        firebaseConnected = false;

        ledRed();

        return;
    }


    LOG_INFO(
        "FIREBASE",
        "Wi-Fi available."
    );


    // --------------------------------------------------------
    // SSL
    // --------------------------------------------------------

    sslClient.setInsecure();


    // --------------------------------------------------------
    // Authentication
    // --------------------------------------------------------

    LOG_INFO(
        "FIREBASE",
        "Initializing authentication..."
    );


    initializeApp(
        firebaseClient,
        firebaseApp,
        getAuth(userAuth),
        firebaseAuthCallback,
        "firebaseAuth"
    );


    // --------------------------------------------------------
    // Realtime Database
    // --------------------------------------------------------

    firebaseApp.getApp<RealtimeDatabase>(
        database
    );


    database.url(
        FIREBASE_DATABASE_URL
    );


    LOG_INFO(
        "FIREBASE",
        "Firebase manager initialized."
    );

    ledGreen();

    firebaseConnected = false;


    // --------------------------------------------------------
    // Start telemetry window
    // --------------------------------------------------------

    resetTelemetry();
}


// ============================================================
// FIREBASE LOOP
// ============================================================

void firebaseManagerLoop()
{
    // --------------------------------------------------------
    // Process Firebase
    // --------------------------------------------------------

    firebaseApp.loop();


    // --------------------------------------------------------
    // Authentication
    // --------------------------------------------------------

    if (firebaseApp.ready())
    {
        if (!firebaseConnected)
        {
            firebaseConnected = true;

            LOG_INFO(
                "FIREBASE",
                "Authentication ready."
            );

            LOG_INFO(
                "FIREBASE",
                "Firebase connected."
            );

            ledGreen();
        }


        // ----------------------------------------------------
        // Device registration
        // ----------------------------------------------------

        registerDevice();


        // ----------------------------------------------------
        // Collect sample every 1 second
        // ----------------------------------------------------

        unsigned long now =
            millis();


        if (
            now - lastSampleMillis >=
            SAMPLE_INTERVAL
        )
        {
            lastSampleMillis =
                now;


            collectTelemetrySample();
        }


        // ----------------------------------------------------
        // Send every 30 seconds
        // ----------------------------------------------------

        if (
            now - telemetryStartMillis >=
            TELEMETRY_INTERVAL
        )
        {
            sendTelemetry();
        }
    }
    else
    {
        firebaseConnected = false;
    }
}


// ============================================================
// CONNECTION STATUS
// ============================================================

bool isFirebaseConnected()
{
    return firebaseConnected;
}
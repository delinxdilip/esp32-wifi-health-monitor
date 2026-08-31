#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include "firebase_manager.h"
#include "device_manager.h"
#include "device_config.h"
#include "time_manager.h"

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
        Serial.printf(
            "[FIREBASE] Event: %s - %s\n",
            result.uid().c_str(),
            result.eventLog().message().c_str()
        );
    }


    if (result.isError())
    {
        Serial.printf(
            "[FIREBASE] Error: %s (%d)\n",
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


    Serial.println();
    Serial.println(
        "[FIREBASE] Registering device..."
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


    Serial.println(
        "[FIREBASE] Device registration requested."
    );


    Serial.print(
        "[FIREBASE] Device path: "
    );

    Serial.println(
        basePath
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

    Serial.println();

    Serial.println(
        "[FIREBASE] Telemetry sample"
    );


    Serial.print(
        "[FIREBASE] Temperature: "
    );

    Serial.println(
        temperature,
        2
    );


    Serial.print(
        "[FIREBASE] RSSI: "
    );

    Serial.println(
        rssi
    );


    Serial.print(
        "[FIREBASE] Quality: "
    );

    Serial.println(
        quality
    );


    Serial.print(
        "[FIREBASE] Free heap: "
    );

    Serial.println(
        heap
    );


    Serial.print(
        "[FIREBASE] Minimum heap: "
    );

    Serial.println(
        minimumHeap
    );


    Serial.print(
        "[FIREBASE] Samples: "
    );

    Serial.println(
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
        Serial.println(
            "[FIREBASE] No telemetry samples."
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
        Serial.println(
            "[FIREBASE] ESP32 timestamp unavailable."
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

    Serial.println();

    Serial.println(
        "[FIREBASE] Sending telemetry..."
    );


    database.push<object_t>(
        firebaseClient,
        telemetryPath,
        json,
        firebaseTelemetryCallback,
        "telemetryPush"
    );


    Serial.println(
        "[FIREBASE] Telemetry push requested."
    );


    Serial.print(
        "[FIREBASE] Samples collected: "
    );

    Serial.println(
        sampleCount
    );


    Serial.print(
        "[FIREBASE] Average temperature: "
    );

    Serial.println(
        averageTemperature,
        2
    );


    Serial.print(
        "[FIREBASE] Average free heap: "
    );

    Serial.println(
        averageHeap,
        0
    );


    Serial.print(
        "[FIREBASE] Minimum free heap: "
    );

    Serial.println(
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
        Serial.printf(
            "[FIREBASE] Telemetry error: %s (%d)\n",
            result.error().message().c_str(),
            result.error().code()
        );

        return;
    }


    if (result.available())
    {
        Serial.print(
            "[FIREBASE] Telemetry write result: "
        );

        Serial.println(
            result.c_str()
        );
    }
}


// ============================================================
// FIREBASE INITIALIZATION
// ============================================================

void firebaseManagerBegin()
{
    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        "[FIREBASE] Initializing..."
    );

    Serial.println(
        "=============================="
    );


    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            "[FIREBASE] Wi-Fi is not connected."
        );

        firebaseConnected = false;

        return;
    }


    Serial.println(
        "[FIREBASE] Wi-Fi available."
    );


    // --------------------------------------------------------
    // SSL
    // --------------------------------------------------------

    sslClient.setInsecure();


    // --------------------------------------------------------
    // Authentication
    // --------------------------------------------------------

    Serial.println(
        "[FIREBASE] Initializing authentication..."
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


    Serial.println(
        "[FIREBASE] Firebase manager initialized."
    );


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


            Serial.println();

            Serial.println(
                "[FIREBASE] Authentication ready."
            );

            Serial.println(
                "[FIREBASE] Firebase connected."
            );
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
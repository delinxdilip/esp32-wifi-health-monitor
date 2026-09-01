import {
    onAuthStateChanged,
    signOut
} from "firebase/auth";

import {
    ref,
    onValue
} from "firebase/database";

import {
    auth,
    database
} from "./firebase.js";


// ============================================================
// STATE
// ============================================================

let devices = {};

let selectedDeviceId = null;

let telemetry = [];

let unsubscribeTelemetry = null;

let wifiHistoryVisible = false;

let healthHistoryVisible = false;

let resizeTimer = null;


// ============================================================
// DOM
// ============================================================

const deviceSelect =
    document.getElementById("deviceSelect");

const logoutButton =
    document.getElementById("logoutButton");

const connectionStatus =
    document.getElementById("connectionStatus");

const connectionText =
    document.getElementById("connectionText");

const themeToggle =
    document.getElementById("themeToggle");

const themeIcon =
    document.getElementById("themeIcon");


// ============================================================
// AUTH
// ============================================================

onAuthStateChanged(
    auth,
    (user) => {

        if (!user) {

            window.location.href =
                "/login.html";

            return;
        }

        initializeDashboard();
    }
);


// ============================================================
// INITIALIZE
// ============================================================

function initializeDashboard() {

    setupLogout();

    setupDeviceSelector();

    setupViewSwitching();

    setupTheme();

    loadDevices();
}


// ============================================================
// DEVICES
// ============================================================

function loadDevices() {

    const devicesRef =
        ref(database, "devices");

    onValue(
        devicesRef,
        (snapshot) => {

            const data =
                snapshot.val();

            if (!data) {

                devices = {};

                showConnection(
                    false,
                    "No devices"
                );

                deviceSelect.innerHTML =
                    "<option>No devices</option>";

                return;
            }

            devices = data;

            populateDeviceSelector();

            showConnection(
                true,
                "Connected"
            );
        },

        (error) => {

            console.error(
                "[DASHBOARD] Device error:",
                error
            );

            showConnection(
                false,
                "Connection error"
            );
        }
    );
}


// ============================================================
// DEVICE SELECTOR
// ============================================================

function populateDeviceSelector() {

    const ids =
        Object.keys(devices);

    deviceSelect.innerHTML = "";

    ids.forEach(
        (deviceId) => {

            const device =
                devices[deviceId];

            const option =
                document.createElement(
                    "option"
                );

            option.value =
                deviceId;

            option.textContent =
                device?.info?.name ||
                deviceId;

            deviceSelect.appendChild(
                option
            );
        }
    );

    if (
        !selectedDeviceId ||
        !devices[selectedDeviceId]
    ) {

        selectedDeviceId =
            ids[0];
    }

    deviceSelect.value =
        selectedDeviceId;

    loadSelectedDevice();
}


// ============================================================
// DEVICE CHANGE
// ============================================================

function setupDeviceSelector() {

    deviceSelect.addEventListener(
        "change",
        () => {

            selectedDeviceId =
                deviceSelect.value;

            loadSelectedDevice();
        }
    );
}


// ============================================================
// LOAD SELECTED DEVICE
// ============================================================

function loadSelectedDevice() {

    if (!selectedDeviceId) {
        return;
    }

    const device =
        devices[selectedDeviceId];

    if (!device) {
        return;
    }

    updateDeviceInfo(
        device.info || {}
    );

    subscribeToTelemetry(
        selectedDeviceId
    );
}


// ============================================================
// DEVICE INFORMATION
// ============================================================

function updateDeviceInfo(info) {

    document.getElementById(
        "deviceName"
    ).textContent =
        info.name ||
        info.deviceId ||
        "Unknown device";

    document.getElementById(
        "deviceModel"
    ).textContent =
        info.model ||
        "Unknown model";

    document.getElementById(
        "firmwareVersion"
    ).textContent =
        info.firmware ||
        "--";
}


// ============================================================
// TELEMETRY SUBSCRIPTION
// ============================================================

function subscribeToTelemetry(deviceId) {

    /*
     * Remove previous Firebase listener.
     */

    if (unsubscribeTelemetry) {

        unsubscribeTelemetry();

        unsubscribeTelemetry =
            null;
    }


    telemetry = [];

    const telemetryRef =
        ref(
            database,
            `devices/${deviceId}/telemetry`
        );


    unsubscribeTelemetry =
        onValue(
            telemetryRef,
            (snapshot) => {

                const data =
                    snapshot.val();

                if (!data) {

                    telemetry = [];

                    updateDashboard();

                    return;
                }


                telemetry =
                    Object.entries(data)
                        .map(
                            ([id, value]) => ({
                                id,
                                ...value
                            })
                        )
                        .filter(
                            (item) =>
                                getTimestamp(item) > 0
                        )
                        .sort(
                            (a, b) =>
                                getTimestamp(a) -
                                getTimestamp(b)
                        );


                updateDashboard();
            },

            (error) => {

                console.error(
                    "[DASHBOARD] Telemetry error:",
                    error
                );
            }
        );
}


// ============================================================
// DASHBOARD UPDATE
// ============================================================

function updateDashboard() {

    if (!telemetry.length) {

        return;
    }

    const latest =
        telemetry[
            telemetry.length - 1
        ];

    updateWifiCard(
        latest
    );

    updateHealthCard(
        latest
    );

    updateLastUpdate(
        latest
    );

    /*
     * Only draw a chart if its history
     * view is currently visible.
     *
     * This dramatically reduces unnecessary
     * canvas rendering.
     */

    if (wifiHistoryVisible) {

        requestAnimationFrame(
            drawWifiChart
        );
    }

    if (healthHistoryVisible) {

        requestAnimationFrame(
            drawHealthChart
        );
    }
}


// ============================================================
// WIFI CARD
// ============================================================

function updateWifiCard(data) {

    const rssi =
        Number(data.rssi);

    const quality =
        Number(data.wifiQuality);


    document.getElementById(
        "wifiRssi"
    ).textContent =
        Number.isFinite(rssi)
            ? rssi
            : "--";


    document.getElementById(
        "wifiQualityValue"
    ).textContent =
        Number.isFinite(quality)
            ? `${Math.round(quality)}%`
            : "--";


    const signal =
        getWifiStatus(rssi);


    document.getElementById(
        "wifiSignalLabel"
    ).textContent =
        signal.label;


    document.getElementById(
        "wifiStatus"
    ).querySelector(
        "span:last-child"
    ).textContent =
        signal.label;


    setStatusClass(
        document.getElementById(
            "wifiStatus"
        ),
        signal.type
    );


    const qualityText =
        Number.isFinite(quality)
            ? `${Math.round(quality)}% quality`
            : "Quality unavailable";


    document.getElementById(
        "wifiQuality"
    ).textContent =
        `${signal.description} • ${qualityText}`;
}


// ============================================================
// WIFI STATUS
// ============================================================

function getWifiStatus(rssi) {

    if (!Number.isFinite(rssi)) {

        return {
            label: "Unknown",
            type: "normal",
            description: "Signal unavailable"
        };
    }


    if (rssi >= -50) {

        return {
            label: "Excellent",
            type: "good",
            description: "Excellent signal"
        };
    }


    if (rssi >= -65) {

        return {
            label: "Good",
            type: "good",
            description: "Strong signal"
        };
    }


    if (rssi >= -75) {

        return {
            label: "Normal",
            type: "normal",
            description: "Acceptable signal"
        };
    }


    if (rssi >= -85) {

        return {
            label: "Weak",
            type: "warning",
            description: "Weak signal"
        };
    }


    return {
        label: "Critical",
        type: "danger",
        description: "Very weak signal"
    };
}


// ============================================================
// HEALTH CARD
// ============================================================

function updateHealthCard(data) {

    const temperature =
        Number(
            data.internalTemperature
        );

    const freeHeap =
        Number(
            data.freeHeap
        );

    const minimumHeap =
        Number(
            data.minimumFreeHeap
        );

    const sampleCount =
        Number(
            data.sampleCount
        );


    document.getElementById(
        "temperatureValue"
    ).textContent =
        Number.isFinite(temperature)
            ? temperature.toFixed(1)
            : "--";


    document.getElementById(
        "freeHeapValue"
    ).textContent =
        formatBytes(freeHeap);


    document.getElementById(
        "minimumHeapValue"
    ).textContent =
        formatBytes(minimumHeap);


    document.getElementById(
        "sampleCountValue"
    ).textContent =
        Number.isFinite(sampleCount)
            ? sampleCount
            : "--";


    document.getElementById(
        "uptimeValue"
    ).textContent =
        formatUptime(
            Number(data.uptime)
        );


    const health =
        getHealthStatus(
            temperature,
            freeHeap
        );


    const healthElement =
        document.getElementById(
            "healthStatus"
        );


    healthElement.querySelector(
        "span:last-child"
    ).textContent =
        health.label;


    setStatusClass(
        healthElement,
        health.type
    );


    document.getElementById(
        "healthHistorySummary"
    ).textContent =
        Number.isFinite(temperature)
            ? `Latest ${temperature.toFixed(1)} °C • Last ${Math.min(telemetry.length, 60)} readings`
            : "Temperature unavailable";
}


// ============================================================
// HEALTH STATUS
// ============================================================

function getHealthStatus(
    temperature,
    heap
) {

    if (
        temperature >= 70 ||
        heap < 50000
    ) {

        return {
            label: "Critical",
            type: "danger"
        };
    }


    if (
        temperature >= 60 ||
        heap < 100000
    ) {

        return {
            label: "Warning",
            type: "warning"
        };
    }


    if (
        temperature >= 50 ||
        heap < 150000
    ) {

        return {
            label: "Normal",
            type: "normal"
        };
    }


    return {
        label: "Healthy",
        type: "good"
    };
}


// ============================================================
// STATUS CLASS
// ============================================================

function setStatusClass(
    element,
    type
) {

    element.classList.remove(
        "good",
        "normal",
        "warning",
        "danger"
    );

    element.classList.add(
        type
    );
}


// ============================================================
// LAST UPDATE
// ============================================================

function updateLastUpdate(data) {

    const timestamp =
        getTimestamp(data);

    if (!timestamp) {
        return;
    }

    const date =
        new Date(timestamp);

    document.getElementById(
        "lastUpdate"
    ).textContent =
        date.toLocaleTimeString(
            [],
            {
                hour: "2-digit",
                minute: "2-digit",
                second: "2-digit"
            }
        );
}


// ============================================================
// TIMESTAMP
// ============================================================

function getTimestamp(data) {

    const serverTimestamp =
        Number(data.serverTimestamp);

    if (
        Number.isFinite(
            serverTimestamp
        ) &&
        serverTimestamp > 0
    ) {

        return serverTimestamp;
    }


    if (data.espTimestamp) {

        const parsed =
            Date.parse(
                data.espTimestamp
            );

        if (
            Number.isFinite(parsed)
        ) {

            return parsed;
        }
    }


    return 0;
}


// ============================================================
// CARD VIEW SWITCHING
// ============================================================

function setupViewSwitching() {

    document.querySelectorAll(
        ".history-button"
    ).forEach(
        (button) => {

            button.addEventListener(
                "click",
                () => {

                    const target =
                        button.dataset.target;

                    swapCard(
                        target,
                        true
                    );
                }
            );
        }
    );


    document.querySelectorAll(
        ".back-button"
    ).forEach(
        (button) => {

            button.addEventListener(
                "click",
                () => {

                    const target =
                        button.dataset.target;

                    swapCard(
                        target,
                        false
                    );
                }
            );
        }
    );
}


// ============================================================
// SWAP CARD
// ============================================================

function swapCard(
    target,
    showHistory
) {

    const overview =
        document.getElementById(
            `${target}Overview`
        );

    const history =
        document.getElementById(
            `${target}History`
        );


    if (target === "wifi") {

        wifiHistoryVisible =
            showHistory;
    }


    if (target === "health") {

        healthHistoryVisible =
            showHistory;
    }


    if (showHistory) {

        overview.classList.remove(
            "active"
        );

        history.classList.add(
            "active"
        );


        requestAnimationFrame(
            () => {

                if (target === "wifi") {

                    drawWifiChart();
                }

                if (target === "health") {

                    drawHealthChart();
                }
            }
        );

    } else {

        history.classList.remove(
            "active"
        );

        overview.classList.add(
            "active"
        );
    }
}


// ============================================================
// WIFI CHART
// ============================================================

function drawWifiChart() {

    const canvas =
        document.getElementById(
            "wifiChart"
        );

    const context =
        prepareCanvas(
            canvas
        );


    const points =
        telemetry
            .slice(-60)
            .filter(
                item =>
                    Number.isFinite(
                        Number(item.rssi)
                    )
            );


    if (points.length < 2) {

        drawEmptyChart(
            canvas,
            context
        );

        return;
    }


    const values =
        points.map(
            item =>
                Number(item.rssi)
        );


    let min =
        Math.floor(
            Math.min(...values) - 5
        );

    let max =
        Math.ceil(
            Math.max(...values) + 5
        );


    /*
     * Prevent zero-height graph.
     */

    if (max === min) {

        max += 5;
        min -= 5;
    }


    drawGrid(
        context,
        canvas,
        min,
        max
    );


    drawSegments(
        context,
        canvas,
        values,
        min,
        max,
        getWifiPointType
    );


    updateWifiHistorySummary(
        values
    );
}


// ============================================================
// WIFI POINT CLASSIFICATION
// ============================================================

function getWifiPointType(
    current,
    previous
) {

    if (!Number.isFinite(previous)) {

        return "normal";
    }


    const change =
        Math.abs(
            current - previous
        );


    /*
     * RSSI spike:
     *
     * 10 dBm or more sudden movement.
     */

    if (change >= 10) {

        return "spike";
    }


    if (current >= -65) {

        return "good";
    }


    return "normal";
}


// ============================================================
// WIFI SUMMARY
// ============================================================

function updateWifiHistorySummary(
    values
) {

    const latest =
        values[values.length - 1];

    const minimum =
        Math.min(...values);

    const maximum =
        Math.max(...values);


    document.getElementById(
        "wifiHistorySummary"
    ).textContent =
        `Latest ${latest} dBm • Range ${minimum} to ${maximum} dBm • Last ${values.length} readings`;
}


// ============================================================
// HEALTH CHART
// ============================================================

function drawHealthChart() {

    const canvas =
        document.getElementById(
            "healthChart"
        );

    const context =
        prepareCanvas(
            canvas
        );


    const points =
        telemetry
            .slice(-60)
            .filter(
                item =>
                    Number.isFinite(
                        Number(
                            item.internalTemperature
                        )
                    )
            );


    if (points.length < 2) {

        drawEmptyChart(
            canvas,
            context
        );

        return;
    }


    const values =
        points.map(
            item =>
                Number(
                    item.internalTemperature
                )
        );


    let min =
        Math.floor(
            Math.min(...values) - 2
        );

    let max =
        Math.ceil(
            Math.max(...values) + 2
        );


    if (max === min) {

        max += 2;
        min -= 2;
    }


    drawGrid(
        context,
        canvas,
        min,
        max
    );


    drawSegments(
        context,
        canvas,
        values,
        min,
        max,
        getHealthPointType
    );


    updateHealthHistorySummary(
        values
    );
}


// ============================================================
// HEALTH POINT CLASSIFICATION
// ============================================================

function getHealthPointType(
    current,
    previous
) {

    if (!Number.isFinite(previous)) {

        return "good";
    }


    const change =
        Math.abs(
            current - previous
        );


    /*
     * Sudden temperature movement.
     */

    if (change >= 3) {

        return "spike";
    }


    /*
     * Temperature condition.
     */

    if (current >= 60) {

        return "spike";
    }


    if (current >= 50) {

        return "normal";
    }


    return "good";
}


// ============================================================
// HEALTH SUMMARY
// ============================================================

function updateHealthHistorySummary(
    values
) {

    const latest =
        values[values.length - 1];

    const minimum =
        Math.min(...values);

    const maximum =
        Math.max(...values);


    document.getElementById(
        "healthHistorySummary"
    ).textContent =
        `Latest ${latest.toFixed(1)} °C • Range ${minimum.toFixed(1)} to ${maximum.toFixed(1)} °C • Last ${values.length} readings`;
}


// ============================================================
// DRAW SEGMENTS
// ============================================================

function drawSegments(
    context,
    canvas,
    values,
    min,
    max,
    classifier
) {

    const width =
        canvas.clientWidth;

    const height =
        canvas.clientHeight;

    const padding = 18;

    const chartWidth =
        width - padding * 2;

    const chartHeight =
        height - padding * 2;


    const xStep =
        chartWidth /
        Math.max(
            1,
            values.length - 1
        );


    function getY(value) {

        return (
            height -
            padding -
            (
                (value - min) /
                (max - min)
            ) *
            chartHeight
        );
    }


    for (
        let i = 1;
        i < values.length;
        i++
    ) {

        const previous =
            values[i - 1];

        const current =
            values[i];


        const x1 =
            padding +
            (i - 1) *
            xStep;

        const x2 =
            padding +
            i *
            xStep;


        const y1 =
            getY(previous);

        const y2 =
            getY(current);


        const type =
            classifier(
                current,
                previous
            );


        context.beginPath();

        context.moveTo(
            x1,
            y1
        );

        context.lineTo(
            x2,
            y2
        );


        context.strokeStyle =
            getChartColor(type);

        context.lineWidth =
            type === "spike"
                ? 3
                : 2;


        context.lineCap =
            "round";

        context.stroke();


        /*
         * Highlight spike point.
         */

        if (
            type === "spike"
        ) {

            context.beginPath();

            context.arc(
                x2,
                y2,
                4,
                0,
                Math.PI * 2
            );


            context.fillStyle =
                getChartColor(
                    "spike"
                );

            context.fill();
        }
    }
}


// ============================================================
// CHART COLORS
// ============================================================

function getChartColor(
    type
) {

    if (type === "spike") {

        return "#ff3b30";
    }

    if (type === "good") {

        return "#34c759";
    }

    return "#0071e3";
}


// ============================================================
// GRID
// ============================================================

function drawGrid(
    context,
    canvas,
    min,
    max
) {

    const width =
        canvas.clientWidth;

    const height =
        canvas.clientHeight;

    const padding = 18;


    context.clearRect(
        0,
        0,
        width,
        height
    );


    context.strokeStyle =
        document.documentElement
            .classList
            .contains("dark")
            ? "rgba(255,255,255,0.08)"
            : "rgba(0,0,0,0.06)";


    context.lineWidth = 1;


    for (
        let i = 1;
        i <= 4;
        i++
    ) {

        const y =
            padding +
            (
                height -
                padding * 2
            ) *
            (i / 5);


        context.beginPath();

        context.moveTo(
            padding,
            y
        );

        context.lineTo(
            width - padding,
            y
        );

        context.stroke();
    }
}


// ============================================================
// CANVAS PREPARATION
// ============================================================

function prepareCanvas(
    canvas
) {

    const context =
        canvas.getContext(
            "2d"
        );


    const ratio =
        window.devicePixelRatio ||
        1;


    const width =
        canvas.clientWidth;

    const height =
        canvas.clientHeight;


    /*
     * Reset canvas transform.
     *
     * This prevents scale()
     * accumulating after redraws.
     */

    context.setTransform(
        1,
        0,
        0,
        1,
        0,
        0
    );


    canvas.width =
        Math.max(
            1,
            Math.floor(
                width * ratio
            )
        );

    canvas.height =
        Math.max(
            1,
            Math.floor(
                height * ratio
            )
        );


    context.scale(
        ratio,
        ratio
    );


    return context;
}


// ============================================================
// EMPTY CHART
// ============================================================

function drawEmptyChart(
    canvas,
    context
) {

    context.clearRect(
        0,
        0,
        canvas.clientWidth,
        canvas.clientHeight
    );


    context.fillStyle =
        document.documentElement
            .classList
            .contains("dark")
            ? "#a1a1a6"
            : "#86868b";


    context.font =
        "13px -apple-system, BlinkMacSystemFont, sans-serif";


    context.textAlign =
        "center";


    context.fillText(
        "Waiting for telemetry history...",
        canvas.clientWidth / 2,
        canvas.clientHeight / 2
    );
}


// ============================================================
// UPTIME
// ============================================================

function formatUptime(
    seconds
) {

    if (
        !Number.isFinite(seconds)
    ) {

        return "--";
    }


    seconds =
        Math.max(
            0,
            Math.floor(seconds)
        );


    const days =
        Math.floor(
            seconds / 86400
        );


    seconds %= 86400;


    const hours =
        Math.floor(
            seconds / 3600
        );


    seconds %= 3600;


    const minutes =
        Math.floor(
            seconds / 60
        );


    if (days > 0) {

        return `${days}d ${hours}h`;
    }


    if (hours > 0) {

        return `${hours}h ${minutes}m`;
    }


    return `${minutes}m`;
}


// ============================================================
// BYTES
// ============================================================

function formatBytes(
    bytes
) {

    if (
        !Number.isFinite(bytes)
    ) {

        return "--";
    }


    if (
        bytes >=
        1024 * 1024
    ) {

        return (
            bytes /
            (1024 * 1024)
        ).toFixed(1) +
        " MB";
    }


    if (
        bytes >= 1024
    ) {

        return (
            bytes /
            1024
        ).toFixed(1) +
        " KB";
    }


    return (
        Math.round(bytes) +
        " B"
    );
}


// ============================================================
// CONNECTION UI
// ============================================================

function showConnection(
    connected,
    text
) {

    connectionText.textContent =
        text;


    connectionStatus.classList.toggle(
        "connected",
        connected
    );
}


// ============================================================
// LOGOUT
// ============================================================

function setupLogout() {

    logoutButton.addEventListener(
        "click",
        async () => {

            try {

                await signOut(auth);

                window.location.href =
                    "/login.html";

            } catch (error) {

                console.error(
                    "[DASHBOARD] Logout failed:",
                    error
                );
            }
        }
    );
}


// ============================================================
// THEME
// ============================================================

function setupTheme() {

    const savedTheme =
        localStorage.getItem(
            "esp32-theme"
        );


    if (savedTheme === "dark") {

        document.documentElement
            .classList
            .add("dark");

        themeIcon.textContent =
            "☀";

    } else {

        themeIcon.textContent =
            "◐";
    }


    themeToggle.addEventListener(
        "click",
        () => {

            const dark =
                document.documentElement
                    .classList
                    .toggle("dark");


            localStorage.setItem(
                "esp32-theme",
                dark
                    ? "dark"
                    : "light"
            );


            themeIcon.textContent =
                dark
                    ? "☀"
                    : "◐";


            /*
             * Redraw visible charts because
             * the grid/text colors change.
             */

            if (
                wifiHistoryVisible
            ) {

                requestAnimationFrame(
                    drawWifiChart
                );
            }


            if (
                healthHistoryVisible
            ) {

                requestAnimationFrame(
                    drawHealthChart
                );
            }
        }
    );
}


// ============================================================
// RESIZE
// ============================================================

window.addEventListener(
    "resize",
    () => {

        clearTimeout(
            resizeTimer
        );


        resizeTimer =
            setTimeout(
                () => {

                    if (
                        wifiHistoryVisible
                    ) {

                        drawWifiChart();
                    }


                    if (
                        healthHistoryVisible
                    ) {

                        drawHealthChart();
                    }

                },
                120
            );
    }
);
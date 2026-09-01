// ============================================================
// DEVICES
// ============================================================

import {
    ref,
    onValue
} from
    "https://www.gstatic.com/firebasejs/11.10.0/firebase-database.js";

import { database } from "./firebase.js";


// ============================================================
// DEVICE PATH
// ============================================================

const devicesRef =
    ref(database, "devices");


// ============================================================
// LISTEN FOR DEVICES
// ============================================================

function observeDevices(
    callback
) {
    return onValue(
        devicesRef,
        function(snapshot) {

            const devices =
                snapshot.val() || {};

            callback(devices);
        }
    );
}


// ============================================================
// GET DEVICE
// ============================================================

function observeDevice(
    deviceId,
    callback
) {
    const deviceRef =
        ref(
            database,
            `devices/${deviceId}`
        );

    return onValue(
        deviceRef,
        function(snapshot) {

            const device =
                snapshot.val();

            callback(device);
        }
    );
}


// ============================================================
// EXPORT
// ============================================================

export {
    observeDevices,
    observeDevice
};
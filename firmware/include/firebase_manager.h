#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <FirebaseClient.h>

// ============================================================
// FIREBASE
// ============================================================

bool isFirebaseConnected();
void firebaseManagerBegin();
void firebaseManagerLoop();
void firebaseTelemetryCallback(
    AsyncResult &result
);

#endif
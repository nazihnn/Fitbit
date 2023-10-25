#pragma once

#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyAQekFNfVvfOajyfIp4hBKmRlKi6_ypn10"
#define DATABASE_URL "https://step-counter-1fd45-default-rtdb.europe-west1.firebasedatabase.app/"

bool isAuthenticated = false;

// firebase initalization
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String device_location = "wrist";
String databasePath = "";
String fuid = "";


class Database{
    public:
    Database(){
        firebase_init();
    }

    void firebase_init(){
    // configure firebase API Key
    config.api_key = API_KEY;
    // configure firebase realtime database url
    config.database_url = DATABASE_URL;
    // Enable WiFi reconnection
    Firebase.reconnectWiFi(true);
    Serial.println("------------------------------------");
    Serial.println("Sign up new user...");
    // Sign in to firebase Anonymously
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("Success");
        isAuthenticated = true;
        // Set the database path where updates will be loaded for this device
        databasePath = "/" + device_location;
        fuid = auth.token.uid.c_str();
    }
    else
    {
        Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str());
        isAuthenticated = false;
    }
    // Assign the callback function for the long running token generation task, see addons/TokenHelper.h
    config.token_status_callback = tokenStatusCallback;
    // Initialise the firebase library
    Firebase.begin(&config, &auth);
    }
};
#include <Arduino.h>
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "NET.h"

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Nazih"
#define WIFI_PASSWORD "Nassar2002"



#define API_KEY "AIzaSyAQekFNfVvfOajyfIp4hBKmRlKi6_ypn10"
#define DATABASE_URL "https://step-counter-1fd45-default-rtdb.europe-west1.firebasedatabase.app/"


#define DEVICE_UID "1X"
#define DEVICE_UID2 "2X"
#define REPORTING_PERIOD_MS 100




FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String device_location = "wrist";
String databasePath = "";
String fuid = "";

unsigned long elapsedMillis = 0;
unsigned long update_interval = 10000;
int count = 0;
bool isAuthenticated = false;

// sensor variable declaration
Adafruit_MPU6050 mpu;
PulseOximeter max_sensor;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
long timer = 0;
int steps = 0;
int kalori = 0;
int BPM = 0;
unsigned long eskiZaman = 0;
unsigned long yeniZaman;
float distanceinonestep = 50;
float distance = 0;
int range = 0;
//float accy = 0;
int threshold = 1.28;


// firbaseb declartion
FirebaseJson steps_json;
FirebaseJson kalori_json;
FirebaseJson distance_json;
FirebaseJson BPM_json;


void init_wifi()
{

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
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

// void database_test()
// {
//   // Check that 10 seconds has elapsed before, device is authenticated and the firebase service is ready.
//   if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready())
//   {
//     elapsedMillis = millis();
//     Serial.println("------------------------------------");
//     Serial.println("Set int test...");
//     // Specify the key value for our data and append it to our path
//     String node = databasePath + "/value";
//     // Send the value our count to the firebase realtime database
//     if (Firebase.set(fbdo, node.c_str(), count++))
//     {
//       // Print firebase server response
//       Serial.println("PASSED");
//       Serial.println("PATH: " + fbdo.dataPath());
//       Serial.println("TYPE: " + fbdo.dataType());
//       Serial.println("ETag: " + fbdo.ETag());
//       Serial.print("VALUE: ");
//       printResult(fbdo); // see addons/RTDBHelper.h
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     else
//     {
//       Serial.println("FAILED");
//       Serial.println("REASON: " + fbdo.errorReason());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//   }
// }

void init_mpu6050()
{
  Wire.begin(22, 21);

  Serial.println("MPU6050 OLED demo");
  if (!mpu.begin())
  {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  Serial.println("Found a MPU-6050 sensor");

  
  
// Initialise steps json data
  steps_json.add("deviceuid", DEVICE_UID);
  steps_json.add("name", "stepcount");
  steps_json.add("value", steps);

  // Print out initial steps values
  String jsonStr;
  steps_json.toString(jsonStr, true);
  Serial.println(jsonStr);

  // Initialise calories json data
  kalori_json.add("deviceuid", DEVICE_UID);
  kalori_json.add("name", "calories");
  kalori_json.add("type", "kcal");
  kalori_json.add("value", kalori);

  // Print out initial calories values
  String jsonStr1;
  kalori_json.toString(jsonStr1, true);
  Serial.println(jsonStr1);

  // Initialise distance json data
  distance_json.add("deviceuid", DEVICE_UID);
  distance_json.add("name", "distance");
  distance_json.add("type", "M");
  distance_json.add("value", distance);

  // Print out initial  distnace values
  String jsonStr2;
  distance_json.toString(jsonStr2, true);
  Serial.println(jsonStr2);

}

void onBeatDetected() {
  Serial.println("Beat!");
}

void init_max(){
Serial.print("Initializing pulse oximeter..");
if (!max_sensor.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  max_sensor.setOnBeatDetectedCallback(onBeatDetected);

  BPM_json.add("deviceuid", DEVICE_UID2);
  BPM_json.add("name", "BPM");
  BPM_json.add("value", BPM);

  // Print out initial temperature values
  String jsonStr3;
  BPM_json.toString(jsonStr3, true);
  Serial.println(jsonStr3);


}

void init_oled(){

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  Serial.println("Found oled");



  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);
}


void setup()
{

  Serial.begin(115200);
  init_wifi();
  firebase_init();
  init_mpu6050();
  init_oled();
  init_max();
}

void print_oled(){

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  max_sensor.update();
  
  
  if (millis()- eskiZaman > 100) {
    
    display.clearDisplay();
    display.setCursor(0, 0);

    
    
    if (abs(a.acceleration.y)  > threshold){
      
      steps++;
      
      
    } 
    distance = steps * distanceinonestep / 100;
    kalori=steps*0.05;
    BPM = max_sensor.getHeartRate();
    
    eskiZaman = millis();

    display.setTextSize(1);
    Serial.print("steps");
    Serial.println(steps);
    display.print("Steps:");
    display.println(steps);
    display.print("calories:");
    display.println(kalori);
   
    display.print("Distance:");
    display.println(distance);
    display.print("BPM:");
    display.println(BPM);
    
  
  display.display();
  //delay(100);
}

if (isnan(steps) || isnan (kalori) || isnan(distance) || isnan (BPM)) {
    Serial.println(F("Failed to read from MPU6050 sensor!"));
    return;
  }

  Serial.printf("stepcount: %.d \n", steps);
  Serial.printf("calories: %.2d \n", kalori);
  Serial.printf("distance: %.2f \n", distance);
  Serial.printf("BPM: %.d \n",  BPM);
  

  steps_json.set("value", steps);
  kalori_json.set("value", kalori);
  distance_json.set("value", distance);
  BPM_json.set("value",BPM);

}


// void updateSensorReadings(){
//   Serial.println("------------------------------------");
//   Serial.println("Reading Sensor data ...");

//   sensors_event_t a, g, temp;
//   mpu.getEvent(&a, &g, &temp);
//   max_sensor.update();
//   if (millis()- eskiZaman > 100) {
    
//     if (abs(a.acceleration.y)  > threshold){
      
//       steps++;
      
      
//     } 
//     distance = steps * distanceinonestep / 100;
//     kalori=steps*0.05;
//     BPM = max_sensor.getHeartRate();
//   }
//   // Check if any reads failed and exit early (to try again).
  
// }

void uploadSensorData() {
  if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready())
    {
      elapsedMillis = millis();

      //updateSensorReadings();
      print_oled();

      String steps_node = databasePath + "/stepcount";  
      String kalori_node = databasePath + "/calories"; 
      String distance_node = databasePath + "/distance"; 
      String BPM_node = databasePath + "/BPM";


      if (Firebase.setJSON(fbdo, steps_node.c_str(), steps_json))
      {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
          Serial.println("ETag: " + fbdo.ETag());
          Serial.print("VALUE: ");
          printResult(fbdo); //see addons/RTDBHelper.h
          Serial.println("------------------------------------");
          Serial.println();
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }

      if (Firebase.setJSON(fbdo, kalori_node.c_str(), kalori_json))
      {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
          Serial.println("ETag: " + fbdo.ETag());
          Serial.print("VALUE: ");
          printResult(fbdo); //see addons/RTDBHelper.h
          Serial.println("------------------------------------");
          Serial.println();
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }

      if (Firebase.setJSON(fbdo, distance_node.c_str(), distance_json))
      {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
          Serial.println("ETag: " + fbdo.ETag());
          Serial.print("VALUE: ");
          printResult(fbdo); //see addons/RTDBHelper.h
          Serial.println("------------------------------------");
          Serial.println();
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }

      if (Firebase.setJSON(fbdo, BPM_node.c_str(), BPM_json))
      {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
          Serial.println("ETag: " + fbdo.ETag());
          Serial.print("VALUE: ");
          printResult(fbdo); //see addons/RTDBHelper.h
          Serial.println("------------------------------------");
          Serial.println();
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          Serial.println("------------------------------------");
          Serial.println();
      }


      
    }
}


void loop()
{
  // put your main code here, to run repeatedly:
  uploadSensorData();
  print_oled();
}

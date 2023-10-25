
#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

                              
#include "addons/TokenHelper.h"// Provide the token generation process info.
#include "addons/RTDBHelper.h"// Provide the RTDB payload printing info and other helper functions.

// firbease intalization
const char* firebaseHost = "step-counter-1fd45.firebaseapp.com";
const char* firebaseAuth = "AIzaSyAQekFNfVvfOajyfIp4hBKmRlKi6_ypn10";
#define API_KEY "AIzaSyAQekFNfVvfOajyfIp4hBKmRlKi6_ypn10"
#define DATABASE_URL "https://step-counter-1fd45-default-rtdb.europe-west1.firebasedatabase.app"
#define USER_EMAIL "Neezo322@gmail.com"
#define USER_PASSWORD "nassar2002"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid;
String databasePath;
String stepaccy = "/accy";
String kalPath = "/kalori";
String distPath = "/distance";
String timePath = "/timestamp";
String parentPath;
int timestamp;
FirebaseJson json;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;


// wifi 
const char* ssid = "Nazih";
const char* password = "Nassar2002";

// variable declartion 
Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
long timer = 0;
int steps = 0;
int kalori = 0;
unsigned long eskiZaman = 0;
unsigned long yeniZaman;
float distanceinonestep = 50; 
float distance;
int range=0;
int threshold = 1.28;
bool peakDetected = false;

// server intalization 
const char* ntpServer = "pool.ntp.org";


void initwifi() {

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

}


unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}




void setup() {

  Serial.begin(115200);

  initwifi();
  Wire.begin(22,21);
  configTime(0, 0, ntpServer);
  //Firebase.begin(&config, &auth);
  


  Serial.println("MPU6050 OLED demo");
  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  Serial.println("Found a MPU-6050 sensor");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
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


  // Assign the api key (required)
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  display.clearDisplay();
  display.setCursor(0, 0);

  Serial.print("Accelerometer ");
  Serial.print("X: ");
  Serial.print(a.acceleration.x, 1);
  Serial.print(" m/s^2, ");
  Serial.print("Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(" m/s^2, ");
  Serial.print("Z: ");
  Serial.print(a.acceleration.z, 1);
  Serial.println(" m/s^2");

  

  if(millis() - timer > 1000){
    Serial.print("accY : ");
    Serial.print(abs(a.acceleration.y ));
    Serial.print(a.acceleration.y );
    //display.println(abs(a.acceleration.y) );
  }
  if (abs(a.acceleration.y)  > threshold){
    
    steps++;
    delay(350);
    
  } 
  distance = steps * distanceinonestep / 100;
  kalori=steps*0.05;
  
  yeniZaman = millis();
  if (yeniZaman - eskiZaman > 3000) {
    display.setTextSize(1);
    Serial.print("Steps ");
    Serial.println(steps);
    display.println("Steps ");
    display.println(steps);
    display.println("calories");
    display.println(kalori);
    display.setCursor(64,16); 
    display.println("Distance");
    display.setCursor(64,23); 
    display.println(distance);
    
    //counter->save(steps);
  display.display();
  delay(100);
}


String firestep = String(steps) ; 
Firebase.pushString("steps", firestep); 


if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    // //Get current timestamp
    // timestamp = getTime();
    // Serial.print ("time: ");
    // Serial.println (timestamp);

    // parentPath= databasePath + "/" + String(timestamp);

    json.set(stepaccy.c_str(), String(a.acceleration.y()));
    //json.set(humPath.c_str(), String(bme.readHumidity()));
    //json.set(presPath.c_str(), String(bme.readPressure()/100.0F));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }




}


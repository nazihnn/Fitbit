#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include <WiFi.h>
// #include <FirebaseESP32.h>

// #include "addons/TokenHelper.h"
// #include "addons/RTDBHelper.h"

#include "Internet.h"
#include "Database.h"


#define SCREEN_ADDRESS 0x3C
#define BPM_UPDATE_INTERVAL_MS 500
#define STEPS_UPDATE_INTERVAL_MS 1000




// #define API_KEY "AIzaSyAQekFNfVvfOajyfIp4hBKmRlKi6_ypn10"
// #define DATABASE_URL "https://step-counter-1fd45-default-rtdb.europe-west1.firebasedatabase.app/"


#define DEVICE_UID "1X"
#define DEVICE_UID2 "2X"


//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TaskHandle_t Task1Handel = NULL;
TaskHandle_t Task2Handel = NULL;
TaskHandle_t Task3Handel = NULL;
TaskHandle_t Task4Handel = NULL;

// classes
Internet internet = Internet();
Database databse = Database();

// MPU6050 sensor initalization 
Adafruit_MPU6050 mpu;
float XAccel, YAccel, ZAccel, value;
const float THRESHOLD = 1.5; // Threshold for peak detection
int step_count = 0; // Step count
float distanceinonestep = 50;
float distance = 0;
int kalori = 0;

// walking 
  // MAGNITUDE 
  const float M_THRESHOLD1 = 12; 
  const float M_THRESHOLD2 = 14.3;
  const float M_vally = 8;
  // Z ACCELERATION
  const float Z_THRESHOLD1 = 11; 
  const float Z_THRESHOLD2 = 11.9;
  // Y ACCELERATION 
  const float Y_THRESHOLD1 = 4; 
  const float Y_THRESHOLD2 = 6;
// running 
  // MAGNITUDE 
  const float M_THRESHOLD1_run = 2; 
  //const float M_THRESHOLD2_run = 25;
const float M_peak_r = 22;

bool stepFlag = false;

// Max30100 sensor initlization 
PulseOximeter pox;
uint32_t lastBpmUpdate = 0;
float bpm = 0;
const int num_samples = 10;
float bpm_sum = 0;
float bpm_avg = 0;

// oled screen
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

// firebase initalization
// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;

// String device_location = "wrist";
// String databasePath = "";
// String fuid = "";

FirebaseJson steps_json;
FirebaseJson kalori_json;
FirebaseJson distance_json;
FirebaseJson BPM_json;

String steps_node ;  
String kalori_node; 
String distance_node ; 
String BPM_node;


//bool isAuthenticated = false;
unsigned long elapsedMillis = 0;
unsigned long update_interval = 15000;


// void init_wifi()
//     {

//     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(1000);
//         Serial.println("Connecting to WiFi...");
//     }
//     Serial.println("Connected to WiFi");

//     Serial.println();
//     Serial.print("Connected with IP: ");
//     Serial.println(WiFi.localIP());
//     Serial.println();
//     }


// void firebase_init(){
//   // configure firebase API Key
//   config.api_key = API_KEY;
//   // configure firebase realtime database url
//   config.database_url = DATABASE_URL;
//   // Enable WiFi reconnection
//   Firebase.reconnectWiFi(true);
//   Serial.println("------------------------------------");
//   Serial.println("Sign up new user...");
//   // Sign in to firebase Anonymously
//   if (Firebase.signUp(&config, &auth, "", ""))
//   {
//     Serial.println("Success");
//     isAuthenticated = true;
//     // Set the database path where updates will be loaded for this device
//     databasePath = "/" + device_location;
//     fuid = auth.token.uid.c_str();
//   }
//   else
//   {
//     Serial.printf("Failed, %s\n", config.signer.signupError.message.c_str());
//     isAuthenticated = false;
//   }
//   // Assign the callback function for the long running token generation task, see addons/TokenHelper.h
//   config.token_status_callback = tokenStatusCallback;
//   // Initialise the firebase library
//   Firebase.begin(&config, &auth);
// }

// void updatesensor(){

//   sensors_event_t a, g, temp;
//   mpu.getEvent(&a, &g, &temp);
//   pox.update();
  
  
  
  
//   display.clearDisplay();
//   display.setCursor(0, 0);

  
  
//   if (abs(a.acceleration.y)  > THRESHOLD){
    
//     step_count++;
    
    
//   } 
//   distance = step_count * distanceinonestep / 100;
//   kalori=step_count*0.05;
//   bpm= pox.getHeartRate();
  
  

// }

void task1(void *pvParameters) {
  //mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

  // Initialise steps json data
  steps_json.add("deviceuid", DEVICE_UID);
  steps_json.add("name", "stepcount");
  steps_json.add("value", step_count);

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

  
  while (1) {

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    XAccel = abs(a.acceleration.x);
    YAccel = abs(a.acceleration.y);
    ZAccel = abs(a.acceleration.z);

    value  =  sqrt(  (XAccel * XAccel) + (YAccel * YAccel) + (ZAccel * ZAccel)   );

   
    if (value < M_vally && stepFlag == false ) {
      stepFlag = true;
      
    }
    if (value > M_THRESHOLD2 && YAccel > Y_THRESHOLD2 && stepFlag == true ) {
       
        step_count++;
       
        stepFlag = false;
     }

    if (value > M_peak_r && stepFlag == false ) {
      stepFlag = true;
      delay(30);

      
    }
    if (value < M_THRESHOLD1_run && stepFlag == true ) {
      
      step_count++;
      stepFlag = false;
      delay(1000);
    } 

    if (value > 40 || value < 0.5  ) {
      stepFlag = false;
    }

    distance = step_count * distanceinonestep / 100;
    kalori=step_count*0.05;
    
   //vTaskDelay (1000/ portTICK_RATE_MS); 
    // Delay for 1 second


    if (isnan(step_count) || isnan (kalori) || isnan(distance) ) {
      Serial.println(F("Failed to read from MPU6050 sensor!"));
      return;
    }

    Serial.printf("stepcount: %.d \n", step_count);
    Serial.printf("calories: %.2d \n", kalori);
    Serial.printf("distance: %.2f \n", distance);
    

    steps_json.set("value", step_count);
    kalori_json.set("value", kalori);
    distance_json.set("value", distance);

  }

}
  


void task2(void *pvParameters) {
  
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  //pox.setSampleRate(MAX30100_SAMPRATE_400HZ); // Set sample rate to 400Hz
  //pox.setPulseAmplitudeReductionFactor(4);// Increase pulse amplitude reduction factor to 4
  
  BPM_json.add("deviceuid", DEVICE_UID2);
  BPM_json.add("name", "BPM");
  BPM_json.add("value", bpm_avg);

  // Print out initial temperature values
  String jsonStr3;
  BPM_json.toString(jsonStr3, true);
  Serial.println(jsonStr3);


  while (1) {
    bpm_sum = 0;
    //bpm_avg = 0;
    
    for (int i = 0; i < num_samples; i++) {
      // Update the pulse oximeter
      pox.update();

      // Get heart rate value
      bpm = pox.getHeartRate();

      // Add heart rate value to sum
      if (bpm >= 50 && bpm <= 110) {
        bpm_sum += bpm;
      
      }
      //delay(100);
    }

    // Calculate average of heart rate values
    bpm_avg = bpm_sum / num_samples;

    if(isnan(bpm_avg)){
    Serial.println(F("Failed to read from max30100 sensor!"));
    return;
    }

   Serial.printf("BPM: %.d \n",  bpm_avg);
   BPM_json.set("value",bpm_avg);

    //delay(1000);
        
  }  
} 




void task3(void *pvParameters) {

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  Serial.println("Found oled");

  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);


  while (1){

    display.clearDisplay();

    // Print the step count and heart rate on the OLED display
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Step Count: ");
    display.println(step_count);
    display.print("Heart Rate: ");
    display.println(bpm_avg);
    display.print("Distance: ");
    display.println(distance);
    display.print("Calories: ");
    display.println(kalori);

    // Display the OLED data
    display.display();


  }

}


void task4(void *pvParameters) {

   steps_node = databasePath + "/stepcount";  
   kalori_node = databasePath + "/calories"; 
    distance_node = databasePath + "/distance"; 
   BPM_node = databasePath + "/BPM";


  while(1){

    //if (millis() - elapsedMillis > update_interval && isAuthenticated && Firebase.ready())
    //{
      elapsedMillis = millis();
       void task1();
       void task2();
      //updatesensor();
     
    
      if (Firebase.setJSON(fbdo, steps_node.c_str(), steps_json))
      {
          Serial.println("PASSED");
         
          printResult(fbdo); //see addons/RTDBHelper.h
          
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          
      }

      if (Firebase.setJSON(fbdo, kalori_node.c_str(), kalori_json))
      {
          Serial.println("PASSED");
         
          printResult(fbdo); //see addons/RTDBHelper.h
          
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
         
      }

      if (Firebase.setJSON(fbdo, distance_node.c_str(), distance_json))
      {
          Serial.println("PASSED");
          
          printResult(fbdo); //see addons/RTDBHelper.h
         
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          
      }

      if (Firebase.setJSON(fbdo, BPM_node.c_str(), BPM_json))
      {
          Serial.println("PASSED");
         
          printResult(fbdo); //see addons/RTDBHelper.h
          
      }
      else
      {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          
      }


      
    //}



  }

}


void setup()
{
  Wire.begin(22, 21);
  Serial.begin(115200);
  internet.init_wifi();
  databse.firebase_init();
 
  mpu.begin();
  pox.begin();
    
  xTaskCreate(task1, "Task 1", 4096, NULL, 4, &Task1Handel);
  xTaskCreatePinnedToCore(task2, "Task 2", 4096, NULL, 3, &Task2Handel,1);
  xTaskCreatePinnedToCore(task3, "Task 3", 2048, NULL, 3, &Task3Handel,1);
  xTaskCreatePinnedToCore(task4, "Task 4", 8192, NULL, 3, &Task4Handel,0);
    
    
}

void loop(){

  Serial.print("MAG: ");
  Serial.println(value);
 // Serial.print("\tHeart Rate: ");
  //Serial.println(bpm_avg);

  //dealy(1000);

}


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define BPM_UPDATE_INTERVAL_MS 500
#define STEPS_UPDATE_INTERVAL_MS 1000


Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

TaskHandle_t Task1Handel = NULL;
TaskHandle_t Task2Handel = NULL;
TaskHandle_t Task3Handel = NULL;

// MPU6050 sensor initalization 
Adafruit_MPU6050 mpu;
float XAccel, YAccel, ZAccel, value;

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

float MAX_WALK = 18.5;
float MAX_RUN = 30.0;
float step_count = 0; // Step count
float step_count_W = 0; // Step count
float step_count_R = 0; // Step count
float dis = 0;
int kalori = 0;
float stepTime = 0;
bool stepFlag = false;
float avg = 0;
float value_sum= 0;
float value_avg= 0;
  const int num_samples2 = 20;



// Max30100 sensor initlization 
PulseOximeter pox;
uint32_t lastBpmUpdate = 0;
float bpm = 0;
const int num_samples = 10;
float bpm_sum = 0;
float bpm_avg = 0;
  


void task1(void *pvParameters) {
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  //const TickType_t SAMPLE_WINDOW = pdMS_TO_TICKS(10000); // 10 seconds 
  //TickType_t sampleStartTime = xTaskGetTickCount();


  while (1) {
    

   // TickType_t sampleElapsedTime = xTaskGetTickCount() - sampleStartTime;
   

   //if (sampleElapsedTime >= SAMPLE_WINDOW) 

      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      XAccel = abs(a.acceleration.x);
      YAccel = abs(a.acceleration.y);
      ZAccel = abs(a.acceleration.z);

      value  =  sqrt(  (XAccel * XAccel) + (YAccel * YAccel) + (ZAccel * ZAccel)   );

      //value_sum += value  ;
   
    

    //value_avg = value_sum/num_samples2;

    
    if (value < M_vally && stepFlag == false ) {
      stepFlag = true;
      
    }
    if (value > M_THRESHOLD2 && YAccel > Y_THRESHOLD2 && stepFlag == true ) {
       
        step_count_W++;
       
        stepFlag = false;
     }

    if (value > M_peak_r && stepFlag == false ) {
      stepFlag = true;
      delay(30);

      
    }
    if (value < M_THRESHOLD1_run && stepFlag == true ) {
      
      step_count_R++;
      stepFlag = false;
      delay(1000);
    } 

    if (value > 40 || value < 0.5  ) {
      stepFlag = false;
    }

    step_count = step_count_W + step_count_R;
    
    dis = (step_count_W * 68 + step_count_R * 120)/100;
    kalori = step_count * 0.05;
    
   //vTaskDelay (10/ portTICK_RATE_MS); 
    // Delay for 1 second
  }
}  


void task2(void *pvParameters) {
  
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  //pox.setSampleRate(MAX30100_SAMPRATE_400HZ); // Set sample rate to 400Hz
  //pox.setPulseAmplitudeReductionFactor(4);// Increase pulse amplitude reduction factor to 4
  

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
    display.println(dis);
    display.print("Calories: ");
    display.println(kalori);

    // Display the OLED data
    display.display();


  }

}



void setup()
{
    Wire.begin(22, 21);
    Serial.begin(115200);

 
    mpu.begin();
    pox.begin();

    
    xTaskCreate(task1, "Task 1", 8192, NULL, 4, &Task1Handel);
    xTaskCreatePinnedToCore(task2, "Task 2", 4096, NULL, 3, &Task2Handel,1);
    xTaskCreatePinnedToCore(task3, "Task 3", 2048, NULL, 3, &Task3Handel,0);
}

void loop(){

  //Serial.print("Step Count: ");
  Serial.print("Acceleration: ");
  Serial.print(XAccel);
  Serial.print(",");
  Serial.print(YAccel);
  Serial.print(",");
  Serial.print(ZAccel);
  Serial.print(",");
  Serial.println(bpm_avg);

 // Serial.print("\tHeart Rate: ");
  //Serial.println(bpm_avg);

  //delay(3);

}


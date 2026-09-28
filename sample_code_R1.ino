#include <Arduino.h>
#include "geeWhiz.h"
#include <math.h>

// ================== Pins ==================
int MOT_PIN = A0;   // motor angle sensor
int BAL_PIN = A1;   // ball position sensor

// ================== Stiction Values ==================
float STICTION_CW = 0.15;
float STICTION_CCW = -0.16;

// ================== Offset ==================
float OFFSET = M_PI;

// ================== Motor Max Range ==================
int RANGE = 16383;

// ================== Square Wave Sign ==================
int SQUARE_WAVE = 1;

// ================== Setup ==================
void setup() {

  analogReadResolution(14);
  pinMode(A5, OUTPUT);   // A5 can be used to measure cycle time using an oscilloscope by connecting the scope to the Arduino Box Motor Leads
  Serial.begin(115200);
  delay(300);

  geeWhizBegin();                 
  set_control_interval_ms(10); // 100 ms loop
  setMotorVoltage(0.0f);

  Serial.println("geeWhiz Started");  
}

// ================== Helpers ==================

float getGearPosition() {
  static bool newAvg = 1;
  static float motorAvg = 0.0f;
  float rawMotor = analogRead(MOT_PIN);

  /* On first run of getGearPosition, set a new average of simply the raw value. */
  if (newAvg) {
    motorAvg = rawMotor;
    newAvg = 0;
  }
  else {
    /* Apply a smoothing factor of 0.25. */
    motorAvg = (0.25 * rawMotor) + ((1 - 0.25) * motorAvg);
  }
  return (-2)*M_PI / RANGE * motorAvg + OFFSET;
}

void pController(float threshold, float k_p, float error) {
  /* Voltage rises to 1 V. */
  float target = 0.1 * SQUARE_WAVE;
  unsigned long startTime = millis();
  /* P-controller update loop. Upon steady state condition, exit update loop.*/
  while (millis() - startTime < 3000) 
  {
    error = target - getGearPosition();
    //Serial.print(getGearPosition());
    //Serial.print(",");
    //Serial.println(target);
    if (abs(error) > threshold)
      setMotorVoltage((error * k_p) + (SQUARE_WAVE == 1 ? STICTION_CCW : STICTION_CW)); // scaling rads to V
  }
  //Serial.println("========= EXITED P CONTROLLER =============");
}

// ================== Loop ==================
void loop() {
  /* Generate a square wave voltage input and have the motor track it using a p-controller. */
  float threshold = 0.02f * 0.2f;
  float k_p = -15.0f;
  float error = 0.2f;
  
  /* Call P-controller. */
  pController(threshold, k_p, error);
  SQUARE_WAVE *= -1;
}

// ================== Control ISR ==================

void interval_control_code(void) {
  // ---- For Serial Plotter Scaling
  int maxy = 17000;
  int miny = 0;
  // ---- Read sensors ----
  float motor = getGearPosition();
  
  int ball  = analogRead(BAL_PIN);

  digitalWrite(A5,HIGH);   // A5 can be used to measure cycle time using an oscilloscope by connecting the scope to the Arduino Box Motor Leads
 /*/ Serial.print(maxy);
    Serial.print(",");
  Serial.print(miny);
    Serial.print(",");
  Serial.print(ball);
  Serial.print(",");*/
  digitalWrite(A5,LOW);   // A5 can be used to measure cycle time using an oscilloscope by connecting the scope to the Arduino Box Motor Leads
 
  Serial.print(motor, 4);
  Serial.print(",");
  Serial.println(SQUARE_WAVE * 0.1);
}

/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/aiden/Desktop/IoT-Engineering/VisualStudioCodeProjects/IoT_FundamentalsFinalProject/src/IoT_FundamentalsFinalProject.ino"
/*
 * Project IoT_FinalFundamentals
 * Description: Temperature/Light level sensor that communicates to 
 * a mobile device as well as with a connected OLED display.
 * Author: Aiden K
 * Date: 7/3/22
 */

//everything I imported
#include <blynk.h>
#include "oled-wing-adafruit.h"
#include <Wire.h>
#include "SparkFun_VCNL4040_Arduino_Library.h"

//VCNL4040 Proximity and ambient light sensor
void displayAL();
void displayTemp();
void setPoints();
void setup();
void loop();
#line 16 "c:/Users/aiden/Desktop/IoT-Engineering/VisualStudioCodeProjects/IoT_FundamentalsFinalProject/src/IoT_FundamentalsFinalProject.ino"
VCNL4040 proximitySensor;
//FeatherWing 128x32 OLED display
OledWingAdafruit display;

SYSTEM_THREAD(ENABLED);

//Inputs
#define DIAL_PIN A3
#define BUTTON D5
#define TEMP_IN A5

//Outputs
#define RED_LED D6
#define GREEN_LED D7
#define BLUE_LED D8

//for communicating with blynk
#define TEMP_GAUGE V0
#define RESET V1

int pointOne = 0;
int pointTwo = 0;

int lowPoint;
int highPoint;

int pastLight;
int ambientLight;

String lightLevel;

//for calculating temp from the input voltage
uint64_t reading;
double voltage;
double tempC;
double tempF;

String OLEDMode = "A";

//triggers when the button is pressed
BLYNK_WRITE(RESET) {
 if (param.asInt()) {
    //resets the points
    pointOne = 0;
    pointTwo = 0;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Clearing set points.");
    Blynk.notify("Set points cleared.");
    display.display();
    delay(1000);

    //runs the user through setting the points again
    setPoints();
 }
}

//Displays the ambient light to the OLED display
void displayAL()
{
  ambientLight = proximitySensor.getAmbient();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ambient Light Level:");
  display.println(ambientLight);
  display.display();
}

//displays temperature to the OLED
void displayTemp()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Temperature:");
  display.println(String(int(tempC)) + " Celsius,");
  display.println(String(int(tempF)) + " Fahrenheit.");
  display.display();
}

//guides the user through the point setting process
void setPoints()
{
  bool isSet = false;

  while (isSet == false)
  {
    //wont allow for 0 or 65535 as set points
    //this is so the set points actually do anything
    //as well as to not confuse the argon
    int dialValue = map(analogRead(DIAL_PIN), 0, 4095, 1, 65534);

    if (digitalRead(BUTTON) == HIGH && pointOne < 1)
    {
      pointOne = dialValue;
      delay(200); // added a delay because a human would
      // press the button for multiple cycles of loop(),
      // thus setting both points in one press
    }
    else if (digitalRead(BUTTON) == HIGH && pointTwo < 1)
    {
      pointTwo = dialValue;
      delay(200);
      isSet = true;
    }

    //need to find the larger and smaller points for future comparisons
    lowPoint = min(pointOne, pointTwo);
    highPoint = max(pointOne, pointTwo);

    //communicating with the user throught the OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Set your points.");
    display.println("Potentiometer:" + String(dialValue));
    display.println("Point One:" + String(pointOne));
    display.println("Point Two:" + String(pointTwo));
    display.display();
  }

  delay(500);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Thank you for setting");
  display.println("your points.");
  display.display();
  delay(1500);
}

void setup()
{
  //initiating everything
  Blynk.begin("ReeKDrWOFRmLjVs3W3Uz6jWYITSo2D5J", IPAddress(167, 172, 234, 162), 8080);

  Wire.begin();

  proximitySensor.begin();
  proximitySensor.powerOnAmbient();

  display.setup();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  pinMode(DIAL_PIN, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(TEMP_IN, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //Initially sets the points
  setPoints();
}

void loop()
{
  //keeps the display and blynk running
  Blynk.run();
  display.loop();

  //finds and transmits temp to blynk
  reading = analogRead(TEMP_IN);
  voltage = (reading * 3.3) / 4095.0;
  tempC = (voltage - 0.5) * 100;
  tempF = tempC * 1.8 + 32.0;
  Blynk.virtualWrite(TEMP_GAUGE, tempF);

  //detects and sets the OLED mode based on the buttons on the OLED display
  if (display.pressedA())
  {
    OLEDMode = "A";
  }
  else if (display.pressedB())
  {
    OLEDMode = "B";
  }
  else if (display.pressedC())
  {
    OLEDMode = "C";
  }

  //decides what to display based off the OLED mode
  if (OLEDMode == "A")
  {
    //displays light
    displayAL();
  }
  else if (OLEDMode == "B")
  {
    //displays temperature
    displayTemp();
  }
  else if (OLEDMode == "C")
  {
    //displays your set points
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Your Points:");
    display.println("High Point:" + String(highPoint));
    display.println("Low Point:" + String(lowPoint));
    display.display();
  }

  //turns off all the leds, but gets nullified for an led when that led gets turned on
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  //need to save the light level from the last cycle to detect if it passed a set point
  pastLight = ambientLight;

  //gets the ambientLight
  ambientLight = proximitySensor.getAmbient();

  //calculates what light level its detecting
  if (ambientLight < lowPoint)
  {
    lightLevel = "LOW";
    digitalWrite(BLUE_LED, HIGH);
  }
  else if (ambientLight > highPoint)
  {
    lightLevel = "HIGH";
    digitalWrite(RED_LED, HIGH);
  }
  else
  {
    lightLevel = "GOOD";
    digitalWrite(GREEN_LED, HIGH);
  }

  //notifies the blynk app when the light level passed a set point
  if (pastLight > highPoint && ambientLight < highPoint)
  {
    Blynk.notify("The Ambient Light Level passed the high point you set. The light level is " + lightLevel + ".");
  }
  else if (pastLight < lowPoint && ambientLight > lowPoint)
  {
    Blynk.notify("The Ambient Light Level passed the low point you set. The light level is " + lightLevel + ".");
  }
}
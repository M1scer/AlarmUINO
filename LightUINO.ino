#include <JC_Button.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <ds3231.h>

 /*
 _____ _               __ __ _ _____ _____ 
|  _  | | __ ___ _____|  |  | |   | |     |
|     | ||. |  _|     |  |  | | | | |  |  |
|__|__|_|___|_| |_|_|_|_____|_|_|___|_____|                                           
AlarmUINO Version 0.01

created by Clemens Jehle and licensed under GNU/GPL.
*/

#define LONG_PRESS 1000

// Define I2C Devices
LiquidCrystal_I2C lcd(0x27, 16, 2);
struct ts rtc;

// Define Buttons
#define buttonOk A0
#define buttonUp A1
#define buttonDown A2

// Define Outputs
#define powerpin 5
#define lcdPowerPin 3

Button okButton(buttonOk);
Button upButton(buttonUp);
Button downButton(buttonDown);

unsigned long previousmillispower = 0;
unsigned long powerintervalmax = 28000;

// LCD Symbols
byte bell[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};
byte clock[] = {
    B00000,
    B01110,
    B10101,
    B10111,
    B10001,
    B01110,
    B00000,
    B00000
    };

// Variables
// Display Off Time
unsigned long lastAction = 0;
unsigned long autoExitDelay = 0;
byte brightness = 0;
bool displayIsOn = true;

void readSettings(){

    brightness = 60;
    autoExitDelay = 10000;
}

void readButtons() {
  okButton.read();
  upButton.read();
  downButton.read();
  keepPowerbankAlive();
  if(okButton.wasReleased() || upButton.wasReleased() || downButton.wasReleased()){
    lastAction = millis();
  }
}

void keepPowerbankAlive() {
    if ((millis() - previousmillispower) > powerintervalmax && !digitalRead(powerpin))  {
        previousmillispower = millis();
        digitalWrite(powerpin,HIGH);
        Serial.println("PowerPin HIGH");
    }
    if ((millis() - previousmillispower) > 500 && digitalRead(powerpin)){
        digitalWrite(powerpin,LOW);
        Serial.println("PowerPin LOW");
    }
}

void displayTime(){
    Serial.println("=== displayTime()");
    lcd.clear();
    char printLine[16];

    // Time & Date
    lcd.setCursor(0,0);
    lcd.write(byte(0));
    lcd.setCursor(2,0);
    sprintf (printLine, "%02d:%02d %02d.%02d.%02d", rtc.hour, rtc.min, rtc.mday, rtc.mon, rtc.year % 100);
    lcd.print(printLine); 

    // Alarm Time & Date
    lcd.setCursor(0,1);
    lcd.write(byte(1));
    lcd.setCursor(2,1);
    //sprintf (printLine, "%02d:%02d %02d.%02d.%02d", rtc.hour, rtc.min, rtc.mday, rtc.mon, rtc.year % 100);
    //lcd.print(printLine); 

}

bool autoExit() {
      if ((millis() - lastAction) > autoExitDelay) 
      {
        Serial.println("=== autoExit()");
        lastAction = millis();
        return true;
      }
      return false;
}

void setup() {
    readSettings();

    // Init Serial Monitor
    Serial.begin(9600);
    delay(100);

    // Init LCD
    lcd.init();
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Lichtwecker");
    lcd.setCursor(0,1);
    lcd.print("V0.01");
    Serial.println("Initialisiere LCD...");
    lcd.createChar(0, clock);
    lcd.createChar(1, bell);

    // Init RTC
    Wire.begin();
    DS3231_init(DS3231_INTCN);
    Serial.println("Initialisiere RTC...");

    // Init Buttons
    pinMode(buttonOk, INPUT_PULLUP);
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    Serial.println("Initialisiere Buttons...");

    // Init Outputs
    pinMode(powerpin,OUTPUT);
    digitalWrite(powerpin, LOW);
    pinMode(lcdPowerPin,OUTPUT);
    analogWrite(lcdPowerPin,brightness);
    Serial.println("Initialisiere Ausgaenge...");

    // Display Time and Date
    delay(2000);
    // get Time
    DS3231_get(&rtc);
    displayTime();

    Serial.println("\n _____ _               __ __ _ _____ _____");
    Serial.println("|  _  | | __ ___ _____|  |  | |   | |     |");
    Serial.println("|     | ||. |  _|     |  |  | | | | |  |  |");
    Serial.println("|__|__|_|___|_| |_|_|_|_____|_|_|___|_____|");                                        
    Serial.println("AlarmUINO Version 0.01\n");
    Serial.println("created by Clemens Jehle and licensed under GNU/GPL.\n\n");

    lastAction = millis();
}

void loop() {
    readButtons();
    
    if(displayIsOn){
        // Turn off LCD after delay
        if(autoExit()){
            displayIsOn = false;
            analogWrite(lcdPowerPin,0);
        }
    }
    // Turn on LCD if a button was released
    else if(okButton.wasReleased() || upButton.wasReleased() || downButton.wasReleased()){
        displayIsOn = true;
        analogWrite(lcdPowerPin,brightness);
    }
          
}
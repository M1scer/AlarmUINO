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

// I2C Devices
LiquidCrystal_I2C lcd(0x27, 16, 2);
struct ts rtc;

// Buttons
#define buttonOk A0
#define buttonUp A1
#define buttonDown A2
Button okButton(buttonOk, 50);
Button upButton(buttonUp, 50);
Button downButton(buttonDown, 50);

// Outputs
#define powerpin 5
#define lcdPowerPin 3

unsigned long previousmillispower = 0;
unsigned long powerintervalmax = 28000;
#define LONG_PRESS 1000

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
byte wench[] = {
    B01001,
    B01001,
    B01001,
    B00110,
    B00110,
    B00110,
    B00110,
    B00000
    };
// Variables
unsigned long lastAction = 0;
unsigned long autoExitDelay = 0;
byte brightness = 0;
bool displayIsOn = true;

void readSettings(){
    Serial.println("=== readSettings()");
    brightness = 60;
    autoExitDelay = 30000;
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
        //Serial.println("PowerPin HIGH");
    }
    if ((millis() - previousmillispower) > 500 && digitalRead(powerpin)){
        digitalWrite(powerpin,LOW);
        //Serial.println("PowerPin LOW");
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

bool exit() {
      if ((millis() - lastAction) > autoExitDelay || okButton.pressedFor(LONG_PRESS)) {
        Serial.println("=== exit()");
        lastAction = millis();
        while (okButton.isPressed()) okButton.read(); //wait for user to release the button
        return true;
      }
      return false;
}

bool mainSettings(){
    Serial.println("=== mainSettings()");
    int setting = 0;
    bool refreshLCD = true;
    do {
        // Check Buttons and Open menu
        readButtons();
        if(okButton.wasReleased()){
            if(setting == 0){           // Date / Time Settings
                if(dateTimeSettings()){
                    Serial.println("=== mainSettings()");
                    refreshLCD = true;
                } 
            } else if(setting == 1){    // Alarm Settings
                if(alarmSettings()){
                    Serial.println("=== mainSettings()");
                    refreshLCD = true;
                }
            } else if(setting == 2){    // System Settings
                if(systemSettings()){
                    Serial.println("=== mainSettings()");
                    refreshLCD = true;
                }
            }
        } else if(upButton.wasReleased()){
            setting++;
            refreshLCD = true;
        } else if(downButton.wasReleased()){
            setting--;
            refreshLCD = true;
        }

        // Refresh Display
        if(refreshLCD){
            refreshLCD = false;
            Serial.print("Setting: ");
            Serial.println(setting);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.write(byte(2));
            lcd.setCursor(2,0);

            switch (setting)
            {
            case 0: // Date / Time Settings
                lcd.print("Date / Time"); 
                break;
            case 1: // Alarm Settings
                lcd.print("Alarm"); 
                break;
            case 2: // System Settings
                lcd.print("System"); 
                break;
            default:
                refreshLCD = true;
                if(setting < 0){
                    setting = 0;
                } else{
                    setting--;
                }
                break;
            }
        }
    } while (!exit());

    return true;
}

bool dateTimeSettings(){
    Serial.println("=== dateTimeSettings()");
    int setting = 0;
    bool refreshLCD = true;
    int hour = rtc.hour;
    int min = rtc.min;
    int mday = rtc.mday;
    int mon = rtc.mon;
    int year = rtc.year;

    do {
        readButtons();
        if(okButton.wasReleased()){
            if (setting == 5) { // Set Clock if settings are confirmed
                Serial.println("Date & Time set");
                rtc.hour = hour;
                rtc.min = min;
                rtc.mday = mday;
                rtc.mon = mon;
                rtc.year = year;
                DS3231_set(rtc);
                return true;
            } else { // Switch Settings
                setting++;
                refreshLCD = true;
            }
        } else if(upButton.wasReleased()){
            if(setting == 0 ) { // Hour
                hour++;
                if (hour > 24)
                {
                    hour = 0;
                } else if (hour < 0 ){
                    hour = 24;
                }
            }
            if(setting == 1 ) { // Minute
                min++;
                if (min > 60)
                {
                    min = 0;
                } else if (min < 0 ){
                    min = 60;
                }
            }
            if(setting == 2 ) { // Month
                mon++;
                if (mon > 12)
                {
                    mon = 0;
                } else if (mon < 0 ){
                    mon = 12;
                }
            }
            if(setting == 3 ) { // Day
                mday++;
                int maxDays;
                if(mon ==  2){ // Max Days February
                    maxDays = 28;
                } 
                else if(mon ==  1 || mon ==  3 || mon ==  5 || mon ==  7 || mon ==  8 || mon ==  10 || mon ==  12) { // Max Days 31
                    maxDays = 31;
                }
                else { // Max Days 30
                    maxDays = 30;
                }
                if (mday > maxDays)
                {
                    mday = 0;
                } else if (mday < 0 ){
                    mday = maxDays;
                }
            }
            if(setting == 4 ) { // Year
                year++;
                if (year > 2122)
                {
                    year = 2022;
                } else if (year < 2022 ){
                    year = 2022;
                }
            }
            refreshLCD = true;
        } else if(downButton.wasReleased()){
            if(setting == 0 ) { // Hour
                hour--;
                if (hour > 24)
                {
                    hour = 0;
                } else if (hour < 0 ){
                    hour = 24;
                }
            }
            if(setting == 1 ) { // Minute
                min--;
                if (min > 60)
                {
                    min = 0;
                } else if (min < 0 ){
                    min = 60;
                }
            }
            if(setting == 2 ) { // Month
                mon--;
                if (mon > 12)
                {
                    mon = 0;
                } else if (mon < 0 ){
                    mon = 12;
                }
            }
            if(setting == 3 ) { // Day
                mday--;
                int maxDays;
                if(mon ==  2){ // Max Days February
                    maxDays = 28;
                } 
                else if(mon ==  1 || mon ==  3 || mon ==  5 || mon ==  7 || mon ==  8 || mon ==  10 || mon ==  12) { // Max Days 31
                    maxDays = 31;
                }
                else { // Max Days 30
                    maxDays = 30;
                }
                if (mday > maxDays)
                {
                    mday = 0;
                } else if (mday < 0 ){
                    mday = maxDays;
                }
            }
            if(setting == 4 ) { // Year
                year--;
                if (year > 2122)
                {
                    year = 2022;
                } else if (year < 2022 ){
                    year = 2022;
                }
            }
            refreshLCD = true;
        }

        // Refresh Display
        if(refreshLCD){
            refreshLCD = false;
            Serial.print("Setting: ");
            Serial.println(setting);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.write(byte(2));
            lcd.setCursor(2,0);
            char printLine[16];

            switch (setting)
            {
            case 0: // Hour Settings
                lcd.print("Hour");                 
                break;
            case 1: // Minute Settings
                lcd.print("Minute"); 
                break;
            case 3: // Day Settings
                lcd.print("Day"); 
                break;
            case 2: // Month Settings
                lcd.print("Month"); 
                break;
            case 4: // Year Settings
                lcd.print("Year"); 
                break;
            case 5: // Confirm Settings              
                lcd.print("Confirm?"); 
                break;
            default:
                refreshLCD = true;
                if(setting < 0){
                    setting = 0;
                } else {
                    setting--;
                }
                break;
            }

            lcd.setCursor(2,1);
            sprintf (printLine, "%02d:%02d %02d.%02d.%02d", hour, min, mday, mon, year % 100);
            lcd.print(printLine); 
        }  
    } while (!exit());

    return true;
}

bool alarmSettings(){
    Serial.println("=== alarmSettings()");
    int setting = 0;
    bool refreshLCD = true;
    do {
        readButtons();
    } while (!exit());

    return true;
}

bool systemSettings(){
    Serial.println("=== systemSettings()");
    int setting = 0;
    bool refreshLCD = true;
    do {
        readButtons();
    } while (!exit());

    return true;
}

void setup() {
    // Init Serial Monitor
    Serial.begin(9600);
    while(!Serial);
    delay(500);

    readSettings();

    // Init LCD
    lcd.init();
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("AlarmUINO");
    lcd.setCursor(0,1);
    lcd.print("V0.01");
    Serial.println("Initialize LCD...");
    lcd.createChar(0, clock);
    lcd.createChar(1, bell);
    lcd.createChar(2, wench);

    // Init RTC
    Wire.begin();
    DS3231_init(DS3231_INTCN);
    Serial.println("Initialize RTC...");

    // Init Buttons
    pinMode(buttonOk, INPUT_PULLUP);
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    Serial.println("Initialize Buttons...");

    // Init Outputs
    pinMode(powerpin,OUTPUT);
    digitalWrite(powerpin, LOW);
    pinMode(lcdPowerPin,OUTPUT);
    analogWrite(lcdPowerPin,brightness);
    Serial.println("Initialize Outputs...");

    // Display Time and Date
    //delay(2000);
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
    bool refreshLCD = false;
    
    if(displayIsOn){
        // Turn off LCD after delay
        if(exit()){
            displayIsOn = false;
            analogWrite(lcdPowerPin,0);
            Serial.println("Display Off");
        }
        // Go into Settings
        else if(okButton.wasReleased()){
            refreshLCD = mainSettings();
        }
    }
    // Turn on LCD if a button was released
    else if(okButton.wasReleased() || upButton.wasReleased() || downButton.wasReleased()){
        displayIsOn = true;
        analogWrite(lcdPowerPin,brightness);
        Serial.println("Display On");
    }
    
    // Display Time if neccessary
    if(refreshLCD){
        displayTime();
    }
          
}
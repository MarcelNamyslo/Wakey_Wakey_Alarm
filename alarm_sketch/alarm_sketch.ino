#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define BUTTON_DELAY = 50 //millis with what rate buttons can be pressed, not nedded if INPUT_PULLUP works right
#define MAX_RANGE = 100;
#define BLINK_RATE = 400;
#define STANDARD_ALARM_DURATION = 10000;

//wifipassword
const char *ssid = "H";
const char *password = "hakansson";

const char* ssid = "Point";
const char* password =  "Point2424";

//const char* ssid = "Hedon Misafir";
//const char* password =  "hedon2020";

//const char* ssid = "DaktiloCafe";
//const char* password = "buldozer2017";

const long utcOffsetInSeconds = 3600;

//to check if an alarm is active
String ifActive;

//realtime
String currentime;
//connection mode (0 = external wifi)  (1 = internal wifi)
const char *connection_mode = "0";
bool isWifiConnected = false;

AsyncWebServer server(80);

String textToSend;
String alarmTimes[20];
String alarms[20];

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const int LDRSensorPin = 3;
const int switchPin = 4;
const int modePin = 5;
const int lightRegPin = 6;
const int rateRegPin = 7;
const int warmPin = 8;
const int coldPin = 9;

bool turnedOn = false;
bool stateSave = turnedOn;
unsigned long lastSwitch= 0;
bool autoMode = false;
bool modeSave = autoMode;
unsigned long lastAutoSwitch= 0;
int LDR_ON_LIGHT = 100; //TODO set correctly
int LDR_IN_DARK = 0; //TODO set correctly
bool brightnessChangeTurnOn = true; //specifies if changeing brightness should turn on the light
int brightness = 0; //in percent
int brightnessSave = brightness;
int coldRate = MAX_RANGE / 2; //the share of the cold white in the cold-warm mix
int rateSave = coldRate

//Alarm alarm = NULL; //defines if we're currently in an alarm
int alarm = 0; //the millis the alarm started

void handleText(AsyncWebServerRequest *request)
{
  if (request->hasParam("text", true))
  {
    textToSend = request->getParam("text", true)->value();
    String timer = String(textToSend[4]) + String(textToSend[5]) + String(textToSend[6]) + String(textToSend[7]) + String(textToSend[8]);
    String index = String(textToSend[2]);


    //if it is a new alarm, otherwise it is "U" for update alarm
    if(String(textToSend[0]) == "N"){

    Serial.println("Received data: " + textToSend);

    Serial.println(timer);

    alarmTimes[index.toInt()] = timer;
    alarms[index.toInt()] = textToSend.substring(2);


    File file = SPIFFS.open("/data.txt", "a");

    file.println(textToSend);
    file.close();

    request->send(200, "text/plain", textToSend.substring(2));

   }else if(String(textToSend[0]) == "U"){

    alarmTimes[index.toInt()] = timer;
    alarms[index.toInt()] = textToSend.substring(2);
    Serial.println(alarmTimes[textToSend[2]]);
    Serial.println(alarms[textToSend[2]]);
   }
  }
}

void setFullLightValue() {
    LDR_ON_LIGHT = analogRead(LDRSensorPin);
}

void setNoLightValue() {
    LDR_IN_DARK = analogRead(LDRSensorPin);
}


void setup() {

  pinMode(switchPin, INPUT_PULLUP);
  pinMode(modePin, INPUT_PULLUP);
  pinMode(lightRegPin, INPUT);
  pinMode(rateRegPin, INPUT);
  pinMode(warmPin, OUTPUT);
  pinMode(coldPin, OUTPUT);

  analogWriteRange(MAX_RANGE);
  analogReadRange(MAX_RANGE);

  Serial.begin(115200);
  timeClient.begin();
  if (connection_mode == "0") {

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println(WiFi.localIP());


    if (!SPIFFS.begin())
    {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  } else {

    startAP();
  }


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });


  server.on("/sendtext", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    handleText(request);
  });

  server.begin();

  attachInterrupt(digitalPinToInterrupt(switchPin), toggleTurnedOn, CHANGE); //TODO: adjust FALLING
  attachInterrupt(digitalPinToInterrupt(modePin), toggleMode, CHANGE);
  //attachInterrupt(analogPinToInterrupt(lightRegPin), adjustBrightness, CHANGE);
  //attachInterrupt(analogPinToInterrupt(rateRegPin), adjustRate, CHANGE);

}



void loop() {
    timeClient.update();

    //currentime= (timeClient.getHours() + timeClient.getMinutes() + timeClient.getSeconds());
    currentime = timeClient.getFormattedTime();
    Serial.println(currentime);
    for (int i = 0; i < 10; i++) {
        ifActive = String(alarms[0][alarms[0].length() -1]);
        if ((alarmTimes[textToSend[i]] + ":00") == String(currentime) && ifActive == "1" ) {
            Serial.println("ALARM!");

        }
    }

    if(checkChanges()) {
        applyLight();
    }
    if(alarm != 0) {
        proceedAlarm();
    }

    delay(1);
}

void toggleTurnedOn() {
    unsigned long millis = millis();
    Serial.println("Switch pressed on:", millis;
    if(millis -lastSwitch > BUTTON_DELAY) {
        turnedOn = !turnedOn; //maybe use stateSave
        Serial.print("State toggled to: ");Serial.println(turnedOn);
        lastSwitch = millis;
    }
}

void toggleMode() {
    unsigned long millis = millis();
    Serial.println("Auto-mode button pressed on:", millis);
    if(millis -lastAutoSwitch > BUTTON_DELAY) {
        autoMode = !autoMode; //maybe use modeSave
        Serial.print("Mode toggled to: ");Serial.println(autoMode);
        lastAutoSwitch = millis;
    }
}

bool checkChanges() {
    bool change = false;
    if(alarm != 0) { //only interrupting alarm
        if(turnedOn != stateSave) {
            alarm = 0;
            turnedOn = stateSave;
        }
        if(autoMode != modeSave) {
            alarm = 0;
            autoMode = modeSave;
        }
    } else {
        if(turnedOn != stateSave) {
            change = true;
            stateSave = turnedOn;
            brightness = (turnedOn ? brightnessSave : 0);
        }
        if(autoMode != modeSave) {
            change = true;
            modeSave = autoMode;
        }
        if(autoMode && !isInTargetLDRRange()) {
            change = true;
        }
    }
    if(checkReadLightSettingChanges()) { //interrupting alarm and adjusting light settings
        brightness = brightnessSave;
        coldRate = rateSave;
        alarm = 0;
        change = true;
    }

    return change;
}

bool checkReadLightSettingChanges() {
    bool change = false;
    temp = analogRead(lightRegPin);
    if(temp != brightnessSave) {
        //if configured, changing brightness when turned off turns light on, otherwise only if brightness was already zero
        if(brightnessChangeTurnOn || brightnessSave == 0) {
            turnedOn = true;
        }
        change = true;
        brightnessSave = temp;
    }
    temp = analogRead(rateRegPin);
    if(temp != rateSave) {
        change = true;
        rateSave = temp;
    }
    return change;
}

void applyLight() {
    if(autoMode && alarm == 0) {
        autoAdjustLight();
    } else {
        applyColdWarmMix(coldRate, brightness);
    }
}

void autoAdjustLight(int LDRValue) {
    int tempBrightness = brightness;
    int currentLDR = analogRead(LDRSensorPin);
    int targetLDR = getTargetLDR();

    if(targetLDR > currentLDR) {
        while(targetLDR > currentLDR && currentLDR < MAX_RANGE) {
            tempBrightness++;
            applyColdWarmMix(coldRate, tempBrightness);
            int currentLDR = analogRead(LDRSensorPin);
        }
    } else if(targetLDR < currentLDR) {
        while(targetLDR < currentLDR && currentLDR > 0) {
            tempBrightness--;
            applyColdWarmMix(coldRate, tempBrightness);
            int currentLDR = analogRead(LDRSensorPin);
        }
    }
}

bool isInTargetLDRRange() {
    int target = getTargetLDR();
    int current = analogRead(LDRSensorPin);
    if(abs(target - current) < abs(LDR_ON_LIGHT - LDR_IN_DARK) / MAX_RANGE) {
    }
}

int getTargetLDR() {
    return map(brightness, 0, MAX_RANGE, LDR_IN_DARK, LDR_ON_LIGHT));
    //return brightness * (LDR_ON_LIGHT - LDR_IN_DARK) +LDR_IN_DARK //alternative
}

void applyColdWarmMix(int rate, int tempBrightness) {
    //setting higher to brightness, other according to ratio
    int highPin = (rate > MAX_RANGE / 2 ? coldPin : warmPin);
    int lowPin = (rate > MAX_RANGE / 2 ? warmPin : coldPin);

    temBrightness = min(temBrightness, MAX_RANGE); //cap to max analogWrite
    analogWrite(highPin, tempBrightness);
    analogWrite(lowPin, (MAX_RANGE - rate) * ((brightness/rate)*MAX_RANGE)); //maybe convert to float

    //alternatively assuming 2 half lit are as bright as one full lit led (stripe)
    /***
    analogWrite(coldPin, coldRate * (brightness/MAX_RANGE));
    analogWrite(warmPin, (MAX_RANGE - coldRate) * (brightness/MAX_RANGE));
    ***/
}

void proceedAlarm() {
    if(alarm == 0 || millis() - alarm > STANDARD_ALARM_DURATION) {
        alarm = 0;
        return;
    }

    int startBrightness = (turnedOn ? brightnessSave : 0);
    int maxBrightness = (turnedOn ? MAX_RANGE : brightnessSave);
    float progress = static_cast<float>() / STANDARD_ALARM_DURATION;
    brightness = map(millis() - alarm, alarm, alarm + STANDARD_ALARM_DURATION, startBrightness, maxBrightness);
    coldRate = map(millis() - alarm, alarm, alarm + STANDARD_ALARM_DURATION, (turnedOn ? coldRate : 0), MAX_RANGE);


//    if(alarm.getType() == RISE_TO_MAX) {
//        brightness = bri
//    } else if(alarm.getType() == RISE_TO_LEVEL) {
//
//    } else if(alarm.getType() == BLINK) {
//        int toggleValue = (brightness != 0 ? 1 : 0);
//        if((millis() - alarm.getNextInitiation()) / BLINK_RATE) % 2 == toggleValue) {
//            brightness = (toggleValue == 0 ? brightnessSave : 0);
//        }
    }
}

void startAP() {
    Serial.println(F("[ INFO ] ESP-RFID is running in Fallback AP Mode"));
    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);
    char ssid[15];
    sprintf(ssid, "MIS-Car-%02x%02x%02x", macAddr[3], macAddr[4], macAddr[5]);
    startAPMode(ssid);
}

bool startAPMode(const char * ssid) {
    WiFi.mode(WIFI_AP);
    Serial.print(F("[ INFO ] Configuring access point... "));
    bool success = WiFi.softAP(ssid, NULL);
    Serial.println(success ? "Ready" : "Failed!");
    // Access Point IP
    IPAddress myIP = WiFi.softAPIP();
    Serial.print(F("[ INFO ] AP IP address: "));
    Serial.println(myIP);
    Serial.printf("[ INFO ] AP SSID: %s\n", ssid);
    isWifiConnected = success;
    return success;
}

String formatTime(int hour, int minute) {
    String time = "";

    if (hour < 10) {
    time += "0";
    }
    time += String(hour) + ":";

    if (minute < 10) {
    time += "0";
    }
    time += String(minute) + ":";

    return time;
}

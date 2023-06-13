#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define BUTTON_DELAY = 50 //millis with what rate buttons can be pressed, not nedded if INPUT_PULLUP works right
#define MAX_RANGE = 100;
#define BLINK_RATE = 400;

const char* ssid = "Point";
const char* password =  "Point2424";

//const char* ssid = "Hedon Misafir";
//const char* password =  "hedon2020";

//const char* ssid = "DaktiloCafe";
//const char* password = "buldozer2017";

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

Alarm alarm = NULL; //defines if we're currently in an alarm

AsyncWebServer server(80);

//not used
void handleSaveData(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String data;
    if (request->hasParam("data")) {
      data = request->getParam("data")->value();
      File file = SPIFFS.open("/data.txt", "w");
      if (!file) {
        request->send(500, "text/plain", "Error opening file for writing.");
        return;
      }
      if (file.print(data) == 0) {
        request->send(500, "text/plain", "Error writing data to file.");
        return;
      }
      file.close();
      request->send(200, "text/plain", "Data saved successfully.");
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  } else {
    request->send(405, "text/plain", "Method Not Allowed");
  }
}


void handlePostRequest(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    if (request->hasParam("data", true)) {
      String data = request->getParam("data", true)->value();

      File file = SPIFFS.open("/data.txt", "w");
      if (!file) {
        //request->send(500, "text/plain", "Error opening file for writing.");
        return;
      }
      if (file.println(data) == 0) {
        //request->send(500, "text/plain", "Error writing data to file.");
        return;
      }
     
      //request->send(200, "text/plain", "Data saved successfully.");
      
      // Do something with the received data
      Serial.println("Received data: " + data);
      file.println(data);

      
      File file1 = SPIFFS.open("/data.txt", "r");
      String data1 = file1.readString();
      file1.close();
      Serial.println("txt data: " + data1);
  
  
  file.close();
    } else {
      request->send(400, "text/plain", "Invalid request");
    }
  } else {
    request->send(405, "text/plain", "Method Not Allowed");
  }
}


void handleGetData(AsyncWebServerRequest *request) {
  File file = SPIFFS.open("/data.txt", "r");
  if (!file) {
    request->send(500, "text/plain", "Error opening file for reading.");
    return;
  }
  String data = file.readString();
  file.close();
  Serial.println("txt send back data: " + data);
  request->send(200, "text/plain", data);
}

void setFullLightValue() {
    LDR_ON_LIGHT = analogRead(LDRSensorPin);
}

void setNoLightValue() {
    LDR_IN_DARK = analogRead(LDRSensorPin);
}



void setup() {

  pinMode(LDRSensorPin, INPUT);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(modePin, INPUT_PULLUP);
  pinMode(lightRegPin, INPUT);
  pinMode(rateRegPin, INPUT);
  pinMode(warmPin, OUTPUT);
  pinMode(coldPin, OUTPUT);

  analogWriteRange(MAX_RANGE);
  analogReadRange(MAX_RANGE);

  Serial.begin(115200);

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/saveData", HTTP_POST, [](AsyncWebServerRequest * request) {
    // Handle the POST request
    handlePostRequest(request);
  });


  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest * request) {
    // Handle the POST request
    handleGetData(request);
  });
  //server.on("/getData", HTTP_GET, handleGetData);

  server.begin();

  attachInterrupt(digitalPinToInterrupt(switchPin), toggleTurnedOn, CHANGE); //TODO: adjust FALLING
  attachInterrupt(digitalPinToInterrupt(modePin), toggleMode, CHANGE);
  //attachInterrupt(analogPinToInterrupt(lightRegPin), adjustBrightness, CHANGE);
  //attachInterrupt(analogPinToInterrupt(rateRegPin), adjustRate, CHANGE);

}



void loop() {
    checkForAlarm();

    if(checkChanges()) {
        applyLight();
    }
    if(alarm != NULL) {
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
    if(alarm != NULL) { //only interrupting alarm
        if(turnedOn != stateSave) {
            alarm = NULL;
            turnedOn = stateSave;
        }
        if(autoMode != modeSave) {
            alarm = NULL;
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
        alarm = NULL;
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
    if(autoMode && alarm != NULL) {
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

void checkForAlarm() {

}

void proceedAlarm() {
    startBrightness =
    if(alarm.getType() == RISE_TO_MAX) {
        brightness = bri
    } else if(alarm.getType() == RISE_TO_LEVEL) {

    } else if(alarm.getType() == BLINK) {
        int toggleValue = (brightness != 0 ? 1 : 0);
        if((millis() - alarm.getNextInitiation()) / BLINK_RATE) % 2 == toggleValue) {
            brightness = (toggleValue == 0 ? brightnessSave : 0);
        }
    }
}


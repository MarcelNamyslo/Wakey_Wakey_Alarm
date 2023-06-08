#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define MAX_RANGE = 100;

const char* ssid = "Point";
const char* password =  "Point2424";

//const char* ssid = "Hedon Misafir";
//const char* password =  "hedon2020";

//const char* ssid = "DaktiloCafe";
//const char* password = "buldozer2017";

const int switchPin = 4;
//const int modePin = 5;
const int lightRegPin = 6;
const int rateRegPin = 7;
const int warmPin = 8;
const int coldPin = 9;

const int
bool turnedOn = false;
bool stateSave = turnedOn;
//bool autoMode = false;
//bool modeSave = autoMode;
int brightness = 0; //in percent
int brightnessSave = brightness;
int coldRate = MAX_RANGE / 2; //the share of the cold white in the cold-warm mix
int rateSave = coldRate

Alarm alarm = NULL;

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
    if(checkChanges()) {
        applyLight();
    }

    delay(1);
}

void toggleTurnedOn() {
    turnedOn = !stateSave//turnedOn; //maybe use stateSave
    Serial.print("State toggled on: ");Serial.println(millis());
}

void toggleMode() {
    autoMode = !modeSave//autoMode; //maybe use modeSave
    Serial.print("Mode toggled on: ");Serial.println(millis());
}
/***
void adjustBrightness() {
    if(alarm == NULL) {
        analogRead(lightRegPin);
    }
}

void adjustRate() {
    if(alarm == NULL) {
        analogRead(lightRegPin);
    }
}
***/
bool checkChanges() {
    bool change = false;
    if(alarm != NULL) {

        if(autoMode != modeSave) {modeSave = autoMode;}
        if(turnedOn != stateSave) {

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
            //TODO calculate brightness from LDR
        }
        if(readLightSettings()) {
            brightness = brightnessSave;
            coldRate = rateSave;
            change = true;
        }
    }

    return change;
}

bool readLightSettings() {
    bool change = false;
    temp = analogRead(lightRegPin);
    if(temp != brightnessSave) {
        change = true;
        brightnessSave = temp;
    }
    temp = analogRead(rateRegPin);
    if(temp != rateSave) {
        change = true;
        rateSave = temp;
    return change;
}

void applyLight() {
    if(autoMode) {
        autoAdjustLight();
    } else {
        analogWrite(coldPin, coldRate * (brightness/MAX_RANGE));
        analogWrite(warmPin, (MAX_RANGE - coldRate) * (brightness/MAX_RANGE));
    }
}

void autoAdjustLight() {

}

}

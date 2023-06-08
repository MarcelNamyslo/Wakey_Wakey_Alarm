#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define MAX_RANGE = 100;

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

const int switchPin = 4;
//const int modePin = 5;
const int lightRegPin = 6;
const int rateRegPin = 7;
const int warmPin = 8;
const int coldPin = 9;

bool turnedOn = false;
bool stateSave = turnedOn;
//bool autoMode = false;
//bool modeSave = autoMode;
int brightness = 0; //in percent
int brightnessSave = brightness;
int coldRate = MAX_RANGE / 2; //the share of the cold white in the cold-warm mix
int rateSave = coldRate

Alarm alarm = NULL;


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

void loop()
{

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

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//wifipassword
const char *ssid = "H";
const char *password = "hakansson";

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

void setup()
{
  Serial.begin(115200);
  timeClient.begin();
  if (connection_mode == "0") {


    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
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

  //Serial.print(timeClient.getDay());s
  //Serial.print(", ");
  //Serial.print(timeClient.getHours());
  //Serial.print(":");
  //Serial.print(timeClient.getMinutes());
  //Serial.print(":");
  //Serial.println(timeClient.getSeconds());
  //Serial.println(timeClient.getFormattedTime());

  delay(1000);
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

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>


//const char* ssid = "Hedon Misafir";
const char* ssid = "Point";
const char* password =  "Point2424";
//const char* password =  "hedon2020";

AsyncWebServer server(80);
String textToSend;
//not used
void handleSaveData(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String data;
    if (request->hasParam("data")) {
      data = request->getParam("data")->value();
      File file = SPIFFS.open("/data.txt", "a");
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

      File file = SPIFFS.open("/data.txt", "a");
      if (!file) {
        //request->send(500, "text/plain", "Error opening file for writing.");
        return;
      }
      //file.close();



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

void handleText(AsyncWebServerRequest *request) {
  if (request->hasParam("text", true)) {
    textToSend = request->getParam("text", true)->value();
    request->send(200, "text/plain", textToSend);
  }
}




void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/saveData", HTTP_POST, [](AsyncWebServerRequest * request) {
    // Handle the POST request
    handlePostRequest(request);
  });

  server.on("/sendtext", HTTP_POST, [](AsyncWebServerRequest * request) {
    handleText(request);
  });



  server.begin();
}



void loop() {

}

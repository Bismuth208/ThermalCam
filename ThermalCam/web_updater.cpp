// ----------------------------------------------------------------------
#include <WiFi.h>

#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include "common.h"

AsyncWebServer server(80);

const char host[] = "ThermalCam";
constexpr char ssid[] = "ThermalCamOTA";
constexpr char password[] = "bismuth208083";


const char srvUpdateIndex[] = "\n\
<div class=\"box card status-card\"> \n\
  <p class=\"title\">Select firmware</p> \n\
  <form method='POST' action='/update' enctype='multipart/form-data'> \n\
    <ul class=\"widget-ul\"> \n\
      <input class=\"myButton\" type='file' name='update'>\n\
      <input class=\"myButton\" type='submit' value='Update'> \n\
    </ul> \n\
  </form> \n\
</div> \n\
";


// ----------------------------------------------------------------------
void handleUpdateDone(AsyncWebServerRequest *request);
void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void handleReboot(AsyncWebServerRequest *request);

void getFooter(AsyncResponseStream *response);
void getUpdatePage(AsyncWebServerRequest *request);


// ----------------------------------------------------------------------
void startMDNS(void)
{
  MDNS.end();
  
  if (MDNS.begin(host)) {
    Serial.println(("MDNS responder started"));
  }

  MDNS.addService("http", "tcp", 80);
}

void start_wi_fi(void)
{
#if 1
  
  WiFi.softAP(ssid, password);

#else
  WiFi.begin(ssid, password);
  
  while (!WiFi.isConnected()) {
    Task<0>::delay(500);
    Serial.print(("."));
  }
#endif
  
  Serial.println((""));
  Serial.println(("WiFi connected"));
  Serial.println(("IP address: "));
  Serial.println(WiFi.localIP());

  startMDNS();
}


void initWebServer(void)
{
  //server.on(("/"), handleRoot );

  server.on(("/update"), HTTP_POST, handleUpdateDone, handleUpdate);
  server.on(("/update"), HTTP_GET, [](AsyncWebServerRequest *request) {
      getUpdatePage(request);
  });

  server.on("/reboot", handleReboot);

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
  
  server.begin();
  Serial.printf("Ready! Open http://%s.local in your browser\n", host);
}

// ----------------------------------------------------------------------
void handleUpdateDone(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  response->addHeader("Connection", "close");
  request->send(response);
  ESP.restart();
}

void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index) {
    Serial.printf("Update Start: %s\n", filename.c_str());
    if(!Update.begin()){
      Update.printError(Serial);
    }
  }
  
  if (!Update.hasError()){
    if(Update.write(data, len) != len){
      Update.printError(Serial);
    }
  }
  
  if (final){
    if(Update.end(true)){
      Serial.printf("Update Success: %uB\n", index+len);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleReboot(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
  response->addHeader("Connection", "close");
  request->send(response);
  ESP.restart();
}

// ------------------------------------------------- //
void getFooter(AsyncResponseStream *response)
{
  response->print("<!DOCTYPE html>\n");
  response->print("<html>\n");
  response->print("<!-- Author: Antonov Alexandr (@Bismuth208) -->\n");
  response->print("<!-- E-mail: bismuth20883@gmail.com -->\n");
  response->print("<!-- Build: 14 october 2020 -->\n");
  response->print("<head>\n");

  //<META http-equiv=Pragma content=no-cache>
  //<META http-equiv=Content-Type content="text/html; charset=utf-8">
  
  response->print("<title>ESP32 Web Updater</title>\n");
  response->print("\n</head>\n");
  response->print("<body>\n<h1 align=\"center\">ESP32 ThermalCam OTA</h1>\n");
}

void getUpdatePage(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Connection", "close");
  getFooter(response);
  
  response->print(srvUpdateIndex);
  response->print("</body>\n</html>\n");
  request->send(response);
}

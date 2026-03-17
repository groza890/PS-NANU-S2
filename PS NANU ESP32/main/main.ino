#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>

#include "web_root.h"
#include "storage.h"
#include "mailer.h"
// #include "azure.h"  // dacă vrei

// #include "azure.h"  // dacă adaugi Azure IoT Hub

// ======== PINOUT ========
const int PIN_LM35   = 32;
const int PIN_LED    = 33;
const int PIN_FLOOD  = 25;

int  buttonState     = HIGH;      // starea debounced curentă
int  lastReading     = HIGH;      // ultimul reading brut
unsigned long lastDebounce = 0;   // momentul ultimei schimbări detectate
const unsigned long debounceDelay = 50; // ms pentru stabilizare

// ======== Wi-Fi ========
const char* SSID = "..........";
const char* PASS = "..........";

// ======== Server ========
AsyncWebServer server(80);

// ======== Stocare ========
Storage msgs("msgs", 10);     // buffer circular de 10 mesaje
Storage floods("floods", 10); // buffer circular de 10 alerte

  String getDateTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return String("0000-00-00 00:00:00");
  char buf[32];
  // Format: YYYY-MM-DD HH:MM:SS
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

 void setup() {
  Serial.begin(115200);
   analogSetPinAttenuation(PIN_LM35, ADC_11db);
   pinMode(PIN_LED, OUTPUT);
   pinMode(PIN_FLOOD, INPUT_PULLUP);

   // Conectare Wi-Fi
   WiFi.begin(SSID, PASS);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500); Serial.print(".");
   }
   Serial.println("\nWiFi conectat: " + WiFi.localIP().toString());

   // Iniţializări storage & mail
   msgs.begin();
   floods.begin();
   Mailer::begin();  // configurează SMTP

  // Setăm fusul orar la Europa/București (GMT+2, cu DST)
  configTzTime("EET-2EEST,M3.5.0/03:00:00,M10.5.0/04:00:00", 
               "pool.ntp.org", "time.nist.gov");

// Așteptăm puțin să se sincronizeze
  Serial.print("Aștept sincronizare NTP ");
  for (int i = 0; i < 10; ++i) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();


  // Servim pagina principală
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send(200, "text/html", htmlPage);
  });

  // Endpoint: returnează temperatura ca JSON
  server.on("/get_temperature", HTTP_GET, [](AsyncWebServerRequest *req){
    int raw = analogRead(PIN_LM35);
    float volt = raw * (3.3f / 4095.0f);
    float temp = volt * 100.0f;
    Serial.printf("DBG Temp → raw=%4d  temp=%.2f°C\n", raw, temp);
    String json = String("{\"temperature\":") + temp + "}";
    req->send(200, "application/json", json);
  });

 // include <ArduinoJson.h> și <ESPAsyncWebServer.h>
server.on("/led_control", HTTP_POST, [](AsyncWebServerRequest *req){
  if (!req->hasParam("state", true)) {
    return req->send(400, "text/plain", "Missing state");
  }
  String state = req->getParam("state", true)->value();
  bool on = (state == "ON");
  digitalWrite(PIN_LED, on ? HIGH : LOW);
  Serial.printf("→ LED %s via POST\n", state.c_str());  // <<< aici
  msgs.add(state);
  req->send(204);
});


  server.on("/messages", HTTP_GET, [](AsyncWebServerRequest *req){
    auto arr = msgs.getAll();
    String s = "[";
    for (int i=0; i<arr.size(); i++) {
      s += "\"" + arr[i] + "\"";
      if (i < arr.size()-1) s += ",";
    }
    s += "]";
    req->send(200, "application/json", s);
  });

  // Flood detection: nu e nevoie endpoint GET dacă vrem doar înregistrare locală
  server.on("/floods", HTTP_GET, [](AsyncWebServerRequest *req){
    auto arr = floods.getAll();
    String s = "[";
    for (int i=0; i<arr.size(); i++) {
      s += "\"" + arr[i] + "\"";
      if (i < arr.size()-1) s += ",";
    }
    s += "]";
    req->send(200, "application/json", s);
  });
  server.on("/floods/delete", HTTP_DELETE, [](AsyncWebServerRequest *req){
    int idx = req->getParam("idx")->value().toInt();
    floods.removeAt(idx);
    req->send(204);
  });

  server.begin();
}

unsigned long lastFloodCheck = 0;
bool floodPrev = HIGH;

void loop() {
  int reading = digitalRead(PIN_FLOOD);
  if (reading != lastReading) {
    lastDebounce = millis();
  }

  // dacă lectura s-a menținut mai mult decât debounceDelay…
  if (millis() - lastDebounce > debounceDelay) {
    // și s-a schimbat efectiv starea debounced
    if (reading != buttonState) {
      buttonState = reading;
      // detectăm frontul LOW (apăsat)
      if(buttonState == LOW){
  String ts = getDateTime();
  String ev = ts + ": Alerta inundatie cu note de 10!";
  floods.add(ev);
  Mailer::sendFloodAlert(ev);
  Serial.println("📧 Flood alert: " + ev);
}
    }
  }

  lastReading = reading;
}

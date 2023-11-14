#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif 

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProDimSwitch.h"
#include "LDR.h"

#define WIFI_SSID         "ENG_SOFT"
#define WIFI_PASS         "flamengo"
#define APP_KEY           "b651ccc0-f55e-4fed-b41c-011c86f81368"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "b560f24d-55c2-4f07-9797-c40ec89e66ef-06420c3b-015a-4973-b464-0434dd0dd38d"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define PUSHBUTTON_ID     "654d761b98e14615dbcc82c3"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define LDRSENSOR_ID      "654d65336405e6bd1caf6c39"
#define BAUD_RATE         115200        

#define MOTOR             27
#define BUTTON            34
#define LDR_PIN           32
#define BUZZER            12
#define LED               13
#define TRIGGER_TIME      120000

LDR &ldr = SinricPro[LDRSENSOR_ID];

std::map<String, int> globalRangeValues;
unsigned long initialTime;
bool running = false;

// RangeController
bool onRangeValue(const String &deviceId, const String& instance, int &rangeValue) {
  Serial.printf("[Device: %s]: Value for \"%s\" changed to %d\r\n", deviceId.c_str(), instance.c_str(), rangeValue);
  globalRangeValues[instance] = rangeValue;
  return true;
}

// RangeController
void updateRangeValue(String instance) {
  float leitura = analogRead(LDR_PIN);
  float mapped = (float)map(leitura,120,2800,100,0);
  Serial.println(leitura);
  Serial.println(mapped);
  ldr.sendRangeValueEvent(instance, mapped);
}


bool onPowerState(const String &deviceId, bool &state) {
  startRoutine();
  return true; // request handled properly
}

void checkButtonPressed() {
  bool check = digitalRead(BUTTON) == HIGH;
  Serial.println(check);
  if(check)
  {
    digitalWrite(MOTOR, LOW);
    digitalWrite(LED, LOW);
    digitalWrite(BUZZER, LOW);
    running = false;
  }
}

void checkRoutineEnded()
{
  unsigned long actualMillis = millis();
  Serial.print("millis: ");
  Serial.println(actualMillis);
  Serial.print("initialTime: ");
  Serial.println(initialTime);
  long diff = actualMillis - initialTime;
  Serial.print("diff: ");
  Serial.println(diff);
  if(diff < TRIGGER_TIME)
    return;
  digitalWrite(MOTOR, LOW);
  digitalWrite(LED, LOW);
  digitalWrite(BUZZER, LOW);
  running = false;
}

void startRoutine() {
  Serial.println("Teste");
  updateRangeValue("rangeInstance1"); 
  digitalWrite(MOTOR, HIGH);
  digitalWrite(LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  running = true;
  initialTime = millis();
}
void setupSinricPro() {
  SinricProDimSwitch &myDimSwitch = SinricPro[PUSHBUTTON_ID];
  // add doorbell device to SinricPro
  myDimSwitch.onPowerState(onPowerState);
  // RangeController
  ldr.onRangeValue("rangeInstance1", onRangeValue);
  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");

  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    WiFi.setAutoReconnect(true);
  #elif defined(ESP32)
    WiFi.setSleep(false); 
    WiFi.setAutoReconnect(true);
  #endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");

  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON, INPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  setupWiFi();
  setupSinricPro();
}

void loop() {
  if(running)
  {
    checkButtonPressed();
    checkRoutineEnded();
  }
  SinricPro.handle();
}

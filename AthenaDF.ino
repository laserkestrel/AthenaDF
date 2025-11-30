#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"
#include "DFRobotDFPlayerMini.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// -----------------------------
// Hardware pins
// -----------------------------
#define DFPLAYER_RX 26  // ESP32 RX (connects to DFPlayer TX)
#define DFPLAYER_TX 27  // ESP32 TX (connects to DFPlayer RX)

// I2C for RTC
#define SDA_PIN 21
#define SCL_PIN 22

// -----------------------------
// DFPlayer / RTC / WebServer
// -----------------------------
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfplayer;
RTC_DS3231 rtc;
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

bool setupDone = false;
bool rtcAvailable = false;
bool spokenThisHour = false;

// -----------------------------
// Test mode
// -----------------------------
bool testMode = true;  // Set true to announce every minute for testing

// -----------------------------
// Mapping table (dual-folder)
// -----------------------------
struct Phrase {
  uint8_t folder;
  uint8_t track;
};

// Folder 01: numbers 1–12
// Folder 02: phrases (Good Morning / It’s / O’Clock)
Phrase hourPhrases[] = {
  {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6},
  {1, 7}, {1, 8}, {1, 9}, {1,10}, {1,11}, {1,12}
};

Phrase greetingPhrases[] = {
  {2, 1}, // Good Morning
  {2, 2}, // It's
  {2, 3}  // O'Clock
};

// -----------------------------
// HTML for captive portal
// -----------------------------
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Athena Clock Setup</title>
<style>
body { font-family: Arial; display:flex; justify-content:center; align-items:center; height:100vh; background:#f0f0f0; }
#container { max-width:300px; padding:20px; background:white; border-radius:12px; box-shadow:0 0 10px rgba(0,0,0,0.1); text-align:center; }
</style>
</head>
<body>
<div id="container">
<h2>Setting Clock…</h2>
<p>Please wait</p>
</div>
<script>
window.onload = () => {
  let ts = Math.floor(Date.now()/1000);
  fetch("/set?ts=" + ts)
    .then(() => { document.body.innerHTML = "<h1>✔ Time Saved to RTC</h1><p>You may close this page.</p>"; });
};
</script>
</body>
</html>
)rawliteral";

// -----------------------------
// Captive portal
// -----------------------------
void startCaptivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ATHENA-SETUP", "");
  delay(300);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.onNotFound([]() {
    server.send(200, "text/html", HTML_PAGE);
  });

  server.on("/set", []() {
    if (!server.hasArg("ts")) return;
    time_t ts = server.arg("ts").toInt();
    Serial.println("\n=== Time Received From Phone ===");
    Serial.printf("Unix Time: %ld\n", ts);
    DateTime dt = DateTime(ts);
    if (rtcAvailable) {
      rtc.adjust(dt);
      Serial.println("RTC Updated.");
    } else {
      Serial.println("RTC NOT FOUND — cannot set hardware clock.");
    }
    setupDone = true;
    server.send(200, "text/plain", "OK");
    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.println("Clock setup complete — Wi-Fi AP disabled.");
  });

  server.begin();
}

// -----------------------------
// Helper: play full sentence
// -----------------------------
void playHourAnnouncement(uint8_t hour) {
  // Simple greeting selection based on hour
  uint8_t greetingIndex = 0; // Always Good Morning for demo

  // Sequence: Greeting -> "It's" -> Hour -> "O'Clock"
  dfplayer.playFolder(greetingPhrases[greetingIndex].folder,
                      greetingPhrases[greetingIndex].track);
  delay(1200);

  dfplayer.playFolder(greetingPhrases[1].folder,
                      greetingPhrases[1].track);
  delay(800);

  // Hour: convert 0–23 -> 1–12
  uint8_t h12 = hour % 12;
  if (h12 == 0) h12 = 12;

  dfplayer.playFolder(hourPhrases[h12 - 1].folder,
                      hourPhrases[h12 - 1].track);
  delay(800);

  dfplayer.playFolder(greetingPhrases[2].folder,
                      greetingPhrases[2].track);
  delay(1200);
}

// -----------------------------
// Setup
// -----------------------------
void setup() {
  Serial.begin(115200);

  // DFPlayer
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  if (!dfplayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not responding!");
    while (true);
  }
  dfplayer.volume(25);
  Serial.println("DFPlayer initialized.");

  // RTC
  Wire.begin(SDA_PIN, SCL_PIN);
  if (rtc.begin()) {
    rtcAvailable = true;
    if (!rtc.lostPower()) {
      Serial.println("RTC already has valid time.");
      setupDone = true;
    } else {
      Serial.println("RTC power lost — needs setup.");
    }
  } else {
    Serial.println("ERROR: RTC not found!");
  }

  // Captive portal if needed
  if (!setupDone) {
    Serial.println("Entering FIRST-TIME SETUP MODE...");
    startCaptivePortal();
  } else {
    Serial.println("Setup done. Ready for hourly announcements.");
  }
}

// -----------------------------
// Loop
// -----------------------------
void loop() {
  // Handle captive portal
  if (!setupDone) {
    dnsServer.processNextRequest();
    server.handleClient();
    return;
  }

  if (!rtcAvailable) {
    Serial.println("No RTC available!");
    delay(2000);
    return;
  }

  DateTime now = rtc.now();

  // Determine if we should announce
  bool shouldAnnounce = false;

  if (testMode) {
    // Announce every minute
    if (!spokenThisHour) shouldAnnounce = true;
  } else {
    // Normal mode: announce only at the top of the hour
    if (now.minute() == 0 && !spokenThisHour) shouldAnnounce = true;
  }

  if (shouldAnnounce) {
    Serial.printf("Playing announcement: %02d:%02d\n", now.hour(), now.minute());
    playHourAnnouncement(now.hour());
    spokenThisHour = true;
  }

  // Reset flag when minute changes
  static uint8_t lastMinute = 255;
  if (now.minute() != lastMinute) {
    spokenThisHour = false;
    lastMinute = now.minute();
  }

  delay(1000);
}

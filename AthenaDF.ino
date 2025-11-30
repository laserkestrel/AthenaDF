#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>

// Use UART1 on ESP32
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfplayer;

// --- Mapping Table ---
struct Phrase {
  uint8_t folder;
  uint8_t track;
};

// Logical words
enum Words {
  ONE = 0,
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  ELEVEN,
  TWELVE,
  GOOD_MORNING,
  ITS,
  OCLOCK
};

// Map logical words → folder/track
const Phrase phraseMap[] = {
  {1, 1},  // ONE
  {1, 2},  // TWO
  {1, 3},  // THREE
  {1, 4},  // FOUR
  {1, 5},  // FIVE
  {1, 6},  // SIX
  {1, 7},  // SEVEN
  {1, 8},  // EIGHT
  {1, 9},  // NINE
  {1, 10}, // TEN
  {1, 11}, // ELEVEN
  {1, 12}, // TWELVE
  {2, 1},  // GOOD_MORNING
  {2, 2},  // ITS
  {2, 3}   // OCLOCK
};

// Approximate track durations (ms)
const uint16_t trackLengthMs[] = {
  800,800,800,800,800,800,800,800,800,800,800,800,  // ONE → TWELVE
  1200, 800, 800 // GOOD_MORNING, ITS, OCLOCK
};

void setup() {
  Serial.begin(115200);
  dfSerial.begin(9600, SERIAL_8N1, 26, 27);

  Serial.println("Initializing DFPlayer...");
  if (!dfplayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not responding!");
    while(true);
  }
  dfplayer.volume(25);
  Serial.println("DFPlayer Initialised!");

  // Simulated current time (1–12 hours)
  uint8_t hour = 3; // Example: 3 o'clock

  // Play a greeting sequence based on time
  Words sequence[] = {GOOD_MORNING, ITS, static_cast<Words>(hour - 1), OCLOCK};
  playSequence(sequence, sizeof(sequence)/sizeof(sequence[0]));
  Serial.println("Done playing time announcement!");
}

void loop() {
  // Nothing needed here; later you can trigger this hourly
}

// Function to play a sequence of word IDs
void playSequence(Words seq[], size_t len) {
  for (size_t i = 0; i < len; i++) {
    Phrase p = phraseMap[seq[i]];
    Serial.print("Playing folder ");
    Serial.print(p.folder);
    Serial.print(", track ");
    Serial.println(p.track);

    dfplayer.playFolder(p.folder, p.track);

    // Wait for track to finish (approximate)
    delay(trackLengthMs[seq[i]]);
  }
}

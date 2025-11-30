#include "DFRobotDFPlayerMini.h"

// UART1 on ESP32
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfplayer;

// --- MAPPING TABLE ---
// Each word maps to (folder, track) pair
struct Phrase {
  uint8_t folder;
  uint8_t track;
};

// Logical word IDs
enum Words {
  GOOD_MORNING = 0,
  ITS,
  OCLOCK,
  ONE,
  TWO
};

// Map logical words → folder/track on SD
const Phrase phraseMap[] = {
  {1, 2},  // GOOD_MORNING → /01/002.mp3
  {1, 3},  // ITS          → /01/003.mp3
  {1, 4},  // OCLOCK       → /01/004.mp3
  {1, 5},  // ONE          → /01/005.mp3
  {1, 6}   // TWO          → /01/006.mp3
};

// Approximate durations in milliseconds for each word
const uint16_t trackLengthMs[] = {1000, 800, 800, 800, 800};

void setup() {
  Serial.begin(115200);

  // RX = 26, TX = 27
  dfSerial.begin(9600, SERIAL_8N1, 26, 27);

  Serial.println("Initializing DFPlayer...");
  if (!dfplayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not responding!");
    while(true);
  }

  dfplayer.volume(25); // 0–30
  delay(200);
  Serial.println("DFPlayer Initialised!");

  // --- Example playback sequence ---
  // "Good Morning, It's One O'Clock"
  Words sequence1[] = {GOOD_MORNING, ITS, ONE, OCLOCK};

  // "Good Morning, It's Two O'Clock"
  Words sequence2[] = {GOOD_MORNING, ITS, TWO, OCLOCK};

  // Play both sequences with a pause in between
  playSequence(sequence1, sizeof(sequence1)/sizeof(sequence1[0]));
  delay(1000);
  playSequence(sequence2, sizeof(sequence2)/sizeof(sequence2[0]));

  Serial.println("Done playing sequences!");
}

void loop() {
  // Nothing needed here for this test
}

// Function to play an array of word IDs
void playSequence(Words seq[], size_t len) {
  for (size_t i = 0; i < len; i++) {
    Phrase p = phraseMap[seq[i]];
    Serial.print("Playing folder ");
    Serial.print(p.folder);
    Serial.print(", track ");
    Serial.println(p.track);

    dfplayer.playFolder(p.folder, p.track);

    // Wait for the track to finish (approximate)
    delay(trackLengthMs[seq[i]]);
  }
}

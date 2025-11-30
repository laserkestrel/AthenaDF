# ESP32 Spoken Clock with DFPlayer Mini

This project implements a **spoken clock** using an ESP32, a DFPlayer Mini, and a small speaker. The clock announces the time dynamically using pre-recorded word samples stored on an SD card.

## Hardware Setup

- **ESP32 Dev Board**
  - UART1 (pins 26/27) connected to DFPlayer Mini TX/RX.
  - Powered via USB (5V) or VIN pin.
- **DFPlayer Mini**
  - Connected to a speaker.
  - Logic level compatible with ESP32 (use a 1.1kΩ voltage divider on TX if needed for 3.3V ESP32 RX).
  - MP3 files stored on an SD card.
- **Speaker**
  - Connected to DFPlayer Mini SPK+ / SPK- pins.

## SD Card Structure

To ensure **deterministic playback**, all audio files are stored in **a single folder** (`01`) on the SD card:

01/002.mp3 → Good Morning
01/003.mp3 → It's
01/004.mp3 → O'clock
01/005.mp3 → One
01/006.mp3 → Two


**Why a folder?**  
- The DFPlayer Mini assigns track numbers to files based on **filesystem order in the root folder**, which is unpredictable.  
- Using a folder guarantees that `playFolder(folder, track)` always plays the correct word.  
- One folder can hold multiple words; separate folders are optional for organization.

## Software / Code Overview

- **Mapping table:** Logical words (e.g., GOOD_MORNING, ONE, TWO) are mapped to `(folder, track)` pairs.
- **Dynamic sequences:** Words are played in arrays, allowing the clock to construct any sentence dynamically.
- **Sequential playback:** The example sketch plays sequences like:
  - "Good Morning, It's One O'Clock"
  - "Good Morning, It's Two O'Clock"

### Example Mapping Table

```cpp
const Phrase phraseMap[] = {
  {1, 2},  // GOOD_MORNING → 01/002.mp3
  {1, 3},  // ITS          → 01/003.mp3
  {1, 4},  // OCLOCK       → 01/004.mp3
  {1, 5},  // ONE          → 01/005.mp3
  {1, 6}   // TWO          → 01/006.mp3
};
```
### Key Design Choices

Folders for deterministic playback: Avoids unpredictable FAT32 ordering in the root folder.

Mapping table: Decouples logical word identity from physical file numbering, enabling dynamic sentence generation.

Dynamic sequences: Makes the spoken clock extensible — can add more words, languages, or phrases easily.

Delay-based playback (initially): Simple to implement; can be replaced by BUSY pin detection for precise timing without guesswork.

### Next Steps / Improvements

Replace delay() with BUSY pin detection to accurately detect when a track finishes.

Implement hourly triggers to announce the time automatically.

Expand SD card with additional phrases or greetings in multiple languages.

Optional: add volume control, start/stop buttons, or network-synced time updates.

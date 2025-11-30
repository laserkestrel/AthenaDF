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

To ensure **deterministic playback**, all audio files are NOT stored in the root, and use folders on the SD card based on purpose.


| Logical Word                           | Folder | Track | File on SD Card  |
|----------------------------------------|--------|-------|------------------|
| ONE                                    | 01     | 001   | 01/001.mp3       |
| TWO                                    | 01     | 002   | 01/002.mp3       |
| THREE                                  | 01     | 003   | 01/003.mp3       |
| FOUR                                   | 01     | 004   | 01/004.mp3       |
| FIVE                                   | 01     | 005   | 01/005.mp3       |
| SIX                                    | 01     | 006   | 01/006.mp3       |
| SEVEN                                  | 01     | 007   | 01/007.mp3       |
| EIGHT                                  | 01     | 008   | 01/008.mp3       |
| NINE                                   | 01     | 009   | 01/009.mp3       |
| TEN                                    | 01     | 010   | 01/010.mp3       |
| ELEVEN                                 | 01     | 011   | 01/011.mp3       |
| TWELVE                                 | 01     | 012   | 01/012.mp3       |

### **Folder 02 – Phrases**
| Logical Word                           | Folder | Track | File on SD Card  |
|----------------------------------------|--------|-------|------------------|
| GOOD_MORNING                           | 02     | 001   | 02/001.mp3       |
| ITS                                    | 02     | 002   | 02/002.mp3       |
| OCLOCK                                 | 02     | 003   | 02/003.mp3       |
| GOOD_AFTERNOON                         | 02     | 004   | 02/004.mp3       |
| GOOD_EVENING                           | 02     | 005   | 02/005.mp3       |
| KALESPERA                              | 02     | 007   | 02/007.mp3       |
| SETUP_REQUEST                          | 02     | 008   | 02/008.mp3       |
| EFKARISTO                              | 02     | 009   | 02/009.mp3       |
| PETROS_MAN_OF_STONE                    | 02     | 010   | 02/010.mp3       |
| YAMAS_PETROS_ANOTHER_GLASS             | 02     | 011   | 02/011.mp3       |
| BEWARE_MORTAL                          | 02     | 012   | 02/012.mp3       |
| KALEMERA                               | 02     | 013   | 02/013.mp3       |
| YASOU                                  | 02     | 014   | 02/014.mp3       |
| OPA                                    | 02     | 015   | 02/015.mp3       |




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

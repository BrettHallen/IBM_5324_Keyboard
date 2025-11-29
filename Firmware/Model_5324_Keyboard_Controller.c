// IBM System/23 Model 5324 Keyboard Controller
// For Teensy 4.1
// https://github.com/BrettHallen/IBM_5324_Keyboard
// Scan 11 columns, read 8 rows, output 7-bit scan code
// Brett Hallen, Nov 2025
// Port Macquarie, Australia

// WORK IN PROGRESS - WORK IN PROGRESS - WORK IN PROGRESS - WORK IN PROGRESS - WORK IN PROGRESS
/*
   IBM System/23 Model 5324 keyboard scan codes
   +----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+---------+---------+
   | 7C | 6F |    | 3E | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 3A | 3B | 3C | 3D |    4B   |   4C    |
   |    | 7F |    |    |    |    |    |    |    |    |    |    |    |    |    |    |1   |         |         |
   +----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
   | 6C | 6D |    | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2A | 2B | 2C | 2D | 47 | 48 | 49 | 4E |
   +----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+    +----+----+----+----+
   | 6E | 7D |    | 54 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B | 59 |    | 44 | 45 | 46 | 4D |
   |    |    |    | 74 |    |    |    |    |    |    |    |    |    |    |    | 79 |    |    |    |    |    |
   +----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+    |
   | 71 | 70 |    | OE | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A |   0B    | 56 | 41 | 42 | 43 |    |
   |    |    |    |    |    |    |    |    |    |    |    |    |    |    |   7B    | 76 |    |    |    |    |
   +----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+    |
   | 72 | 73 |    | 7E |                              0F                      | 68 |    40   |   4A    |    |
   |    |    |    | 5E |                                                      | 78 |         |         |    |
   |----+----+    +----+----+----+----+----+----+----+----+----+----+----+----+----+---------+---------+----+
   
   Typamatic scan codes:
   0F
   3D
   70
   71
   72
   73
*/

const int NUM_COLS = 11;
const int NUM_ROWS = 8;
const int SCAN_CODE_BITS = 7;
const int colPins[NUM_COLS] = {24,23,22,21,20,25,26,27,28,29,30}; // Columns A-K
const int rowPins[NUM_ROWS] = {36,37,38,39,40,41,42,43};           // Rows 1-8
const int bitPins[7] = {4,5,6,7,8,9,10};                           // Scan code bits 0-6 (LSB first)
const int dataStrobePin = 11;                                      // Pulse low on new code
const int delayStrobePin = 12;                                     // System ACK (input, optional)
const int resetPin = 13;                                           // System reset (input)

// Scan code lookup table
// 0x00 marks unused code
byte scanCode[NUM_ROWS][NUM_COLS] = 
{
  // Column  A     B     C     D     E     F     G     H     I     J     K
  // Row 1: CMD     ~`   @2    $4    ^6    *8    )0    +=          *+
           {0x6F, 0x3E, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x00, 0x4B, 0x00},
  // Row 2: ERASE  |1    #3    %5    &7          (9    _-   <--    8     /- 
           {0x7C, 0x31, 0x33, 0x35, 0x37, 0x00, 0x39, 0x3B, 0x3D, 0x48, 0x4C},
  // Row 3: HOLD  TAB    W     R     Y           I     P     |\    7     9
           {0x6D, 0x20, 0x22, 0x24, 0x26, 0x00, 0x28, 0x2A, 0x2C, 0x47, 0x49},
  // Row 4: INQ,  CAPS   Q     E     T     U     O     !¢  ENTER   4    Field -
           {0x6C, 0x54, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D, 0x44, 0x4E}, 
  // Row 5: ERROR Shift  A     D     G     J     L     "'    }{    5     6
           {0x6E, 0x57, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x59, 0x45, 0x46},  
  // Row 6: TEST  ScrDn  S     F     H           K     :;    NL    1     3
           {0x7D, 0x70, 0x12, 0x14, 0x16, 0x00, 0x18, 0x1A, 0x56, 0x41, 0x43},
  // Row 7: ScrUp  -->   ><    C     B     N     M     .    Shift  2    Field +
           {0x71, 0x73, 0x0E, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0B, 0x42, 0x4D},
  // Row 8: <--   ALT    Z     X     V    SPACE  ,     ?    EXIT   0     .
           {0x72, 0x7E, 0x01, 0x02, 0x04, 0x0F, 0x08, 0x0A, 0x68, 0x40, 0x4A} 
};

// Key state tracking
bool prevState[NUM_ROWS][NUM_COLS];
unsigned long lastTypamatic[NUM_ROWS][NUM_COLS];

// Typamatic lookup table (marked **, repeat while held)
// Typamatic key is non-zero
byte isTypamatic[NUM_ROWS][NUM_COLS] = 
{
  // Column A     B     C     D     E     F     G     H     I     J     K
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 1
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 }, // Row 2
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 3
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 4
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 5
          {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 6
          {0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 7
          {0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }  // Row 8
};

// Scan code lookup table for make/break keys (marked *, send on release)
byte makeBreakScanCode[NUM_ROWS][NUM_COLS] = 
{
  // Column A     B     C     D     E     F     G     H     I     J     K
          {0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 1
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 2
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 3
          {0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Row 4
          {0x00, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0x00, 0x00 }, // Row 5
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00 }, // Row 6
          {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7B, 0x00, 0x00 }, // Row 7
          {0x00, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00 }  // Row 8
};

void setup() 
{
  Serial.begin(38400);
  // Initialise the columns
  for (int i = 0; i < NUM_COLS; i++) 
  {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH); // active low
  }
  // Initialise the rows
  for (int i = 0; i < NUM_ROWS; i++) 
  {
    pinMode(rowPins[i]); // external pull-ups,  no need for internal
  }
  // Initialise the scan code bus
  for (int i = 0; i < SCAN_CODE_BITS; i++) 
  {
    pinMode(bitPins[i], OUTPUT);
    digitalWrite(bitPins[i], LOW); // 0x00 = no scan code
  }
  pinMode(dataStrobePin, OUTPUT);
  digitalWrite(dataStrobePin, HIGH); // Idle high
  pinMode(delayStrobePin, INPUT_PULLUP); // Optional system ACK
  pinMode(resetPin, INPUT_PULLUP);
  // Init states
  memset(prevState, 0, sizeof(prevState));
  // Pulse reset on startup if needed
  if (digitalRead(resetPin) == LOW)
  	resetKeyboard();
  Serial.println("IBM System/23 Model 5324 Keyboard Emulator Ready");
}

void loop() 
{
  // Check for system reset
  if (digitalRead(resetPin) == LOW) 
  {
    resetKeyboard();
    delay(100);
  }

  bool keyActive = false;
  for (int c = 0; c < NUM_COLS; c++) 
  {
    digitalWrite(colPins[c], LOW);  // Drive column low
    delayMicroseconds(10);  // Settle time

    for (int r = 0; r < NUM_ROWS; r++) 
    {
      bool pressed = (digitalRead(rowPins[r]) == LOW);  // Active low
      if (pressed != prevState[r][c]) 
      {
        if (pressed) 
        {
          handleKeyPress(r, c);
        } 
        else if (isMakeBreak[r][c]) 
        {
          handleKeyRelease(r, c);  // Send on release for * keys
        }
        prevState[r][c] = pressed;
        keyActive = true;
      } 
      else if (pressed && isTypamatic[r][c]) 
      {
        // Typamatic repeat
        unsigned long now = millis();
        if (now - lastTypamatic[r][c] > 100) 
        {  // 100ms repeat rate
          handleKeyPress(r, c);
          lastTypamatic[r][c] = now;
        }
      }
    }
    digitalWrite(colPins[c], HIGH);  // Release column
  }

  if (!keyActive) 
  {
    // Idle: clear bits?
    setScanCode(0x00);
  }
  delay(10);  // Scan rate ~100Hz, debounce
}

void handleKeyPress(int row, int col) 
{
  byte code = scanCodes[row][col];
  if (code != 0x00) 
  {
    sendScanCode(code);
    if (isTypamatic[row][col]) 
    	lastTypamatic[row][col] = millis();
    Serial.print("Key pressed: Row "); Serial.print(row+1); Serial.print(", Column "); Serial.print((char)(col+65));
    Serial.print(" --> 0x"); Serial.println(code, HEX);
  }
}

void handleKeyRelease(int row, int col) 
{
  byte code = scanCodes[row][col];
  if (code != 0x00) 
  {
    sendScanCode(code);  // Or invert: sendScanCode(code | 0x80); if system expects break
    Serial.print("Key released: Row "); Serial.print(row+1); Serial.print(", Column "); Serial.print((char)(col+65));
    Serial.print(" --> 0x"); Serial.println(code, HEX);
  }
}

void sendScanCode(byte code) 
{
  setScanCode(code);
  // Pulse DATA STROBE (system interrupt)
  digitalWrite(dataStrobePin, LOW);
  delayMicroseconds(50);  // Match original timing
  digitalWrite(dataStrobePin, HIGH);
  // Optional: Wait for DELAY STROBE ACK
  // while (digitalRead(delayStrobePin) == HIGH) {}  // Block until low
}

void setScanCode(byte code) 
{
  for (int b = 0; b < 7; b++) 
  {
    digitalWrite(bitPins[b], (code & (1 << b)) ? HIGH : LOW);
  }
}

void resetKeyboard() 
{
  setScanCode(0x00);
  // Clear typamatic timers, etc.
  memset(lastTypamatic, 0, sizeof(lastTypamatic));
  Serial.println("Keyboard Reset");
}

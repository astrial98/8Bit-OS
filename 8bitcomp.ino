#include <EEPROM.h>

// 1,024 Bytes of Raw Volatile SRAM Workspace
byte RAM[1024]; 
byte A = 0;    
byte B = 0;    
bool zeroFlag = false;
int PC = 0;
bool running = false;

int writeAddress = 0;
bool processingWrite = false;
char highNibble = '\0';

// ARCHITECTURE CONTROL SETTINGS
const int BIT_MODE_PIN = 2; // Short to ground for 16-bit mode (1,024B), leave open for 8-bit mode (256B)
bool is16BitMode = false;
int memoryCeiling = 256;

byte hexCharToValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0xFF;
}

void printHex(byte b) {
  if (b < 0x10) Serial.print("0");
  Serial.print(b, HEX);
}

void printHex16(int w) {
  printHex((w >> 8) & 0xFF);
  printHex(w & 0xFF);
}

// NATIVE SYSTEM DICTIONARY MANUAL
void printSystemHelp() {
  Serial.println(F("\n====== HARDWARE MONITOR CORE MANUAL ======"));
  Serial.println(F("W       - Sequential memory loader (starts at 0000)"));
  Serial.println(F("D       - High-speed direct RAM flasher (type 'END' to finish)"));
  Serial.println(F("R       - Boot and execute active guest app code space"));
  Serial.println(F("M       - Output full hexadecimal SRAM matrix map grid"));
  Serial.println(F("C       - Flush all memory variables and wipe SRAM cells"));
  Serial.println(F("H       - Render this system command manual"));
  Serial.println(F("------------------------------------------"));
  Serial.print(F("HARDWARE: Pin 2 Open  -> 8-Bit Mode (256B System Ceiling)\n"));
  Serial.print(F("          Pin 2 Ground-> 16-Bit Mode (1,024B Expanded Storage)\n"));
  Serial.println(F("=========================================="));
  Serial.flush();
}

void executeDirectSramMap() {
  Serial.println(F("\n--- DIRECT SRAM MAP INPUT MODE ---"));
  Serial.flush();

  int loadAddr = 0;
  char high = '\0';
  
  // Clean pipeline buffer before capturing data
  while(Serial.available() > 0) { Serial.read(); } 

  // Fast reader loop: Runs at absolute raw hardware clock speed
  while (loadAddr < memoryCeiling) {
    while (Serial.available() == 0) {}
    char c = Serial.read();

    // The Stop Marker: If you send a dollar sign '$', it exits instantly!
    if (c == '$') {
      Serial.print(F("\n[SUCCESS] Mapped "));
      Serial.print(loadAddr);
      Serial.println(F(" bytes directly into SRAM."));
      break;
    }

    byte numericValue = hexCharToValue(c);
    if (numericValue == 0xFF) continue; // Skip spaces/newlines

    if (high == '\0') {
      high = c;
    } else {
      RAM[loadAddr] = (hexCharToValue(high) << 4) | numericValue;
      loadAddr++;
      high = '\0';
    }
  }
}


void executeProgram() {
  while(Serial.available() > 0) { Serial.read(); } 
  PC = 0; A = 0; B = 0; zeroFlag = false; running = true;
  
  Serial.println(F("\n--- RUNNING ---"));
  Serial.flush();

  while (running && PC < memoryCeiling) {
    byte opcode = RAM[PC]; 

    switch (opcode) {
      case 0x00: running = false; Serial.println(F("\n--- HALT ---")); break;
      case 0x01: A = RAM[PC + 1]; PC += 2; break;
      case 0x02: B = RAM[PC + 1]; PC += 2; break;
      case 0x03: A = A + B; PC += 1; break;
      case 0x04: A = A - B; PC += 1; break;
      case 0x05: Serial.print(A); Serial.flush(); PC += 1; break;
      case 0x06: Serial.print((char)A); Serial.flush(); PC += 1; break;
      case 0x07: zeroFlag = (A == B); PC += 1; break;
      
      case 0x08: { 
        int target = is16BitMode ? ((RAM[PC + 1] << 8) | RAM[PC + 2]) : RAM[PC + 1];
        if (zeroFlag) PC = target; else PC += (is16BitMode ? 3 : 2);
        break;
      }
      case 0x09: { 
        PC = is16BitMode ? ((RAM[PC + 1] << 8) | RAM[PC + 2]) : RAM[PC + 1];
        break;
      }
      case 0x0A: zeroFlag = (A == RAM[PC + 1]); PC += 2; break;
      
      case 0x0B: { 
        int targetAddr = is16BitMode ? ((RAM[PC + 1] << 8) | RAM[PC + 2]) : RAM[PC + 1];
        PC += (is16BitMode ? 3 : 2);
        while (RAM[targetAddr] != 0x00 && targetAddr < memoryCeiling) {
          Serial.print((char)RAM[targetAddr]);
          targetAddr++;
        }
        Serial.flush(); 
        break;
      }
      case 0x0C: while (Serial.available() == 0) {} A = Serial.read(); PC += 1; break;
      case 0x0D: { 
        int targetAddr = is16BitMode ? ((RAM[PC + 1] << 8) | RAM[PC + 2]) : RAM[PC + 1];
        RAM[targetAddr] = A; PC += (is16BitMode ? 3 : 2);
        break; 
      }
      default: running = false; break;
    }
  }
}

void printMemoryDump() {
  Serial.println(F("\n--- SRAM MAP ---"));
  for (int i = 0; i < memoryCeiling; i++) {
    if (i % 16 == 0) {
      if (is16BitMode) printHex16(i); else printHex(i);
      Serial.print(F(": "));
    }
    printHex(RAM[i]); Serial.print(F(" "));
    if ((i + 1) % 16 == 0) Serial.println();
  }
  Serial.println(F("----------------"));
  Serial.flush();
}

void setup() {
  pinMode(BIT_MODE_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  delay(1000); 
  memset(RAM, 0, sizeof(RAM));
  
  if (digitalRead(BIT_MODE_PIN) == LOW) {
    is16BitMode = true; memoryCeiling = 1024;
    Serial.println(F("\n*** SYSTEM INITIALIZED: 16-BIT EXPANDED MODE (1,024B) ***"));
  } else {
    is16BitMode = false; memoryCeiling = 256;
    Serial.println(F("\n*** SYSTEM INITIALIZED: CLASSIC 8-BIT MODE (256B) ***"));
  }
  
  Serial.println(F("COMMANDS: W (WRITE), D (DIRECT MAP), R (RUN), M (MAP), C (CLEAR), H (HELP)"));
  Serial.print(F("\n> ")); Serial.flush();
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (processingWrite) { processingWrite = false; Serial.println(F("\nLOADED.")); }
      highNibble = '\0'; Serial.print(F("\n> ")); Serial.flush(); return;
    }
    if (!processingWrite) {
      if (c == ' ' || c == '\t') continue;
      if (c == 'r' || c == 'R') { executeProgram(); return; } 
      if (c == 'm' || c == 'M') { printMemoryDump(); return; }
      if (c == 'c' || c == 'C') { memset(RAM, 0, sizeof(RAM)); writeAddress = 0; Serial.println(F("\nSRAM FLUSHED.")); return; }
      if (c == 'd' || c == 'D') { executeDirectSramMap(); return; }
      
      // THE NATIVE HELP TRIGGER COMMAND
      if (c == 'h' || c == 'H') { printSystemHelp(); return; }
      
      if (c == 'w' || c == 'W') { processingWrite = true; writeAddress = 0; continue; }
      return;
    }
    if (processingWrite) {
      byte hexVal = hexCharToValue(c); if (hexVal == 0xFF) continue;
      if (highNibble == '\0') { highNibble = c; } 
      else {
        if (writeAddress < memoryCeiling) {
          RAM[writeAddress] = (hexCharToValue(highNibble) << 4) | hexVal; writeAddress++;
        }
        highNibble = '\0';
      }
    }
  }
}

/* 
 *              ___   ___
 *            /_  \_/  _ \
 *           /  _\/ \_ /_ \
 *          /  /  \_/  \   \
 *         |  / \_/ \_ / \  |
 *         | | / \_/ \_ / | |
 *          \ \_ / \_ / _/ /
 *           \_ \_/_ \_/_ /
 *             \_\  /_/_/
 *                ||
 *                ||
 *         _______||_______
 *        /                \
 *Made by astrial on "7/3/2026" 11:01 EST
*/
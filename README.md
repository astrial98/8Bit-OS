# 🐾 Astrial 8-Bit/16-Bit Framework v6.0 🐾



## 🛠️ Hardware Requirements
* **Microcontroller:** Arduino Uno (ATmega328P)
* **The Architecture Toggle Jumper Wire:** 
  * Leave **Pin 2 Open** -> Classic 8-Bit Mode (256-Byte SRAM, 1-byte addresses)
  * Short **Pin 2 to GND** -> Advanced 16-Bit Mode (1,024-Byte SRAM, 2-byte long addresses)

## 🖥️ Active Operational Core Commands
* `W` - Sequential memory array loader (always starts writing at 0000)
* `D` - High-speed Direct RAM mapping (DMA stream mode - end your packet string with `$`)
* `R` - Boot and execute active guest application code space
* `M` - Output full hexadecimal SRAM matrix map grid layout
* `C` - Flush all memory variables and sanitize SRAM cells
* `H` - Render the system command dictionary manual

## 🔑 The 13 Foundational Machine Opcodes
* `00` - **HALT** (Cleanly terminates program execution loops)
* `01 [val]` - **LDA** (Load immediate value into Register A)
* `02 [val]` - **LDB** (Load immediate value into Register B)
* `03` - **ADD** (A = A + B)
* `04` - **SUB** (A = A - B)
* `05` - **PRNTA** (Prints current Register A value as an integer)
* `06` - **PRNTC** (Prints current Register A value as an ASCII text character)
* `07` - **CMP** (Sets Zero Flag true if A == B)
* `08 [addr]` - **JEQ** (Conditional branch jump to target address if Zero Flag is true)
* `09 [addr]` - **JMP** (Unconditional branch jump to target address pointer)
* `0A [val]` - **CPI** (Compares Register A directly to an immediate value)
* `0B [addr]` - **PRNTSTR** (Streams null-terminated text array starting from target address)
* `0C` - **KEYIN** (Pauses CPU thread cycle and captures a single raw keyboard keystroke into Register A)
* `0D [addr]` - **STA** (Direct pointer mutation - stores Register A data directly into a target address cell)

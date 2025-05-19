#include "W83C42.h"

W83C42* W83C42::instance = nullptr;


W83C42::W83C42(uint8_t D0, uint8_t D1, uint8_t D2,
	uint8_t D3, uint8_t D4, uint8_t D5,
	uint8_t D6, uint8_t D7, uint8_t CS,
	uint8_t WR, uint8_t RD, uint8_t RST,
	uint8_t A2, uint8_t INT) {
	instance = this;
	this->CS = CS;
	this->WR = WR;
	this->RD = RD;
	this->A2 = A2;
	this->RST = RST;
	this->INT = INT;


	pinMode(CS, OUTPUT);
	pinMode(WR, OUTPUT);
	pinMode(RD, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(RST, OUTPUT);
	pinMode(INT, INPUT);

	this->DATAPINS[0] = D0;
	this->DATAPINS[1] = D1;
	this->DATAPINS[2] = D2;
	this->DATAPINS[3] = D3;
	this->DATAPINS[4] = D4;
	this->DATAPINS[5] = D5;
	this->DATAPINS[6] = D6;
	this->DATAPINS[7] = D7;

	pinInput();

	digitalWrite(CS, 1);
	digitalWrite(WR, 1);
	digitalWrite(RD, 1);
	digitalWrite(A2, 0);

}
int		W83C42::readBuffer(char* outputBuffer, int maxLen) {
	int count = 0;
	int i = start;
	while (i != end && count < maxLen) {
		outputBuffer[count++] = buffer[i];
		i = (i + 1) % 128;
	}
	start = end = 0;

	return count;  // number of bytes read
}
bool	W83C42::begin() {
	// Performs a reset routine, and a self test on the W83C42
	digitalWrite(RST, 0);
	delayMicroseconds(100);
	digitalWrite(RST, 1);
	delayMicroseconds(100);
	uint8_t result = selfTest(false);
	if (result != 0x55) {
		Serial.print("Self test not passed, returned 0x");
		Serial.println(result, HEX);
		return false;
	}
	else {
		Serial.println("Self test passed.");
		attachInterrupt(digitalPinToInterrupt(2), ISR_WRAPPER, RISING);
		return true;
	}
}
void	W83C42::disable() {
	// Disables the keyboard interface
	sendCommand(0);
}
void	W83C42::enable() {
	// Enables the keyboard interface
	sendCommand(0x69);
}
uint8_t	W83C42::selfTest(bool print) {
	/*
	  Description:
		Function that makes the W83C42 perform a self test on itself
		and outputs the not formatted result to the serial monitor.
	  Datasheet reference:
		Page 11.
	*/

	out(0xAA, true);                      // 0xAA - "Self test" command
	uint8_t data = in(false, false);         // Read the data that W83C42 put on the bus
	if (print) {
		Serial.print("Self test result: 0x");
		Serial.println(data, HEX);
	}
	return data;
}
uint8_t W83C42::readInputPort(bool print) {
	/*
	  Description:
		Function that reads the input port of the keyboard controller
		and outputs the result to the serial monitor.
	*/

	out(0xC0, true);				// 0xC0 - "Read Input Port" command
	byte data = in(false, false);	// Read the data that W83C42 put on the bus

	if (print) {
		Serial.println();
		Serial.println();

		Serial.println("Input ports: ");

		Serial.print("RAM: ");
		Serial.println(data & 0b00010000 ? 1 : 0);

		Serial.print("JUMPER: ");
		Serial.println(data & 0b00100000 ? 1 : 0);

		Serial.print("DISPLAY TYPE: ");
		Serial.println(data & 0b01000000 ? 1 : 0);

		Serial.print("K/B INHIBIT: ");
		Serial.println(data & 0b10000000 ? 1 : 0);

		Serial.println();
		Serial.println();
	}
	return data;
}
uint8_t W83C42::readStatus(bool print) {
	/*
	  Description:
		Function that reads the status register of the keyboard controller
		and outputs the result to the serial monitor.
	*/

	// Datasheet: The status register (...) may be read at any time.
	uint8_t data = in(true, false);
	if (print) {
		Serial.println();
		Serial.println();

		Serial.println("Status register: ");

		Serial.print("OBF: ");
		Serial.println(data & 0b00000001 ? "FULL" : "EMPTY");

		Serial.print("IBF: ");
		Serial.println(data & 0b00000010 ? "FULL" : "EMPTY");

		Serial.print("SYS FLAG: ");
		Serial.println(data & 0b00000100 ? 1 : 0);

		Serial.print("LAST MESSAGE: ");
		Serial.println(data & 0b00001000 ? "COMMAND" : "DATA");

		Serial.print("K/B INHIBIT: ");
		Serial.println(data & 0b00010000 ? "NO" : "YES");

		Serial.print("TRANSMIT TIME OUT: ");
		Serial.println(data & 0b00100000 ? "YES" : "NO");

		Serial.print("RECEIVE TIME OUT: ");
		Serial.println(data & 0b01000000 ? "YES" : "NO");

		Serial.print("PARITY ERROR: ");
		Serial.println(data & 0b10000000 ? "YES" : "NO");

		Serial.println();
		Serial.println();
	}
	return data;
}
uint8_t W83C42::readCommand(bool print) {
	/*
	  Description:
		Function that reads the command byte of the keyboard controller
		and outputs the result to the serial monitor.
	*/

	out(0x20, true);                      // 0x20 - "Read Command Byte of Keyboard Controller" command
	byte data = in(false, false);
	if (print) {
		Serial.println();
		Serial.println();

		Serial.println("Command register: ");

		Serial.print("ENABLE OBF INT: ");
		Serial.println(data & 0b00000001 ? "YES" : "NO");

		Serial.print("RESERVED: ");
		Serial.println(data & 0b00000010 ? 1 : 0);

		Serial.print("SYS FLAG: ");
		Serial.println(data & 0b00000100 ? 1 : 0);

		Serial.print("KEYBOARD INHIBIT DISABLED: ");
		Serial.println(data & 0b00001000 ? "YES" : "NO");

		Serial.print("KEYBOARD DISABLED: ");
		Serial.println(data & 0b00010000 ? "YES" : "NO");

		Serial.print("IBM PC COMPATIBLE MODE: ");
		Serial.println(data & 0b00100000 ? "ON" : "OFF");

		Serial.print("IBM PC MODE: ");
		Serial.println(data & 0b01000000 ? "ON" : "OFF");

		Serial.print("RESERVED: ");
		Serial.println(data & 0b10000000 ? 1 : 0);

		Serial.println();
		Serial.println();
	}
	return data;
}
void	W83C42::sendCommand(uint8_t data) {
	/*
	  Description:
		Function that sends data to the Command Byte of the W83C42.
	  Datasheet reference:
		Page 11, command 60.
	*/

	out(0x60, true);                      // 0x60 - "Write Command Byte of Keyboard Controller" command
	out(data, false);
}
void	W83C42::disableKeyboardInterface() {
	/*
	  Description:
		Function that sends the 0xAD command to the W83C42 - "Disable Keyboard Feature".
	*/
	out(0xAD, true);
	delayMicroseconds(10);
}
void	W83C42::enableKeyboardInterface() {
	/*
	  Description:
		Function that sends the 0xAE command to the W83C42 - "Enable Keyboard Interface".
	*/

	out(0xAE, true);
	delayMicroseconds(10);
}
void	W83C42::waitForIBF() {
	/*
	  Description:
		Function that waits until the IBF bit is 0 - meaning the W83C42 is ready to receive data.

	  Datasheet reference:
		[Data is to be sent to the chip] "only if the input buffer full bit in the status
		register is set to 0."
	*/
	while ((in(true, false) & 0b00000010)) {
		delayMicroseconds(1);
	}
}
void	W83C42::waitForOBF() {
	/*
	  Description:
		Function that waits until the OBF bit is 1 - meaning valid data is put on the data bus by the W83C42.

	  Datasheet reference:
		"The output buffer should be read only when the
		output buffer full bit in the register is 1."
	*/

	while (!(in(true, false) & 0b00000001)) {
		delayMicroseconds(1);
	}
}
uint8_t	W83C42::in(bool command, bool interrupt) {
	/*
	Description:
		Function that works the same way as the x86 IN instruction.

	Parameters:
		bool command - A boolean parameter that distinguishes
					  a data read from a status register read.
					  false = data
					  true = status
	*/
	if (!command && !interrupt)  waitForOBF();                   // Wait until the keyboard is ready to send data

	uint8_t value = 0;

	digitalWrite(A2, command ? 1 : 0);
	digitalWrite(CS, 0);
	digitalWrite(RD, 0);
	delayMicroseconds(1);                         // Datasheet: RD to Drive Data Delay - 20ns

	for (int i = 0; i < 8; i++) {
		value |= (digitalRead(DATAPINS[i]) << i);   // Reading the data present on the data bus
	}
	digitalWrite(RD, 1);
	digitalWrite(CS, 1);

	return value;
}
void	W83C42::out(uint8_t data, bool command) {
	/*
	  Description:
		Function that works the same way as the x86 OUT instruction.

	  Parameters:
		byte data   -  Data to be output on the data bus.
		bool command - A boolean parameter that distinguishes
					   a data write from a command write.
					   false = data
					   true = command
	*/

	waitForIBF();                                     // Wait until the keyboard is ready to receive data

	pinOutput();                                      // Set data bus to output

	digitalWrite(A2, command ? 1 : 0);
	digitalWrite(CS, 0);

	for (int i = 0; i < 8; i++) {
		digitalWrite(DATAPINS[i], (data >> i) & 0x01);   // Outputing the data on the data bus
	}

	delayMicroseconds(1);
	digitalWrite(WR, 0);
	delayMicroseconds(1);
	digitalWrite(WR, 1);
	digitalWrite(CS, 1);

	pinInput();
}
void	W83C42::pinInput() const {
	/*
	Description:
		Function that cuts the Arduino from the bus
		allowing the W83C42 to drive the bus
	*/
	for (int i = 0; i < 8; i++) {
		pinMode(DATAPINS[i], INPUT);
	}
}
void	W83C42::pinOutput() const {
	/*
	Description:
		Function that connects the Arduino to the bus
		allowing it to drive the bus
	*/

	for (int i = 0; i < 8; i++) {
		pinMode(DATAPINS[i], OUTPUT);
	}
}
void	W83C42::ISR_WRAPPER() {
	if (instance) instance->BUFFER_ISR();
}
void	W83C42::BUFFER_ISR() {
	/*
	  Description:
		Interrupt service routine for the W83C42 INT signal.
		Translates the incoming scancodes and puts them in a buffer from where
		the scancodes can be read.
	*/

	int scancode = in(false, true);
	switch (scancode) {
	case BACKSPACE:
		buffer[end] = '\b';
		buffer[end] = ' ';
		buffer[end] = '\b';
		end += 3;
		break;
	case ESC:
		break;
	case TAB:
		buffer[end] = '\t';
		end += 1;
		break;
	case L_SHIFT:
	case R_SHIFT:
		SHIFT_PRESSED = true;
		break;
	case L_SHIFT | 0x80:
	case R_SHIFT | 0x80:
		SHIFT_PRESSED = false;
		break;
	case CAPS_LOCK:
		CAPSLOCK_ON = !CAPSLOCK_ON;
		break;
	default:
		if (scancode < 0x80 && !(scancode & 0x80)) {
			if (SHIFT_PRESSED) {
				buffer[end] = SCAN_CODES_SHIFTED[scancode];
			}
			else {
				if (CAPSLOCK_ON && SCAN_CODES[scancode] >= 'a' && SCAN_CODES[scancode] <= 'z') {
					buffer[end] = SCAN_CODES[scancode] - 0x20;
				}
				else buffer[end] = SCAN_CODES[scancode];
			}
			end = end + 1;
		}
		break;
	}
}

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
	/*
	Description:
		Transfers .maxLen. bytes from the data buffer to the given
		.outputBuffer.

	Parameters:
		char* outputBuffer:
			Character buffer to be written to.
		int maxLen:
			Maximum ammount of bytes to be read from the buffer.

	Return value:
		Number of bytes read from the buffer.
	*/
	int count = 0;
	int i = start;
	while (i != end && count < maxLen) {
		outputBuffer[count++] = buffer[i];
		i = (i + 1) % 128;
	}
	start = end = 0;

	return count;
}
bool	W83C42::begin() {
	/*
	Description:
		Performs a reset routine, and a self test on the W83C42

	Return value:
		true - self test passed (0x55)
		fale - self test failed
	*/
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
		return true;
	}
}
void	W83C42::disable() {
	// Disables the keyboard interface
	writeCommand(0);
	detachInterrupt(digitalPinToInterrupt(INT));
}
void	W83C42::enable() {
	// Enables the keyboard interface
	writeCommand(0x69);
	attachInterrupt(digitalPinToInterrupt(INT), ISR_WRAPPER, RISING);
}
uint8_t	W83C42::selfTest(bool print) {
	/*
	Description:
		Function that makes the W83C42 perform a self test on itself
		and outputs the not formatted result to the serial monitor.

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor.
			false - do not print the formatted result to the serial monitor.

	Return value:
		A byte of data received from the W83C42.

	Datasheet reference:
		Page 11.
	*/

	out(0xAA, true);						// 0xAA - "Self test" command
	uint8_t data = in(false, false);        // Read the data that W83C42 put on the bus
	if (print) {
		Serial.print("Self test result: 0x");
		Serial.println(data, HEX);
	}
	return data;
}
uint8_t W83C42::interfaceTest(bool print) {
	/*
	Description:
		Function that makes the W83C42 perform an interface test with the keyboard.

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor

	Return value:
		A byte of data received from the W83C42.

	Datasheet reference:
		Page 10.

	Notes:
		No where in the datasheet is said what should be the output of this test.
	*/
	out(0xAB, true);                      // 0xAB - "Interface Tes" command
	byte data = in(false, false);
	if (print) {
		Serial.println();
		Serial.println();

		Serial.print("Result of the interface test: 0x");

		Serial.println(data, HEX);

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

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor

	Return value:
		A byte of data received from the W83C42.

	Datasheet reference:
		The status register (...) may be read at any time.
	*/

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

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor

	Return value:
		A byte of data received from the W83C42.
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
uint8_t W83C42::readInputPort(bool print) {
	/*
	Description:
		Function that reads the input port of the keyboard controller
		and outputs the result to the serial monitor.

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor
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
uint8_t W83C42::readOutputPort(bool print) {
	/*
	Description:
		Function that reads the output port of the keyboard controller
		and outputs the result to the serial monitor.

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor

	Return value:
		A byte of data received from the W83C42.

	Datasheet reference:
		Page 10.
	*/

	out(0xD0, true);                      // 0xD0 - "Read Output Port" command
	byte data = in(false, false);
	if (print) {
		Serial.println();
		Serial.println();

		Serial.println("Output port: ");

		Serial.print("SYSTEM RESET: ");
		Serial.println(data & 0b00000001 ? 1 : 0);

		Serial.print("GATE A20: ");
		Serial.println(data & 0b00000010 ? 1 : 0);

		Serial.print("UNDEFINED: ");
		Serial.println(data & 0b00000100 ? 1 : 0);

		Serial.print("UNDEFINED: ");
		Serial.println(data & 0b00001000 ? 1 : 0);

		Serial.print("OUTPUT BUFFER FULL: ");
		Serial.println(data & 0b00010000 ? 1 : 0);

		Serial.print("INPUT BUFFER EMPTY: ");
		Serial.println(data & 0b00100000 ? 1 : 0);

		Serial.print("KB CLOCK OUTPUT: ");
		Serial.println(data & 0b01000000 ? 1 : 0);

		Serial.print("KB DATA OUTPUT: ");
		Serial.println(data & 0b10000000 ? 1 : 0);

		Serial.println();
		Serial.println();
	}
	return data;
}
uint8_t W83C42::readTestInputs(bool print) {
	/*
	Description:
		Function that reads the test pins (Pin 1, Pin 39) of the keyboard controller
		and outputs the result to the serial monitor.

	Parameters:
		bool print:
			true - print the formatted result to the serial monitor
			false - do not print the formatted result to the serial monitor

	Return value:
		A byte of data received from the W83C42.

	Datasheet reference:
		Page 10.
	*/
	out(0xE0, true);                      // 0xE0 - "Read Test Inputs" command
	uint8_t data = in(false, false);
	if (print) {
		Serial.println();
		Serial.println();

		Serial.println("Test inputs: ");

		Serial.print("T0 KEYBOARD CLOCK: ");
		Serial.println(data & 0b00000001 ? 1 : 0);

		Serial.print("T1 KEYBOARD DATA: ");
		Serial.println(data & 0b00000010 ? 1 : 0);

		Serial.println();
		Serial.println();
	}
	return data;
}
void    W83C42::writeOutputPort(uint8_t data) {
	/*
	Description:
		Function that sends data to the output port of the W83C42.

	Datasheet reference:
		Page 11
	*/

	out(0xD1, true);                      // 0xD1 - "Write Output Port" command
	out(data, false);
}
void	W83C42::writeCommand(uint8_t data) {
	/*
	Description:
		Function that sends data to the Command Byte of the W83C42.

	Datasheet reference:
		Page 11, command 60.
	*/

	out(0x60, true);						// 0x60 - "Write Command Byte of Keyboard Controller" command
	out(data, false);
}
void	W83C42::disableKeyboardInterface() {
	/*
	Description:
		Function that disables the keyboard interface".
	*/
	out(0xAD, true);						// 0xAD - "Disable Keyboard Feature" command
	delayMicroseconds(10);
}
void	W83C42::enableKeyboardInterface() {
	/*
	Description:
		Function that disables the keyboard interface.
	*/

	out(0xAE, true);						// 0xAE - "Enable Keyboard Interface" command
	delayMicroseconds(10);
}
void	W83C42::enableTranslation(){
	/*
	Description:
		Function that disables the translation of scancodes,
		instead of characters being sent to the symbolBuffer, the 
		scancode buffer will be filled up.
	*/

	this->translate = true;
}
void	W83C42::disableTranslation() {
	/*
	Description:
		Function that enables the translation of scancodes,
		instead of scancodes being sent to the scancodeBuffer, the
		symbolBuffer will be filled up.
	*/
	this->translate = false;
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
		bool command:
			A boolean parameter that distinguishes
			a data read from a status register read.
			false = data
			true = status
	*/
	if (!command && !interrupt)  waitForOBF();    // Wait until the keyboard is ready to send data

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
		byte data:
			Data to be output on the data bus.
		bool command:
			A boolean parameter that distinguishes
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
		Function that cuts the Arduino from the data bus
		allowing the W83C42 to drive the bus.
	*/
	for (int i = 0; i < 8; i++) {
		pinMode(DATAPINS[i], INPUT);
	}
}
void	W83C42::pinOutput() const {
	/*
	Description:
		Function that connects the Arduino to the data bus
		allowing it to drive the bus.
	*/

	for (int i = 0; i < 8; i++) {
		pinMode(DATAPINS[i], OUTPUT);
	}
}
void	W83C42::BUFFER_ISR() {
	/*
	Description:
		Interrupt service routine for the W83C42 INT signal.
		Translates the incoming scancodes and puts them in a buffer from where
		the scancodes can be read.
	Notes:
		Disabling the scancode translation not yet implemented.
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
void	W83C42::ISR_WRAPPER() {
	/*
	Description:
		A wrapper function that allows the ISR to be a member of a class.
	*/
	if (instance) instance->BUFFER_ISR();
}

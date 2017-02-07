//--------------------------------------------------\\
// Projekt: Generalprøveprojektet - Fjernstyret bil \\
//      Periode: Uge 3 (17 Jan) - Uge 6 (7 Feb)     \\
//              Andreas og Marcus - 3C              \\
//--------------------------------------------------\\

// Includes
#include <SoftwareSerial.h>;

// Settings
#define FLASH_DELAY 350

#define SPEED_FOR 150
#define SPEED_BACK 200
#define SPEED_TURN 90

#define ALIVE_LENGTH 2000

#define TURN_MAX_LEFT 190
#define TURN_MAX_RIGHT 270

#define TURN_MIDDLE 230
#define TURN_MIDDLE_ACCEPT 20

// Commands
#define CMD_WAIT 'Z'
#define CMD_ALIVE 'X'

#define CMD_BACKWARDS 'S'
#define CMD_BACKWARDS_RESET 'G'

#define CMD_LEFT_BLINK 'Q'
#define CMD_LEFT_BLINK_RESET 'R'

#define CMD_LEFT_TURN 'A'
#define CMD_LEFT_TURN_RESET 'F'

#define CMD_RIGHT_BLINK 'E'
#define CMD_RIGHT_BLINK_RESET 'Y'

#define CMD_RIGHT_TURN 'D'
#define CMD_RIGHT_TURN_RESET 'H'

#define CMD_HAZARD 'C'

// Pins
#define PIN_BLUE_TX 10
#define PIN_BLUE_RX 11

#define PIN_MOTOR_BACK_FOR 5
#define PIN_MOTOR_BACK_BACK 6

#define PIN_MOTOR_FRONT_LEFT 3
#define PIN_MOTOR_FRONT_RIGHT 9

#define PIN_MOTOR_FRONT_READ A0

#define PIN_LIGHT_FRONT 4
#define PIN_LIGHT_BACK1_R 8
#define PIN_LIGHT_BACK1_G 7
#define PIN_LIGHT_BACK2_R 13
#define PIN_LIGHT_BACK2_G 12

int pinsInput[] = {
	PIN_MOTOR_FRONT_READ
};

int pinsOutput[] = {
	PIN_MOTOR_BACK_FOR, PIN_MOTOR_BACK_BACK,
	PIN_MOTOR_FRONT_LEFT, PIN_MOTOR_FRONT_RIGHT,

	PIN_LIGHT_FRONT,
	PIN_LIGHT_BACK1_R, PIN_LIGHT_BACK1_G,
	PIN_LIGHT_BACK2_R, PIN_LIGHT_BACK2_G
};

// Bluetooth
SoftwareSerial bluetoothSerial(PIN_BLUE_TX, PIN_BLUE_RX); 

/*
 * Function: setupPins
 * ----------------------------
 * Initialize the correct INPUT/OUTPUT values for the used ports.
 *
 * @returns: void
 */
void setupPins() {

	// Loop through INPUT pins
	int inputCounts = sizeof(pinsInput) / sizeof(*pinsInput);
	for(int v = 0; v < inputCounts; v++) {
		pinMode(pinsInput[v], INPUT);
	}

	// Loop through OUTPUT pins
	int outputCounts = sizeof(pinsOutput) / sizeof(*pinsOutput);
	for(int v = 0; v < outputCounts; v++) {
		pinMode(pinsOutput[v], OUTPUT);
	}

}

/*
 * Function: setup
 * ----------------------------
 * Setup function for Arduino (Run once)
 *
 * @returns: void
 */
void setup() {

	// Begin serial monitor
	Serial.begin(9600);

	// Begin bluetooth
	bluetoothSerial.begin(9600);

	// Initialize Ports
	setupPins();

}

/*
 * Function: flashLight
 * ----------------------------
 * Flash the debug LED on the port defined
 *
 * @param previousFlash: Previous flash millis
 * @param port: Port that the flash should be on
 * @param delay: Amount of time to wait
 *
 * @returns: unsigned long millis
 */
unsigned long flashLight(unsigned long previousFlash, int port, int delay = FLASH_DELAY) {

	// Check last flash millis
	if(millis() <= previousFlash+delay) {
		return previousFlash;
	}

	// Toggle light
	digitalWrite(port, !digitalRead(port));

	// Return millis
	return millis();

}

/*
 * Function: handleBlinker
 * ----------------------------
 * Handle the blinker lights
 *
 * @param isBlinking: Bool for blink on current side
 * @param isBraking: Bool for braking
 * @param timestamp: Timestamp for current flash blink
 * @param pinRed: Pin number for the red led
 * @param pinGreen: Pin number for the green led
 *
 * @returns: unsigned long timestamp
 */
int blinkingLeft = 0;
int blinkingRight = 0;

int brakingOn = 0;
int hazardOn = 0;

unsigned long frontFlash = millis();
unsigned long leftFlash = millis();
unsigned long rightFlash = millis();

unsigned long handleBlinker(int isBlinking, int isBraking, int isHazard, unsigned long timestamp, int pinRed, int pinGreen) {

	// Current action
	if(isBlinking || isHazard) {
		digitalWrite(pinRed, LOW);
		return flashLight(timestamp, pinGreen);
	}else if(isBraking){
		digitalWrite(pinRed, HIGH);
		digitalWrite(pinGreen, LOW);
	}else{
		digitalWrite(pinRed, HIGH);
		digitalWrite(pinGreen, HIGH);
	}

	// Return
	return timestamp;

}

/*
 * Function: handleLights
 * ----------------------------
 * Handle lights for the vehicle
 *
 * @param command: Bluetooth signal
 *
 * @returns: void
 */
void handleLights(char command) {
	
	// Front lights
	if(hazardOn) {
		frontFlash = flashLight(frontFlash, PIN_LIGHT_FRONT);
	}else{
		digitalWrite(PIN_LIGHT_FRONT, HIGH);
	}

	// Handle bluetooth commands
	switch (command) {

		// Toogle left blinker
		case CMD_LEFT_BLINK:
			blinkingLeft = 1;
			break;
		case CMD_LEFT_BLINK_RESET:
			blinkingLeft = 0;
			break;

		// Toogle right blinker
		case CMD_RIGHT_BLINK:
			blinkingRight = 1;
			break;
		case CMD_RIGHT_BLINK_RESET:
			blinkingRight = 0;
			break;

		// Toggle hazard lights
		case CMD_HAZARD:
			hazardOn = !hazardOn;
			break;

	}

	// Back left light
	leftFlash = handleBlinker(blinkingLeft, brakingOn, hazardOn, leftFlash, PIN_LIGHT_BACK1_R, PIN_LIGHT_BACK1_G);

	// Back right light
	rightFlash = handleBlinker(blinkingRight, brakingOn, hazardOn, rightFlash, PIN_LIGHT_BACK2_R, PIN_LIGHT_BACK2_G);

}

/*
 * Function: writeToFront
 * ----------------------------
 * Write analog signal to front motors
 *
 * @param left: Left analog power
 * @param right: Right analog power
 *
 * @returns: void
 */
void writeToFront(int left = 0, int right = 0) {
	analogWrite(PIN_MOTOR_FRONT_LEFT, left);
	analogWrite(PIN_MOTOR_FRONT_RIGHT, right);
}

/*
 * Function: writeToBack
 * ----------------------------
 * Write analog signal to back motors
 *
 * @param forward: Forward analog power
 * @param backward: Backward analog power
 *
 * @returns: void
 */
void writeToBack(int forward = 0, int backward = 0) {
	analogWrite(PIN_MOTOR_BACK_FOR, forward);
	analogWrite(PIN_MOTOR_BACK_BACK, backward);
}

/*
 * Function connectionAlive
 * ----------------------------
 * Check if the Bluetooth connection is alive
 *
 * @param command: Bluetooth signal
 *
 * @returns: boolean
 */
unsigned long previousAlive = millis();

bool connectionAlive(char command) {

	// Check alive command
	if(command == CMD_ALIVE) {
		previousAlive = millis();
	}

	// Return alive status
	return !((previousAlive+ALIVE_LENGTH) <= millis());

}

/*
 * Function: handleMovement
 * ----------------------------
 * Handle movement for the vehicle
 *
 * @param command: Bluetooth signal
 *
 * @returns: void
 */
int forwardSpeed = SPEED_FOR;
int direction = 0;
int turnDirection = 0;
void handleMovement(char command) {

	// Check if bluetooth is alive
	bool bluetoothAlive = connectionAlive(command);
	if(!bluetoothAlive) {
		writeToBack();
		writeToFront();
		return;
	}

	// Handle novement speed from bluetooth
	if(isDigit(command)) {
		int commandInt = (int) command - 48;
		forwardSpeed = (255/9) * commandInt;

		if(forwardSpeed != 0) {
			direction = 1;
		}
	}

	// Handle bluetooth commands
	switch (command) {

		// Toggle backwards direction
		case CMD_BACKWARDS:
			direction = -1;
			break;
		case CMD_BACKWARDS_RESET:
			direction = 0;
			break;

		// Toogle left turn
		case CMD_LEFT_TURN:
			turnDirection = 1;
			break;
		case CMD_LEFT_TURN_RESET:
			turnDirection = 0;
			break;

		// Toggle right turn
		case CMD_RIGHT_TURN:
			turnDirection = -1;
			break;
		case CMD_RIGHT_TURN_RESET:
			turnDirection = 0;
			break;

	}

	// Handle back motor direction
	switch (direction) {

		// Drive forward
	    case 1:
	    	writeToBack(forwardSpeed);
			break;

		// Drive backwards
	    case -1:
	    	writeToBack(0, SPEED_BACK);
	    	break;

	    // Do nothing
	    default:
			writeToBack();
			break;
	}

	// Handle front motor direction and position
	int readMotorPos = analogRead(PIN_MOTOR_FRONT_READ);
	switch (turnDirection) {

		// Turn left
	    case 1:
	    	// Check if maximum reached
	    	if(readMotorPos <= TURN_MAX_LEFT) {
	    		writeToFront();
	    	}else{
	    		writeToFront(SPEED_TURN);
	    	}

			break;

		// Turn right
	    case -1:
	    	// Check if maximum reached
	    	if(readMotorPos >= TURN_MAX_RIGHT) {
				writeToFront();
	    	}else{
	    		writeToFront(0, SPEED_TURN);
			}

	    	break;

	    // Center turning
	    default:
	    	// Check if maximum reached
	    	if(readMotorPos <= (TURN_MIDDLE - TURN_MIDDLE_ACCEPT)) {
	    		writeToFront(0, SPEED_TURN);
			}else if(readMotorPos >= (TURN_MIDDLE + TURN_MIDDLE_ACCEPT)){
				writeToFront(SPEED_TURN, 0);
			}else{
				writeToFront();
			}

			break;
	}

}

/*
 * Function: retrieveBluetooth
 * ----------------------------
 * Returns signal from bluetooth
 *
 * @returns: char
 */
char retrieveBluetooth() {

	// Available
	if(bluetoothSerial.available()) {
		return bluetoothSerial.read();
	}

	// Unavailable
	return CMD_WAIT;

}

/*
 * Function: loop
 * ----------------------------
 * Loop function for Arduino (Run every tick)
 *
 * @returns: void
 */
char bluetoothRaw;
void loop() {

	// Bluetooth
	bluetoothRaw = retrieveBluetooth();
	if(bluetoothRaw != 'Z' && bluetoothRaw != 'X') {
		Serial.println(bluetoothRaw);
	}

	// Handle actions
	handleMovement(bluetoothRaw);
	handleLights(bluetoothRaw);

}
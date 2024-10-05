#include <WiFi.h> 


// Initialize motor pin definitions
#define motorFL_in1 17  // Front Left Motor Pin 1
#define motorFL_in2 16  // Front Left Motor Pin 2
#define motorFR_in3 25  // Front Right Motor Pin 3
#define motorFR_in4 26  // Front Right Motor Pin 4
#define motorBL_in3 23  // Back Left Motor Pin 3
#define motorBL_in4 22  // Back Left Motor Pin 4
#define motorBR_in1 33  // Back Right Motor Pin 1
#define motorBR_in2 32  // Back Right Motor Pin 2

// PWM control variables
const int pwmFreq       = 5000;  // 5 kHz frequency
const int pwmResolution = 9;    // 16-bit resolution (0-65535)
const int maxPWM        = (1UL << pwmResolution) -1; // Max PWM value for 16-bit resolution
const int setMaxPWM     = 1.0 / 1 * maxPWM;
const int dPadRpm   = 1.0 / 1 * maxPWM;
const int deadZone      = 60;

// Define the PWM channels
const int pwmChannel_FL1 = 0;
const int pwmChannel_FL2 = 1;
const int pwmChannel_FR1 = 2;
const int pwmChannel_FR2 = 3;
const int pwmChannel_BL1 = 4;
const int pwmChannel_BL2 = 5;
const int pwmChannel_BR1 = 6;
const int pwmChannel_BR2 = 7;

// Ramp variables
unsigned long previousMillis_FL = 0, previousMillis_FR = 0, previousMillis_BL = 0, previousMillis_BR = 0;
int controlValueRamp_FL = 0, controlValueRamp_FR = 0, controlValueRamp_BL = 0, controlValueRamp_BR = 0;
int rampTime = 7;  // Adjust ramp time
// Motor outputs

// Controller inputs
int stickLeft_X  = 0;//ABS_X +-32K
int stickLeft_Y  = 0;//ABS_Y +-32K
int stickRight_X = 0;//ABS_RX +-32K
int stickRight_Y = 0;//ABS_RY +-32K

int trigRight = 0;//ABS_RZ +255
int trigLeft  = 0;//ABS_Z +255

int shoudlerRight = 0;//BTN_TR 1
int shoudlerLeft  = 0;//BTN_TL 1

int dPad_R = 0;//ABS_HAT0X 1
int dPad_L = 0;//ABS_HAT0X -1
int dPad_U = 0;//ABS_HAT0Y -1
int dPad_D = 0;//ABS_HAT0Y 1

int key_X = 0;//BTN_WEST 1
int key_Y = 0;//BTN_NORTH 1
int key_A = 0;//BTN_SOUTH 1
int key_B = 0;//BTN_EAST 1

int back = 0;//BTN_START 1
int start= 0;//BTN_SELECT 1

// Wi-Fi credentials (replace with your own SSID and password)
const char* ssid     = "BSNL FIBERNET_15";  // Replace with your Wi-Fi SSID
const char* password = "08816221515";       // Replace with your Wi-Fi password

WiFiServer server(80);  // Create a server on port 80 for HTTP communication

void setup() {
  // Initialize your motors and serial communication (if needed)
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial started");  // Print start message

  wiFi_setup(); // wifi connect
  // Set motor control pins as output
  // pinMode(motorFL_in1, OUTPUT);
  // pinMode(motorFL_in2, OUTPUT);
  // pinMode(motorFR_in3, OUTPUT);
  // pinMode(motorFR_in4, OUTPUT);
  // pinMode(motorBL_in3, OUTPUT);
  // pinMode(motorBL_in4, OUTPUT);
  // pinMode(motorBR_in1, OUTPUT);
  // pinMode(motorBR_in2, OUTPUT);

  // Setup PWM functionality for each motor input

  // Setup PWM channels for each motor pin
  ledcAttachChannel(motorFL_in1, pwmFreq, pwmResolution, pwmChannel_FL1);
  ledcAttachChannel(motorFL_in2, pwmFreq, pwmResolution, pwmChannel_FL2);
  ledcAttachChannel(motorFR_in3, pwmFreq, pwmResolution, pwmChannel_FR1);
  ledcAttachChannel(motorFR_in4, pwmFreq, pwmResolution, pwmChannel_FR2);
  ledcAttachChannel(motorBL_in3, pwmFreq, pwmResolution, pwmChannel_BL1);
  ledcAttachChannel(motorBL_in4, pwmFreq, pwmResolution, pwmChannel_BL2);
  ledcAttachChannel(motorBR_in1, pwmFreq, pwmResolution, pwmChannel_BR1);
  ledcAttachChannel(motorBR_in2, pwmFreq, pwmResolution, pwmChannel_BR2);
}

void loop() {
  getControllerInput();
  // Drive the motors based on the potentiometer input
  delay(10);  // Small delay to avoid spamming the motors
}

void driveCar_leftJoyStick() {
  // Assuming joystick gives values from -32768 to 32767
  int joystickX = stickLeft_X;  // X-axis input
  int joystickY = stickLeft_Y;  // Y-axis input
  int turn = -trigLeft+trigRight; // Rotation input

  // Mapping joystick input directly to PWM range (0 to setMaxPWM)
  int pwmX = map(joystickX, -32768, 32767, -setMaxPWM, setMaxPWM);
  int pwmY = map(joystickY, -32768, 32767, -setMaxPWM, setMaxPWM);
  int pwmTurn = map(turn, -255, 255, -setMaxPWM, setMaxPWM);

  // Calculate theta (direction) and power (magnitude)
  float theta = atan2(pwmY, pwmX);  // Direction angle in radians
  float power = hypot(pwmX, pwmY);  // Magnitude of movement (0 to setMaxPWM)


  // Calculate sin and cos based on theta (direction)
  float sinVal = sin(theta - PI / 4);  // Adjusted for mecanum wheels
  float cosVal = cos(theta - PI / 4);
  float maxVal = max(abs(sinVal), abs(cosVal));

  // Calculate motor values based on power, direction, and turn
  int frontLeft  = (power * cosVal / maxVal) + pwmTurn;
  int frontRight = (power * sinVal / maxVal) - pwmTurn;
  int backLeft   = (power * sinVal / maxVal) + pwmTurn;
  int backRight  = (power * cosVal / maxVal) - pwmTurn;
  // Limit motor values to the PWM range (0 to setMaxPWM)
  frontLeft   = constrain(frontLeft , -setMaxPWM, setMaxPWM);
  frontRight  = constrain(frontRight, -setMaxPWM, setMaxPWM);
  backLeft    = constrain(backLeft  , -setMaxPWM, setMaxPWM);
  backRight   = constrain(backRight , -setMaxPWM, setMaxPWM);

  Serial.print("Degrees : ");Serial.print(theta*180/PI);Serial.print(" | Power : ");Serial.print(pwmX);
  Serial.print(", ");Serial.print(pwmY);Serial.print(", ");Serial.print(power);Serial.print(" | Rotate : ");Serial.println(map(turn, -255, 255, -360, 360));
  Serial.print( "Power__FL : ");Serial.print(frontLeft);Serial.print(" | Power_FR : ");Serial.print(frontRight);
  Serial.print(" | Power_BL : ");Serial.print(backLeft);Serial.print(" | Power_BR : ");Serial.println(backRight);

  // Now drive your motors
  driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, frontLeft , controlValueRamp_FL, previousMillis_FL);  // Front Left
  driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, frontRight, controlValueRamp_FR, previousMillis_FR);  // Front Right
  driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, backLeft  , controlValueRamp_BL, previousMillis_BL);  // Back Left
  driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, backRight , controlValueRamp_BR, previousMillis_BR);  // Back Right
}

void driveCar_Dpad(){
  // Create 4-bit number, assigning each direction to a bit
  int dPad_4bit = (dPad_R << 3) | (dPad_L << 2) | (dPad_U << 1) | dPad_D;
  int frontLeft,frontRight,backLeft,backRight;
  dPad2Pwm_mecanum(dPad_4bit, frontLeft, frontRight, backLeft, backRight);
  // Now drive your motors
  Serial.print( "Power__FL : ");Serial.print(frontLeft);Serial.print(" | Power_FR : ");Serial.print(frontRight);
  Serial.print(" | Power_BL : ");Serial.print(backLeft);Serial.print(" | Power_BR : ");Serial.println(backRight);
  driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, frontLeft , controlValueRamp_FL, previousMillis_FL);  // Front Left
  driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, frontRight, controlValueRamp_FR, previousMillis_FR);  // Front Right
  driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, backLeft  , controlValueRamp_BL, previousMillis_BL);  // Back Left
  driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, backRight , controlValueRamp_BR, previousMillis_BR);  // Back Right

}

void driveMotorPWM(const int pwmChannel1, const int pwmChannel2, const int controlValue, int &controlValueRamp, unsigned long &previousMillis) {
  unsigned long currentMillis = millis();

  int rampIncrement = (controlValue - controlValueRamp) / rampTime;
  controlValueRamp = (currentMillis - previousMillis >= rampTime) ? controlValue : controlValueRamp + rampIncrement;
  previousMillis = currentMillis;

  // Apply the ramped control value to the motor
  if (controlValueRamp > deadZone) {
    ledcWriteChannel(pwmChannel1, controlValueRamp);  // Forward
    ledcWriteChannel(pwmChannel2, 0);                 // Stop backward
  } else if (controlValueRamp < deadZone * -1) {
    ledcWriteChannel(pwmChannel1, 0);                  // Stop forward
    ledcWriteChannel(pwmChannel2, -controlValueRamp);  // Backward
  } else {
    // Stop the motor
    ledcWriteChannel(pwmChannel1, 0);
    ledcWriteChannel(pwmChannel2, 0);
  }

}

void wiFi_setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  // Wait until connected to Wi-Fi
    delay(1000);
    Serial.println("Connecting to WiFi...");  // Notify about connection status
  }
  Serial.println("Connected to WiFi");  // Wi-Fi connection successful

  server.begin();                    // Start the HTTP server
  Serial.println("Server started");  // Notify that server has started
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address of ESP32
  WiFi.setSleep(false);            // Disable Wi-Fi power saving mode for stable connection
}

void getControllerInput() {
  WiFiClient client = server.accept();  // Accept incoming client connections
  client.setTimeout(5000);              // Set timeout for the client connection to 5 seconds

  if (client) {  // If a new client is connected
    Serial.println("New Client connected");
    String currentLine = "";  // Store incoming data one line at a time

    while (client.connected()) {  // Continue while client is still connected
      if (client.available()) {   // If there's data available to read from client
        char c = client.read();   // Read the incoming data one character at a time
        if (c == '\n') {          // If the line is completely read (newline character)
          // If it's an empty line (indicates the end of the request header)
          if (currentLine.length() == 0) {
            // Send HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // Send HTML content to the client
            client.println("ESP32 Web Server");
            client.println();
            break;  // Break out of the while loop once response is sent
          } else {
            // Process and print the current line of data
            Serial.println(currentLine);
            updateInputs(currentLine);  // Process the client input
            if (dPad_L + dPad_D + dPad_R + dPad_U > 0) {
              driveCar_Dpad();
            } else {
              driveCar_leftJoyStick();
            }

            currentLine = "";  // Clear the current line for the next input
          }
        } else if (c != '\r') {  // Ignore carriage returns
          currentLine += c;      // Add characters to the current line
        }
      }
      yield();  // Allow background tasks like Wi-Fi handling to execute
    }
    client.stop();                          // Close the client connection when finished
    Serial.println("Client disconnected");  // Notify that the client has disconnected
  }
}

void updateInputs(String data) {
  char command;   // Variable to hold command type (Button/Axis/D-pad)
  char code[20];  // Array to hold button/axis/D-pad code
  int value;      // Variable to hold the state/value of button/axis/D-pad

  // Parse incoming data using sscanf (expecting a format like "B btnX 1" or "A axisY 512")
  if (sscanf(data.c_str(), "%c %s %d", &command, code, &value) == 3) {
    if (command == 'B') {
      if (strcmp(code, "BTN_EAST")       == 0) key_B = value;
      else if (strcmp(code, "BTN_WEST")  == 0) key_X = value;
      else if (strcmp(code, "BTN_NORTH") == 0) key_Y = value;
      else if (strcmp(code, "BTN_SOUTH") == 0) key_A = value;
      else if (strcmp(code, "BTN_TR")    == 0) shoudlerRight = value;
      else if (strcmp(code, "BTN_TL")    == 0) shoudlerLeft = value;
      else if (strcmp(code, "BTN_START") == 0) back  = value;
      else if (strcmp(code, "BTN_SELECT")== 0) start = value;
    } else if (command == 'A') {
      if (strcmp(code, "ABS_X")      == 0) stickLeft_X  = value;
      else if (strcmp(code, "ABS_Y") == 0) stickLeft_Y  = value;
      else if (strcmp(code, "ABS_RX")== 0) stickRight_X = value;
      else if (strcmp(code, "ABS_RY")== 0) stickRight_Y = value;
      else if (strcmp(code, "ABS_Z") == 0) trigLeft     = value;
      else if (strcmp(code, "ABS_RZ")== 0) trigRight    = value;
    } else if (command == 'D') {
      if (strcmp(code, "ABS_HAT0X") == 0) {
        dPad_R = (value == 1) ? 1 : 0;
        dPad_L = (value == -1) ? 1 : 0;
      }
      if (strcmp(code, "ABS_HAT0Y") == 0) {
        dPad_U = (value == -1) ? 1 : 0;
        dPad_D = (value == 1) ? 1 : 0;
      }
    }
  }
}

void dPad2Pwm_mecanum(int dPad_4bit, int &frontLeft, int &frontRight, int &backLeft, int &backRight) {

  switch (dPad_4bit) {
    case 8:  // dPad_R (right) pressed, 8 == 0b1000
      Serial.println("Right");
      frontLeft = dPadRpm;
      frontRight = -dPadRpm;
      backLeft = -dPadRpm;
      backRight = dPadRpm;
      break;
      
    case 4:  // dPad_L (left) pressed, 4 == 0b0100
      Serial.println("Left");
      frontLeft = -dPadRpm;
      frontRight = dPadRpm;
      backLeft = dPadRpm;
      backRight = -dPadRpm;
      break;

    case 2:  // dPad_U (up) pressed, 2 == 0b0010
      Serial.println("Up");
      frontLeft = dPadRpm;
      frontRight = dPadRpm;
      backLeft = dPadRpm;
      backRight = dPadRpm;
      break;

    case 1:  // dPad_D (down) pressed, 1 == 0b0001
      Serial.println("Down");
      frontLeft = -dPadRpm;
      frontRight = -dPadRpm;
      backLeft = -dPadRpm;
      backRight = -dPadRpm;
      break;

    case 10:  // Up + Right pressed, 10 == 0b1010
      Serial.println("Up + Right");
      frontLeft = dPadRpm;
      frontRight = 0;
      backLeft = 0;
      backRight = dPadRpm;
      break;

    case 6:  // Up + Left pressed, 6 == 0b0110
      Serial.println("Up + Left");
      frontLeft = 0;
      frontRight = dPadRpm;
      backLeft = dPadRpm;
      backRight = 0;
      break;

    case 9:  // Down + Right pressed, 9 == 0b1001
      Serial.println("Down + Right");
      frontLeft = 0;
      frontRight = -dPadRpm;
      backLeft = -dPadRpm;
      backRight = 0;
      break;

    case 5:  // Down + Left pressed, 5 == 0b0101
      Serial.println("Down + Left");
      frontLeft = -dPadRpm;
      frontRight = 0;
      backLeft = 0;
      backRight = -dPadRpm;
      break;

    default:
      Serial.println("No D-Pad input or combination not handled.");
      frontLeft = 0;
      frontRight = 0;
      backLeft = 0;
      backRight = 0;
      break;
  }
}


// void drive_4WD() {
//   // Fetch the joystick input values
//   int xValue = trigRight-trigLeft; // Left joystick X-axis (-255 to 255)
//   int yValue = stickLeft_Y; // Left joystick Y-axis (-32k to 32k)
  
//   // Normalize the joystick input to motor control values (PWM range 0-setMaxPWM)
//   int motorSpeedX = map(xValue, -255, 255, -setMaxPWM, setMaxPWM);
//   int motorSpeedY = map(yValue, -32768, 32767, -setMaxPWM, setMaxPWM);

//   // Define motor actions based on joystick inputs
//   if (motorSpeedY > 0) {
//     // Forward motion
//     driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, motorSpeedY, controlValueRamp_FL, previousMillis_FL);  // Front Left
//     driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, motorSpeedY, controlValueRamp_FR, previousMillis_FR);  // Front Right
//     driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, motorSpeedY, controlValueRamp_BL, previousMillis_BL);  // Back Left
//     driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, motorSpeedY, controlValueRamp_BR, previousMillis_BR);  // Back Right
//   } else if (motorSpeedY < 0) {
//     // Backward motion
//     driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, motorSpeedY, controlValueRamp_FL, previousMillis_FL);  // Front Left
//     driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, motorSpeedY, controlValueRamp_FR, previousMillis_FR);  // Front Right
//     driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, motorSpeedY, controlValueRamp_BL, previousMillis_BL);  // Back Left
//     driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, motorSpeedY, controlValueRamp_BR, previousMillis_BR);  // Back Right
//   } else if (motorSpeedX > 0) {
//     // Clockwise rotation
//     driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, motorSpeedX, controlValueRamp_FL, previousMillis_FL);  // Front Left
//     driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, -motorSpeedX, controlValueRamp_FR, previousMillis_FR); // Front Right
//     driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, motorSpeedX, controlValueRamp_BL, previousMillis_BL);  // Back Left
//     driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, -motorSpeedX, controlValueRamp_BR, previousMillis_BR); // Back Right
//   } else if (motorSpeedX < 0) {
//     // Anticlockwise rotation
//     driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, motorSpeedX, controlValueRamp_FL, previousMillis_FL);  // Front Left
//     driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, -motorSpeedX, controlValueRamp_FR, previousMillis_FR); // Front Right
//     driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, motorSpeedX, controlValueRamp_BL, previousMillis_BL);  // Back Left
//     driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, -motorSpeedX, controlValueRamp_BR, previousMillis_BR); // Back Right
//   } else {
//     // Stop motors if no joystick movement
//     driveMotorPWM(pwmChannel_FL1, pwmChannel_FL2, 0, controlValueRamp_FL, previousMillis_FL);  // Front Left
//     driveMotorPWM(pwmChannel_FR1, pwmChannel_FR2, 0, controlValueRamp_FR, previousMillis_FR);  // Front Right
//     driveMotorPWM(pwmChannel_BL1, pwmChannel_BL2, 0, controlValueRamp_BL, previousMillis_BL);  // Back Left
//     driveMotorPWM(pwmChannel_BR1, pwmChannel_BR2, 0, controlValueRamp_BR, previousMillis_BR);  // Back Right
//   }
// }
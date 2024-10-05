#include <DRV8833.h>  // https://github.com/TheDIYGuy999/DRV8833
#include <WiFi.h>  // Use WiFi library for ESP32

// Initialize DRV8833 H-Bridge motor pin definitions
#define motorFL_in1 26  // Front Left Motor Pin 1
#define motorFL_in2 25  // Front Left Motor Pin 2
#define motorFR_in3 39  // Front Right Motor Pin 3
#define motorFR_in4 36  // Front Right Motor Pin 4
#define motorBL_in3 33  // Back Left Motor Pin 3
#define motorBL_in4 32  // Back Left Motor Pin 4
#define motorBR_in1 35  // Back Right Motor Pin 1
#define motorBR_in2 34  // Back Right Motor Pin 2

// Motor control constants
#define MAX_PWM 255
#define deadZone 60            // Dead zone for motor control to prevent jitter
#define setMaxPWM ((int)(1.0 / 4 * MAX_PWM))  // This sets 1/4 of the MAX_PWM value
#define rampTime 7             // Ramp time for motor speed changes

// Wi-Fi credentials (replace with your own SSID and password)
const char* ssid = "BSNL FIBERNET_15";  // Replace with your Wi-Fi SSID
const char* password = "08816221515";   // Replace with your Wi-Fi password

WiFiServer server(80);  // Create a server on port 80 for HTTP communication



// Function prototypes
void driveMotor();          // Function to drive motors based on input
void move_4WD();           // Function to control 4WD movement
void move_mecanum();       // Function to control mecanum wheel movement
void wiFi_setup();              //Function to connect to WiFi

// Enumeration for different movement directions
enum Direction {
  F,            // Forward
  B,            // Backward
  R,            // Right
  L,            // Left
  FR,           // Forward Right
  FL,           // Forward Left
  B_R,           // Backward Right
  BL,           // Backward Left
  F_PIVOT_CW,   // Forward Pivot Clockwise
  B_PIVOT_CW,   // Backward Pivot Clockwise
  R_PIVOT_CW,   // Right Pivot Clockwise
  L_PIVOT_CW,   // Left Pivot Clockwise
  F_PIVOT_ACW,  // Forward Pivot Anti-clockwise
  B_PIVOT_ACW,  // Backward Pivot Anti-clockwise
  R_PIVOT_ACW,  // Right Pivot Anti-clockwise
  L_PIVOT_ACW,  // Left Pivot Anti-clockwise
  CW,           // Clockwise
  ACW           // Anti-clockwise
};

// Motor initialization with dead zone and PWM capabilities
DRV8833 MotorFL(motorFL_in1, motorFL_in2, 0, MAX_PWM, deadZone, false, true);  // Motor 1: Front Left
DRV8833 MotorFR(motorFR_in3, motorFR_in4, 0, MAX_PWM, deadZone, false, true);  // Motor 2: Front Right
DRV8833 MotorBL(motorBL_in3, motorBL_in4, 0, MAX_PWM, deadZone, false, true);  // Motor 3: Back Left
DRV8833 MotorBR(motorBR_in1, motorBR_in2, 0, MAX_PWM, deadZone, false, true);  // Motor 4: Back Right

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud
  delay(1000);
  Serial.println("Serial started");  // Print start message

  wiFi_setup(); // wifi connect
  // Set motor control pins as output
  pinMode(motorFL_in1, OUTPUT);
  pinMode(motorFL_in2, OUTPUT);
  pinMode(motorFR_in3, OUTPUT);
  pinMode(motorFR_in4, OUTPUT);
  pinMode(motorBL_in3, OUTPUT);
  pinMode(motorBL_in4, OUTPUT);
  pinMode(motorBR_in1, OUTPUT);
  pinMode(motorBR_in2, OUTPUT);
}

void loop() {
  // Drive the motors based on the potentiometer input
  driveMotor();
}

void driveMotor() {
  int speed1 = 512;  // Initialize speed for Motor 1
  int speed2 = 512;  // Initialize speed for Motor 2

  // Read potentiometer value and map it to speed range
  speed1 = map(analogRead(A0), 0, 4095, -MAX_PWM, MAX_PWM);  // Map to range -MAX_PWM to MAX_PWM
  speed2 = -1 * speed1;  // Invert speed for the opposite motors

  // ***************** Note! The ramp time is intended to protect the gearbox! *******************
  // Drive motors with specified speed, max PWM, and ramp time
  MotorFL.drive(speed1, setMaxPWM, rampTime, true, true);  // Motor 1: Front Left
  MotorFR.drive(speed2, setMaxPWM, rampTime, true, true);  // Motor 2: Front Right
  MotorBL.drive(speed1, setMaxPWM, rampTime, true, true);  // Motor 3: Back Left
  MotorBR.drive(speed2, setMaxPWM, rampTime, true, true);  // Motor 4: Back Right
}

void move_4WD(Direction dir, int v_fl, int v_fr, int v_bl, int v_br) {
  switch (dir) {
    case B:
      v_fl *= -1;
      v_fr *= -1;
      v_bl *= -1;
      v_br *= -1;  // Backward
    case F:        // Forward
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(v_br, setMaxPWM, rampTime, true, true);
      break;

    case F_PIVOT_ACW:
      v_bl *= -1;
      v_br *= -1;     // Forward Pivot ACW
    case F_PIVOT_CW:  // Forward Pivot CW
      // Implement forward pivot logic
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(-v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;

    case B_PIVOT_ACW:
      v_fl *= -1;
      v_fr *= -1;     // Forward Pivot ACW
    case B_PIVOT_CW:  // Forward Pivot CW
                      // Implement forward pivot logic
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(-v_br, setMaxPWM, rampTime, true, true);
      break;

    case ACW:
      v_fl *= -1;
      v_fr *= -1;
      v_bl *= -1;
      v_br *= -1;
    case CW:  // Clockwise
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(-v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(-v_br, setMaxPWM, rampTime, true, true);
      break;

    default:
      // Stop motors if an unknown direction is provided
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;
  }
}

void move_mecanum(Direction dir, int v_fl, int v_fr, int v_bl, int v_br) {
  switch (dir) {
    case B:
      v_fl *= -1;
      v_fr *= -1;
      v_bl *= -1;
      v_br *= -1;  // Backward
    case F:        // Forward
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(v_br, setMaxPWM, rampTime, true, true);
      break;

    case R:
      v_fl *= -1;
      v_fr *= -1;
      v_bl *= -1;
      v_br *= -1;  // Right
    case L:        // Left
      MotorFL.drive(-v_bl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(-v_br, setMaxPWM, rampTime, true, true);
      break;

    case BL: v_fl *= -1; v_br *= -1;  // Backward Left
    case FR:                          // Forward Right
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(v_br, setMaxPWM, rampTime, true, true);
      break;

    case B_R: v_fr *= -1; v_bl *= -1;  // Backward Right
    case FL:                          // Forward Left
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;

    case F_PIVOT_ACW:
      v_bl *= -1;
      v_br *= -1;     // Forward Pivot ACW
    case F_PIVOT_CW:  // Forward Pivot CW
      // Implement forward pivot logic
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(-v_br, setMaxPWM, rampTime, true, true);
      break;

    case B_PIVOT_ACW:
      v_fl *= -1;
      v_fr *= -1;     // Forward Pivot ACW
    case B_PIVOT_CW:  // Forward Pivot CW
                      // Implement forward pivot logic
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(-v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;

    case L_PIVOT_ACW:
      v_fr *= -1;
      v_br *= -1;     // Forward Pivot ACW
    case L_PIVOT_CW:  // Forward Pivot CW
      // Implement forward pivot logic
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(v_br, setMaxPWM, rampTime, true, true);
      break;

    case R_PIVOT_ACW:
      v_fl *= -1;
      v_bl *= -1;     // Forward Pivot ACW
    case R_PIVOT_CW:  // Forward Pivot CW
      // Implement forward pivot logic
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;

    case ACW:
      v_fl *= -1;
      v_fr *= -1;
      v_bl *= -1;
      v_br *= -1;
    case CW:  // Clockwise
      MotorFL.drive(v_fl, setMaxPWM, rampTime, true, true);
      MotorFR.drive(-v_fr, setMaxPWM, rampTime, true, true);
      MotorBL.drive(v_bl, setMaxPWM, rampTime, true, true);
      MotorBR.drive(-v_br, setMaxPWM, rampTime, true, true);
      break;

    default:
      // Stop motors if an unknown direction is provided
      MotorFL.drive(0, setMaxPWM, rampTime, true, true);
      MotorFR.drive(0, setMaxPWM, rampTime, true, true);
      MotorBL.drive(0, setMaxPWM, rampTime, true, true);
      MotorBR.drive(0, setMaxPWM, rampTime, true, true);
      break;
  }
}

void wiFi_setup(){
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {  // Wait until connected to Wi-Fi
    delay(1000);
    Serial.println("Connecting to WiFi...");  // Notify about connection status
  }
  Serial.println("Connected to WiFi");  // Wi-Fi connection successful

  server.begin();  // Start the HTTP server
  Serial.println("Server started");  // Notify that server has started
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address of ESP32
  WiFi.setSleep(false);  // Disable Wi-Fi power saving mode for stable connection
}

void getControllerInput(){
  WiFiClient client = server.accept();  // Accept incoming client connections
  client.setTimeout(5000);  // Set timeout for the client connection to 5 seconds

  if (client) {  // If a new client is connected
    Serial.println("New Client connected");
    String currentLine = "";  // Store incoming data one line at a time

    while (client.connected()) {  // Continue while client is still connected
      if (client.available()) {  // If there's data available to read from client
        char c = client.read();  // Read the incoming data one character at a time
        if (c == '\n') {  // If the line is completely read (newline character)
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
            processData(currentLine);  // Process the client input
            currentLine = "";  // Clear the current line for the next input
          }
        } else if (c != '\r') {  // Ignore carriage returns
          currentLine += c;  // Add characters to the current line
        }
      }
      yield();  // Allow background tasks like Wi-Fi handling to execute
    }
    client.stop();  // Close the client connection when finished
    Serial.println("Client disconnected");  // Notify that the client has disconnected
  }

}

void processData(String data) {
  char command;  // Variable to hold command type (Button/Axis/D-pad)
  char code[20];  // Array to hold button/axis/D-pad code
  int value;  // Variable to hold the state/value of button/axis/D-pad

  // Parse incoming data using sscanf (expecting a format like "B btnX 1" or "A axisY 512")
  if (sscanf(data.c_str(), "%c %s %d", &command, code, &value) == 3) {
    // If the command is a button press ('B')
    if (command == 'B') {
      Serial.print("Button ");
      Serial.print(code);  // Print button code
      Serial.print(" state: ");
      Serial.println(value);  // Print button state (pressed/released)
    } 
    // If the command is axis movement ('A')
    else if (command == 'A') {
      Serial.print("Axis ");
      Serial.print(code);  // Print axis code
      Serial.print(" value: ");
      Serial.println(value);  // Print axis value
    } 
    // If the command is D-pad direction ('D')
    else if (command == 'D') {
      Serial.print("D-pad ");
      Serial.print(code);  // Print D-pad code
      Serial.print(" state: ");
      Serial.println(value);  // Print D-pad state
    }
  } else {
    Serial.println("Invalid input format");  // Error message for incorrect input format
  }
}
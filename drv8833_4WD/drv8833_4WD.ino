#include <DRV8833.h>  // https://github.com/TheDIYGuy999/DRV8833

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
#define deadZone 30            // Dead zone for motor control to prevent jitter
#define setMaxPWM ((int)(1.0 / 4 * MAX_PWM))  // This sets 1/4 of the MAX_PWM value
#define rampTime 7             // Ramp time for motor speed changes

// Function prototypes
void driveMotor();          // Function to drive motors based on input
void move_4WD();           // Function to control 4WD movement
void move_mecanum();       // Function to control mecanum wheel movement

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

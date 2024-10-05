//.py script send data
// This code reads recives input data of controller to arduina IDE via serial.

#include <Arduino.h>
// String sum = "Start\n";

void setup() {
    Serial.begin(115200);  // Set the baud rate to match Python script

    // Initialize LittleFS
}

void loop() {
    // Check if data is available to read
    if (Serial.available()) {
        String inputString = Serial.readStringUntil('\n');  // Read until newline


        

        char command;
        char code[20];  // Array to hold button/axis code
        int value;

        // Use sscanf to parse the input
        if (sscanf(inputString.c_str(), "%c %s %d", &command, code, &value) == 3) {
          
            // Print to Serial Monitor
            if (command == 'B') {
                Serial.print("Button ");
                Serial.print(code);
                Serial.print(" state: ");
                Serial.println(value);
            } else if (command == 'A') {
                Serial.print("Axis ");
                Serial.print(code);
                Serial.print(" value: ");
                Serial.println(value);
            } else if (command == 'D') {
                Serial.print("D-pad ");
                Serial.print(code);
                Serial.print(" state: ");
                Serial.println(value);
            }

            // Recreate the input string for storage
            // String formattedString = String(command) + " " + String(code) + " " + String(value) +"\n"  ;
            // sum += formattedString ;
            // Serial.println(sum);
            
        } else {
            Serial.println("Invalid input format");
        }
    }
}







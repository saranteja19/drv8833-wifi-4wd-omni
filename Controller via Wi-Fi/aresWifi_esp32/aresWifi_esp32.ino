//.py script send data
// This code reads recived input data of controller to (ESP32) arduino IDE via WiFi.

#include <WiFi.h>  // Use WiFi library for ESP32

// Wi-Fi credentials (replace with your own SSID and password)
const char* ssid = "BSNL FIBERNET_15";  // Replace with your Wi-Fi SSID
const char* password = "08816221515";   // Replace with your Wi-Fi password

WiFiServer server(80);  // Create a server on port 80 for HTTP communication

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud rate
  delay(1000);
  Serial.println("start");  // Print start message

  // Connect to the Wi-Fi network using provided credentials
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

void loop() {
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

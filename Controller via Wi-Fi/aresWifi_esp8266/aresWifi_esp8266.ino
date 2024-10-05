//.py script send data
// This code reads recived input data of controller to (ESP8266) arduino IDE via WiFi.

#include <ESP8266WiFi.h>

const char* ssid = "BSNL FIBERNET_15";  // Replace with your Wi-Fi SSID
const char* password = "08816221515";   // Replace with your Wi-Fi password

WiFiServer server(80);  // Create a server on port 80

void setup() {
  Serial.begin(115200);  // Initialize serial
  //Serial.setDebugOutput(true);
  delay(1000);
  Serial.println("start");
  //Serial.println(ESP.getResetReason());

  WiFi.begin(ssid, password);  // Connect to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.begin();  // Start the server
  Serial.println("Server started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());      // Print IP address
  WiFi.setSleepMode(WIFI_NONE_SLEEP);  // Disable Wi-Fi power saving mode
}

void loop() {
  WiFiClient client = server.accept();  // Use accept() instead of available()
  client.setTimeout(5000);

  if (client) {
    Serial.println("New Client connected");
    String currentLine = "";  // Store current line of incoming data

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();  // Read data from the client
        if (c == '\n') {         // If the entire line has been read
          // Process data here
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("ESP8266 Web Server");
            client.println();
            break;
          } else {
            Serial.println(currentLine);  // Print to serial monitor
            processData(currentLine);
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;  // Add character to the current line
        }
      }
      yield();
    }
    client.stop();  // Close the connection
    Serial.println("Client disconnected");
  }
}

void processData(String data) {
  char command;
  char code[20];  // Array to hold button/axis code
  int value;

  // Use sscanf to parse the input
  if (sscanf(data.c_str(), "%c %s %d", &command, code, &value) == 3) {

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

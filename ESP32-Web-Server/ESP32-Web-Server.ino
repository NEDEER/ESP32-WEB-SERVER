#include <WiFi.h>

// Network credentials
const char* ssid     = "NEDEER";
const char* password = "00009999";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variables to store the current LED states
String statePin33 = "off";
String statePin34 = "off";

// Output variables to GPIO pins
const int ledPin33 = 33;
const int ledPin34 = 22;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin33, OUTPUT);      // Set the LED pin mode
  digitalWrite(ledPin33, LOW);    // Turn LED off by default
  pinMode(ledPin34, OUTPUT);      // Set the LED pin mode
  digitalWrite(ledPin34, LOW);    // Turn LED off by default

  WiFi.softAP(ssid, password);

  // Print IP address and start web server
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // Print a message out in the serial port
    String currentLine = "";                // Make a String to hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      // Loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // If there's bytes to read from the client
        char c = client.read();             // Read a byte, then
        Serial.write(c);                    // Print it out the serial monitor
        header += c;
        if (c == '\n') {                    // If the byte is a newline character
          // If the current line is blank, you got two newline characters in a row.
          // That's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Turn the GPIOs on and off
            if (header.indexOf("GET /33/on") >= 0) {
              statePin33 = "on";
              digitalWrite(ledPin33, HIGH);               // Turn the LED on
            } else if (header.indexOf("GET /33/off") >= 0) {
              statePin33 = "off";
              digitalWrite(ledPin33, LOW);                // Turn the LED off
            }
            
            if (header.indexOf("GET /34/on") >= 0) {
              statePin34 = "on";
              digitalWrite(ledPin34, HIGH);               // Turn the LED on
            } else if (header.indexOf("GET /34/off") >= 0) {
              statePin34 = "off";
              digitalWrite(ledPin34, LOW);                // Turn the LED off
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: monospace; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: red; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button2 { background-color: blue; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}");
            client.println(".button3 { background-color: gray; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 32px; margin: 2px; cursor: pointer;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<p>Control LED State</p>");

            if (statePin33 == "off") {
              client.println("<p><a href=\"/33/on\"><button class=\"button\">LED 1 ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/33/off\"><button class=\"button3\">LED 1 OFF</button></a></p>");
            }
            if (statePin34 == "off") {
              client.println("<p><a href=\"/34/on\"><button class=\"button2\">LED 2 ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/34/off\"><button class=\"button3\">LED 2 OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // If you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // If you got anything else but a carriage return character
          currentLine += c;      // Add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

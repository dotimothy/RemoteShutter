#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "htmls.h"


/* ESP32RemoteShutter.ino: A Wireless DSLR Remote Shutter using the ESP32 */
/* Author: Timothy Do */

const char* ssid = "ESP32-RemoteShutter";  // Name of the ESP Network 
const char* password = NULL; // Set it to Any Password Your Like, NULL is open network

const int capturePin = 32; // The Pin that is Connected to the Relay of the Remote Shutter

DNSServer dnsServer;
WebServer server(80);

void handleRoot() { /* Serves Home Page */
  Serial.println("Homepage Requested");
  digitalWrite(capturePin,LOW);
  server.send(200,"text/html",index_html); 
}

void redirectToHome() {
    server.sendHeader("Location","/");
    server.send(302, "text/html", ""); 
}

void handleOn() {  /* Handles On */
  digitalWrite(capturePin,HIGH);
  server.send(200,"text/html",on_html);
}

void handleOff() {  /* Handles Off */
  digitalWrite(capturePin,LOW);
  server.send(200,"text/html",off_html);
}

void setup() {
  // setting up ESP32
  pinMode(capturePin, OUTPUT); 
  digitalWrite(capturePin, LOW); 
  Serial.begin(115200);

  // Starting the Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP(); // Listing the IP if Serial Monitor is Connected 
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Starting the DNS Server
  dnsServer.start(53,"*",IP); 

  // Starting the Web Server
  server.on("/", handleRoot); 
  server.on("/on.html",handleOn);
  server.on("/on",handleOn);
  server.on("/off.html",handleOff);
  server.on("/off",handleOff);
  server.onNotFound(redirectToHome);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
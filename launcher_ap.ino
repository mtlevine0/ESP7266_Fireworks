/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

#define ARMCODE 8675309

/* Set these to your desired credentials. */
const char *ssid = "SpamESP";
const char *password = "baseball";

int dataPin = 16;
int latchPin = 5;
int clockPin = 4;
int armPin = 0;
int launchCode = 0xff;
bool armedStatus = false;
bool powerStatus = false;

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}

void handleLaunch() {
  int inputValue = server.arg(0).toInt();
  if(inputValue < 8 && inputValue >= 0){
    launchCode = ~(0x01 << inputValue);
    Serial.println(launchCode);  
      
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, launchCode);
    digitalWrite(latchPin, HIGH);
    // Burn delay, then turn the relays off again
    delay(1000);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 0xff);
    digitalWrite(latchPin, HIGH);
    
    server.send(200, "text/html", "true");
  }else{
    server.send(400, "text/html", "false");
  }
}

void handleArm(){
  int armCodeVar = server.arg(0).toInt();
  if(armCodeVar == ARMCODE){
     powerStatus = true;
     server.send(200, "text/html", "true");
  }else{
    powerStatus = false;
    server.send(400, "text/html", "false");
  }
}

void handlePower(){
  int powerCodeVar = server.arg(0).toInt();
  if(powerCodeVar == 1 && powerStatus == true){
    server.send(200, "text/html", "true, on");
    digitalWrite(armPin, HIGH);
  }else if(powerCodeVar == 0){
    server.send(200, "text/html", "true, off");
    digitalWrite(armPin, LOW);
  }else{
    server.send(400, "text/html", "false");
    digitalWrite(armPin, LOW);
  }
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val){
  uint8_t i;
  for (i = 0; i < 8; i++)  {
    if(bitOrder == LSBFIRST){
      digitalWrite(dataPin, !!(val & (1 << i)));
    }else{      
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    }      
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);            
  }
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(armPin, LOW);
  
	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);
  server.on("/launch", handleLaunch);
  server.on("/arm", handleArm);
  server.on("/power", handlePower);
	server.begin();
	Serial.println("HTTP server started");

  // Relays are active low, set them high so we don't go early.
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, 0xff);
  digitalWrite(latchPin, HIGH);
}

void loop() {
	server.handleClient();
}


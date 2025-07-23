// By Hailey Kaylor for Jeremy Musson at Pinyon Environmental

// Include libraries
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "RTClib.h"

// Initialize byte vectors for holdindg address and distance
byte GET_ADDRESS[8] = {0xFF, 0x03, 0x02, 0x00, 0x00, 0x01, 0x90, 0x6C};
byte GET_DISTANCE[8] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x85, 0xF6};
byte distanceData[2] = {0x00, 0x00};
int distance;

// Initialize array to hold distance data for 24 hours
int dataArray[300][7];

// Initialize RTC
RTC_DS3231 rtc;
// Create vector for days of the week
char daysOfTheWeek[7][12] = {
  "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"
};

// Define pins on ESP8266 and set them to SoftwareSerial
#define RX_PIN D5
#define TX_PIN D6
SoftwareSerial sensorSerial(RX_PIN, TX_PIN);
#define TRN_PIN D0

// WiFi credentials
const char* ssid     = "moto g pure_7147";
const char* password = "3204e336344b";

// How long to wait until request times out
#define TIMEOUT_DELAY 5000

// Set lasttime
unsigned long lasttime = 0;

// Define ESP8266 server
ESP8266WebServer server(80);

// Function to send data to the sensor
void sendData(String note, byte *data)
{
    // Set lasttime
    lasttime = millis();

    // Enables ESP to write to sensor
    digitalWrite(TRN_PIN, 1);

    // Print address from sensor
    Serial.print(note);
    for (int i = 0; i < 8; i++)
    {
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    Serial.flush();
    sensorSerial.write(data, 8);
    sensorSerial.flush();

    // Stop writing to sensor
    digitalWrite(TRN_PIN, 0);

    // Wait 10ms
    delay(10);
}

// Function to receive data from the sensor
void receiveData()
{
    // Set bytes to 0
    byte data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // While sensor is unavailable and has no data to transmit
    while (sensorSerial.available() == 0)
    {
        // If lasted too long unavailable, the request times out
        if (millis() - lasttime > TIMEOUT_DELAY)
        {
            Serial.println("Request Timed Out...");
            break;
        }
    }

    // While sensor has data to transmit
    if (sensorSerial.available() > 0)
    {
        // Receive and read bytes from sensor
        sensorSerial.readBytes(data, 7);
        // Print bytes
        for (int i = 0; i < 7; i++)
        {
            Serial.print(data[i], HEX);
            Serial.print(" ");
            // Transfer 4th and 5th bytes to another vector
            // These are the bytes with the distance data
            if (i == 3 || i == 4) {
                distanceData[i-3] = data[i];
            }
        }
        Serial.println();

        // Convert distance data from hex to decimal
        distance = (distanceData[0] * 256) + distanceData[1];
        // Print distance data
        Serial.print(distance);
        Serial.println();
    }
    // Wait 10ms
    delay(10);
}

// Code to run on setup
void setup()
{
    // Begin serial with baud rate 115,200
    Serial.begin(115200);
    // Begin sensor serial with baud rate 9,600
    sensorSerial.begin(9600);

    // Set pin modes and begin program
    pinMode(TRN_PIN, OUTPUT);
    Serial.println("\nBegin Program...");

    // Connect to WiFi and print ESP IP address
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Wi-Fi connected, IP = ");
    Serial.println(WiFi.localIP());

    // Registers HTTP-endpoint on ESP
    server.on("/", [](){
      server.send(200, "text/plain", String(distance));
    });

    // Start the server
    server.begin();
    Serial.println("HTTP server started");

    // Registers 2nd HTTP-endpoint on ESP to talk to the Pi
    server.on("/data", []() {
        // Initialize Json rows for a CSV file
        StaticJsonDocument<4096> doc;
        JsonArray rows = doc.createNestedArray("readings");

        // Move data to Json
        for (int i = 0; i < 300; i++) {
            JsonArray row = rows.createNestedArray();
            for (int j = 0; j < 7; j++) {
                row.add(dataArray[i][j]);
            }
        }

        // Send data out to the Pi when called
        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    });

    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1) delay(10);
    }
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(2025, 1, 1, 1, 1, 0));
    }

    // Get ESP address
    sendData("Getting Address: ", GET_ADDRESS);

    // Receive data from the sensor
    receiveData();
}

// Function to run repeatedly
void loop()
{
    // Print distance from sensor
    sendData("Getting Distance: ", GET_DISTANCE);
    receiveData();

    // Handle the client
    server.handleClient();

    // Initialize variables for current date and time
    DateTime now = rtc.now();
    uint32_t unixt = now.unixtime();
    uint8_t dow   = now.dayOfTheWeek();

    // Store in a circular buffer of 300 positions
    static int idx = 0;
    dataArray[idx][0] = distance;
    dataArray[idx][1] = now.year();
    dataArray[idx][2] = now.month();
    dataArray[idx][3] = now.day();
    dataArray[idx][4] = now.hour();
    dataArray[idx][5] = now.minute();
    dataArray[idx][6] = now.second();
    idx = (idx + 1) % 300;  

    // Print the timestamp
    char tbuf[64];
    snprintf(tbuf, sizeof(tbuf),
        "%s, %04u-%02u-%02u %02u:%02u:%02u",
        daysOfTheWeek[dow],
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second()
    );
    Serial.println(tbuf);

    // Wait a little less than 5 minutes
    // Accounting for delay in sensor to transmit to ESP and ESP to receive it
    delay(290000);
}

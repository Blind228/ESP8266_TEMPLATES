/*
  Wemos D1 Mini code template for MQTT and OTA projects.
*/
#include "secrets.h"  //For Credentials
#include <ESP8266WiFi.h>  //For ESP8266
#include <PubSubClient.h> //For MQTT
#include <ESP8266mDNS.h>  //For OTA
#include <WiFiUdp.h>      //For OTA
#include <ArduinoOTA.h>   //For OTA

// Update the "secrets.h" file with values suitable for your network.
// WIFI
const char* wifi_ssid = SECRET_WIFI_SSID;
const char* wifi_passwd = SECRET_WIFI_PASSWD;
// MQTT
const char* mqtt_server = SECRET_MQTT_SERVER;
const char* mqtt_user = SECRET_MQTT_USER;
const char* mqtt_passwd = SECRET_MQTT_PASSWD;

const char* mqtt_client_id = "ESP8266-"; // ClientID on MQTT
const char* outTopic = "/confirmation"; // Confirmation Topic
const char* inTopic = "/test";  // Choose topic to subscribe to.

// OTA
const char* ota_passwd = SECRET_OTA_PASSWD;

// MQTT client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Necesary to make Arduino Software autodetect OTA device
WiFiServer TelnetServer(8266);

//Variables Definition
//Built-in LED on Wemos D1 Mini is GPIO2 (D4)
// const int ledPin = 4;
// const int relayPin = 4;


// USER DEFINED SETUP
void user_setup() {   // ****************INSERT SETUP COMMANDS HERE********************
  // Use this function to define pins and other setup commands.
  pinMode(LED_BUILTIN, OUTPUT); // Initialise builtin LED.
  digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off by making the voltage HIGH

}

// USER DEFINED LOGIC
void user_loop() {    // ****************INSERT LOOP COMMANDS HERE*********************

  // Blink test
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW turns the LED on for the ESP-8266)
  delay(1000);                  // Wait for 1 seconds
  digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off by making the voltage HIGH
  delay(5000);                  // Wait for 5 seconds

  // MQTT comunications HERE
  // mqtt_client.publish ("/topic", "payload");
  // ANY LOGIC ACTIVATED BY RECEIVED MQTT PAYLOADS FROM SUBSCRIBED
  // TOPICS MUST BE IMPLEMENTED IN TH CALLBACK FUNCTION.(SEE BELOW)
}

// Callback function for MQTT. This is used to receive messages
// on subscribed topics. Not needed if not subsrcibed to any topics.
void callback(char* topic, byte* payload, unsigned int length) {
  // Conver the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("], ");
  Serial.println(message);

  if (message == "test") {  // Define what message is the trigger.
    mqtt_client.publish(outTopic, "MQTT message received!");
  }
}

// ---------Past this point no modification is needed.--------------

// INITIAL SETUP.
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to connect.
  }
  Serial.println("\r\nBooting...");

  setup_wifi(); // Connect to WIFI.

  ota_setup();  // Setup OTA.

  mqtt_setup(); // Setup MQTT.

  user_setup(); // User setup.

  Serial.println("Setup completed! Running app...");
}


// MAIN LOOP.
void loop() {

  ArduinoOTA.handle();

  if (!mqtt_client.connected()) {
    mqtt_reconnect(); // If MQTT connection is lost, reconnect.
  }

  mqtt_client.loop();

  user_loop();
}


// ------------FUNCTIONS------------------------

// WIFI SETUP FUNCTION.
void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_passwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// MQTT SETUP FUNCTION.
void mqtt_setup() {
  Serial.println("Configuring MQTT server...");

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  Serial.printf("   Server IP: %s\r\n", mqtt_server);
  Serial.printf("   Username:  %s\r\n", mqtt_user);
  Serial.printf("   Cliend Id: %s\r\n", mqtt_client_id);
  Serial.println("   MQTT setup completed.");
}


// MQTT RECONNECTION FUNCTION.
void mqtt_reconnect() {
  while (!mqtt_client.connected()) {  // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (mqtt_client.connect(mqtt_client_id, mqtt_user, mqtt_passwd)) {
      mqtt_client.publish(outTopic, mqtt_client_id);  // Publish ID to show we're connected.
      mqtt_client.subscribe(inTopic); // Re-Subscribe.
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// OTA SETUP FUNCTION.
void ota_setup() {
  Serial.print("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device
  ArduinoOTA.onStart([]() {
    Serial.println("OTA starting...");
  });
  ArduinoOTA.setPassword(ota_passwd);
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update finished!");
    Serial.println("Rebooting...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA in progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("   OTA setup completed.");
}

/* 


*/

#include <ESP8266WiFi.h>  //For ESP8266
#include <PubSubClient.h> //For MQTT
#include <ESP8266mDNS.h>  //For OTA
#include <WiFiUdp.h>      //For OTA
#include <ArduinoOTA.h>   //For OTA

//WIFI configuration
#define wifi_ssid "wifi_ssid"
#define wifi_password "wifi_password"

// OTA configuration
#define ota_password "ota_password"

//MQTT configuration
#define mqtt_server "255.255.255.255" // Internal IP adress of your MQTT broker.
#define mqtt_user "mqtt_user"        // If you don't want to use a username and password for MQTT see the
#define mqtt_password "mqtt_password"  // comments in the "void mqtt_reconnect()" function lower in the code.
#define use_chip_id true // Set to false to remove chip ID from "mqtt_client_id".
String mqtt_client_id = "ESP8266-"; //This text is concatenated with ChipId if previous line set to true.

// MQTT client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Necesary to make Arduino Software autodetect OTA device
WiFiServer TelnetServer(8266);

// USER DEFINED SETUP
void setup_variables() {    // ****************INSERT SETUP COMMANDS HERE********************
  
  pinMode(LED_BUILTIN, OUTPUT); // Initialise builtin LED.
  
}

// USER DEFINED LOGIC
void setup_loop() {         // ****************INSERT LOOP COMMANDS HERE*********************
  
  // Blink test
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW turns the LED on for the ESP-8266)
  delay(1000);                  // Wait for 1 seconds
  digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off by making the voltage HIGH
  delay(5000);                  // Wait for 5 seconds

  // MQTT comunications
  // mqtt_client.publish ("/topic", "payload"); 
}


// INITIAL SETUP LOOP.
void setup() {
  Serial.begin(9600);
  while (!Serial) {delay(10);} // wait for serial port to connect.
  Serial.println("\r\nBooting...");

  setup_wifi(); // Connect to wifi.

  setup_variables(); // User function.

  Serial.print("Configuring OTA device...");
  TelnetServer.begin();   //Necesary to make Arduino Software autodetect OTA device
  ArduinoOTA.onStart([]() {
    Serial.println("OTA starting...");
  });
  ArduinoOTA.setPassword(ota_password);
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
  Serial.println("OK");

  Serial.println("Configuring MQTT server...");

if (use_chip_id == true) {
   mqtt_client_id = mqtt_client_id + ESP.getChipId();
}
else {
  mqtt_client_id = mqtt_client_id;
}
  
  mqtt_client.setServer(mqtt_server, 1883);
  Serial.printf("   Server IP: %s\r\n", mqtt_server);
  Serial.printf("   Username:  %s\r\n", mqtt_user);
  Serial.println("   Cliend Id: " + mqtt_client_id);
  Serial.println("   MQTT configured!");

  Serial.println("Setup completed! Running app...");
}

// MAIN LOOP.
void loop() {

  ArduinoOTA.handle();

  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
  
  setup_loop();
}



// SETUP_WIFI FUNCTION.
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("OK");
  Serial.print("   IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT RECONNECTION FUNCTION.
void mqtt_reconnect() {
  while (!mqtt_client.connected()) {  // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (mqtt_client.connect(mqtt_client_id.c_str(), mqtt_user, mqtt_password)) {
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

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include <PubSubClient.h>

#include <SoftwareSerial.h>
#include "Sds011.h"


//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_token[34] = "12345678";




// media su 20 campioni a cui vengono tolti il minimo e il massimo valore
static const int SAMPLES = 20;

//5 minuti standby
static const long STAND_BY = 1000;
// RX, TX
SoftwareSerial mySerial(7, 8);
sds011::Sds011 sensor(mySerial);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;
void setup_wifi() {
  delay(10);
  // Connessione al network WiFi
  Serial.println();
  Serial.print("Connecting ... ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // LED on se si riceve ‘1’ come primo carattere.
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW); // LED on quando il voltaggio è LOW
  } else {
    digitalWrite(BUILTIN_LED, HIGH); // LED off
  }
}
void reconnect() {
  // Ritentiamo finchè non siamo connessi
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Tentativo di connessione
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Una volta connessi pubblichiamo un messaggio
      client.publish("outTopic", "hello world");
      // ... e ci sottoscriviamo nuovamente ai topic
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 5 secondi d’attesa prima di un nuovo tentativo
      delay(5000);
    }
  }
}




//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_token, json["mqtt_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_token("mqtt", "mqtt token", mqtt_token, 32);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_token);

  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_token, custom_mqtt_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_token"] = mqtt_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());


  pinMode(BUILTIN_LED, OUTPUT); // Inizializziamo il BUILTIN_LED come un output
  setup_wifi();

  unsigned int port = atoi (mqtt_port);
  Serial.println("mqtt_port:" + port );

  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  mySerial.begin(9600);
  sensor.set_sleep(false);
  sensor.set_mode(sds011::QUERY);

}

void loop() {

  float pm25, pm10;
  bool ok;

  sensor.set_sleep(false);
  delay(1000);
  ok = sensor.query_data_auto(&pm25, &pm10, SAMPLES);
  sensor.set_sleep(true);
  pm25 = float(pm25) / 10;
  pm10 = float(pm10) / 10;

  // text display tests
  Serial.print("Pm2.5:");
  Serial.print(String(pm25));
  Serial.print("  Pm10 :");
  Serial.println(String(pm10));



  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  String dataString;
  dataString =  String(pm25) + ";" + String(pm10);
  char charBuf[50];
  dataString.toCharArray(charBuf, 50);

  client.publish("outTopic", charBuf);




  delay(STAND_BY);

}



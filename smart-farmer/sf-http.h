#include <SPI.h>
#include <EthernetENC.h>

class SmartFarmerHttp {
  public:
    void begin() {
      ip = new IPAddress(192, 168, 5, 5);
      server = new EthernetServer(HTTP_PORT);


      Serial.println("[ETHR] Initialize Ethernet with DHCP:");
      if (Ethernet.begin(mac) == 0) {
        Serial.println("[ETHR] Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println("[ETHR] Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          while (true) {
            delay(1); // do nothing, no point running without Ethernet hardware
          }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("[ETHR] Ethernet cable is not connected.");
        }
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(mac, *ip);
      } else {
        Serial.print("[ETHR] DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
      }

      server->begin();
    }

    void run() {
      EthernetClient client = server->available();
      if (client) {
        boolean currentLineIsBlank = true;
        String readString = "";
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            readString += c;

            if (c == '\n' && currentLineIsBlank) {
              //              Serial.println(readString);

              const byte firstSpace = readString.indexOf(" ");
              const byte secondSpace = readString.indexOf(" ", firstSpace + 1);

              String httpFirstLine = readString.substring(0, readString.indexOf("\n"));
              String path = httpFirstLine.substring(firstSpace + 1, secondSpace);

              if (path.indexOf("/sensors") >= 0) {
                StaticJsonDocument<JSON_OBJECT_SIZE(10)> actualAgroclimateData;

                actualAgroclimateData["ph"]                   = sf_sensors.getPH();
                actualAgroclimateData["light_intensity"]      = sf_sensors.getLightIntensity();
                actualAgroclimateData["nutrient_flow"]        = sf_sensors.getNutrientFlow();
                actualAgroclimateData["tds"]                  = sf_sensors.getTDS();
                actualAgroclimateData["ec"]                   = sf_sensors.getEC();

                String JSONOutput = "";
                serializeJson(actualAgroclimateData, JSONOutput);
                sendData(JSONOutput.c_str(), client);
              }

              else if (path.indexOf("/light/0") >= 0) {
                sf_actuators.setGrowthLight(false);
                Serial.println("[CORR] The growth lamp configuration saved to off");
                sendData("The growth lamp configuration saved to off", client);
              }

              else if (path.indexOf("/light/1") >= 0) {
                sf_actuators.setGrowthLight(true);
                Serial.println("[CORR] The growth lamp configuration saved to on");
                sendData("The growth lamp configuration saved to on", client);
              }

              else if (path.indexOf("/config") >= 0) {
                Serial.println("[HTTP] HTTP POST received to \"config\" endpoint with body:");
                String postData = "";
                while (client.available()) {
                  char c = client.read();
                  postData += c;
                }

                const byte keyCount = 3;
                int separator[keyCount];

                const int capacity = JSON_OBJECT_SIZE(10);
                StaticJsonDocument<capacity> receivedConfig;

                receivedConfig["message"] = "The configuration has been saved.";

                for (int i = 0; i < keyCount; i++) {
                  String singlePostData = "";
                  if ( i == 0 ) {
                    separator[i] = postData.indexOf('&');
                    singlePostData = postData.substring(i, separator[i]);
                  } else {
                    separator[i] = postData.indexOf('&', separator[i - 1] + 1);
                    singlePostData = postData.substring(separator[i - 1] + 1, separator[i]);
                  }

                  String key = singlePostData.substring(0, singlePostData.indexOf("="));
                  float value = singlePostData.substring(singlePostData.indexOf("=") + 1).toFloat();

                  receivedConfig["config"][key] = value;

                  Serial.print("[HTTP] - ");
                  Serial.print(key);
                  Serial.print(": ");
                  Serial.println(value);
                }

                if ( receivedConfig["config"].containsKey("ph") &&
                     receivedConfig["config"].containsKey("light_intensity") &&
                     receivedConfig["config"].containsKey("nutrient_flow") ) {
                  float ph              = receivedConfig["config"]["ph"];
                  float light_intensity = receivedConfig["config"]["light_intensity"];
                  float nutrient_flow   = receivedConfig["config"]["nutrient_flow"];

                  Serial.println("[CONF] The settings from network has been saved!");

                  if (sf_actuators.setPh(ph)) {
                    Serial.print  ("[CONF] - pH\t\t : ");
                    Serial.println(ph);
                  }

                  if (sf_actuators.setLightIntensity(light_intensity)) {
                    Serial.print  ("[CONF] - light intensity : ");
                    Serial.println(light_intensity);
                  }

                  if (sf_actuators.setNutrientFlow(nutrient_flow)) {
                    Serial.print  ("[CONF] - nutrient flow\t : ");
                    Serial.println(nutrient_flow);
                  }
                } else {
                  Serial.println("[CONF] The received configuration has missing key(s).");
                }

                String serializedConfig = "";
                serializeJson(receivedConfig, serializedConfig);
                sendData(serializedConfig.c_str(), client);
                break;
              }

              else {
                Serial.println("[HTTP] Client connected to unconfigured endpoint.");
                sendNotFound(client);
              }
              break;
            }
            if (c == '\n') currentLineIsBlank = true; else if (c != '\r') currentLineIsBlank = false;
          }
        }
        client.stop();
      }
    }
  private:
    IPAddress* ip;
    EthernetServer* server;

    void sendData(const char* data, EthernetClient client) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(data);
    }

    void sendNotFound(EthernetClient client) {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println("You have lost!");
    }
};

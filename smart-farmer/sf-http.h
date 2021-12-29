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
        lcdPrint("Static IP", IpAddress2String(Ethernet.localIP()));
      } else {
        Serial.print("[ETHR] DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
        lcdPrint("DHCP IP", IpAddress2String(Ethernet.localIP()));
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

              if ( path == "/" ) {
                String html = "<h2>Welcome to the Smart Farming Microcontroler API!</h2>";
                html += "<h4>Go to the a link below to do some action ;)</h4>";
                html += "<ul>";
                html += "<li><a href=\"/sensors\">Check sensor reading value</a></li>";
                html += "<li><a href=\"/light/1\">Turn on the growth light</a></li>";
                html += "<li><a href=\"/light/0\">Turn off the growth light</a></li>";
                html += "</ul>";
                html += "<h4>Set the parameters setpoint below.</h4>";
                html += "<form action=\"/config\" method=\"post\">";
                html += "<label for=\"ph\">pH:</label><br/>";
                html += "<input type=\"text\" id=\"ph\" name=\"ph\"><br/>";
                html += "<label for=\"lightintensity\">Light intensity:</label><br/>";
                html += "<input type=\"text\" id=\"lightintensity\" name=\"light_intensity\"><br/>";
                html += "<label for=\"nutrientflow\">Nutrient flow:</label><br/>";
                html += "<input type=\"text\" id=\"nutrientflow\" name=\"nutrient_flow\"><br/><br/>";
                html += "<input type=\"submit\" value=\"Submit\">";
                html += "</form>";
                html += "<h5>Copyright &copy; 2021 Fikri Rida P.</h5>";

                sendHTML(html.c_str(), client);
              }

              else if (path == "/sensors") {
                StaticJsonDocument<JSON_OBJECT_SIZE(10)> actualAgroclimateData;

                actualAgroclimateData["ph"]                   = sf_sensors.getPH();
                actualAgroclimateData["light_intensity"]      = sf_sensors.getLightIntensity();
                actualAgroclimateData["nutrient_flow"]        = sf_sensors.getNutrientFlow();
                actualAgroclimateData["tds"]                  = sf_sensors.getTDS();
                actualAgroclimateData["ec"]                   = sf_sensors.getEC();

                String JSONOutput = "";
                serializeJson(actualAgroclimateData, JSONOutput);
                sendData(JSONOutput.c_str(), client);
                lcdPrint("Sensors data", "sent!");
              }

              else if (path == "/light/0") {
                sf_actuators.setGrowthLight(false);
                Serial.println("[CORR] The growth lamp configuration saved to off");
                sendHTML("The growth lamp configuration saved to off", client);
                lcdPrint("Growth lamp", "turned off!");
              }

              else if (path == "/light/1") {
                sf_actuators.setGrowthLight(true);
                Serial.println("[CORR] The growth lamp configuration saved to on");
                sendHTML("The growth lamp configuration saved to on", client);
                lcdPrint("Growth lamp", "turned on!");
              }

              else if (path == "/config") {
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

                  Serial.println("[CONF] Saving the settings from network...");

                  StaticJsonDocument<JSON_OBJECT_SIZE(10)> savedConfig;
                  savedConfig["message"] = "The configuration has been saved.";

                  if (sf_actuators.setPh(ph)) {
                    Serial.print  ("[CONF] - pH\t\t : ");
                    Serial.println(ph);
                    savedConfig["config"]["ph"] = ph;
                    #ifdef SAVE_CONFIG_EEPROM
                      sf_eeprom.savePh(ph);
                    #endif
                  }

                  if (sf_actuators.setLightIntensity(light_intensity)) {
                    Serial.print  ("[CONF] - light intensity : ");
                    Serial.println(light_intensity);
                    savedConfig["config"]["light_intensity"] = light_intensity;
                    #ifdef SAVE_CONFIG_EEPROM
                      sf_eeprom.saveLightIntensity(light_intensity);
                    #endif
                  }

                  if (sf_actuators.setNutrientFlow(nutrient_flow)) {
                    Serial.print  ("[CONF] - nutrient flow\t : ");
                    Serial.println(nutrient_flow);
                    savedConfig["config"]["nutrient_flow"] = nutrient_flow;
                    #ifdef SAVE_CONFIG_EEPROM
                      sf_eeprom.saveNutrientFlow(nutrient_flow);
                    #endif
                  }

                  Serial.println("[CONF] The new configuration has been saved!");

                  lcdPrint("New config", "saved!");
                  String serializedConfig = "";
                  serializeJson(savedConfig, serializedConfig);
                  sendData(serializedConfig.c_str(), client);
                } else {
                  Serial.println("[CONF] The received configuration has missing key(s).");
                  sendHTML("Failed to save, the received parameter setpoints were not complete", client);
                }
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
      switch (Ethernet.maintain()) {
        case 1:
          //renewed fail
          Serial.println("[ETHR] Error: renewed fail");
          break;

        case 2:
          //renewed success
          Serial.print("[ETHR] Renewed success, my IP address: ");
          Serial.println(Ethernet.localIP());
          break;

        case 3:
          //rebind fail
          Serial.println("[ETHR] Error: rebind fail");
          break;

        case 4:
          //rebind success
          Serial.print("[ETHR] Rebind success, my IP address: ");
          Serial.println(Ethernet.localIP());
          break;

        default:
          //nothing happened
          break;
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

    void sendHTML(const char* data, EthernetClient client) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<html><head><title>Microcontroller API</title></head><body>");
      client.println(data);
      client.println("</body></html>");
    }

    void sendNotFound(EthernetClient client) {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println("You have lost!");
    }
};

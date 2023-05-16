#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <TimeLib.h>
#include <array>

ESP8266WiFiMulti WiFiMulti;

const uint8_t mapListSize = 4;
const int mapRows = 6;
const int mapCols = 6;

// const char *monitorServerIP = "192.168.43.176";
const char *phoneSSID = "RT-GPON-E150";
const char *phonePASSWORD = "5qEg26eS";
const char *universalPassword = "123456789";
const char *messageType[] {
  "",
  "/handshake",
  "/setconfig",
  "/getmap",
  "/maptomonitor",
  "/printmap"
};

const String serverURL = "http://192.168.0.2/mapTest.php";

std::array<String, mapListSize> mapList;
std::array<int, mapListSize> unitSteps;

/*
std::array<int, mapListSize> unitID = {
  0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39
};
*/
std::array<int, mapListSize> unitID = {
  20, 21, 22
};

enum ResponseCode {
   CODE_SUCCESS = 2000,
  CODE_FAR_AWAY = 2001,
  CODE_ROOT_SUCCESS = 2055,
  CODE_ROOT_HAS_TIME = 2056,
  CODE_INVALID_ARGS = 3000,
  CODE_ERROR = 4000,
  CODE_NOT_FOUND = 4004
};

String makeIP(uint8_t ID) {
  String IP = "192.168.0.";
  IP += String(ID + 100);
  return IP;
}

String makeURL(uint8_t unitID, const char *messageType) {
  String URL = "http://" + makeIP(unitID) + messageType;
  return URL;
}

String makeConfigMessage() {
  uint8_t ID = idGenerator();
  //Serial.println(ID);
  if (ID != 0) {
    String setTimeMessage = "?TIME=" + String(now()) + "&ID=" + String(ID) + "&TURN=" + String(turnGenerator(ID));
    return setTimeMessage;
  }
  return "";
}

String makeServerMessage(uint8_t id, uint16_t steps, const String &map) {
  String message;
  message = "?ID=" + String(id) + "&STEPS=" + String(steps) + "&MAP=" + map;
  return message;
}

uint8_t idGenerator() {
  for (uint8_t i = 0; i < mapList.size(); i++) {
    if (mapList[i].isEmpty()) {
      return i + 20;
    }
  }
  return 0;
}

uint8_t turnGenerator(uint8_t ID) {
  return (ID - 20) % 3 + 1;
}

void setClientConfig() {
  WiFi.mode(WIFI_STA);
}

wl_status_t setUnitConnection(uint8_t ID) {
  Serial.println("Wait ID ------------------>" + String(ID));
  WiFi.begin(String(ID).c_str(), universalPassword);
  delay(4000);
  return WiFi.status();
}

wl_status_t setServerConnection() {
  WiFi.begin(phoneSSID, phonePASSWORD);
  delay(6000);
  return WiFi.status();
}

bool handleMapResponse(const String &mapResponse, uint8_t ID) {
  Serial.println("Map length --------------------->" + String(mapResponse.length()));
  if (mapResponse.length() >= mapCols * mapRows) {
    String  map,
            steps;

    size_t counter = 0;

    Serial.println("[CLIENT] [HTTP] [GET] new map from ID:" + String(ID));
    for (char responseChar: mapResponse) {
      if (counter < mapCols * mapRows ) {
        map += responseChar;
      } else {
        steps += responseChar;
      }
      counter++;
    }

    Serial.println("[CLIENT] [HTTP] [GET] map:" + map);
    Serial.println("[CLIENT] [HTTP] [GET] steps:" + steps);

    mapList[ID - 20] = map;
    unitSteps[ID - 20] = steps.toInt();
    return true;
    
  } else {
    Serial.println("[CLIENT] [HTTP] [GET] Error from ID:" + String(ID) + ": MAP NOT VALID...");
    return false;
  }
}

uint16_t sendUnitConfig() {
  HTTPClient http;
  WiFiClient client;

  uint8_t defaultID = 0;

  http.begin(client, makeURL(defaultID, messageType[2]) + makeConfigMessage());

  Serial.println("[CLIENT] [HTTP] [GET] set time request to: " + makeURL(defaultID, messageType[2]) + makeConfigMessage() + "\n");
  uint16_t httpCode = http.GET();

  if (httpCode != CODE_ROOT_HAS_TIME) {
    uint8_t ID = idGenerator();
    String mapResponse = http.getString();
    handleMapResponse(mapResponse, ID);

    Serial.println("[CLIENT] [HTTP] [GET] new map: " + mapList[ID - 20]);
  } else {
    Serial.println("[CLIENT] [HTTP] [GET] unit has configs");
  }

  http.end();
  return httpCode;
}

uint16_t sendMapRequest(uint8_t ID) {
  HTTPClient http;
  WiFiClient client;

  http.begin(client, makeURL(ID, messageType[4]));

  Serial.println("[CLIENT] [HTTP] [GET] map request to: " + makeURL(ID, messageType[3]) + " ...\n");
  int httpCode = http.GET();

  if (httpCode == CODE_ROOT_SUCCESS) {
     String mapResponse = http.getString();
     handleMapResponse(mapResponse, ID);
  }

  http.end();
  return httpCode;
}


uint16_t sendInformationToServer(uint8_t id, uint16_t steps, const String& map) {
  Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Time period: " + String(getTimePeriod()));
  Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Set new connection to SERVER ");

  if (setServerConnection() == WL_CONNECTED) {
    Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Information was sent to server SUCCESS");
    HTTPClient http;
    WiFiClient client;

    http.begin(client, serverURL + makeServerMessage(id, steps, map));

    Serial.println("[CLIENT] [HTTP] [GET] map request to: " + serverURL + makeServerMessage(id, steps, map) + " \n");
    int httpCode = http.GET();

    http.end();

    Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Connection is close.");
    WiFi.disconnect();
    return httpCode;
    
  } else {
    Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Information was sent to server NOT SUCCESS");
    Serial.println("[CLIENT] [CONNECTION_TO_SERVER] Connection is close.");
    WiFi.disconnect();
    return static_cast<uint16_t>(CODE_ERROR);
  }
  

}

uint8_t getTimePeriod() {
  if (now() % 60 < 20) {
    return 1;

  } else if ((now() % 60 >= 20) && (now() % 60 < 40)) {
    return 2;

  } else {
    return 3;
  }
}

uint8_t getAvailableUintID() {
  uint8_t timePeriod = getTimePeriod();

  if (mapList[timePeriod - 1].isEmpty()) {
    return 0;
  } else {
    return unitID[timePeriod - 1];
  }
}

void unitCommunication() {
  Serial.println("[CLIENT] [COMMUNICATION] Time period: " + String(getTimePeriod()));
  Serial.println("[CLIENT] [COMMUNICATION] Set new connection to ID: " + String(getAvailableUintID()));

  uint8_t lastTimePeriod = getTimePeriod();

  while (lastTimePeriod == getTimePeriod()) {
    if (setUnitConnection(getAvailableUintID()) == WL_CONNECTED) {
    Serial.println("[CLIENT] [COMMUNICATION] Connection success!");

      while (WiFi.status() == WL_CONNECTED) {
        uint8_t currentUnitID = WiFi.SSID().toInt();
        switch (currentUnitID) {
          case 0:
            sendUnitConfig();
            break;
          default:
            if (sendMapRequest(currentUnitID) == CODE_ROOT_SUCCESS) {
              sendInformationToServer(currentUnitID, unitSteps[currentUnitID - 20], mapList[currentUnitID - 20]);
            }
            break;         
        }
        delay(1000);
      }

      Serial.println("[CLIENT] [COMMUNICATION] Connection closed");

    } else {
      Serial.println("[CLIENT] [COMMUNICATION] Connection fallied.");
    }
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("[CLIENT] start client");
  Serial.println("[CLIENT] [CONFIG] set client configs");

  setClientConfig();
}

void loop() {
  Serial.println("[CLIENT] Start unit communication");
  unitCommunication();
}

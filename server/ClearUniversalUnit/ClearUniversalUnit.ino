/*-----------------------------------------------------------------------------CLIENT_INCLUDES------------------------------*/
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
/*-----------------------------------------------------------------------------SERVER_INCLUDES------------------------------*/
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
/*-----------------------------------------------------------------------------CONFIGURATIONS-----------------------------*/
#include <TimeLib.h>

#define UNIT_CONDITION_OUTPUT D0
#define CLOSE_READING_OUTPUT D1
#define OPEN_WRITING_INPUT D5
#define DRIVING_CONDITION_INPUT D7

SoftwareSerial uartChanel(D9, D10);

uint16_t delayMovement = 2000;
uint16_t unitSettingServerTime = 5000;

bool hasConfig = false;

//String mapResponse;

const uint8_t maxDifferntCarNum = 20;

uint8_t selfX = 0,
        selfY = 0,
        selfTurn;

int selfID = 0;
int numberOfSteps = 0;

std::array<int, maxDifferntCarNum> differentID = {
  20, 21, 22
};

std::array<int, 3> positionsRemoteUnitsX;
std::array<int, 3> positionsRemoteUnitsY;


const char *selfSSID;
const char *password = "123456789";

const char *messageType[]{
  "",
  "/handshake",
  "/setconfig",
  "/getmap",
  "/maptomonitor"
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
/*------------------------------------------------------------------------MAP_CONFIG-------------------------------------*/
//std::vector <std::vector<uint16_t>> map_;
const int mapRows = 6,
          mapCols = 6;

//int localMap [rows][cols];
/*------------------------------------------------------------------------MAP_FUNC----------------------------------------*/
enum class CellType {
  VOID = 0,
  OBSTACLE = 1,
  FOG_OF_WAR = 2,
  ERROR = 3,
  DIFFERENT_POSITION = 8,
  SELF_POSITION = 9
};


String localMap;

void changeAllCellTypes(CellType oldType, CellType newType) {
  for (char &cellType : localMap) {
    if (cellType == cellTypeToChar(oldType)) {
      cellType = cellTypeToChar(newType);
    }
  }
}

int cellTypeToInt(CellType cellType) {
  return static_cast<int>(cellType);
}

constexpr char cellTypeToChar(CellType cellType) {
  switch (cellType) {
    case CellType::VOID:
      return '0';

    case CellType::OBSTACLE:
      return '1';

    case CellType::FOG_OF_WAR:
      return '2';

    case CellType::ERROR:
      return '3';

    case CellType::DIFFERENT_POSITION:
      return '8';

    case CellType::SELF_POSITION:
      return '9';

    default:
      return '1';
  }
}

String cellTypeToString(CellType cellType) {
  return String(static_cast<uint8_t>(cellType));
}

CellType charToCellType(char cellMap) {
  switch (cellMap) {
    case static_cast<char>(CellType::VOID):
      return CellType::VOID;

    case static_cast<char>(CellType::OBSTACLE):
      return CellType::OBSTACLE;

    case static_cast<char>(CellType::FOG_OF_WAR):
      return CellType::FOG_OF_WAR;

    case static_cast<char>(CellType::ERROR):
      return CellType::DIFFERENT_POSITION;

    case static_cast<char>(CellType::SELF_POSITION):
      return CellType::SELF_POSITION;

    default:
      return CellType::ERROR;
  }
}

void generateLocalMap() {
  for (size_t i = 0; i < mapRows; i++) {
    for (size_t j = 0; j < mapCols; j++) {
      if (i == selfX && j == selfY) {
        localMap += cellTypeToString(CellType::SELF_POSITION);

      } else {
        localMap += cellTypeToString(CellType::FOG_OF_WAR);
      }
    }
  }
}

void mapPrint() {
  size_t currentColl = 0;

  uartChanel.println("Map LENGTH = " + String(localMap.length()));

  for (char &cell : localMap) {
    if (currentColl >= mapCols) {
      uartChanel.print("\n");
      currentColl = 0;
    }

    if (currentColl < mapCols) {
      uartChanel.print(cell);
      uartChanel.print(" ");
      currentColl++;
    }
  }
  uartChanel.println("\n");
}

size_t coordinateToCharPosition(uint8_t X, uint Y) {
  return ((X * mapCols) + (Y));
}

void setSelfPosition(uint8_t newSelfX, uint8_t newSelfY) {
  for (char &cell : localMap) {
    if (cell == cellTypeToChar(CellType::SELF_POSITION)) {
      cell = cellTypeToChar(CellType::VOID);
    }
  }

  localMap[coordinateToCharPosition(newSelfX, newSelfY)] =  cellTypeToChar(CellType::SELF_POSITION);
  selfX = newSelfX;
  selfY = newSelfY;
  numberOfSteps++;
}

void addNewCellTypeToMap(int X, int Y, CellType cellType) {
  localMap[coordinateToCharPosition(X, Y)] = cellTypeToChar(cellType);
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

uint8_t getAvailableID() {
  uint8_t timePeriod = getTimePeriod();
  return differentID[timePeriod - 1];
}

void handshakeResponseParse(const String &handshakeResponse) {
  size_t counterX = 0, counterY = 0;
  bool secondPart = false;
  String coordinate;

  for (char num : handshakeResponse) {
    switch (num) {
      case ';':
        secondPart = true;
        break;

      case ':':
        if (!secondPart && coordinate.length() > 0) {
          positionsRemoteUnitsX[counterX] = coordinate.toInt();
          uartChanel.println("X" + String(counterX) + " = " + positionsRemoteUnitsX[counterX]);
          coordinate = "";
          counterX++;

        } else if (coordinate.length() > 0) {
          positionsRemoteUnitsX[counterY] = coordinate.toInt();
          uartChanel.println("Y" + String(counterY) + " = " + positionsRemoteUnitsY[counterY]);
          coordinate = "";
          counterY++;
        }
        break;

      default:
        if (!secondPart) {
          coordinate += num;
        } else {
          coordinate += num;
        }
        break;
    }
  }
}

void mapResponseParse(const String &differentMap) {
  size_t counter = 0;

  if (mapValid(differentMap)) {
    uartChanel.println("[MAP] MAP VALID");

    for (const char &cellOfDifferentMap : differentMap) {
      switch (cellOfDifferentMap) {

        case cellTypeToChar(CellType::SELF_POSITION):
          localMap[counter] = cellTypeToChar(CellType::DIFFERENT_POSITION);
          break;

        case cellTypeToChar(CellType::FOG_OF_WAR):
          break;

        case cellTypeToChar(CellType::OBSTACLE):
          if (localMap[counter] != cellTypeToChar(CellType::VOID)) {
            localMap[counter] = cellTypeToChar(CellType::OBSTACLE);
          }
          break;

        case cellTypeToChar(CellType::VOID):
          localMap[counter] = cellTypeToChar(CellType::VOID);
          break;

        default:
          break;
      }
      counter++;
    }

    counter = 0;

    for (size_t i = 0; i < mapRows; i++) {
      for (size_t j = 0; j < mapCols; j++) {
        if (i == selfX && j == selfY) {
          localMap[counter] = cellTypeToChar(CellType::SELF_POSITION);
          break;
        }
        counter++;
      }
    }
  } else {
    uartChanel.println("[MAP] MAP NOT VALID");
  }
}

bool mapValid(const String &map1D) {
  if (!map1D.isEmpty() && map1D.length() == mapCols * mapRows) {
    return true;

  } else {
    return false;
  }
}
/*-----------------------------------------------------------------------------------------CLIENT_CODE-----------------------------*/

String makeIP(uint8_t ID) {
  String IP = "192.168.0.";
  IP += String(ID + 100);
  return IP;
}

String makeURL(uint8_t carID, const char *messageType) {
  String URL = "http://" + makeIP(carID) + messageType;
  return URL;
}

String makePositionsRemoteUnitsMessage() {
  String message;
  for (int X : positionsRemoteUnitsX) {
    message += String(X);
    message += String(':');
  }

  message += String(';');

  for (int Y : positionsRemoteUnitsY) {
    message += String(Y);
    message += String(':');
  }
  return message;
}

String makeHandshakeMessage() {
  String handshakeMessage = "?ID=" + String(selfID) + "&" + "X=" + String(selfX) + "&" + "Y=" + String(selfY);
  return handshakeMessage;
}

String makeMapMessage() {
  String mapArg = localMap;
  String mapMessage = "?ROWS=" + String(mapRows) + "&COLS=" + String(mapCols) + "&MAP=" + mapArg;
  return mapMessage;
}

String makeMonitorMessage() {
  String message = localMap + String(numberOfSteps);
  return message;
}

wl_status_t setUnitConnection(uint8_t ID) {
  WiFi.begin(String(ID).c_str(), password);
  delay(4000);
  return WiFi.status();
}

void setUnitConfig(uint8_t ID, uint8_t turn) {
  selfID = ID;
  selfTurn = turn;
}

void setAccessPointConfig(uint8_t selfID) {
  IPAddress localIp(192, 168, 1, selfID);
  IPAddress gateway = localIp;
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAP(setSelfSSID(selfID), password);
  WiFi.softAPConfig(localIp, gateway, subnet);
  setNetworkConfig(selfID);
}

const String setSelfSSID(uint8_t selfID) {
  String SSID = String(selfID);
  return SSID;
}

void setClientConfig() {
  WiFi.mode(WIFI_STA);
}

void setNetworkConfig(uint8_t selfID) {
  WiFi.mode(WIFI_AP);
}

bool timeValid(String time_) {
  bool valid;
  for (size_t i = 0; i < time_.length(); i++) {
    valid = false;
    for (size_t j = 0; j < 10; j++) {
      if (time_[i] != String(j)[0]) {
        valid = true;
      }
    }
    if (valid == false) {
      return valid;
    }
  }
  return valid;
}

/*------------------------------------REQUESTS----------------------------------*/
uint16_t getHandshakeRequest(uint8_t ID) {
  HTTPClient http;
  WiFiClient client;
  String serverResponse;

  http.begin(client, makeURL(ID, messageType[1]) + makeHandshakeMessage());

  uartChanel.print("[CLIENT] [HTTP] [GET] Handshake to: " + makeURL(ID, messageType[1]) + makeHandshakeMessage() + " ...\n");

  serverResponse = http.getString();

  uartChanel.print("[CLIENT] [HTTP] [GET] Handshake response: " + serverResponse + "\n");
  handshakeResponseParse(serverResponse);

  int httpCode = http.GET();

  http.end();
  return httpCode;
}

uint16_t getMapRequest(uint8_t ID) {
  HTTPClient http;
  WiFiClient client;

  http.begin(client, makeURL(ID, messageType[3]) + makeMapMessage());

  uartChanel.print("[CLIENT] [HTTP] [GET] map request to: " + makeURL(ID, messageType[3]) + " ...\n");

  uartChanel.println("[CLIENT] [HTTP] [GET] Send map: " + localMap);
  int httpCode = http.GET();

  if (httpCode = CODE_SUCCESS) {
    String newMap = http.getString();
    uartChanel.println("[CLIENT] [HTTP] [GET] get new map: " + newMap);
    mapResponseParse(newMap);
    mapPrint();

  } else {
    uartChanel.println("[CLIENT] [HTTP] [GET] get map error: " + httpCode);
  }

  http.end();
  return httpCode;
}

/*--------------------------------------HANDLERS--------------------------------------*/
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void handleMapMonitorRequest() {
  uartChanel.println("[SERVER] [MAP] [MONITOR] Send map:" + localMap);
  server.send(CODE_ROOT_SUCCESS, "text/html", makeMonitorMessage());
}

void handleSetConfigRequest() {
  if (hasConfig) {
    server.send(CODE_ROOT_HAS_TIME, "text/html", localMap);
  } else {
    if (server.args() == 3) {
      if (!hasConfig) {
        hasConfig = true;

        uartChanel.println("[MAP] [CONFIG] Set locale map.");

        setUnitConfig(server.arg(1).toInt(), server.arg(2).toInt());
        generateLocalMap();
        mapPrint();

        server.send(CODE_ROOT_SUCCESS, "text/html", makeMonitorMessage());

        uartChanel.println("[SERVER] [CONFIG] send map: " + localMap);
        setTime(static_cast<time_t>(server.arg(0).toInt()));

        delay(1000);
        server.stop();
        server.close();
        setAccessPointConfig(selfID);

        uartChanel.println("[SERVER] [CONFIG] Real time(Y:M:D:H:M:S): " + String(year()) + ":" + String(month()) + ":" + String(day()) + ":"
                           + String(hour()) + ":" + String(minute()) + ":" + String(second()));
        uartChanel.print("[SERVER] [CONFIG] Self ID: ");
        uartChanel.println(selfID);
        uartChanel.println("[SERVER] [CONFIG] Self turn: " + String(selfTurn));

      } else {
        uartChanel.println("[SERVER] [CONFIG] Server has configs");
        server.send(CODE_ERROR, "text/html", "<h1>Has configs</h1>");
      }

    } else {
      uartChanel.println("[SERVER] [CONFIG] Config args error");
      server.send(CODE_ERROR, "text/html", "<h1>Error args...</h1>");
    }
  }
}

void handleHandshakeRequest() {
  if (server.args() == 3) {
    uint8_t differentX = server.arg(1).toInt(),
            differentY = server.arg(2).toInt();

    positionsRemoteUnitsX[server.arg(0).toInt() - 20] = differentX;
    positionsRemoteUnitsY[server.arg(0).toInt() - 20] = differentY;


    if (((differentX == selfX) && abs(differentY - selfY) == 1) || ((differentY == selfY) && abs(differentX - selfX) == 1) || abs(differentY - selfY) && abs(differentX - selfX) == 1) {

      uartChanel.println("[SERVER] Get handshake: ID = " + server.arg(0) + "; Position(x:y): " + String(positionsRemoteUnitsX[server.arg(0).toInt() - 20]) + ":" + String(positionsRemoteUnitsY[server.arg(0).toInt() - 20]));
      server.send(CODE_SUCCESS, "text/html", makePositionsRemoteUnitsMessage());

    } else {
      uartChanel.println("[SERVER] Get handshake, but far away...");
      server.send(CODE_FAR_AWAY, "text/html", makePositionsRemoteUnitsMessage());
    }

  } else {
    uartChanel.println("[SERVER] Get handshake, but Invalid arguments...");
    server.send(CODE_INVALID_ARGS, "text/html", makePositionsRemoteUnitsMessage());
  }
}

void handleMapRequest() {
  if (server.args() == 3 && server.arg(0).toInt() == mapRows && server.arg(1).toInt() == mapCols) {
    uartChanel.print("[SERVER] GET different map: ");
    uartChanel.println(server.arg(2));

    uartChanel.println("[SERVER] Parsing map...");
    mapResponseParse(server.arg(2));

    uartChanel.println("[SERVER] Print new map:");
    mapPrint();

    uartChanel.println("[SERVER] Send self map: " + localMap);
    server.send(CODE_SUCCESS, "text/html", localMap);

    delay(2000);

  } else {
    uartChanel.println("[SERVER] Get map, invalid arguments...");
    server.send(CODE_FAR_AWAY, "text/html", "<h1>You are far away</h1>");
  }
}

void ledON() {
  digitalWrite(2, LOW);
}

void ledOFF() {
  digitalWrite(2, HIGH);
}

bool coordinateValid(int X, int Y) {
  if (X >= 0 && Y >= 0) {
    return true;
  }
  return false;
}

bool obstacleValid(int obstacleX, int obstacleY) {
  bool matchedX = false, matchedY = false;

  if (!coordinateValid(obstacleX, obstacleY)) {
    return false;
  }

  for (int remoteUnitPositionX : positionsRemoteUnitsX) {
    if (obstacleX == remoteUnitPositionX) {
      matchedX = true;
    }
  }

  for (int remoteUnitPositionY : positionsRemoteUnitsY) {
    if (obstacleY == remoteUnitPositionY) {
      matchedY = true;
    }
  }

  if (matchedX && matchedY) {
    return false;

  } else {
    return true;
  }
}

void coordinateProcessing(int X, int Y, bool obstacleFlag) {
  if (coordinateValid(X,Y)) {
    if (obstacleFlag) {
      if (obstacleValid(X,Y)) {
        addNewCellTypeToMap(X, Y, CellType::OBSTACLE);
      } else {
        addNewCellTypeToMap(X, Y, CellType::VOID);
      }
    } else {
      addNewCellTypeToMap(X, Y, CellType::VOID);
    }
  }
}

void addUnitMapEnviroment(bool forwardObstacle, bool backObstacle, bool rightObstacle, bool leftObstacle) {
  int forwardX = selfX + 1, forwardY = selfY,
      backX = selfX - 1, backY = selfY,
      rightX = selfX, rightY = selfY + 1,
      leftX = selfX, leftY = selfY - 1;
    
  uartChanel.println("F:" + String(forwardX) + ':' + String(forwardY) + ':' + String(forwardObstacle));
  uartChanel.println("B:" + String(backX) + ':' + String(backY) + ':' + String(backObstacle));
  uartChanel.println("R:" + String(rightX) + ':' + String(rightY) + ':' + String(rightObstacle));
  uartChanel.println("L:" + String(leftX) + ':' + String(leftY) + ':' + String(leftObstacle));

  bool matchedX = false, matchedY = false;

  coordinateProcessing(forwardX, forwardY, forwardObstacle);
  coordinateProcessing(backX, backY, backObstacle);
  coordinateProcessing(rightX, rightY, rightObstacle);
  coordinateProcessing(leftX, leftY, leftObstacle);
}

void messageBodyParser(const String &messageBody) {
  uint8_t count = 0;
  uint8_t newX, newY;

  char startChar = '<',
       endChar = '>';

  bool obstacleLeft = false,
       obstacleRight = false,
       obstacleBack = false,
       obstacleForward = false;

  for (char i : messageBody) {
    if (i != ':' && i != startChar && i != endChar) {
      switch (count) {
        case 0:
          newX = String(i).toInt();
          break;

        case 1:
          newY = String(i).toInt();
          break;

        case 2:
          obstacleForward = String(i).toInt();
          break;

        case 3:
          obstacleBack = String(i).toInt();
          break;

        case 4:
          obstacleRight = String(i).toInt();
          break;

        default:
          obstacleLeft = String(i).toInt();
          break;
      }
      count++;
    }
  }
  uartChanel.println("MESSAGE UART NEW Y ------------>" + newY);
  setSelfPosition(newX, newY);
  addUnitMapEnviroment(obstacleForward, obstacleBack, obstacleRight, obstacleLeft);
  uartChanel.flush();
}

void pinModeSetting() {
  pinMode(UNIT_CONDITION_OUTPUT, OUTPUT);
  pinMode(CLOSE_READING_OUTPUT, OUTPUT);
  pinMode(OPEN_WRITING_INPUT, INPUT);
  pinMode(DRIVING_CONDITION_INPUT, INPUT);
}

void setUnitConditionServer(bool condition) {
  if (condition) {
    digitalWrite(UNIT_CONDITION_OUTPUT, HIGH);
  } else {
    digitalWrite(UNIT_CONDITION_OUTPUT, LOW);
  }
}

void setUnitCloseReading(bool condition) {
  if (condition) {
    digitalWrite(CLOSE_READING_OUTPUT, HIGH);
  } else {
    digitalWrite(CLOSE_READING_OUTPUT, LOW);
  }
}

bool carWritingCondition() {
  if (digitalRead(OPEN_WRITING_INPUT) == HIGH) {
    return true;
  }
  return false;
}

bool carDrivingCondition() {
  if (digitalRead(DRIVING_CONDITION_INPUT) == HIGH) {
    return true;
  }
  return false;
}

bool timeOfServer() {
  int seconds = millis() / 1000;

  if ((seconds % 60) < 20) {
    return true;
  } else {
    return false;
  }
}

bool uartCharacterValid(char chekingChar) {
  String possibleCharacters = "0123456789:";

  for (char validChar : possibleCharacters) {
    if (chekingChar == validChar) {
      return true;
    }
  }
  return false;
}

bool uartStringValid(const String &chekingString) {
  if (chekingString.length() < 11) {
    return false;
  }

  for (char chekingChar : chekingString) {
    if (!uartCharacterValid(chekingChar)) {
      return false;
    }
  }
  return true;
}

void uartProcessing() {
  size_t counter = 0;
  char recievedCharacter;
  String messageBody;

  if (uartChanel.available()) {
    recievedCharacter = uartChanel.read();
    messageBody = uartChanel.readStringUntil('>');

    if (uartStringValid(messageBody)) {
      messageBodyParser(messageBody);

      setUnitCloseReading(true);
      delay(600);
      setUnitCloseReading(false);
    }
  }
}


void startConfigServer() {
  const uint8_t defaultID = 0;
  setUnitConditionServer(true);
  delay(1000);

  uartChanel.println();
  uartChanel.println();
  uartChanel.println();

  uartChanel.println("[SERVER] [CONFIG] Configurating access point...");

  setAccessPointConfig(defaultID);

  uartChanel.println("[SERVER] [CONFIG] Start access point.");
  WiFi.enableAP(true);

  uartChanel.println("[SERVER] [CONFIG] Start configuration HTTP server.");
  server.begin();
  uartChanel.println("[SERVER] [CONFIG] Wait configurations...");

  server.on(messageType[2], handleSetConfigRequest);

  while (!hasConfig) {
    setUnitConditionServer(true);
    server.handleClient();
  }

  uartChanel.println("[SERVER] [CONFIG] Stop configuration HTTP server.");
  server.stop();
  server.close();
  WiFi.enableAP(false);
}

void startServer() {
  ledON();

  uartChanel.println();
  uartChanel.println();
  uartChanel.println();

  uartChanel.println("[SERVER] Start HTTP server.");
  uartChanel.println("[SERVER] Configurating access point...");

  setAccessPointConfig(selfID);
  uartChanel.println("[SERVER] Start access point.");
  WiFi.enableAP(true);

  IPAddress myIP = WiFi.softAPIP();

  uartChanel.print("[SERVER] AP IP address: ");
  uartChanel.println(myIP);

  server.on("/", handleRoot);
  server.on(messageType[1], handleHandshakeRequest);
  server.on(messageType[2], handleSetConfigRequest);
  server.on(messageType[3], handleMapRequest);
  server.on(messageType[4], handleMapMonitorRequest);

  server.begin();
  uartChanel.println("[SERVER] Satart handle of client.");

  setUnitConditionServer(true);
  delay(1000);

  if (carWritingCondition()) {
    uartProcessing();
  }

  while (getAvailableID() == selfID) {
    server.handleClient();
  }

  uartChanel.println("[SERVER] Stop HTTP server.");

  server.stop();
  server.close();
  WiFi.enableAP(false);
}

void startClient() {
  ledOFF();
  setUnitConditionServer(false);

  changeAllCellTypes(CellType::DIFFERENT_POSITION, CellType::VOID);

  uartChanel.println("Print Old Map:");
  mapPrint();

  uartChanel.println();
  uartChanel.println();
  uartChanel.println();

  uartChanel.println("[CLIENT] Start wi-fi client.");

  while (getAvailableID() != selfID) {
    uint8_t availableID = getAvailableID();

    if (setUnitConnection(availableID) == WL_CONNECTED) {
      uartChanel.println("[CLIENT] [COMMUNICATION] Connection success!");

      bool hasHandshake = false;
      uint16_t httpCode = 0;

      while (WiFi.status() == WL_CONNECTED) {
        if (!hasHandshake) {
          uartChanel.println("[CLIENT] [HTTP] Try send handshake");
          httpCode = getHandshakeRequest(availableID);
          hasHandshake = true;
          delay(1000);

        } else if (httpCode == CODE_SUCCESS) {
          uartChanel.println("[CLIENT] [HTTP] Handshake code was success");
          uartChanel.println("[CLIENT] [HTTP] Try send map");

          httpCode = getMapRequest(availableID);
          delay(1000);

        } else {
          hasHandshake = false;
        }
      }

      uartChanel.println("[CLIENT] [COMMUNICATION] Connection closed");

    } else {
      uartChanel.println("[CLIENT] [COMMUNICATION] Connection fallied.");
    }

    if (carWritingCondition()) {
      uartProcessing();
    }
  }
}

void begin() {
  if (!carDrivingCondition()) {
    if (getAvailableID() == selfID) {

      startServer();
    } else {
      startClient();
    }
  }
}

/*-------------------------NODE_MCU_CODE-----------------------------*/
void setup() {
  pinModeSetting();
  for (size_t i = 0; i < positionsRemoteUnitsX.max_size(); i++) {
    positionsRemoteUnitsX[i] = 0;
    positionsRemoteUnitsY[i] = 0;
  }

  uartChanel.begin(115200);
  pinMode(2, OUTPUT);
  startConfigServer();
}

void loop() {
  begin();
}
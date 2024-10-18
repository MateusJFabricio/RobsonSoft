#include <WiFi.h>              //Wifi control
#include <WebServer.h>         //API
#include <WebSocketsServer.h>  //Web Socker server
#include <ArduinoJson.h>       //JSON
#include <AsyncTCP.h>          //TPC Comm
#include <SPIFFS.h>            //Write and Read file
#include <Wire.h>              //
#include <ESPmDNS.h>           //DNS
#include <MobaTools.h>         //StepMotors and Servo
#include <Platform.h>          //PLC Comm
#include "Settimino.h"         //PLC Comm
#include "elk.h"               //Compilador

struct CompilerDataType {
  int Speed;
  int Aceleracao;
  long Aproximacao;  //Famoso zoneamento
  bool Ordem[16];
  bool Evento[16];
  uint16_t ProgNum;
  bool PlayCode;
  String Code;
};
CompilerDataType CompilerData;

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

const char *ssid = "DRONE";
const char *password = "robosoft";

struct PLCConnectionType {
  uint8_t IP[4];
  uint16_t Rack;
  uint16_t Slot;
  int DBReceive;
  int DBSend;
  bool Connected;
  unsigned long plcLastUpdate;
};

PLCConnectionType PLCConnectionData = { { 0, 0, 0, 0 }, 0, 0, 100, 101, false, 0 };

struct PLCDataExchangeType {
  struct {
    bool LifeBit;
    bool AutoMode;
    bool SafetyOK;
    bool Spare[13];
    uint16_t ProgNum;  //2 bytes
    bool Evento[16];   //16 bits
  } FromPLC;           //Total de 6 bytes

  struct {
    bool LifeBit;
    bool InAutoMode;
    bool Spare[14];
    uint16_t ActualProg;  //2 bytes
    bool Ordem[16];       //16 bits
  } ToPLC;                //Total de 6 bytes
};

PLCDataExchangeType PLCDataExchange;

IPAddress PLC(
  PLCConnectionData.IP[0],
  PLCConnectionData.IP[1],
  PLCConnectionData.IP[2],
  PLCConnectionData.IP[3]);  // PLC Address

S7Client PLCClient;

struct ManualMoveType {
  struct {
    float MoveX;
    float MoveY;
    float MoveZ;
  } MoveAngle;

  struct {
    float MoveX;
    float MoveY;
    float MoveZ;
  } MoveLinear;

  struct {
    float Joint1;
    float Joint2;
    float Joint3;
    float Joint4;
    float Joint5;
  } MoveJoint;

  struct {
    bool Open;
    bool Close;
    bool MoveOpen;
    bool MoveClose;
  } Gripper;

  struct {
    bool DeadMan;
    bool StartAutomatic;
  } General;

  struct {
    float LastUpdate;
    bool IsOnline;
  } Telemetry;
};
struct JointParam {
  int minAngle;
  int maxAngle;
  float jointRatio;
  int pinTrigger;
  int pinDir;
  int pinEnable;
};
struct GripperParam {
  int minAngle;
  int maxAngle;
  int pinTrigger;
  int angleOpenned;
  int angleClosed;
};

GripperParam gripperParam = { 40, 120, 21, 40, 120 };

struct RobotStateType {
  struct {
    bool MoveAngle;
    bool MoveJoint;
    bool MoveLinear;
  } MoveType;
  struct {
    bool Manual;
    bool ManualCem;
    bool Auto;
  } ModoOperacao;
  float Speed;
};

RobotStateType RobotState;

JointParam joint1Param = { -360, 360, 15.0 / 160.0, 13, 12, 14 };
JointParam joint2Param = { -360, 360, 21.4 / 120.0, 27, 26, 25 };
JointParam joint3Param = { -360, 360, (21.4 / 95.0) / 5.0, 33, 32, 0 };  //Sem enable
JointParam joint4Param = { -360, 360, 1, 4, 16.0, 17 };
JointParam joint5Param = { -360, 360, 15.0 / 70.0, 5, 18, 19 };

ManualMoveType Commands;

MoToStepper joint1(6400, STEPDIR);
MoToStepper joint2(6400, STEPDIR);
MoToStepper joint3(8000, STEPDIR);
MoToStepper joint4(20000, STEPDIR);
MoToStepper joint5(20000, STEPDIR);
MoToServo gripper;

//Parameters
StaticJsonDocument<2000> Parameters;
unsigned long lastPingTime = millis();

void setup() {
  Serial.begin(115200);
// Initialize SPIFFS
#ifdef ESP32
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#else
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    delay(1000);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("robsonsoft")) {
    Serial.println("Error starting mDNS");
    return;
  }

  ReadParameters();

  //Configura Motor
  Serial.println("Configurando motores");

  joint1.attach(joint1Param.pinTrigger, joint1Param.pinDir);  //14 enable
  joint2.attach(joint2Param.pinTrigger, joint2Param.pinDir);  //25 enable
  joint3.attach(joint3Param.pinTrigger, joint3Param.pinDir);  //sem enable
  joint4.attach(joint4Param.pinTrigger, joint4Param.pinDir);  //17 enable
  joint5.attach(joint5Param.pinTrigger, joint5Param.pinDir);  //19 enable
  gripper.attach(gripperParam.pinTrigger);

  pinMode(joint1Param.pinEnable, OUTPUT);
  pinMode(joint2Param.pinEnable, OUTPUT);
  //pinMode(joint3Param.pinEnable, OUTPUT);
  pinMode(joint4Param.pinEnable, OUTPUT);
  pinMode(joint5Param.pinEnable, OUTPUT);

  EnableAllMotors();

  Serial.println("Finalizado as configuracoes dos motores");

  // Inicia o WebSocket server
  Serial.println("Iniciando o wesocket");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.enableCORS();
  server.on("/getRobotStatus", APIGetRobotStatus);
  server.on("/getJointAngles", APIGetJointAngles);
  server.on("/getJointLimits", APIGetJointLimits);
  server.on("/getPLCConectionStatus", APIGetPLCConnectionStatus);
  server.on("/postJointCalibration", HTTP_POST, APIPostJointCalibration);
  server.on("/postJointLimits", HTTP_POST, APIPostJointLimits);
  server.on("/postRobotState", HTTP_POST, APIPostRobotState);
  server.on("/postSaveExternalCode", HTTP_POST, APIPostSaveExternalCode);
  server.on("/postStartExternalCode", HTTP_POST, APIPostStartExternalCode);
  server.on("/postPLCConfig", HTTP_POST, APIPostPLCConfig);
  server.on("/postPLCConectar", HTTP_POST, APIPostPLCConectar);
  server.on("/postPLCDesconectar", HTTP_POST, APIPostPLCDesconectar);
  server.begin();

  Serial.println("Inicializa as tasks");

  xTaskCreate(TaskWebServerAPI, "TaskWebServerAPI", 10000, NULL, 1, NULL);
  xTaskCreate(TaskWebSocket, "TaskWebSocket", 10000, NULL, 1, NULL);
  xTaskCreate(TaskCheckComm, "TaskCheckComm", 10000, NULL, 1, NULL);
  xTaskCreate(TaskCommPLC, "TaskCommPLC", 20000, NULL, 1, NULL);
  xTaskCreate(TaskExternalProg, "TaskExternalProg", 20000, NULL, 1, NULL);

  Serial.println("Finaliza a inicializacao das tasks");
}

void loop() {

  if (RobotState.ModoOperacao.Manual) {
    ManualMove();
  } else {
    //ExecuteJS();
  }
  /*
  if (!IsJointMoving(1)){
    if (GetCurrentAngle(1) == 0){
      MoveJointPosition(1, 90, 60, 200);
      Serial.println("Movendo para 90 graus");
    }

    if (GetCurrentAngle(1) == 90){
      delay(1000);
      MoveJointPosition(1, 270, 60, 200);
      Serial.println("Movendo para 270 graus");
    }

    if (GetCurrentAngle(1) == 270){
      delay(1000);
      MoveJointPosition(1, 180, 60, 2000);
      Serial.println("Movendo para 180 graus");
    }

    if (GetCurrentAngle(1) == 180){
      delay(1000);
      MoveJointPosition(1, 360, 60, 200);
      Serial.println("Movendo para 360 graus");
    }

    if (GetCurrentAngle(1) == 360){
      ResetJointPosition(1);
      Serial.println("Reset da posicao");
    }
  }
  
  Serial.print("Posicao:");
  Serial.println(GetCurrentAngle(1));
  */
  //MoveJointSpeed(1, true, 100, 60.0, 100);
  //delay(5000);
  //StopMotor(1);
  //delay(2000);
}
/************************************/

/************* ROBOT CONTROL ****************/

void ManualMove() {
  Serial.print(",DeadMan:");
  Serial.print(Commands.General.DeadMan);
  Serial.print(",MoveJoint:");
  Serial.print(RobotState.MoveType.MoveJoint);
  Serial.print(",Joint1:");
  Serial.print(Commands.MoveJoint.Joint1);
  Serial.print(",Joint1Position:");
  Serial.print(GetCurrentAngle(1));
  Serial.print(",Joint2:");
  Serial.print(Commands.MoveJoint.Joint2);
  Serial.print(",Gripper:");
  Serial.print(gripper.read());
  Serial.print(",ConvSpeedJ1:");
  Serial.print(ConvertSpeed(Commands.MoveJoint.Joint1 * RobotState.Speed, joint1Param.jointRatio));
  Serial.print(",Speed:");
  Serial.println(RobotState.Speed);

  if (Commands.General.DeadMan) {
    if (RobotState.MoveType.MoveJoint) {
      MoveJointSpeed(1, Commands.MoveJoint.Joint1 * RobotState.Speed, 1);
      MoveJointSpeed(2, Commands.MoveJoint.Joint2 * RobotState.Speed, 1);
      MoveJointSpeed(3, Commands.MoveJoint.Joint3 * RobotState.Speed, 1);
      MoveJointSpeed(4, Commands.MoveJoint.Joint4 * RobotState.Speed, 1);
      MoveJointSpeed(5, Commands.MoveJoint.Joint5 * RobotState.Speed, 1);
    }

    if (Commands.Gripper.Open) {
      MoveGripperPosition(gripperParam.angleOpenned, RobotState.Speed);
    }

    if (Commands.Gripper.Close) {
      MoveGripperPosition(gripperParam.angleClosed, RobotState.Speed);
    }

    if (Commands.Gripper.MoveOpen) {
      MoveGripperSpeed(RobotState.Speed);
    }

    if (Commands.Gripper.MoveClose) {
      MoveGripperSpeed(RobotState.Speed * -1);
    }

  } else {
    StopAllMotors();
    StopGripper();
  }
}
/************************************/

/************* TASKS ****************/
void TaskExternalProg(void *pvParameters) {
  Serial.println("Task TaskExternalProg Iniciada");

  while (true) {
    if (RobotState.ModoOperacao.Auto) {
      Serial.println("Aguardando PlayCode");
      if (CompilerData.PlayCode) {
        Serial.print("Rodando codigo externo. Codigo: ");
        Serial.println(CompilerData.Code.c_str());
        int val = ExecuteJS(CompilerData.Code.c_str());
        CompilerData.PlayCode = false;
        Serial.print("Finalizando codigo externo com codigo: ");
        Serial.println(val);
      } else {
        delay(500);
      }
    } else {
      delay(500);
    }
  }
}
void TaskCheckComm(void *pvParameters) {
  Serial.println("Task CheckComm Iniciada");
  const TickType_t xFrequency = pdMS_TO_TICKS(300);  // ciclo de 500 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();    // pega o tick atual

  while (true) {
    Commands.Telemetry.IsOnline = millis() - lastPingTime < 500;
    if (!Commands.Telemetry.IsOnline) {
      Commands.General.DeadMan = false;
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // delay até o próximo ciclo
  }
}
void TaskWebServerAPI(void *parameter) {
  const TickType_t xFrequency = pdMS_TO_TICKS(400);  // ciclo de 500 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();    // pega o tick atual

  while (true) {
    //Roda a API
    server.handleClient();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // delay até o próximo ciclo
  }
}
void TaskWebSocket(void *parameter) {
  const TickType_t xFrequency = pdMS_TO_TICKS(5);  // ciclo de 500 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();  // pega o tick atual
  while (true) {
    // Mantém o WebSocket ativo
    webSocket.loop();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // delay até o próximo ciclo
  }
}
void TaskCommPLC(void *parameter) {
  Serial.println("TaskCommPLC Iniciada");
  const TickType_t xFrequency = pdMS_TO_TICKS(500);  // ciclo de 500 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();    // pega o tick atual
  bool lastBit = false;
  while (true) {
    if (PLCConnectionData.Connected) {
      PLCRead();

      //Check connection
      if ((millis() - PLCConnectionData.plcLastUpdate) > 5000) {
        PLCDisconnect();
        Serial.println("Conexao com o PLC fechada devido ao TimeOut de 5 segundos");
      } else {
        if (PLCDataExchange.FromPLC.LifeBit != lastBit) {
          PLCConnectionData.plcLastUpdate = millis();
          lastBit = PLCDataExchange.FromPLC.LifeBit;
        }
      }

      //RobotState.SafetyOK = PLCDataExchange.FromPLC.SafetyOK;
      //RobotState.ModoOperacao.Auto = PLCDataExchange.FromPLC.AutoMode;
      CompilerData.ProgNum = PLCDataExchange.FromPLC.ProgNum;
      for (int i = 0; i < 16; i++) {
        CompilerData.Evento[i] = PLCDataExchange.FromPLC.Evento[i];
      }

      PLCDataExchange.ToPLC.LifeBit = !PLCDataExchange.ToPLC.LifeBit;
      PLCDataExchange.ToPLC.InAutoMode = RobotState.ModoOperacao.Auto;
      PLCDataExchange.ToPLC.ActualProg = swap(PLCDataExchange.FromPLC.ProgNum);
      for (int i = 0; i < 16; i++) {
        CompilerData.Ordem[i] = PLCDataExchange.ToPLC.Ordem[i];
      }

      PLCWrite();
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // delay até o próximo ciclo
  }
}
/************************************/

/************* APIs ****************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type != WStype_DISCONNECTED || type != WStype_ERROR) {
    lastPingTime = millis();
  }

  if (type == WStype_TEXT) {
    // Recebe a mensagem do cliente
    String message = String((char *)payload);

    // Cria um documento JSON
    StaticJsonDocument<1000> doc;

    // Tenta deserializar o JSON
    DeserializationError error = deserializeJson(doc, message);
    switch (error.code()) {
      case DeserializationError::Ok:
        break;
      case DeserializationError::InvalidInput:
        Serial.println("Invalid input!");
        return;
      case DeserializationError::NoMemory:
        Serial.println("Not enough memory");
        return;
      default:
        Serial.println("Deserialization failed");
        return;
    }

    // Acessa os valores do JSON
    Commands.MoveAngle.MoveX = doc["MOVE_ANGLE"]["MOVE_X"];
    Commands.MoveAngle.MoveY = doc["MOVE_ANGLE"]["MOVE_Y"];
    Commands.MoveAngle.MoveZ = doc["MOVE_ANGLE"]["MOVE_Z"];

    Commands.MoveLinear.MoveX = doc["MOVE_LINEAR"]["MOVE_X"];
    Commands.MoveLinear.MoveY = doc["MOVE_LINEAR"]["MOVE_Y"];
    Commands.MoveLinear.MoveZ = doc["MOVE_LINEAR"]["MOVE_Z"];

    Commands.MoveJoint.Joint1 = doc["MOVE_JOINT"]["JOINT1"];
    Commands.MoveJoint.Joint2 = doc["MOVE_JOINT"]["JOINT2"];
    Commands.MoveJoint.Joint3 = doc["MOVE_JOINT"]["JOINT3"];
    Commands.MoveJoint.Joint4 = doc["MOVE_JOINT"]["JOINT4"];
    Commands.MoveJoint.Joint5 = doc["MOVE_JOINT"]["JOINT5"];

    Commands.Gripper.Open = doc["GRIPPER"]["OPEN"];
    Commands.Gripper.Close = doc["GRIPPER"]["CLOSE"];
    Commands.Gripper.MoveOpen = doc["GRIPPER"]["MOVE_OPEN"];
    Commands.Gripper.MoveClose = doc["GRIPPER"]["MOVE_CLOSE"];

    Commands.General.DeadMan = doc["GENERAL"]["DEAD_MAN"];
    Commands.General.StartAutomatic = doc["GENERAL"]["START_AUTOMATIC"];
  }
}
void APIGetRobotStatus() {
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  jsonDocument.clear();  // Clear json buffer
  JsonObject json = jsonDocument.to<JsonObject>();

  json["MODO_OPERACAO"]["MANUAL"] = RobotState.ModoOperacao.Manual;
  json["MODO_OPERACAO"]["MANUAL_CEM"] = RobotState.ModoOperacao.ManualCem;
  json["MODO_OPERACAO"]["AUTO"] = RobotState.ModoOperacao.Auto;

  json["DEAD_MAN"] = Commands.General.DeadMan;
  json["SPEED"] = RobotState.Speed;

  json["MOVE_TYPE"]["MOVE_ANGLE"] = RobotState.MoveType.MoveAngle;
  json["MOVE_TYPE"]["MOVE_JOINT"] = RobotState.MoveType.MoveJoint;
  json["MOVE_TYPE"]["MOVE_LINEAR"] = RobotState.MoveType.MoveLinear;

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void APIGetJointAngles() {
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  jsonDocument.clear();  // Clear json buffer
  JsonObject json = jsonDocument.to<JsonObject>();
  json["Joint1"] = GetCurrentAngle(1);
  json["Joint2"] = GetCurrentAngle(2);
  json["Joint3"] = GetCurrentAngle(3);
  json["Joint4"] = GetCurrentAngle(4);
  json["Joint5"] = GetCurrentAngle(5);
  json["Gripper"] = GripperGetCurrentAngle();

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void APIGetJointLimits() {
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  jsonDocument.clear();  // Clear json buffer
  JsonObject json = jsonDocument.to<JsonObject>();

  json["Joint1"]["MinAngle"] = joint1Param.minAngle;
  json["Joint1"]["MaxAngle"] = joint1Param.maxAngle;

  json["Joint2"]["MinAngle"] = joint2Param.minAngle;
  json["Joint2"]["MaxAngle"] = joint2Param.maxAngle;

  json["Joint3"]["MinAngle"] = joint3Param.minAngle;
  json["Joint3"]["MaxAngle"] = joint3Param.maxAngle;

  json["Joint4"]["MinAngle"] = joint4Param.minAngle;
  json["Joint4"]["MaxAngle"] = joint4Param.maxAngle;

  json["Joint5"]["MinAngle"] = joint5Param.minAngle;
  json["Joint5"]["MaxAngle"] = joint5Param.maxAngle;

  json["Gripper"]["MinAngle"] = gripperParam.minAngle;
  json["Gripper"]["MaxAngle"] = gripperParam.maxAngle;

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void APIGetPLCConnectionStatus() {
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  jsonDocument.clear();  // Clear json buffer
  JsonObject json = jsonDocument.to<JsonObject>();

  json["PLC"]["IP"][0] = PLCConnectionData.IP[0];
  json["PLC"]["IP"][1] = PLCConnectionData.IP[1];
  json["PLC"]["IP"][2] = PLCConnectionData.IP[2];
  json["PLC"]["IP"][3] = PLCConnectionData.IP[3];

  json["PLC"]["RACK"] = PLCConnectionData.Rack;
  json["PLC"]["SLOT"] = PLCConnectionData.Slot;

  json["CONNECTION"] = PLCConnectionData.Connected;

  json["RECEIVE"]["LIFE_BIT"] = PLCDataExchange.FromPLC.LifeBit;
  json["RECEIVE"]["AUTO_MODE"] = PLCDataExchange.FromPLC.LifeBit;
  json["RECEIVE"]["SAFETY_OK"] = PLCDataExchange.FromPLC.LifeBit;
  json["RECEIVE"]["PROGNUM"] = PLCDataExchange.FromPLC.ProgNum;
  for (int i = 0; i < 16; i++) {
    json["RECEIVE"]["EVENTOS"][i] = PLCDataExchange.FromPLC.Evento[i];
  }

  json["SEND"]["LIFE_BIT"] = PLCDataExchange.ToPLC.LifeBit;
  json["SEND"]["IN_AUTO_MODE"] = PLCDataExchange.ToPLC.InAutoMode;
  json["SEND"]["ACTUAL_PROG"] = PLCDataExchange.ToPLC.ActualProg;
  for (int i = 0; i < 16; i++) {
    json["SEND"]["ORDENS"][i] = PLCDataExchange.ToPLC.Ordem[i];
  }

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void APIPostJointCalibration() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    if (doc.containsKey("Joint1")) {
      if (doc["Joint1"]) {
        ResetJointPosition(1);
      }
    }

    if (doc.containsKey("Joint2")) {
      if (doc["Joint2"]) {
        ResetJointPosition(2);
      }
    }

    if (doc.containsKey("Joint3")) {
      if (doc["Joint3"]) {
        ResetJointPosition(3);
      }
    }

    if (doc.containsKey("Joint4")) {
      if (doc["Joint4"]) {
        ResetJointPosition(4);
      }
    }

    if (doc.containsKey("Joint5")) {
      if (doc["Joint5"]) {
        ResetJointPosition(5);
      }
    }

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostJointLimits() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    if (doc.containsKey("Joint1")) {
      joint1Param.minAngle = doc["Joint1"]["MinAngle"];
      joint1Param.maxAngle = doc["Joint1"]["MaxAngle"];
      Parameters["Joint1Param"] = doc["Joint1"];
    }

    if (doc.containsKey("Joint2")) {
      joint2Param.minAngle = doc["Joint2"]["MinAngle"];
      joint2Param.maxAngle = doc["Joint2"]["MaxAngle"];
      Parameters["Joint2Param"] = doc["Joint2"];
    }

    if (doc.containsKey("Joint3")) {
      joint3Param.minAngle = doc["Joint3"]["MinAngle"];
      joint3Param.maxAngle = doc["Joint3"]["MaxAngle"];
      Parameters["Joint3Param"] = doc["Joint3"];
    }

    if (doc.containsKey("Joint4")) {
      joint4Param.minAngle = doc["Joint4"]["MinAngle"];
      joint4Param.maxAngle = doc["Joint4"]["MaxAngle"];
      Parameters["Joint4Param"] = doc["Joint4"];
    }

    if (doc.containsKey("Joint5")) {
      joint5Param.minAngle = doc["Joint5"]["MinAngle"];
      joint5Param.maxAngle = doc["Joint5"]["MaxAngle"];
      Parameters["Joint5Param"] = doc["Joint5"];
    }

    if (doc.containsKey("Gripper")) {
      gripperParam.minAngle = doc["Gripper"]["MinAngle"];
      gripperParam.maxAngle = doc["Gripper"]["MaxAngle"];
      Parameters["GripperParam"] = doc["Gripper"];
    }

    SaveParameters();

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostGripperPosition() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    gripperParam.angleOpenned = doc["Gripper"]["PosOpen"];
    gripperParam.angleClosed = doc["Gripper"]["PosClose"];
    Parameters["GripperParam"] = doc["Gripper"];

    SaveParameters();

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostRobotState() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    RobotState.ModoOperacao.Manual = doc["MODO_OPERACAO"]["MANUAL"];
    RobotState.ModoOperacao.ManualCem = doc["MODO_OPERACAO"]["MANUAL_CEM"];
    RobotState.ModoOperacao.Auto = doc["MODO_OPERACAO"]["AUTOMATICO"];
    RobotState.Speed = doc["SPEED"];

    RobotState.MoveType.MoveLinear = doc["MOVE_TYPE"]["MOVE_LINEAR"];
    RobotState.MoveType.MoveJoint = doc["MOVE_TYPE"]["MOVE_JOINT"];
    RobotState.MoveType.MoveAngle = doc["MOVE_TYPE"]["MOVE_ANGLE"];

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostSaveExternalCode() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    CompilerData.Code = doc["code"].as<String>();  //Guarda o codigo e aguarda inicializar

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostStartExternalCode() {
  CompilerData.PlayCode = true;
  // Resposta ao cliente
  server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
}
void APIPostPLCConfig() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");  // Recebe o corpo da requisição

    // Cria um objeto JSON para armazenar os dados recebidos
    StaticJsonDocument<200> doc;

    // Deserializa o JSON recebido
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("Erro ao parsear JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"JSON inválido\"}");
      return;
    }

    PLCConnectionData.IP[0] = doc["IP"][0];
    PLCConnectionData.IP[1] = doc["IP"][1];
    PLCConnectionData.IP[2] = doc["IP"][2];
    PLCConnectionData.IP[3] = doc["IP"][3];

    PLCConnectionData.Rack = doc["RACK"];
    PLCConnectionData.Slot = doc["SLOT"];

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
void APIPostPLCConectar() {
  bool sucesso = PLCConnect();

  // Resposta ao cliente
  server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
}
void APIPostPLCDesconectar() {
  PLCDisconnect();

  // Resposta ao cliente
  server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
}
/************************************/

/************* COMPILER ****************/
jsval_t WaitTime(struct js *js, jsval_t *args, int nargs) {
  delay(js_getnum(args[0]));
  return js_mknum(0);
}
jsval_t OpenGripper(struct js *js, jsval_t *args, int nargs) {
  MoveGripperPosition(gripperParam.angleOpenned, 100);
  while (!GripperOpenned()) {
    delay(1);
  }
  return js_mknum(0);
}
jsval_t CloseGripper(struct js *js, jsval_t *args, int nargs) {
  Serial.println("Comando CloseGripper");
  MoveGripperPosition(gripperParam.angleClosed, 100);
  while (!GripperClosed()) {
    delay(1);
  }
  return js_mknum(0);
}
jsval_t SetAprox(struct js *js, jsval_t *args, int nargs) {
  CompilerData.Aproximacao = js_getnum(args[0]);
  return js_mknum(0);
}
jsval_t SetSpeed(struct js *js, jsval_t *args, int nargs) {
  Serial.println("Comando Speed");
  CompilerData.Speed = js_getnum(args[0]);
  return js_mknum(0);
}
jsval_t PLCReadEvent(struct js *js, jsval_t *args, int nargs) {
  int index = js_getnum(args[0]);
  if (index >= 1 && index <= 16) {
    return js_mknum(PLCDataExchange.FromPLC.Evento[index - 1]);
  }
  return js_mknum(0);
}
jsval_t PLCSetOrdem(struct js *js, jsval_t *args, int nargs) {
  int index = js_getnum(args[0]);
  if (index >= 1 && index <= 16) {
    PLCDataExchange.ToPLC.Ordem[index - 1] = true;
  }
  return js_mknum(0);
}
jsval_t PLCResetOrdem(struct js *js, jsval_t *args, int nargs) {
  int index = js_getnum(args[0]);
  if (index >= 1 && index <= 16) {
    PLCDataExchange.ToPLC.Ordem[index - 1] = false;
  }
  return js_mknum(0);
}
/**
 * @brief Move all joints.
 *
 * Move all joints to the target passed by parameters.
 *
 * @param arg[1] The angle to the joint 1.
 * @param arg[2] The angle to the joint 2.
 * @param arg[3] The angle to the joint 3.
 * @param arg[4] The angle to the joint 4.
 * @param arg[5] The angle to the joint 5.
 * @return void.
 */
jsval_t MoveJ(struct js *js, jsval_t *args, int nargs) {
  //Serial.println(js_getnum(args[0]) + js_getnum(args[1]));

  //Move para o ponto todos os motores
  for (int i = 0; i < 5; i++) {
    MoveJointPosition(i + 1, js_getnum(args[i]), CompilerData.Speed);
  }

  //Aguarda o movimento estar finalizado
  /*
  int jointMoveFinished = 0;
  while(jointMoveFinished!=5){
    jointMoveFinished = 0;
    for(int i = 0; i < 5; i++){
      if (TargetReached(i+1, CompilerData.Aproximacao, js_getnum(args[i]))){
        jointMoveFinished++;
      }
    }
  }
  */
  return js_mknum(0);
}
jsval_t MoveGripper(struct js *js, jsval_t *args, int nargs) {
  //js_getnum(args[0]) + js_getnum(args[1]));

  //Move gripper para a posicao em angulo
  MoveGripperPosition(js_getnum(args[0]), 100);
  while (js_getnum(args[0]) != GripperGetCurrentAngle()) {
    delay(10);
  }

  return js_mknum(0);
}
int ExecuteJS(const char *jsCode) {
  // Calcule o tamanho do código JavaScript
  size_t codeLength = strlen(jsCode);  // Obtém o tamanho da string

  // Calcule o tamanho total do buffer necessário
  size_t totalBufferSize = codeLength + 200;
  totalBufferSize = 5000;

  // Se o comprimento for zero, retorna
  if (codeLength == 0) return 0;

  // Cria a instância JS com o buffer calculado
  struct js *js = js_create(new char[totalBufferSize], totalBufferSize);
  if (js == nullptr) {
    Serial.println("Falha ao criar instância JS.");
    return 0;
  }

  jsval_t global = js_glob(js);
  //Imports
  js_set(js, global, "MoveJ", js_mkfun(MoveJ));
  js_set(js, global, "SetSpeed", js_mkfun(SetSpeed));
  js_set(js, global, "SetAprox", js_mkfun(SetAprox));
  js_set(js, global, "MoveGripper", js_mkfun(MoveGripper));
  js_set(js, global, "OpenGripper", js_mkfun(OpenGripper));
  js_set(js, global, "CloseGripper", js_mkfun(CloseGripper));
  js_set(js, global, "WaitTime", js_mkfun(WaitTime));

  js_set(js, global, "PLCReadEvent", js_mkfun(PLCReadEvent));
  js_set(js, global, "PLCSetOrdem", js_mkfun(PLCSetOrdem));
  js_set(js, global, "PLCResetOrdem", js_mkfun(PLCResetOrdem));

  // Executa o código JS
  jsval_t v = js_eval(js, jsCode, ~0U);
  return 1;
}
/************************************/

/************* PLC ****************/
bool PLCConnect() {
  PLCDataExchangeType PLCDataExchange;

  IPAddress PLC(
    PLCConnectionData.IP[0],
    PLCConnectionData.IP[1],
    PLCConnectionData.IP[2],
    PLCConnectionData.IP[3]);  // PLC Address

  int Result = PLCClient.ConnectTo(PLC, PLCConnectionData.Rack, PLCConnectionData.Slot);

  if (Result != 0) {
    Serial.print("Erro de conexao com o PLC - IP:");
    Serial.print(PLCConnectionData.IP[0]);
    Serial.print(".");
    Serial.print(PLCConnectionData.IP[2]);
    Serial.print(".");
    Serial.print(PLCConnectionData.IP[3]);
    Serial.print(".");
    Serial.print(PLCConnectionData.IP[4]);
    Serial.print(" - Rack: ");
    Serial.print(PLCConnectionData.Rack);
    Serial.print(" - Slot: ");
    Serial.print(PLCConnectionData.Slot);
    Serial.println("");
  }

  PLCConnectionData.Connected = Result == 0;
  PLCConnectionData.plcLastUpdate = millis();

  return Result == 0;
}
void PLCDisconnect() {
  PLCClient.Disconnect();
  PLCConnectionData.Connected = false;
}
void PLCRead() {
  if (PLCConnectionData.Connected) {
    byte Buffer[512];  //Tamanho da PDU 512 bytes
    for (int i = 0; i < 6; i++) {
      Buffer[i] = 0;
    }

    uint Result = PLCClient.ReadArea(S7AreaDB, PLCConnectionData.DBReceive, 0, 3, &Buffer);

    //Erro critico. Tem que desconectar
    if (Result & 0x00FF) {
      Serial.println("PLC SEVERE ERROR, disconnecting...");
      PLCDisconnect();
      return;
    }

    //General bits
    PLCDataExchange.FromPLC.LifeBit = Buffer[0] & 0x01;
    PLCDataExchange.FromPLC.AutoMode = (Buffer[0] >> 1) & 0x01;
    PLCDataExchange.FromPLC.SafetyOK = (Buffer[0] >> 2) & 0x01;
    //Prog num
    PLCDataExchange.FromPLC.ProgNum = swap(Buffer[2] | (Buffer[3] << 8));
    //Events bits
    for (int i = 0; i < 8; i++) {
      PLCDataExchange.FromPLC.Evento[i] = (Buffer[4] >> i) & 0x01;
    }
    for (int i = 0; i < 8; i++) {
      PLCDataExchange.FromPLC.Evento[i + 8] = (Buffer[5] >> i) & 0x01;
    }
  }
}
void PLCWrite() {
  if (PLCConnectionData.Connected) {
    byte Buffer[512];  //Tamanho da PDU 512 bytes
    for (int i = 0; i < 6; i++) {
      Buffer[i] = 0;
    }

    // General data
    Buffer[0] |= PLCDataExchange.ToPLC.LifeBit & 0x01;            //Life bit
    Buffer[0] |= (PLCDataExchange.ToPLC.InAutoMode & 0x01) << 1;  //In Auto Mode

    // Bytes 1 e 2: ActualProg
    Buffer[2] = PLCDataExchange.ToPLC.ActualProg & 0xFF;         // LSB
    Buffer[3] = (PLCDataExchange.ToPLC.ActualProg >> 8) & 0xFF;  // MSB

    // Bytes 3 e 4: Ordem bits
    for (int i = 0; i < 8; i++) {
      Buffer[4] |= (PLCDataExchange.ToPLC.Ordem[i] & 0x01) << i;
    }

    for (int i = 0; i < 8; i++) {
      Buffer[5] |= (PLCDataExchange.ToPLC.Ordem[i + 8] & 0x01) << i;
    }

    int result = PLCClient.WriteArea(S7AreaDB, PLCConnectionData.DBSend, 0, 3, &Buffer);

    if (result != 0) {
      Serial.println("Erro ao enviar os dados para o PLC");
    }
  }
}
/************************************/

/************* JOINTS ****************/
void EnableAllMotors() {
  for (int i = 1; i <= 5; i++) {
    EnableJoint(i);
  }
}
void DisableAllMotors() {
  for (int i = 1; i <= 5; i++) {
    DisableJoint(i);
  }
}
void EnableJoint(int index) {
  if (index == 1) {
    digitalWrite(joint1Param.pinEnable, LOW);
  }
  if (index == 2) {
    digitalWrite(joint2Param.pinEnable, LOW);
  }
  //if (index == 3) {
  //  digitalWrite(join3Param.pinEnable, LOW);
  //}
  if (index == 4) {
    digitalWrite(joint4Param.pinEnable, LOW);
  }
  if (index == 5) {
    digitalWrite(joint5Param.pinEnable, LOW);
  }
}
void DisableJoint(int index) {
  if (index == 1) {
    digitalWrite(joint1Param.pinEnable, HIGH);
  }
  if (index == 2) {
    digitalWrite(joint2Param.pinEnable, HIGH);
  }
  //if (index == 3) {
  //  digitalWrite(joint3Param.pinEnable, HIGH);
  //}
  if (index == 4) {
    digitalWrite(joint4Param.pinEnable, HIGH);
  }
  if (index == 5) {
    digitalWrite(joint5Param.pinEnable, HIGH);
  }
}
void StopAllMotors() {
  for (int i = 1; i <= 6; i++) {
    StopMotor(i);
  }
}
void StopMotor(int index) {
  // if (!IsJointMoving(index))
  // {
  //   return;
  // }

  if (index == 1) {
    joint1.rotate(0);
  }

  if (index == 2) {
    joint2.rotate(0);
  }

  if (index == 3) {
    joint3.rotate(0);
  }

  if (index == 4) {
    joint4.rotate(0);
  }

  if (index == 5) {
    joint5.rotate(0);
  }
}
void EmergencyStop() {
  joint1.stop();
  joint2.stop();
  joint3.stop();
  joint4.stop();
  joint5.stop();
  StopGripper();
}
void MoveJointSpeed(int index, float speed, int ramp) {
  bool direction = speed > 0;

  if (speed < 0) {
    speed *= -1;
  }

  if (speed == 0) {
    StopMotor(index);
    return;
  }

  if (index == 1) {
    joint1.setRampLen(ramp);
    joint1.setSpeed(ConvertSpeed(speed, joint1Param.jointRatio));

    if (InnerRange(GetCurrentAngle(1), joint1Param.minAngle, joint1Param.maxAngle)) {
      joint1.rotate(direction ? 1 : -1);
    } else if (GetCurrentAngle(1) > joint1Param.maxAngle && direction < 0) {
      joint1.rotate(-1);
    } else if (GetCurrentAngle(1) < joint1Param.minAngle && direction > 0) {
      joint1.rotate(1);
    } else {
      StopMotor(1);
    }
  }

  if (index == 2) {
    joint2.setRampLen(ramp);
    joint2.setSpeed(ConvertSpeed(speed, joint2Param.jointRatio));
    if (InnerRange(GetCurrentAngle(2), joint2Param.minAngle, joint2Param.maxAngle)) {
      joint2.rotate(direction ? 1 : -1);
    } else if (GetCurrentAngle(2) > joint2Param.maxAngle && direction < 0) {
      joint2.rotate(-1);
    } else if (GetCurrentAngle(2) < joint2Param.minAngle && direction > 0) {
      joint2.rotate(1);
    } else {
      StopMotor(2);
    }
  }

  if (index == 3) {
    joint3.setRampLen(ramp);
    joint3.setSpeed(ConvertSpeed(speed, joint3Param.jointRatio));
    if (InnerRange(GetCurrentAngle(3), joint3Param.minAngle, joint3Param.maxAngle)) {
      joint3.rotate(direction ? 1 : -1);
    } else if (GetCurrentAngle(3) > joint3Param.maxAngle && direction < 0) {
      joint3.rotate(-1);
    } else if (GetCurrentAngle(3) < joint3Param.minAngle && direction > 0) {
      joint3.rotate(1);
    } else {
      StopMotor(3);
    }
  }

  if (index == 4) {
    joint4.setRampLen(ramp);
    joint4.setSpeed(ConvertSpeed(speed, joint4Param.jointRatio));
    if (InnerRange(GetCurrentAngle(4), joint4Param.minAngle, joint4Param.maxAngle)) {
      joint4.rotate(direction ? 1 : -1);
    } else if (GetCurrentAngle(4) > joint4Param.maxAngle && direction < 0) {
      joint4.rotate(-1);
    } else if (GetCurrentAngle(4) < joint4Param.minAngle && direction > 0) {
      joint4.rotate(1);
    } else {
      StopMotor(4);
    }
  }

  if (index == 5) {
    joint5.setRampLen(ramp);
    joint5.setSpeed(ConvertSpeed(speed, joint5Param.jointRatio));
    if (InnerRange(GetCurrentAngle(5), joint5Param.minAngle, joint5Param.maxAngle)) {
      joint5.rotate(direction ? 1 : -1);
    } else if (GetCurrentAngle(5) > joint5Param.maxAngle && direction < 0) {
      joint5.rotate(-1);
    } else if (GetCurrentAngle(5) < joint5Param.minAngle && direction > 0) {
      joint5.rotate(1);
    } else {
      StopMotor(5);
    }
  }
}
void MoveJointPosition(int index, float position, float speed) {
  int ramp = 1;

  if (speed == 0) {
    StopMotor(index);
    return;
  }

  if (index == 1) {
    if (GetCurrentAngle(1) != position) {
      joint1.setSpeed(ConvertSpeed(speed, joint1Param.jointRatio));
      joint1.setRampLen(ramp);
      joint1.write(range(position * joint1Param.jointRatio, joint1Param.minAngle, joint1Param.maxAngle));
    }
  }

  if (index == 2) {
    if (GetCurrentAngle(2) != position) {
      joint2.setSpeed(ConvertSpeed(speed, joint2Param.jointRatio));
      joint2.setRampLen(ramp);
      joint2.write(range(position * joint2Param.jointRatio, joint2Param.minAngle, joint2Param.maxAngle));
    }
  }

  if (index == 3) {
    if (GetCurrentAngle(3) != position) {
      joint3.setSpeed(ConvertSpeed(speed, joint3Param.jointRatio));
      joint3.setRampLen(ramp);
      joint3.write(range(position * joint3Param.jointRatio, joint3Param.minAngle, joint3Param.maxAngle));
    }
  }

  if (index == 4) {
    if (GetCurrentAngle(4) != position) {
      joint4.setSpeed(ConvertSpeed(speed, joint4Param.jointRatio));
      joint4.setRampLen(ramp);
      joint4.write(range(position * joint4Param.jointRatio, joint4Param.minAngle, joint4Param.maxAngle));
    }
  }

  if (index == 5) {
    if (GetCurrentAngle(5) != position) {
      joint5.setSpeed(ConvertSpeed(speed, joint5Param.jointRatio));
      joint5.setRampLen(ramp);
      joint5.write(range(position * joint5Param.jointRatio, joint5Param.minAngle, joint5Param.maxAngle));
    }
  }
}
float ConvertSpeed(float speedPorCento, float axisRatio) {
  float maxSpeed = 45;  //°/s max

  float speed = speedPorCento / 100 * maxSpeed;         //Converte porcentagem da velocidade
  speed = (speed * 60 /*segundos*/) / 360.0 /*graus*/;  //converte para RPM
  speed = speed * 10;                                   //Constante da biblioteca
  speed = speed / axisRatio;
  return speed;  //Multiplica pela razão da polia com o eixo
}
float GetCurrentAngle(int joint) {
  if (joint == 1) {
    return ((float)joint1.read()) * joint1Param.jointRatio;
  }

  if (joint == 2) {
    return ((float)joint2.read()) * joint2Param.jointRatio;
  }

  if (joint == 3) {
    return ((float)joint3.read()) * joint3Param.jointRatio;
  }

  if (joint == 4) {
    return ((float)joint4.read()) * joint4Param.jointRatio;
  }

  if (joint == 5) {
    return ((float)joint5.read()) * joint5Param.jointRatio;
  }

  return 0;
}
bool TargetReached(int index, int zone, long target) {
  long angle = GetCurrentAngle(index);
  bool aproxMin = abs(GetCurrentAngle(index) - target);
  return aproxMin < zone;
}
bool IsJointMoving(int joint) {
  if (joint == 1) {
    return joint1.moving() != 0;
  }

  if (joint == 2) {
    return joint2.moving() != 0;
  }

  if (joint == 3) {
    return joint3.moving() != 0;
  }

  if (joint == 4) {
    return joint4.moving() != 0;
  }

  if (joint == 5) {
    return joint5.moving() != 0;
  }

  return false;
}
void ResetJointPosition(int index) {
  if (index == 1) {
    joint1.setZero();
  }

  if (index == 2) {
    joint2.setZero();
  }

  if (index == 3) {
    joint3.setZero();
  }

  if (index == 4) {
    joint4.setZero();
  }

  if (index == 5) {
    joint5.setZero();
  }
}
/************************************/

/************* GRIPPER ****************/
void StopGripper() {
  return;
  gripper.write(gripper.read());
}
void MoveGripperPosition(long position, float speed) {
  if (speed == 0) {
    StopGripper();
    return;
  }

  if (GripperGetCurrentAngle() != position) {
    gripper.setSpeed(speed);
    gripper.write(range(position, gripperParam.minAngle, gripperParam.maxAngle));
  }
}
void MoveGripperSpeed(float speed) {
  MoveGripperPosition(GripperGetCurrentAngle() + (speed > 0 ? 1 : -1), speed);  //Soma 1 na posição atual
}
byte GripperGetCurrentAngle() {
  return gripper.read();
}
bool IsGripperMoving() {
  return gripper.moving() != 0;
}
bool GripperOpenned() {
  return gripper.read() == gripperParam.angleOpenned;
}
bool GripperClosed() {
  return gripper.read() == gripperParam.angleClosed;
}
bool ResetGripperPosition() {
  gripper.detach();
  gripper.attach(gripperParam.pinTrigger);
}
/************************************/

/************* UTILS ****************/
void ReadParameters() {
  Serial.println("Reading Parameters");
  String data = readFile(SPIFFS, "/Parameters.json");

  if (data == "") {
    Serial.println("Parameters not found!");

    char buffer[2000];
    serializeJson(Parameters, buffer);
    writeFile(SPIFFS, "/Parameters.json", buffer);
  } else {
    DeserializationError error = deserializeJson(Parameters, data);

    if (error) {
      Serial.println("Erro ao ler os Parametros do Robo");
    } else {
      Serial.println("Parameters read with success");
    }

    if (Parameters.containsKey("GripperParam")) {
      gripperParam.minAngle = Parameters["GripperParam"]["MinAngle"];
      gripperParam.maxAngle = Parameters["GripperParam"]["MaxAngle"];
      gripperParam.angleClosed = Parameters["GripperParam"]["PosClose"];
      gripperParam.angleOpenned = Parameters["GripperParam"]["PosOpen"];

      joint1Param.minAngle = Parameters["Joint1Param"]["MinAngle"];
      joint1Param.maxAngle = Parameters["Joint1Param"]["MaxAngle"];

      joint2Param.minAngle = Parameters["Joint2Param"]["MinAngle"];
      joint2Param.maxAngle = Parameters["Joint2Param"]["MaxAngle"];

      joint3Param.minAngle = Parameters["Joint3Param"]["MinAngle"];
      joint3Param.maxAngle = Parameters["Joint3Param"]["MaxAngle"];

      joint4Param.minAngle = Parameters["Joint4Param"]["MinAngle"];
      joint4Param.maxAngle = Parameters["Joint4Param"]["MaxAngle"];

      joint5Param.minAngle = Parameters["Joint5Param"]["MinAngle"];
      joint5Param.maxAngle = Parameters["Joint5Param"]["MaxAngle"];
    }
  }
}
uint16_t swap(uint16_t data) {
  return (data >> 8) | (data << 8);
}
float mapFloat(float value, float minVal, float maxVal, float min, float max) {
  return (value - minVal) * (max - min) / (maxVal - minVal) + min;
}
bool InnerRange(long value, long min, long max) {
  return true;
  return value > min && value < max;
}
float range(float value, float min, float max) {
  if (value < min) {
    value = min;
  }

  if (value > max) {
    value = max;
  }

  return value;
}
void SaveParameters() {
  char buffer[2000];
  serializeJson(Parameters, buffer);
  writeFile(SPIFFS, "/Parameters.json", buffer);
}
String readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <MobaTools.h>

#include "elk.h"
struct CompilerDataType{
  int Speed;
  int Aceleracao;
  long Aproximacao; //Famoso zoneamento
  bool PlayCode;
  String Code;
};
CompilerDataType CompilerData;

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

const char *ssid = "DRONE";
const char *password = "robosoft";

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

struct RobotStateType{
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

JointParam joint1Param = { -360, 360, 1 , 13, 12, 14};
JointParam joint2Param = { -360, 360, 1 , 27, 26, 25};
JointParam joint3Param = { -360, 360, 1 , 33, 32, 0}; //Sem enable
JointParam joint4Param = { -360, 360, 1 , 4, 16, 17};
JointParam joint5Param = { -360, 360, 1 , 5, 18, 19};
JointParam gripperParam = { 0, 90 };

ManualMoveType Commands;

MoToStepper joint1(6400, STEPDIR);
MoToStepper joint2(8000, STEPDIR);
MoToStepper joint3(20000, STEPDIR);
MoToStepper joint4(6400, STEPDIR);
MoToStepper joint5(6400, STEPDIR);

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

  joint1.attach(join1Param.pinTrigger, join1Param.pinDir);  //14 enable
  joint2.attach(join2Param.pinTrigger, join2Param.pinDir);  //25
  joint3.attach(join3Param.pinTrigger, join3Param.pinDir);  //sem enable
  joint4.attach(join4Param.pinTrigger, join4Param.pinDir);  //17
  joint5.attach(join5Param.pinTrigger, join5Param.pinDir);    //19

  pinMode(join1Param.pinEnable, OUTPUT);
  pinMode(join2Param.pinEnable, OUTPUT);
  //pinMode(join3Param.pinEnable, OUTPUT);
  pinMode(join4Param.pinEnable, OUTPUT);
  pinMode(join5Param.pinEnable, OUTPUT);

  Serial.println("Finalizado as configuracoes dos motores");

  // Inicia o WebSocket server
  Serial.println("Iniciando o wesocket");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.enableCORS();
  server.on("/getRobotStatus", APIGetRobotStatus);
  server.on("/getJointAngles", APIGetJointAngles);
  server.on("/postJointCalibration", HTTP_POST, APIPostJointCalibration);
  server.on("/postJointLimits", HTTP_POST, APIPostJointLimits);
  server.on("/postRobotState", HTTP_POST, APIPostRobotState);
  server.on("/postSaveExternalCode", HTTP_POST, APIPostSaveExternalCode);
  server.begin();

  Serial.println("Inicializa as tasks");

  xTaskCreate(TaskWebServerAPI, "TaskWebServerAPI", 10000, NULL, 1, NULL);
  xTaskCreate(TaskWebSocket, "TaskWebSocket", 10000, NULL, 1, NULL);
  xTaskCreate(TaskCheckComm, "TaskCheckComm", 10000, NULL, 1, NULL);
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
  Serial.print(",Joint2:");
  Serial.print(Commands.MoveJoint.Joint2);
  Serial.print(",Speed:");
  Serial.println(RobotState.Speed);

  if (Commands.General.DeadMan) {
    if (RobotState.MoveType.MoveJoint) {
      MoveJointSpeed(1, Commands.MoveJoint.Joint1 * RobotState.Speed, 10);
      MoveJointSpeed(2, Commands.MoveJoint.Joint2 * RobotState.Speed, 10);
      MoveJointSpeed(3, Commands.MoveJoint.Joint3 * RobotState.Speed, 10);
      MoveJointSpeed(4, Commands.MoveJoint.Joint4 * RobotState.Speed, 10);
      MoveJointSpeed(5, Commands.MoveJoint.Joint5 * RobotState.Speed, 10);
    }
  } else {
    StopAllMotors();
  }
}
/************************************/

/************* TASKS ****************/
void TaskExternalProg(void *pvParameters) {
  Serial.println("Task TaskExternalProg Iniciada");

  while (true) {
    if (RobotState.ModoOperacao.Auto){
      if (CompilerData.PlayCode){
        ExecuteJS(CompilerData.Code.c_str());
        CompilerData.PlayCode = false;
      }else{
        delay(500);
      }
    }else{
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
      //Commands.General.DeadMan  = false;
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

  json["MOVE_TYPE"]["ROTATIONAL"] = RobotState.MoveType.MoveAngle;
  json["MOVE_TYPE"]["JOINT"] = RobotState.MoveType.MoveJoint;
  json["MOVE_TYPE"]["LINEAR"] = RobotState.MoveType.MoveLinear;

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
void APIGetJointAngles() {
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  jsonDocument.clear();  // Clear json buffer
  JsonObject json = jsonDocument.to<JsonObject>();
  json["Joint1"] = GetCurrentAngle(1);
  json["Joint2"] = 0;
  json["Joint3"] = 0;
  json["Joint4"] = 0;
  json["Joint5"] = 0;
  json["Gripper"] = 0;

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

    if (doc.containsKey("Gripper")) {
      if (doc["Gripper"]) {
        ResetGripperPosition();
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
void APIPostSaveExternalCode(){
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

    CompilerData.Code  = doc["code"].as<String>(); //Guarda o codigo e aguarda inicializar

    // Resposta ao cliente
    server.send(200, "application/json", "{\"status\":\"sucesso\",\"msg\":\"JSON recebido com sucesso\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"erro\",\"msg\":\"Corpo vazio no POST\"}");
  }
}
/************************************/

/************* COMPILER ****************/

jsval_t Wait(struct js *js, jsval_t *args, int nargs) {
  delay(js_getnum(args[0]));
  return js_mknum(0);
}
jsval_t OpenGripper(struct js *js, jsval_t *args, int nargs) {
  
  return js_mknum(0);
}
jsval_t CloseGripper(struct js *js, jsval_t *args, int nargs) {
  
  return js_mknum(0);
}
jsval_t SetAprox(struct js *js, jsval_t *args, int nargs) {
  CompilerData.Aproximacao = js_getnum(args[0]);
  return js_mknum(0);
}
jsval_t SetSpeed(struct js *js, jsval_t *args, int nargs) {
  CompilerData.Speed = js_getnum(args[0]);
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
  for(int i = 0; i < 6; i++){
    MoveJointPosition(i+1, args[i], CompilerData.Speed, CompilerData.Aceleracao);
  }

  //Aguarda o movimento estar finalizado
  int jointMoveFinished = 0;
  while(jointMoveFinished!=6){
    jointMoveFinished = 0;
    for(int i = 0; i < 6; i++){
      if (TargetReached(i+1, CompilerData.Aproximacao, args[i])){
        jointMoveFinished++;
      }
    }
  }

  return js_mknum(0);
}
int ExecuteJS(const char* jsCode) {
  // Calcule o tamanho do código JavaScript
  size_t codeLength = strlen(jsCode);  // Obtém o tamanho da string

  // Calcule o tamanho total do buffer necessário
  size_t totalBufferSize = codeLength + 200;

  // Se o comprimento for zero, retorna
  if (codeLength == 0) return 0;

  // Cria a instância JS com o buffer calculado
  struct js* js = js_create(new char[totalBufferSize], totalBufferSize);
  if (js == nullptr) {
      Serial.println("Falha ao criar instância JS.");
      return 0;
  }

  jsval_t global = js_glob(js);
  //Imports
  js_set(js, global, "MoveJ", js_mkfun(MoveJ));
  js_set(js, global, "SetSpeed", js_mkfun(SetSpeed));
  js_set(js, global, "SetAprox", js_mkfun(SetAprox));
  js_set(js, global, "OpenGripper", js_mkfun(OpenGripper));
  js_set(js, global, "CloseGripper", js_mkfun(CloseGripper));
  js_set(js, global, "Wait", js_mkfun(Wait));

  // Executa o código JS
  jsval_t v = js_eval(js, jsCode, ~0U);
  return 1;
}
/************************************/

/************* UTILS ****************/
void ReadParameters() {
  Serial.println("Reading Parameters");
  String data = readFile(SPIFFS, "/Parameters.json");

  if (data == "") {
    Serial.println("Parameters not found!");

    Parameters["param1"] = 1.0;

    char buffer[2000];
    serializeJson(Parameters, buffer);
    writeFile(SPIFFS, "/Parameters.json", buffer);
  } else {
    DeserializationError error = deserializeJson(Parameters, data);

    if (error) {
      Serial.println("Erro ao ler os Parametros do Drone");
    } else {
      Serial.println("Parameters read with success");
    }
  }
}
float mapFloat(float value, float minVal, float maxVal, float min, float max) {
  return (value - minVal) * (max - min) / (maxVal - minVal) + min;
}
void EnableJoint(int index) {
  if (index == 1) {
    digitalWrite(join1Param.pinEnable, LOW);
  }
  if (index == 2) {
    digitalWrite(join2Param.pinEnable, LOW);
  }
  //if (index == 3) {
  //  digitalWrite(join3Param.pinEnable, LOW);
  //}
  if (index == 4) {
    digitalWrite(join4Param.pinEnable, LOW);
  }
  if (index == 5) {
    digitalWrite(join5Param.pinEnable, LOW);
  }
}
void DisableJoint(int index) {
  if (index == 1) {
    digitalWrite(join1Param.pinEnable, HIGH);
  }
  if (index == 2) {
    digitalWrite(join2Param.pinEnable, HIGH);
  }
  //if (index == 3) {
  //  digitalWrite(join3Param.pinEnable, HIGH);
  //}
  if (index == 4) {
    digitalWrite(join4Param.pinEnable, HIGH);
  }
  if (index == 5) {
    digitalWrite(join5Param.pinEnable, HIGH);
  }
}
void StopAllMotors() {
  for (int i = 1; i <= 6; i++) {
    StopMotor(i);
  }
}
void StopMotor(int index) {
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
    joint1.setSpeed(speed * 10);
    
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
    joint2.setSpeed(speed * 10);
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
    joint3.setSpeed(speed * 10);
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
    joint4.setSpeed(speed * 10);
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
    joint5.setSpeed(speed * 10);
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
void MoveJointPosition(int index, long position, float speed, int ramp) {
  if (index == 1) {
    if (joint1.currentPosition() != position) {
      joint1.setSpeed(speed * 10);
      joint1.setRampLen(ramp);
      joint1.write(range(position * joint1Param.jointRatio, joint1Param.minAngle, joint1Param.maxAngle));
    }
  }

  if (index == 2) {
    if (joint2.currentPosition() != position) {
      joint2.setSpeed(speed * 10);
      joint2.setRampLen(ramp);
      joint2.write(range(position * joint2Param.jointRatio, joint2Param.minAngle, joint2Param.maxAngle));
    }
  }

  if (index == 3) {
    if (joint3.currentPosition() != position) {
      joint3.setSpeed(speed * 10);
      joint3.setRampLen(ramp);
      joint3.write(range(position * joint3Param.jointRatio, joint3Param.minAngle, joint3Param.maxAngle));
    }
  }

  if (index == 4) {
    if (joint4.currentPosition() != position) {
      joint4.setSpeed(speed * 10);
      joint4.setRampLen(ramp);
      joint4.write(range(position * joint4Param.jointRatio, joint4Param.minAngle, joint4Param.maxAngle));
    }
  }

  if (index == 5) {
    if (joint5.currentPosition() != position) {
      joint5.setSpeed(speed * 10);
      joint5.setRampLen(ramp);
      joint5.write(range(position * joint5Param.jointRatio, joint5Param.minAngle, joint5Param.maxAngle));
    }
  }
}
long GetCurrentAngle(int joint) {
  if (joint == 1) {
    return joint1.read() * joint1Param.jointRatio;
  }

  if (joint == 2) {
    return joint2.read() * joint2Param.jointRatio;
  }

  if (joint == 3) {
    return joint3.read() * joint3Param.jointRatio;
  }

  if (joint == 4) {
    return joint4.read() * joint4Param.jointRatio;
  }

  if (joint == 5) {
    return joint5.read() * joint5Param.jointRatio;
  }

  return 0;
}
bool TargetReached(int index, int zone, long target){
  long angle = GetCurrentAngle(index);
  bool aproxMin = abs(GetCurrentAngle(index) - target);
  return aproxMin < zone;
}
bool IsJointMoving(int joint) {
  if (joint == 1) {
    return joint1.moving() != 0;
  }

  return false;
}
void ResetJointPosition(int index) {
  if (index == 1) {
    joint1.setZero();
  }
}
void ResetGripperPosition() {
}
bool InnerRange(long value, long min, long max) {
  return true;
  return value > min && value < max;
}
int range(int value, int min, int max) {
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
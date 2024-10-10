#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <MobaTools.h>

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);

const char* ssid = "DRONE";
const char* password = "robosoft";

struct ManualMoveType {
  struct{
      bool MoveAngle;
      bool MoveJoint;
      bool MoveLinear;
    } MoveType;
  
  struct{
    float MoveX;
    float MoveY;
    float MoveZ;
    float Speed;
  } MoveAngle;

  struct{
    float MoveX;
    float MoveY;
    float MoveZ;
    float Speed;
  } MoveLinear;

  struct{
    float Move;
    int JointNumber;
    float Speed;
  } MoveJoint;

  struct{
    bool DeadMan;
    bool EnableManualMove;
    bool StartAutomatic;
  } General;

  struct{
    float LastUpdate;
    bool IsOnline;
  } Telemetry;
};

struct JointParam{
  int minAngle;
  int maxAngle;
};

JointParam joint1Param = {-360, 360};
JointParam joint2Param = {-360, 360};
JointParam joint3Param = {-360, 360};
JointParam joint4Param = {-360, 360};
JointParam joint5Param = {-360, 360};

ManualMoveType Commands;

MoToStepper joint1( 6400, STEPDIR );


//Parameters
StaticJsonDocument<2000> Parameters;
unsigned long lastPingTime = millis();

void setup(){
  Serial.begin(115200);
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  if (false){
    ReadParameters();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    delay(1000);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("robsonsoft")) {
    Serial.println
    ("Error starting mDNS");
    return;
  }

  //Configura Motor
  Serial.println("Configurando motores");
  joint1.attach(13, 12);
  joint1.setSpeed(600);                   // = 60 rpm
  joint1.setRampLen(100);                        // 100 steps to achive set speed
  Serial.println("Finalizado as configuracoes dos motores");

  server.enableCORS();
  //server.on("/getValues", APIGetAngles);
  //server.on("/postAxisCalibration", HTTP_POST, APIPostAxisCalibration);
  server.begin();

  Serial.println("Inicializa as tasks");
  xTaskCreatePinnedToCore(
      TaskWebServerAPI, /* Function to implement the task */
      "TaskWebServerAPI", /* Name of the task */
      20000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      NULL,  /* Task handle. */
      1);
  xTaskCreatePinnedToCore(
      TaskWebSocket, /* Function to implement the task */
      "TaskWebSocket", /* Name of the task */
      20000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      NULL,  /* Task handle. */
      1);
  xTaskCreate(TaskCheckComm, "TaskCheckComm", 1000, NULL, 1, NULL);
  Serial.println("Finaliza a inicializacao das tasks");

  //Velocidade maxima do motor
  joint1.setMaxSpeed(60);

}

void loop(){

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

void ManualMove(){
  if (Commands.MoveType.MoveJoint){
    
  }
}
/************************************/

/************* TASKS ****************/
void TaskCheckComm(void *pvParameters){
  Serial.println("Task CheckComm Iniciada");
  const TickType_t xFrequency = pdMS_TO_TICKS(300); // ciclo de 500 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();  // pega o tick atual

  while (true) {
    Commands.Telemetry.IsOnline = millis() - lastPingTime < 500;
    Commands.General.EnableManualMove  = false;
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency); // delay até o próximo ciclo
  }
}

void TaskWebServerAPI( void * parameter) {
  while(true){
    //Roda a API
    server.handleClient();
  }
}

void TaskWebSocket( void * parameter) {
  while(true){
    // Mantém o WebSocket ativo
    webSocket.loop();
  }
}
/************************************/

/************* APIs ****************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type != WStype_DISCONNECTED || type != WStype_ERROR){
    lastPingTime = millis();
  }
  
  if(type == WStype_TEXT) {
    // Recebe a mensagem do cliente
    String message = String((char*) payload);
    
    // Cria um documento JSON
    StaticJsonDocument<200> doc;

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
    if (doc.containsKey("MOVE_TYPE")){
      
    }
  }
}

/************************************/

/************* UTILS ****************/
void ReadParameters(){
  Serial.println("Reading Parameters");
  String data = readFile(SPIFFS, "/Parameters.json");

  if(data == ""){
    Serial.println("Parameters not found!");

    Parameters["param1"] = 1.0;

    char buffer[2000];
    serializeJson(Parameters, buffer);
    writeFile(SPIFFS, "/Parameters.json", buffer);
  }else{
    DeserializationError error = deserializeJson(Parameters, data);

    if (error){
      Serial.println("Erro ao ler os Parametros do Drone");
    }else{
      Serial.println("Parameters read with success");
    }
  }
}

float mapFloat(float value, float minVal, float maxVal, float min, float max){
  return (value - minVal) * (max - min) / (maxVal - minVal) + min;
}
void EnableJoint(int index){
  if (index == 1){
    //digitalWrite(joint1.Pins.Enable, HIGH);
  }

}
void DisableJoint(int index){
  if (index == 1){
    //digitalWrite(joint1.Pins.Enable, LOW);
  }
}
void MoveJointSpeed(int index, bool forward, int direction, float speed, int ramp){
  if (index == 1){
    joint1.setRampLen(ramp);
    joint1.setSpeed( speed * 10 );
    if (InnerRange(GetCurrentAngle(1), joint1Param.minAngle, joint1Param.maxAngle)){
      joint1.rotate(forward ? 1 : -1);
    }else{
      StopMotor(1);
    }
  }
}
void StopMotor(int index){
  if (index == 1){
    joint1.rotate(0);
  }
}
void EmergencyStop(){
  joint1.stop();
}
void MoveJointPosition(int index, long position, float speed, int ramp){
  if (index == 1){
    if (joint1.currentPosition() != position){
      joint1.setSpeed(speed * 10);
      joint1.write(range(position, joint1Param.minAngle, joint1Param.maxAngle);
    }
  }
}
long GetCurrentAngle(int joint){
  if (joint == 1){
    return joint1.read();
  }
  return 0;
}
bool IsJointMoving(int joint){
  if (joint == 1){
    return joint1.moving() != 0;
  }

  return false;
}
void ResetJointPosition(int index){
  if (index == 1){
    joint1.setZero();
  }
}
bool InnerRange(long value, long min, long max){
  return value > min && value < max;
}
int range(int value, int min, int max){
  if (value < min){
    value = min;
  }

  if (value > max){
    value = max;
  }

  return value;
}
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
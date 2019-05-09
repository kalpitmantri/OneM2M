// Version 1.0

#include <ESP8266WiFi.h>
//#include <Adafruit_Sensor.h>
#include "Timer.h"
#include <DHT.h>

#define DHTTYPE DHT11
///////////////Parameters & Constants/////////////////
// WIFI params
char* WIFI_SSID = "ayush";    // Configure here the SSID of your WiFi Network
char* WIFI_PSWD = "ayush504"; // Configure here the PassWord of your WiFi Network
int WIFI_DELAY  = 100; //ms

// oneM2M : CSE params
String CSE_IP      = "192.168.43.24"; //Configure here the IP Address of your oneM2M CSE
int   CSE_HTTP_PORT = 8080;
String CSE_NAME    = "in-name";
String CSE_M2M_ORIGIN  = "admin:admin";

// oneM2M : resources' params
String DESC_CNT_NAME = "DESCRIPTOR";
String DATA_CNT_NAME = "DATA";
String CMND_CNT_NAME = "COMMAND";
int TY_AE  = 2;   
int TY_CNT = 3; 
int TY_CI  = 4;
int TY_SUB = 23;

// HTTP constants
int LOCAL_PORT = 9999;
char* HTTP_CREATED = "HTTP/1.1 201 Created";
char* HTTP_OK    = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms

//MISC
int LUM_PIN_TempIN = D6;  // Adapt it accordding to your wiring
int LUM_PIN_TempOUT = D7;  // Adapt it accordding to your wiring4 
int LUM_PIN_Dust = D8;  // Adapt it accordding to your wiring
int LUM_PIN_LDR_IN = D5;  // Adapt it accordding to your wiring
int LUM_PIN_LDR_OUT = D0;  // Adapt it accordding to your wiring
int LED_PIN_Light = D1;  // Adapt it accordding to your wiring. Use D0 for onboarded led
int LED_PIN_Window = D2;  // Adapt it accordding to your wiring. Use D0 for onboarded led
int LED_PIN_Curtain = D3;  // Adapt it accordding to your wiring. Use D0 for onboarded led
int LED_PIN_AC = D4;  // Adapt it accordding to your wiring. Use D0 for onboarded led

DHT dht1(LUM_PIN_TempOUT,DHTTYPE);
DHT dht2(LUM_PIN_TempIN,DHTTYPE);

#define ON_STATE  HIGH // *** External LED is active at HIGH, while Onboarded LED is active at LOW. 
#define OFF_STATE LOW  // *** Adapat it is according to your config

int SERIAL_SPEED  = 115200;

#define DEBUG

///////////////////////////////////////////


// Global variables 
Timer t;              // time object used for achiebing pseudo-parallelism by the µController
WiFiServer server(LOCAL_PORT);    // HTTP Server (over WiFi). Binded to listen on LOCAL_PORT contant
WiFiClient client;
String context = "";        // The targeted actuator
String command = "";        // The received command


String doPOST(String url, int ty, String rep) {

  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
                       "X-M2M-Origin: " + CSE_M2M_ORIGIN + "\r\n" +
                       "Content-Type: application/json;ty=" + ty + "\r\n" +
                       "Content-Length: " + rep.length() + "\r\n"
                       "Connection: close\r\n\n" +
                       rep;
  Serial.println("connecting to " + CSE_IP + ":" + CSE_HTTP_PORT + " ...");

  WiFiClient client;
  if (!client.connect(CSE_IP, CSE_HTTP_PORT)) {
    Serial.println("Connection failed !");
    return "error";
  }

#ifdef DEBUG
  Serial.println(postRequest);
#endif

  // Send the HTTP POST request
  client.print(postRequest);

  // Manage a timeout
  unsigned long startTime = millis();
  while (client.available() == 0) {
    if (millis() - startTime > REQUEST_TIME_OUT) {
      Serial.println("Client Timeout");
      client.stop();
      return "error";
    }
  }

  // If success, Read the HTTP response
  String result = "";
  client.setTimeout(500);
  if (client.available()) {
    result = client.readStringUntil('\r');
    Serial.println(result);
  }
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection...");
  return result;
}

String createAE(String ae) {
  String aeRepresentation =
    "{\"m2m:ae\": {"
    "\"rn\":\"" + ae + "\","
    "\"api\":\"org.demo." + ae + "\","
    "\"rr\":\"true\","
    "\"poa\":[\"http://" + WiFi.localIP().toString() + ":" + LOCAL_PORT + "/" + ae + "\"]"
    "}}";
#ifdef DEBUG
  Serial.println(aeRepresentation);
#endif
  return doPOST("/" + CSE_NAME, TY_AE, aeRepresentation);
}

String createCNT(String ae, String cnt) {
  String cntRepresentation =
    "{\"m2m:cnt\": {"
    "\"mni\":\"10\","         // maximum number of instances
    "\"rn\":\"" + cnt + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae, TY_CNT, cntRepresentation);
}

String createCI(String ae, String cnt, String ciContent) {
  String ciRepresentation =
    "{\"m2m:cin\": {"
    "\"con\":\"" + ciContent + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + cnt, TY_CI, ciRepresentation);
}

String createSUB(String ae) {
  String subRepresentation =
    "{\"m2m:sub\": {"
    "\"rn\":\"SUB_" + ae + "\","
    "\"nu\":[\"" + CSE_NAME + "/" + ae  + "\"], "
    "\"nct\":1"
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + CMND_CNT_NAME, TY_SUB, subRepresentation);
}

void push(){
  task_tempIN();
  //delay(5000);
  task_tempOUT();
  //delay(5000);
  task_LDR_IN();
  //delay(5000);
  task_LDR_OUT();
  //delay(5000);
  task_Dust();
//  int sensortemp1;
//  int sensortemp2;
//  int sensorldr1;
//  int sensorldr2;
//  int sensordust;
//
//  sensortemp1 = random(10,20);
//  sensortemp2 = random(10,20);
//  sensorldr1 = random(10,20); 
//  sensorldr2 = random(10,20);
//  sensordust = random(10,20);
//
//  #ifdef DEBUG
//  Serial.println("temp1 value = " + sensortemp1);
//  Serial.println("temp2 value = " + sensortemp2);
//  Serial.println("ldr1 value = " + sensorldr1);
//  Serial.println("ldr2 value = " + sensorldr2);
//  Serial.println("dust value = " + sensordust);
//#endif
//
//  String temp1Content = String(sensortemp1);
//  String temp2Content = String(sensortemp2);
//  String ldr1Content = String(sensorldr1);
//  String ldr2Content = String(sensorldr2);
//  String dustContent = String(sensordust);
//
//  createCI("TempIN", DATA_CNT_NAME, citemp1);
//  createCI("TempOUT", DATA_CNT_NAME, citemp2);
//  createCI("LDR_IN", DATA_CNT_NAME, cildr1);
//  createCI("LDR_OUT", DATA_CNT_NAME, cildr2);
//  createCI("Dust", DATA_CNT_NAME, cidust);
}


void registerModule(String module, bool isActuator, String intialDescription, String initialData) {
  if (WiFi.status() == WL_CONNECTED) {
    String result;
    // 1. Create the ApplicationEntity (AE) for this sensor
    result = createAE(module);
    if (result == HTTP_CREATED) {
#ifdef DEBUG
      Serial.println("AE " + module + " created  !");
#endif

      // 2. Create a first container (CNT) to store the description(s) of the sensor
      result = createCNT(module, DESC_CNT_NAME);
      if (result == HTTP_CREATED) {
#ifdef DEBUG
        Serial.println("CNT " + module + "/" + DESC_CNT_NAME + " created  !");
#endif


        // Create a first description under this container in the form of a ContentInstance (CI)
        result = createCI(module, DESC_CNT_NAME, intialDescription);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CI " + module + "/" + DESC_CNT_NAME + "/{initial_description} created !");
#endif
        }
      }

      // 3. Create a second container (CNT) to store the data  of the sensor
      result = createCNT(module, DATA_CNT_NAME);
      if (result == HTTP_CREATED) {
#ifdef DEBUG
        Serial.println("CNT " + module + "/" + DATA_CNT_NAME + " created !");
#endif

        // Create a first data value under this container in the form of a ContentInstance (CI)
        result = createCI(module, DATA_CNT_NAME, initialData);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CI " + module + "/" + DATA_CNT_NAME + "/{initial_aata} created !");
#endif
        }
      }

      // 3. if the module is an actuator, create a third container (CNT) to store the received commands
      if (isActuator) {
        result = createCNT(module, CMND_CNT_NAME);
        if (result == HTTP_CREATED) {
#ifdef DEBUG
          Serial.println("CNT " + module + "/" + CMND_CNT_NAME + " created !");
#endif

          // subscribe to any ne command put in this container
          result = createSUB(module);
          if (result == HTTP_CREATED) {
#ifdef DEBUG
            Serial.println("SUB " + module + "/" + CMND_CNT_NAME + "/SUB_" + module + " created !");
#endif
          }
        }
      }
    }
  }
}

void init_IO() { //TO DO
  // Configure pin 0 for LED control
  pinMode(LED_PIN_Curtain, OUTPUT);
  digitalWrite(LED_PIN_Curtain, LOW);
    pinMode(LED_PIN_Window, OUTPUT);
  digitalWrite(LED_PIN_Window, LOW);
    pinMode(LED_PIN_Light, OUTPUT);
  digitalWrite(LED_PIN_Light, LOW);
    pinMode(LED_PIN_AC, OUTPUT);
  digitalWrite(LED_PIN_AC, LOW);

  // Configure pin 2 for Light Sensor
//  pinMode(LUM_PIN, INPUT);
}
void task_IO() {
}

void init_WiFi() {
  Serial.println("Connecting to  " + String(WIFI_SSID) + " ...");
  WiFi.persistent(false);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  // wait until the device is connected to the wifi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_DELAY);
    Serial.print(".");
  }

  // Connected, show the obtained ip address
  Serial.println("WiFi Connected ==> IP Address = " + WiFi.localIP().toString());
}
void task_WiFi() {
}

void init_HTTPServer() {
  server.begin();
  Serial.println("Local HTTP Server started !");
}
void task_HTTPServer() {//TO DO
  // Check if a client is connected
  client = server.available();
  if (!client)
    return;

  // Wait until the client sends some data
  Serial.println("New client connected. Receiving request <=== ");
  
  while (!client.available()) {
#ifdef DEBUG
    Serial.print(".");
#endif
    delay(5);
  }

  // Read the request
  client.setTimeout(500);
  String request = client.readString();
  Serial.println(request);

  int start, end;
  // identify the right module (sensor or actuator) that received the notification
  // the URL used is ip:port/ae
  start = request.indexOf("/");
  end = request.indexOf("HTTP") - 1;
  context = request.substring(start+1, end);
#ifdef DEBUG
  Serial.println(String() + "context = [" + start + "," + end + "] -> " + context);
#endif

  // ingore verification messages 
  if (request.indexOf("vrq") > 0) {
    client.flush();
    client.print(HTTP_OK);
    delay(5);
    client.stop();
    Serial.println("Client disconnected");
    return;
  }

  //Parse the request and identify the requested command from the device
  //Request should be like "[operation_name]"
  start = request.indexOf("[");  
  end = request.indexOf("]"); // first occurence fo 
  command = request.substring(start+1, end);
#ifdef DEBUG
  Serial.println(String() + + "command = [" +  start + "," + end + "] -> " + command);
#endif

  client.flush();
  client.print(HTTP_OK);
  delay(5);
  client.stop();
  Serial.println("Client disconnected");
}

void init_tempIN(){
  String initialDescription = "Name = TemperatureSensor\t"
                              "Unit = DegreeCelcius\t"
                              "Location = InsideRoom\t";
  String initialData = "0";
  registerModule("TempIN", false, initialDescription, initialData);    
}
void task_tempIN() {
  int sensorValue;
  sensorValue = (int)dht1.readTemperature();
//  sensorValue = random(15,28);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("Temperature IN value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("TempIN", DATA_CNT_NAME, ciContent);
}

void init_tempOUT(){
  String initialDescription = "Name = TemperatureSensor\t"
                              "Unit = DegreeCelcius\t"
                              "Location = OutsideRoom\t";
  String initialData = "0";
  registerModule("TempOUT", false, initialDescription, initialData);    
}
void task_tempOUT() {
  int sensorValue;
  sensorValue = (int)dht2.readTemperature();
//  sensorValue = random(15,28);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("Temperature OUT value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("TempOUT", DATA_CNT_NAME, ciContent);
}

void init_Dust(){
  String initialDescription = "Name = DustSensor\t"
                              "Unit = Unit\t"
                              "Location = InsideRoom\t";
  String initialData = "0";
  registerModule("Dust", false, initialDescription, initialData);    
}
void task_Dust() {
  int sensorValue;
  sensorValue = digitalRead(LUM_PIN_Dust);
//  sensorValue = random(0,1);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("Dust value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("Dust", DATA_CNT_NAME, ciContent);
}

void init_LDR_IN(){
  String initialDescription = "Name = LightSensor\t"
                              "Unit = LUX\t"
                              "Location = InsideRoom\t";
  String initialData = "0";
  registerModule("LDR_IN", false, initialDescription, initialData);    
}
void task_LDR_IN() {
  int sensorValue;
  sensorValue = digitalRead(LUM_PIN_LDR_IN);
//  sensorValue = random(2);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("LDR IN value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("LDR_IN", DATA_CNT_NAME, ciContent);
}

void init_LDR_OUT(){
  String initialDescription = "Name = LightSensor\t"
                              "Unit = LUX\t"
                              "Location = OutsideRoom\t";
  String initialData = "0";
  registerModule("LDR_OUT", false, initialDescription, initialData);    
}
void task_LDR_OUT() {
  int sensorValue;
  sensorValue = digitalRead(LUM_PIN_LDR_OUT);
//  sensorValue = random(2);   // if no sensor is attached, you can use random values 
#ifdef DEBUG
  Serial.println("LDR OUT value = " + sensorValue);
#endif

  String ciContent = String(sensorValue);

  createCI("LDR_OUT", DATA_CNT_NAME, ciContent);
}

void init_ledLight() {
  String initialDescription = "Name = LedActuator\t"
                              "Location = InsiteRoom\t";
  String initialData = "switchOff";
  registerModule("ledLight", true, initialDescription, initialData);
}
void command_ledLight(String cmd) {
  if (cmd == "switchOn") {
#ifdef DEBUG
    Serial.println("Switching on the LED for Light...");
#endif
    digitalWrite(LED_PIN_Light, ON_STATE);
  }
  else if (cmd == "switchOff") {
#ifdef DEBUG
    Serial.println("Switching off the LED for Light...");
#endif
    digitalWrite(LED_PIN_Light, OFF_STATE);
  }
}

void init_ledWindow() {
  String initialDescription = "Name = LedActuator\t"
                              "Location = InsiteRoom\t";
  String initialData = "switchOff";
  registerModule("ledWindow", true, initialDescription, initialData);
}
void command_ledWindow(String cmd) {
  if (cmd == "switchOn") {
#ifdef DEBUG
    Serial.println("Switching on the LED for Light for Window...");
#endif
    digitalWrite(LED_PIN_Window, ON_STATE);
  }
  else if (cmd == "switchOff") {
#ifdef DEBUG
    Serial.println("Switching off the LED for Window...");
#endif
    digitalWrite(LED_PIN_Window, OFF_STATE);
  }
}

void init_ledCurtain() {
  String initialDescription = "Name = LedActuator\t"
                              "Location = InsiteRoom\t";
  String initialData = "switchOff";
  registerModule("ledCurtain", true, initialDescription, initialData);
}
void command_ledCurtain(String cmd) {
  if (cmd == "switchOn") {
#ifdef DEBUG
    Serial.println("Switching on the LED for Light for Curtain...");
#endif
    digitalWrite(LED_PIN_Curtain, ON_STATE);
  }
  else if (cmd == "switchOff") {
#ifdef DEBUG
    Serial.println("Switching off the LED for Curtain...");
#endif
    digitalWrite(LED_PIN_Curtain, OFF_STATE);
  }
}

void init_ledAC() {
  String initialDescription = "Name = LedActuator\t"
                              "Location = InsiteRoom\t";
  String initialData = "switchOff";
  registerModule("ledAC", true, initialDescription, initialData);
}
void command_ledAC(String cmd) {
  if (cmd == "switchOn") {
#ifdef DEBUG
    Serial.println("Switching on the LED for Light for AC...");
#endif
    digitalWrite(LED_PIN_AC, ON_STATE);
  }
  else if (cmd == "switchOff") {
#ifdef DEBUG
    Serial.println("Switching off the LED for AC...");
#endif
    digitalWrite(LED_PIN_AC, OFF_STATE);
  }
}

void setup() { // TO DO
  // intialize the serial liaison
  Serial.begin(SERIAL_SPEED);

  // configure sensors and actuators HW
  init_IO();

  // Connect to WiFi network
  init_WiFi();

  // Start HTTP server
  init_HTTPServer();

  // register sensors and actuators
  init_tempIN();
  init_tempOUT();
  init_Dust();
  init_LDR_IN();
  init_LDR_OUT();

  init_ledLight();
  init_ledWindow();
  init_ledCurtain();
  init_ledAC();

  // Schedule any periodic task here: t.every(period, function_to_call);
  t.every(20000, push);
}

// Main loop of the µController
void loop() {
  delay(5000);
  // update the time counter
  t.update();

  // Check if a client is connected
  task_HTTPServer();

  // analyse the received command (if any)
  if (context != "") {
    if (context == "ledAC") {
      command_ledAC(command);
    }
    else if (context == "ledLight") {
      command_ledLight(command);
    }
    else if (context == "ledWindow") {
      command_ledWindow(command);
    }
    else if (context == "ledCurtain") {
      command_ledCurtain(command);
    }
    else
      Serial.println("The targte AE does not exist ! ");
  }

  // reset "command" and "context" variables for future received requests
  context = "";
  command = "";
}

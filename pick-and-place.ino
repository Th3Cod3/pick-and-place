// **************************
// Aansluitingen voor UNO
// 5 V -> 5 V
// GND -> GND
// MISO -> 12
// MOSI -> 11
// SCS -> 10
// SCLK -> 13
// **************************
#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
// #include "DeltaRobotData.h"
#define SENSOR_PIN 6
#define VACUUM_PIN 7
#define CILINDER_PIN 2
#define W5500_SELECT_PIN 10

byte rcvBuffer[10];
// legacy
bool vacuum = false;
byte* p_b = (byte*)&vacuum;
int integ = 0;
byte* p_i = (byte*)&integ;
float xPos = 0;
byte* p_f = (byte*)&xPos;
char string[] = { 'B', 'A' };
byte* p_c = (byte*)&string;
const int lengte = 2 * sizeof(vacuum) + sizeof(integ) + sizeof(xPos) + 2;

byte tcpBuffer[lengte];
byte* p_pointer[lengte];
int j, invoer;

// refactor
// DeltaRobotData deltaRobotData;

// Ethernet config
// --arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ipW5500(192, 168, 0, 15);  // IP address van de W5500
EthernetClient clientW5500;

// --PLC
IPAddress ipPlcServer(192, 168, 0, 11);  // IP address van de server (PLC)
int serverPort = 2000;                  // Poortnummer van de server (PLC)

String getSerialBuffer() {
  String buffer;
  if (!Serial.available())
    return buffer;

  buffer = Serial.readString();
  buffer.trim();

  return buffer;
}

bool waitForCmdInSerial(String expected) {
  while (!Serial.available())
    ;

  String serialBuffer = getSerialBuffer();
  Serial.print("Command: ");
  Serial.println(serialBuffer);
  bool result = serialBuffer.equals(expected);

  if (!result)
    Serial.println("[Unrecognized command]");

  return result;
}

void checkConnection() {
  uint8_t connected = 0;

  while (!connected) {
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("HW not found.");
      Serial.println("Send 'r' to retry");
      if (waitForCmdInSerial("r")) {
        connected = 1;
      }
      continue;
    }

    while (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
      Serial.println("Send 'r' to retry");
      if (!waitForCmdInSerial("r")) {
        connected = 1;
      }
      continue;
    }

    // give the Ethernet shield a second to initialize
    delay(1000);

    Serial.println("connecting...");
    int errorCode = clientW5500.connect(ipPlcServer, serverPort);
    if (errorCode) {
      Serial.println("connected");
      break;
    } else {
      // no connection to server
      Serial.println("connection failed");
      Serial.print("Error code: ");
      Serial.println(errorCode);
      delay(500);
      Serial.println("Send 'r' to retry");
      if (!waitForCmdInSerial("r")) {
        connected = 1;
      }
      continue;
    }
  }
}

void syncTCPBuffer_legacy() {
  // Zet de adressen van de entry's van array buf in pointer array p_pointer[]
  for (j = 0; j < sizeof(tcpBuffer) / sizeof(tcpBuffer[0]); j++)
    p_pointer[j] = &tcpBuffer[j];
}

void startCommand() {
  while (!Serial)
    ;  // wait for serial port to connect.

  do {
    Serial.println("Send 's' to startup");
  } while (!waitForCmdInSerial("s"));
}

void setup() {
  Serial.begin(115200);
  // startCommand();
  pinMode(SENSOR_PIN, INPUT);
  pinMode(VACUUM_PIN, OUTPUT);
  pinMode(CILINDER_PIN, OUTPUT);
  // int statusLed = 0;
  // while(1) {
  //   statusLed = !statusLed;
  //   if (statusLed) {
  //     digitalWrite(VACUUM_PIN, HIGH);
  //     digitalWrite(CILINDER_PIN, HIGH);
  //   } else {
  //     digitalWrite(CILINDER_PIN, LOW);
  //     digitalWrite(VACUUM_PIN, LOW);
  //   }
  //   delay(1500);
  // }

  syncTCPBuffer_legacy();

  // Setup W5500
  Ethernet.init(W5500_SELECT_PIN);
  Ethernet.begin(mac, ipW5500);  // start the Ethernet connection
}

void loop() {
  // if the server is disconnected, ask to reconnect:
  if (!clientW5500.connected()) {
    checkConnection();
  }

  // Stuur de nieuwe waarden naar de plc
  updateData_legacy();
  updateTcpBuffer_legacy();
  if (clientW5500.connected()) {
    clientW5500.write(tcpBuffer, sizeof tcpBuffer);
  }
  delay(20);
  int i = 0;
  while (clientW5500.available()) {
    rcvBuffer[i] = clientW5500.read();
    i++;
  }

  if (rcvBuffer[0]) {
    digitalWrite(VACUUM_PIN, HIGH);
  } else {
    digitalWrite(VACUUM_PIN, LOW);
  }
}

void printTcpBuffer_legacy() {
  for (int j = 0; j < sizeof(tcpBuffer) / sizeof(tcpBuffer[0]); j++) {
    Serial.print(tcpBuffer[j], HEX);
    Serial.print(" | ");
    Serial.println(tcpBuffer[j], BIN);
  }
}

void updateData_legacy() {
  vacuum = digitalRead(SENSOR_PIN);
  integ++;
}

void swapToLittleEndian_legacy() {
  // Windows gebruikt de big endian volgorde voor ints en floats, de plc little endian
  // Verwissel daarom de byte-volgorde van ints en van floats. Gebruikt wordt de xor-swap
  tcpBuffer[2] ^= tcpBuffer[3];
  tcpBuffer[3] ^= tcpBuffer[2];
  tcpBuffer[2] ^= tcpBuffer[3];  // int
  tcpBuffer[4] ^= tcpBuffer[7];
  tcpBuffer[7] ^= tcpBuffer[4];
  tcpBuffer[4] ^= tcpBuffer[7];  // float byte1<=>byte4
  tcpBuffer[5] ^= tcpBuffer[6];
  tcpBuffer[6] ^= tcpBuffer[5];
  tcpBuffer[5] ^= tcpBuffer[6];  // float byte2<=>byte3
}

void updateTcpBuffer_legacy() {
  // Zet adres van eerste byte van variabele in een byte-pointer
  byte* p_b = (byte*)&vacuum;
  byte* p_i = (byte*)&integ;
  byte* p_f = (byte*)&xPos;
  byte* p_c = (byte*)&string;
  // Vul buf met bool, bool, int, float, 2x char begin eind
  for (j = 0; j < 1; j++)
    *p_pointer[j] = *(p_b + j - 0);  // j = 0 bool 1 byte in arduino j = 1
  for (j = j; j < 2; j++)
    *p_pointer[j] = *(p_b + j - 1);  // j = 1 bool 2 bytes in plc j = 2
  for (j = j; j < 4; j++)
    *p_pointer[j] = *(p_i + j - 2);  // j = 2 int want 2 bits j = 4
  for (j = j; j < 8; j++)
    *p_pointer[j] = *(p_f + j - 4);  // j = 4 float/real 4 bits j = 8
  for (j = j; j < 10; j++)
    *p_pointer[j] = *(p_c + j - 8);  // j = 8 2 characters j = 10

  // printTcpBuffer_legacy();

  swapToLittleEndian_legacy();
}
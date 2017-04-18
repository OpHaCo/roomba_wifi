/******************************************************************************
 * @file    roomba spark firmware
 * @author  Rémi Pincent - INRIA
 * @date    16 sept. 2015   
 *
 * @brief Spark core firmware controlling croomba over WIFI
 * 
 * Project : roomba_wifi - https://github.com/OpHaCo/roomba_wifi
 * Contact:  Rémi Pincent - remi.pincent@inria.fr
 * 
 * Revision History:
 * Refer https://github.com/OpHaCo/roomba_wifi
 * 
 * LICENSING
 * roomba_wifi (c) by Rémi Pincent
 * 
 * roomba_wifi is licensed under a
 * Creative Commons Attribution 3.0 Unported License.
 * 
 * Credits : 
 *     https://community.sparkdevices.com/t/sparkbot-manually-automatically-vacuum-your-living-room/625
 *    http://skaterj10.hackhut.com/2013/01/22/roomba-hacking-101/
 *    https://community.particle.io/t/sparkbot-spark-core-roomba/625
 * 
 * You should have received a copy of the license along with this
 * work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
 *****************************************************************************/

/**************************************************************************
 * Include Files
 **************************************************************************/
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>

/**************************************************************************
 * Macros
 **************************************************************************/
#define NO_BYTE_READ 0x100

#define LOG_SERIAL Serial
#define SERIAL_ROOMBA_CMD_TX Serial1

/**************************************************************************
 * Manifest Constants
 **************************************************************************/
static const char* _mqttRoombaCmdsTopic = "roomba/roombaCmds";
static const char* _mqttRoombaInputTopic = "roomba/roombaInput";
static const char* _mqttRoombaAliveTopic = "roomba/alive";
static const char* _mqttRoombaName = "roomba";

static const int CLEAN_PIN = D7;                                      // CLEAN_PIN controls clean button 
static const int SOFT_UART_RX = D1;                                   // Rx to communicate with roomba
static const int SOFT_UART_TX = D2;                                   // Not used

/** Use software serial Rx - to communicate with roomba - otherwise, 
 * on Rx impedance too high, logical 0 set by Roomba seen at 2.2V on Rx!
 */
static SoftwareSerial _softSerial(SOFT_UART_RX, SOFT_UART_TX);
#define SERIAL_ROOMBA_CMD_RX _softSerial

static const char* _ssid = "your_ssid";
static const char* _password = "your_wifi_pass";
static const char* _mqttServer = "mqtt_broker_address"; 

/**************************************************************************
 * Type Definitions
 **************************************************************************/
typedef enum{
    OFF = 1,
    PASSIVE = 2,
    SAFE = 3,
    FULL = 4
}EIOMode;

typedef struct{
    String cmdName;
    void (*controlCmd)(void);
}TsControlCommandMap;

typedef struct{
    String cmdName;
    void (*controlCmd)(void);
}TsCommandMap;

typedef struct{
    String cmdName;
    int (*inputCmd)(void);
}TsInputCommandMap;

/**************************************************************************
 * Local Functions Declarations
 **************************************************************************/
static void connectMQTT(void);
static void setupWifi(void);
static void setupOTA(void);
static void goForward();
static void goBackward();
static void spinLeft();
static void spinRight();
static void stop();
static void updateSensors();
static void playSong();
static void vibgyor();
static void vacuumOn();
static void vacuumOff();
static void goHome();
static void clean();
static void powerOn();
static void powerOff();
static void gainControl();
static void setPassiveMode();
static void setSafeMode();
static void freeControl();
static EIOMode getMode(void);
static uint16_t getBatteryCharge(void);
static int readByte(int8_t& readByte, int timeout);
static int roombaControl(String command);
static void mqttCallback(char* topic, byte* payload, unsigned int length);

/*********************************************************
 * COMMANDS REQUIRING AT LEAST PASSIVE MODE - AFTER 
 * COMMAND EXECUTION ROOMBA GOES BACK IN PASSIVE MODE
 ********************************************************/ 
static TsControlCommandMap pToPControlCmds[] = 
{
    {"GOHOME",               &goHome},
    {"CLEAN",                &clean},
};

/*********************************************************
 * COMMANDS REQUIRING SAFE MODE - AFTER COMMAND EXECUTION 
 * ROOMBA STAY IN THIS MODE
 ********************************************************/ 
static TsControlCommandMap sToSControlCmds[] = 
{
    {"STOP",                 &stop},
    {"BACK",                 &goBackward},
    {"FORWARD",              &goForward},
    {"RIGHT",                &spinRight},
    {"LEFT",                 &spinLeft},
    {"SONG",                 &playSong},
    {"VACUUMON",             &vacuumOn},
    {"VACUUMOFF",            &vacuumOff},
    {"VIBGYOR",              &vibgyor},
};

/**********************************************
 * FOR THESE COMMANDS NO MODE REQUIRED
 *********************************************/ 
static TsControlCommandMap cmds[] = 
{
    {"POWERON",              &powerOn},
    {"POWEROFF",             &powerOff},
    {"FREECONTROL",          &freeControl},
    {"GAINCONTROL",          &gainControl}
    /** Add here other output commands */
};

/**********************************************
 * INPUT CMDS
 *********************************************/ 
static TsInputCommandMap inputCmds[] = 
{
    {"GETMODE",                (int (*)(void)) (&getMode)},
    {"GETBATT",                (int (*)(void)) (&getBatteryCharge)},
    /** Add here other input commands */
};


/**************************************************************************
 * Static Variables
 **************************************************************************/                              
static char _sensorbytes[10];
static WiFiClient _wifiClient;
static PubSubClient _mqttClient(_mqttServer, 1883, mqttCallback, _wifiClient);
/** connection indicator */
static bool _ledStatus = HIGH;

/**************************************************************************
 * Global Functions Defintions
 **************************************************************************/
void setup() {
  // Pinmode definitions and begin serial
  pinMode(CLEAN_PIN, INPUT_PULLUP);                     // sets the pins as input
  SERIAL_ROOMBA_CMD_TX.begin(115200);
  SERIAL_ROOMBA_CMD_RX.begin(115200);
  LOG_SERIAL.begin(115200);
  LOG_SERIAL.println("start roomba control");

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  _ledStatus = HIGH;
  digitalWrite(LED_BUILTIN, _ledStatus);
  
  setupWifi();
  connectMQTT();
  setupOTA();
}


// Loop is empty since we are waiting for commands from the API
void loop() {
    

    if (WiFi.status() != WL_CONNECTED)
    {
      setupWifi();
    }
    else
    {
      _mqttClient.loop();
      if (!_mqttClient.connected()){
        connectMQTT();
      }
      ArduinoOTA.handle(); 
    }
}

 /**************************************************************************
 * Local Functions Definitions
 **************************************************************************/
static void connectMQTT(void)
{
  LOG_SERIAL.print("Connecting to ");
  LOG_SERIAL.print(_mqttServer);
  LOG_SERIAL.print(" as ");
  LOG_SERIAL.println(_mqttRoombaName);

  _ledStatus = HIGH;
  digitalWrite(LED_BUILTIN, _ledStatus);

  while (true)
  {
    if (_mqttClient.connect(_mqttRoombaName)) {
      _mqttClient.loop();
      _mqttClient.subscribe(_mqttRoombaCmdsTopic);
      LOG_SERIAL.println("Connected to MQTT broker");
      LOG_SERIAL.print("send alive message to topic ");
      LOG_SERIAL.println(_mqttRoombaAliveTopic);
      if (!_mqttClient.publish(_mqttRoombaAliveTopic, "alive")) {
        LOG_SERIAL.println("Publish failed");
      }
      /** if mqtt client loop not called when several mqtt api calls are done.
      * At some point, mqtt client api calls fail
      * */
      _mqttClient.loop();
      _ledStatus = LOW;
      digitalWrite(LED_BUILTIN, _ledStatus);
      return;
    }
    else
    {
      LOG_SERIAL.println("connection to MQTT failed\n");
    }
    _ledStatus = !_ledStatus;
    digitalWrite(LED_BUILTIN, _ledStatus);
    delay(1000);
  }
}

static void setupWifi(void)
{
  uint8_t mac[6];

  _ledStatus = HIGH;
  digitalWrite(LED_BUILTIN, _ledStatus);
  
  WiFi.macAddress(mac);
  /** !!! reverse order - https://www.arduino.cc/en/Reference/WiFiMACAddress */
  LOG_SERIAL.println(String("MAC: ") +
    String(mac[0],HEX) + 
    String(":") + 
    String(mac[1],HEX) + 
    String(":") + 
    String(mac[2],HEX) + 
    String(":") + 
    String(mac[3],HEX) + 
    String(":") + 
    String(mac[4],HEX) + 
    String(":") + 
    String(mac[5],HEX));

  LOG_SERIAL.print("Connecting to ");
  LOG_SERIAL.println(_ssid);
  
  WiFi.begin(_ssid, _password);
  
  while (WiFi.status() != WL_CONNECTED) {
    _ledStatus = !_ledStatus;
    digitalWrite(LED_BUILTIN, _ledStatus);
    delay(500);
    printf(".");
  }

  LOG_SERIAL.println("");
  LOG_SERIAL.println("WiFi connected");  
  LOG_SERIAL.println("IP address: ");
  LOG_SERIAL.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
    LOG_SERIAL.println("MDNS responder started");
  }  
}

static void setupOTA(void)
{
  //Set actions for OTA
  ArduinoOTA.onStart([]() {
    LOG_SERIAL.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    LOG_SERIAL.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    LOG_SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    LOG_SERIAL.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) LOG_SERIAL.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) LOG_SERIAL.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) LOG_SERIAL.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) LOG_SERIAL.println("Receive Failed");
    else if (error == OTA_END_ERROR) LOG_SERIAL.println("End Failed");
  });
  ArduinoOTA.begin();
  LOG_SERIAL.println("OTA initialized");
  
}

static void stop() {
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 137);               // DRIVE command
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);              // 0 means stop
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x80);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);
}

static void goForward() {
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 137);               // DRIVE command
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x01);              // 0x01F4 = 500 = full speed ahead!
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xF4);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xF9);              // Our roomba has a limp, so it this correction keeps it straight
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xC0);

}
static void goBackward() {
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 137);               // DRIVE command
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xff);              // 0xff38 == -200
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x38);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x80);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);
}
static void spinLeft() {
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 137);               // DRIVE command
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);              // 0x00c8 == 200
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xc8);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x01);              // 0x0001 == 1 == spin left
}
static void spinRight() {
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 137);               // DRIVE command
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0x00);              // 0x00c8 == 200
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xc8);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xff);
  SERIAL_ROOMBA_CMD_TX.write((unsigned char) 0xff);              // 0xffff == -1 == spin right
}

static void clean() {                                      
  SERIAL_ROOMBA_CMD_TX.write(135);                               // Starts a cleaning cycle, so command 143 can be initiated
}

static void powerOn()
{
    //GPIO connected to clean button
    pinMode(CLEAN_PIN, OUTPUT);
    digitalWrite(CLEAN_PIN, LOW);
    delay(300);
    pinMode(CLEAN_PIN, INPUT_PULLUP);
}

static void powerOff()
{
    //GPIO connected to clean button
    pinMode(CLEAN_PIN, OUTPUT);
    digitalWrite(CLEAN_PIN, LOW);
    delay(2000);
}

static void goHome() {                                     // Sends the Roomba back to it's charging base
  //clean();                                          // Starts a cleaning cycle, so command 143 can be initiated
  //delay(5000);
  SERIAL_ROOMBA_CMD_TX.write(143);                               // Sends the Roomba home to charge
  delay(50);
}

static void playSong() {                                   // Makes the Roomba play a little ditty
  SERIAL_ROOMBA_CMD_TX.write(140);                         	    // Define a new song
  SERIAL_ROOMBA_CMD_TX.write(0);                                 // Write to song slot #0
  SERIAL_ROOMBA_CMD_TX.write(8);                                 // 8 notes long
  SERIAL_ROOMBA_CMD_TX.write(60);                                // Everything below defines the C Major scale 
  SERIAL_ROOMBA_CMD_TX.write(32); 
  SERIAL_ROOMBA_CMD_TX.write(62);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(64);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(65);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(67);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(69);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(71);
  SERIAL_ROOMBA_CMD_TX.write(32);
  SERIAL_ROOMBA_CMD_TX.write(72);
  SERIAL_ROOMBA_CMD_TX.write(32);
 
  SERIAL_ROOMBA_CMD_TX.write(141);                               // Play a song
  SERIAL_ROOMBA_CMD_TX.write(0);                                 // Play song slot #0
}

static void vacuumOn() {                                   // Turns on the vacuum
  SERIAL_ROOMBA_CMD_TX.write(138);
  SERIAL_ROOMBA_CMD_TX.write(7);
}

static void vacuumOff() {                                  // Turns off the vacuum
  SERIAL_ROOMBA_CMD_TX.write(138);
  SERIAL_ROOMBA_CMD_TX.write(0);
}

static void vibgyor() {                                    // Makes the main LED behind the power button on the Roomba pulse from Green to Red
  SERIAL_ROOMBA_CMD_TX.write(139);
  
  for (int i=0;i<255;i++){ 
    SERIAL_ROOMBA_CMD_TX.write(139);                             // LED seiral command
    SERIAL_ROOMBA_CMD_TX.write(0);                               // We don't want any of the other LEDs
    SERIAL_ROOMBA_CMD_TX.write(i);                               // Color dependent on i
    SERIAL_ROOMBA_CMD_TX.write(255);                             // FULL INTENSITY!
    delay(5);                                       // Wait between cycles so the transition is visible
    }

  for (int i=0;i<255;i++){ 
    SERIAL_ROOMBA_CMD_TX.write(139);
    SERIAL_ROOMBA_CMD_TX.write(0);
    SERIAL_ROOMBA_CMD_TX.write(i);
    SERIAL_ROOMBA_CMD_TX.write(255);
    delay(5);
    }

  for (int i=0;i<255;i++){ 
    SERIAL_ROOMBA_CMD_TX.write(139);
    SERIAL_ROOMBA_CMD_TX.write(0);
    SERIAL_ROOMBA_CMD_TX.write(i);
    SERIAL_ROOMBA_CMD_TX.write(255);
    delay(5);
    }

  for (int i=0;i<255;i++){ 
    SERIAL_ROOMBA_CMD_TX.write(139);
    SERIAL_ROOMBA_CMD_TX.write(0);
    SERIAL_ROOMBA_CMD_TX.write(i);
    SERIAL_ROOMBA_CMD_TX.write(255);
    delay(5);
    }

  SERIAL_ROOMBA_CMD_TX.write(139);
  SERIAL_ROOMBA_CMD_TX.write(0);
  SERIAL_ROOMBA_CMD_TX.write(0);
  SERIAL_ROOMBA_CMD_TX.write(0);

}

static void gainControl() {                                // Gain control of the Roomba once it's gone back into passive mode
  setSafeMode();
  SERIAL_ROOMBA_CMD_TX.write(132);                               // Full control mode
  delay(50);
}

static void setPassiveMode() {                                // Gain control of the Roomba once it's gone back into passive mode
  // Get the Roomba into control mode 
  SERIAL_ROOMBA_CMD_TX.write(128);   
  delay(50);    
}

static void setSafeMode() {                                // Gain control of the Roomba once it's gone back into passive mode
  setPassiveMode(); 
  SERIAL_ROOMBA_CMD_TX.write(130);                               // Safe mode
  delay(50);  
}

static void freeControl()
{
  // Get the Roomba into control mode 
  SERIAL_ROOMBA_CMD_TX.write(128);                               // Passive mode
  delay(50);
}

static EIOMode getMode(void)
{
  int8_t loc_readByte = 0;
  
  /** Get sensor packet */ 
  SERIAL_ROOMBA_CMD_TX.write(142);
  SERIAL_ROOMBA_CMD_TX.write(35);

  if(readByte(loc_readByte, 100) == NO_BYTE_READ)
  {
      return (EIOMode)-1;
  }
  else
  {
      /** +1 increment... in order to not returining 0 */
      if (!_mqttClient.publish(_mqttRoombaInputTopic, String(loc_readByte + 1).c_str())) {
        LOG_SERIAL.println("Publish failed");
      }
      return  (EIOMode)(loc_readByte + 1);
  }
}

static uint16_t getBatteryCharge()
{
  int8_t loc_readByte = 0;
  uint16_t battLevel = 0;
  
  /** Get sensor packet */ 
  SERIAL_ROOMBA_CMD_TX.write(142);
  SERIAL_ROOMBA_CMD_TX.write(25);

  if(readByte(loc_readByte, 50) == NO_BYTE_READ)
  {
      return -1;
  }
  
  battLevel = loc_readByte << 8;
  if(readByte(loc_readByte, 50) == NO_BYTE_READ)
  {
      return -1;
  }
  else if (!_mqttClient.publish(_mqttRoombaInputTopic, String(loc_readByte + battLevel).c_str())) {
    LOG_SERIAL.println("Publish failed");
  }
  return loc_readByte + battLevel;
}

static int readByte(int8_t& readByte, int timeout)
{
  int count = 0;
  const uint8_t DELAY = 10;
  //ceil 
  const int MAX_INDEX = 1 + ((timeout -1)/DELAY);

  for(count = 0; count <= MAX_INDEX; count++)
  {
      if(SERIAL_ROOMBA_CMD_RX.available())
      {
          readByte = SERIAL_ROOMBA_CMD_RX.read();
          return  0;
      }
      delay(DELAY);
  }
  return NO_BYTE_READ;
}

static int roombaControl(String command)
{
  int cmdIndex = 0;    
  /****************************************************
   * HANDLE COMMANDS NOT REQUIRING ANY MODE
   * *************************************************/
  for(cmdIndex = 0; cmdIndex < sizeof(cmds)/sizeof(cmds[0]); cmdIndex++)
  {
      if(cmds[cmdIndex].cmdName.equals(command))
      {
          cmds[cmdIndex].controlCmd();
          return 0;
      }
  }

  /****************************************************
   * HANDLE COMMANDS REQURING PASSIVE MODE
   * *************************************************/
  for(cmdIndex = 0; cmdIndex < sizeof(pToPControlCmds)/sizeof(pToPControlCmds)[0]; cmdIndex++)
  {
      if(pToPControlCmds[cmdIndex].cmdName.equals(command))
      {
          setPassiveMode();
          pToPControlCmds[cmdIndex].controlCmd();
          return 0;
      }
  }
  
  /****************************************************
   * HANDLE COMMANDS REQURING SAFE MODE
   * *************************************************/
  for(cmdIndex = 0; cmdIndex < sizeof(sToSControlCmds)/sizeof(sToSControlCmds)[0]; cmdIndex++)
  {
      if(sToSControlCmds[cmdIndex].cmdName.equals(command))
      {
          setSafeMode();
          sToSControlCmds[cmdIndex].controlCmd();
          /** ROOMBA NOW IN SAFE MODE!!! **/
          return 0;
      }
  }  
 
   /****************************************************
   * HANDLE INPUT COMMANDS
   * *************************************************/
  for(cmdIndex = 0; cmdIndex < sizeof(inputCmds)/sizeof(inputCmds)[0]; cmdIndex++)
  {
      if(inputCmds[cmdIndex].cmdName.equals(command))
      {
          setPassiveMode();
          /** Flush input buffer */
          while(SERIAL_ROOMBA_CMD_RX.available())
          {
            SERIAL_ROOMBA_CMD_RX.read();
          }
          return inputCmds[cmdIndex].inputCmd();
      }
  }

  // If none of the commands were executed, return false
  return -1;
}  


static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    LOG_SERIAL.print("received MQTT payload ");
    LOG_SERIAL.print(message);
    LOG_SERIAL.print(" on topic ");
    LOG_SERIAL.println(topic);
    if(strcmp(topic, _mqttRoombaCmdsTopic) == 0)
    {
        roombaControl(message);
    }
}

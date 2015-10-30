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

typedef enum{
    OFF = 1,
    PASSIVE = 2,
    SAFE = 3,
    FULL = 4
}EIOMode;

#define NO_BYTE_READ 0x100

void goForward();
void goBackward();
void spinLeft();
void spinRight();
void stop();
void updateSensors();
void playSong();
void vacuumOn();
void vacuumOff();
void goHome();
void clean();
void powerOn();
void powerOff();
void gainControl();
void setPassiveMode();
void setSafeMode();
void freeControl();
EIOMode getMode(void);
uint16_t getBatteryCharge(void);
int readByte(int8_t& readByte, int timeout);
int roombaControl(String command);

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
};

/**********************************************
 * INPUT CMDS
 *********************************************/ 
static TsInputCommandMap inputCmds[] = 
{
    {"GETMODE",                (int (*)(void)) (&getMode)},
    {"GETBATT",                (int (*)(void)) (&getBatteryCharge)},
};

// Variable definions
int ddPin = D0;                                      // ddPin controls clean button                               
char sensorbytes[10];

#define bumpright (sensorbytes[0] & 0x01)
#define bumpleft  (sensorbytes[0] & 0x02)

// Setup
void setup() {

  Particle.function("roombaAPI", roombaControl);            // Expose the roomba function to the Spark API

// Pinmode definitions and begin serial
  pinMode(ddPin, INPUT_PULLUP);                     // sets the pins as input
  Serial1.begin(115200);
}


// Loop is empty since we are waiting for commands from the API
void loop() {
}


void stop() {
  Serial1.write((unsigned char) 137);               // DRIVE command
  Serial1.write((unsigned char) 0x00);              // 0 means stop
  Serial1.write((unsigned char) 0x00);
  Serial1.write((unsigned char) 0x80);
  Serial1.write((unsigned char) 0x00);
}

void goForward() {
  Serial1.write((unsigned char) 137);               // DRIVE command
  Serial1.write((unsigned char) 0x01);              // 0x01F4 = 500 = full speed ahead!
  Serial1.write((unsigned char) 0xF4);
  Serial1.write((unsigned char) 0xF9);              // Our roomba has a limp, so it this correction keeps it straight
  Serial1.write((unsigned char) 0xC0);

}
void goBackward() {
  Serial1.write((unsigned char) 137);               // DRIVE command
  Serial1.write((unsigned char) 0xff);              // 0xff38 == -200
  Serial1.write((unsigned char) 0x38);
  Serial1.write((unsigned char) 0x80);
  Serial1.write((unsigned char) 0x00);
}
void spinLeft() {
  Serial1.write((unsigned char) 137);               // DRIVE command
  Serial1.write((unsigned char) 0x00);              // 0x00c8 == 200
  Serial1.write((unsigned char) 0xc8);
  Serial1.write((unsigned char) 0x00);
  Serial1.write((unsigned char) 0x01);              // 0x0001 == 1 == spin left
}
void spinRight() {
  Serial1.write((unsigned char) 137);               // DRIVE command
  Serial1.write((unsigned char) 0x00);              // 0x00c8 == 200
  Serial1.write((unsigned char) 0xc8);
  Serial1.write((unsigned char) 0xff);
  Serial1.write((unsigned char) 0xff);              // 0xffff == -1 == spin right
}

void clean() {                                      
  Serial1.write(135);                               // Starts a cleaning cycle, so command 143 can be initiated
}

void powerOn()
{
    //GPIO connected to clean button
    pinMode(ddPin, OUTPUT);
    digitalWrite(ddPin, LOW);
    delay(300);
    pinMode(ddPin, INPUT_PULLUP);
}

void powerOff()
{
    //GPIO connected to clean button
    pinMode(ddPin, OUTPUT);
    digitalWrite(ddPin, LOW);
    delay(2000);
    pinMode(ddPin, INPUT_PULLUP);
}

void goHome() {                                     // Sends the Roomba back to it's charging base
  //clean();                                          // Starts a cleaning cycle, so command 143 can be initiated
  //delay(5000);
  Serial1.write(143);                               // Sends the Roomba home to charge
  delay(50);
}

void playSong() {                                   // Makes the Roomba play a little ditty
  Serial1.write(140);                         	    // Define a new song
  Serial1.write(0);                                 // Write to song slot #0
  Serial1.write(8);                                 // 8 notes long
  Serial1.write(60);                                // Everything below defines the C Major scale 
  Serial1.write(32); 
  Serial1.write(62);
  Serial1.write(32);
  Serial1.write(64);
  Serial1.write(32);
  Serial1.write(65);
  Serial1.write(32);
  Serial1.write(67);
  Serial1.write(32);
  Serial1.write(69);
  Serial1.write(32);
  Serial1.write(71);
  Serial1.write(32);
  Serial1.write(72);
  Serial1.write(32);
 
  Serial1.write(141);                               // Play a song
  Serial1.write(0);                                 // Play song slot #0
}

void vacuumOn() {                                   // Turns on the vacuum
  Serial1.write(138);
  Serial1.write(7);
}

void vacuumOff() {                                  // Turns off the vacuum
  Serial1.write(138);
  Serial1.write(0);
}

void vibgyor() {                                    // Makes the main LED behind the power button on the Roomba pulse from Green to Red
  Serial1.write(139);
  
  for (int i=0;i<255;i++){ 
    Serial1.write(139);                             // LED seiral command
    Serial1.write(0);                               // We don't want any of the other LEDs
    Serial1.write(i);                               // Color dependent on i
    Serial1.write(255);                             // FULL INTENSITY!
    delay(5);                                       // Wait between cycles so the transition is visible
    }

  for (int i=0;i<255;i++){ 
    Serial1.write(139);
    Serial1.write(0);
    Serial1.write(i);
    Serial1.write(255);
    delay(5);
    }

  for (int i=0;i<255;i++){ 
    Serial1.write(139);
    Serial1.write(0);
    Serial1.write(i);
    Serial1.write(255);
    delay(5);
    }

  for (int i=0;i<255;i++){ 
    Serial1.write(139);
    Serial1.write(0);
    Serial1.write(i);
    Serial1.write(255);
    delay(5);
    }

  Serial1.write(139);
  Serial1.write(0);
  Serial1.write(0);
  Serial1.write(0);

}

void gainControl() {                                // Gain control of the Roomba once it's gone back into passive mode
  setSafeMode();
  Serial1.write(132);                               // Full control mode
  delay(50);
}

void setPassiveMode() {                                // Gain control of the Roomba once it's gone back into passive mode
  // Get the Roomba into control mode 
  Serial1.write(128);   
  delay(50);    
}

void setSafeMode() {                                // Gain control of the Roomba once it's gone back into passive mode
  setPassiveMode(); 
  Serial1.write(130);                               // Safe mode
  delay(50);  
}

void freeControl()
{
  // Get the Roomba into control mode 
  Serial1.write(128);                               // Passive mode
  delay(50);
}

EIOMode getMode(void)
{
  int8_t loc_readByte = 0;
  
  Serial1.write(35);

  if(readByte(loc_readByte, 100) == NO_BYTE_READ)
  {
      return (EIOMode)-1;
  }
  else
  {
      /** +1 increment... in order to not returining 0 */
      return  (EIOMode)(loc_readByte + 1);
  }
}

uint16_t getBatteryCharge()
{
  int8_t loc_readByte = 0;
  uint16_t battLevel = 0;
  
  Serial1.write(25);

  if(readByte(loc_readByte, 50) == NO_BYTE_READ)
  {
      return -1;
  }
  battLevel = loc_readByte << 8;
  
  if(readByte(loc_readByte, 50) == NO_BYTE_READ)
  {
      return -1;
  }
  return loc_readByte + battLevel;
}

int readByte(int8_t& readByte, int timeout)
{
  int count = 0;
  const uint8_t DELAY = 10;
  //ceil 
  const int MAX_INDEX = 1 + ((timeout -1)/DELAY);

  for(count = 0; count <= MAX_INDEX; count++)
  {
      if(Serial1.available())
      {
          readByte = Serial1.read();
          return  0;
      }
      delay(DELAY);
  }
  return NO_BYTE_READ;
}

int roombaControl(String command)
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
          return inputCmds[cmdIndex].inputCmd();
      }
  }

  // If none of the commands were executed, return false
  return -1;
}  
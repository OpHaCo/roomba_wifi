// SparkBot Firmware
// Roomba controller over WIFI
//https://github.com/Lahorde/roomba_wifi
// Link to project share on Spark Community site below:
// 
// https://community.sparkdevices.com/t/sparkbot-manually-automatically-vacuum-your-living-room/625
//
// Credits to http://skaterj10.hackhut.com/2013/01/22/roomba-hacking-101/ for the tips!

// Start with function definitions
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
void power();
void gainControl();
void freeControl();
int roombaControl(String command);

// Variable definions
int ddPin = D0;                                      // ddPin controls clean button
int ledPin = D7;                                     // User LED on the Core for status notification
char sensorbytes[10];

#define bumpright (sensorbytes[0] & 0x01)
#define bumpleft  (sensorbytes[0] & 0x02)

// Setup
void setup() {

  Spark.function("roombaAPI", roombaControl);            // Expose the roomba function to the Spark API
  
  power();

// Pinmode definitions and begin serial
  pinMode(ddPin, INPUT_PULLUP);                     // sets the pins as input
  pinMode(ledPin, OUTPUT);                          // sets the pins as output
  Serial1.begin(115200);
  digitalWrite(ledPin, HIGH);                       // say we're alive

  // Get the Roomba into control mode 
  Serial1.write(128);                               // Passive mode
  delay(50);
  Serial1.write(130);                               // Safe mode
  delay(50);
  Serial1.write(132);                               // Full Control mode
  delay(50);
  digitalWrite(ledPin, LOW);                        // Setup is complete when the D7 user LED goes out
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

void power()
{
    //GPIO connected to clean button
    pinMode(ddPin, OUTPUT);
    digitalWrite(ddPin, LOW);
    delay(300);
    pinMode(ddPin, INPUT_PULLUP);
}

void goHome() {                                     // Sends the Roomba back to it's charging base
  clean();                                          // Starts a cleaning cycle, so command 143 can be initiated
  delay(5000);
  Serial1.write(143);                               // Sends the Roomba home to charge
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
  // Get the Roomba into control mode 
  Serial1.write(128);                               // Passive mode
  delay(50);
  Serial1.write(130);                               // Safe mode
  delay(50);
  Serial1.write(132);                               // Full control mode
  delay(50);
}


void freeControl()
{
  // Get the Roomba into control mode 
  Serial1.write(128);                               // Passive mode
  delay(50);
}

void updateSensors() {                              // Requests a sensor update from the Roomba.  The sensors on our Roomba are broken.
  Serial1.write(142);
  Serial1.write(1);                                 // sensor packet 1, 10 bytes
  delay(100);                                       // wait for sensors 
  int i = 0;
  while(Serial1.available()) {
    int c = Serial1.read();
    if( c==-1 ) {
      for( int i=0; i<5; i ++ ) {                   // say we had an error via the LED
        digitalWrite(ledPin, HIGH); 
        delay(50);
        digitalWrite(ledPin, LOW);  
        delay(50);
      }
    }
    sensorbytes[i++] = c;
  }    
}

int roombaControl(String command)
{
  if(command.substring(0,4) == "STOP")              // STOP command
  {
    stop();
    return 1;
  }

  if(command.substring(0,4) == "BACK")              // BACK command
  {
    goBackward();
    return 1;
  }

  if(command.substring(0,7) == "FORWARD")          // FORWARD command
  {
    goForward();
    return 1;
  }

  if(command.substring(0,5) == "RIGHT")             // RIGHT command
  {
    spinRight();
    return 1;
  }

  if(command.substring(0,4) == "LEFT")              // LEFT command
  {
    spinLeft();
    return 1;
  }

  if(command.substring(0,4) == "SONG")              // SONG command
  {
    playSong();
    return 1;
  }

  if(command.substring(0,8) == "VACUUMON")         // VACUUMON command
  {
    vacuumOn();
    return 1;
  }

  if(command.substring(0,9) == "VACUUMOFF")        // VACUUMOFF command
  {
    vacuumOff();
    return 1;
  }

  if(command.substring(0,8) == "VIBGYOR")          // VIBGYOR command
  {
    vibgyor();
    return 1;
  }

  if(command.substring(0,6) == "GOHOME")            // GOHOME command
  {
    goHome();
    return 1;
  }

  if(command.substring(0,11) == "GAINCONTROL")      // GAINCONTROL command
  {
    gainControl();
    return 1;
  }
  
  if(command.substring(0,5) == "CLEAN")      // CLEAN command
  {
    clean();
    return 1;
  }
  
  if(command.substring(0,5) == "POWER")      // Power command
  {
    power();
    return 1;
  }
  
  if(command.substring(0,11) == "FREECONTROL")      // Power command
  {
    freeControl();
    return 1;
  }

  // If none of the commands were executed, return false
  return -1;
}
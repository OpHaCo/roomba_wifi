# Description
Resurrect your old Roomba! Add Wifi connectivity and control it from anyway! 

Project has been done in Amiqual4Home Equipex Creativity Lab - https://amiqual4home.inria.fr/ 

# Prerequisities
## hardware
 * Spark Core with latest firmware updates :
     
    ```
    sudo particle flash --usb cc3000
    sudo particle flash --usb deep_update_2014_06
    ```
    
 * OR spark photon with latest firmware https://github.com/spark/firmware/releases

 * Roomba - here model R3MOD24A - Unfortunately, commands can be sent to Roomba trew UART but INPUT commands are not received : no activity on Roomba Tx

## software 
 * MQTT broker - (do not use mosquitto 1.4.3 - https://github.com/hirotakaster/MQTT/issues/13 ) 

# Setup
## hardware 
* For this model, no input on external connector can be used to wake up roomba. Clean button is configured in pullup, when pressed it is grounded through 1k resistor. Connecting button input to D0 through 8k resistor gives control on it. 

<img src="https://raw.githubusercontent.com/Lahorde/roomba_wifi/master/img/roomba_hack_0.JPG" width="500">

* A support has been done to put photon near DC connector

<img src="https://raw.githubusercontent.com/Lahorde/roomba_wifi/master/img/roomba_hack_photon.jpg" width="500">

* After soldering it, roomba can be closed... Modification is now invisible. Photon led and buttons :

<img src="https://raw.githubusercontent.com/Lahorde/roomba_wifi/master/img/roomba_hack_led.JPG" width="500">

# Commands

Commands can be sent either using MQTT protocol or using Particle API. 
For details refer : https://github.com/OpHaCo/roomba_wifi/blob/master/doc/iRobot_Roomba_500_Open_Interface_Spec.pdf

## Control commands - as string
 * CMD_NAME = 
  * AFTER THESE COMMANDS ROOMBA IS IN PASSIVE MODE
    * GOHOME
    * CLEAN
  * AFTER THESE COMMANDS ROOMBA IS IN SAFE MODE
    * FORWARD
    * BACK
    * LEFT
    * RIGHT
    * STOP
    * SONG
    * VACUUMON
    * VACUUMOFF
    * VIBGYOR   : leds
  

 * GAINCONTROL
 * FREECONTROL
 * POWERON
 * POWEROFF

## Roomba control over particle API

Command line syntax :

    curl https://api.spark.io/v1/devices/SPARK_CORE_ID/roombaAPI -d access_token='YOUR_TOKEN' -d "params=CMD_NAME"

ex :
    
    curl https://api.spark.io/v1/devices/SPARK_CORE_ID/roombaAPI -d access_token='YOUR_TOKEN' -d "params=SONG"
    {
      "id": "ID",
      "last_app": "",
      "connected": true,
      "return_value": 0
    }

## Roomba control over MQTT
 * a valid MQTT broker must be defined in Photon code mqttserver variable 

### topics
#### "roomba/roombaCmds" : roomba control commands
In this topic, payload containing command name must be sent, 

e.g :

    mosquitto_pub -h BROKER_IP -t roomba/roombaCmds -m SONG
 
#### "roomba/particleCloud" : roomba cloud connection
To enable cloud connection
    
    mosquitto_pub -h BROKER_IP -t roomba/particleCloud -m ENABLE
    
To disable cloud connection
    
    mosquitto_pub -h BROKER_IP -t roomba/particleCloud -m DISABLE
    
## Input commands
Return value as integer in "return_value" field

 * CMD_NAME = 
   * AFTER THESE COMMANDS ROOMBA IS IN PASSIVE MODE
     * GETMODE 
      * return value
    
    ```
    1 OFF
    2 PASSIVE
    3 SAFE
    4 FULL
    ```
     * GETMODE 
      * return value : current roomba charge in mAh
      
ex :

    curl https://api.spark.io/v1/devices/SPARK_CORE_ID/roombaAPI -d access_token='YOUR_TOKEN' -d "params=GETMODE"
    {
      "id": "ID",
      "last_app": "",
      "connected": true,
      "return_value": 1
    }
    
# Test

    export BROKER_IP=your_broker_ip
    source ./roomba_control.sh
    rc_while
    
# Experienced issues
 * roomba credentials & keys lost when battery fully discharded :
  * bug : http://community.particle.io/t/eeprom-persistence-issue/16514 - merged in firmware
  * to solve it (when it blinks red and magenta after restoring wifi credentials) - use key doctor https://community.particle.io/t/photon-flashing-cyan-and-blinking-orange/21530/3
  
    
# References
 * https://community.particle.io/t/sparkbot-spark-core-roomba/625
 * https://community.particle.io/t/getting-mac-address-of-unconnected-core/8473/5
 * http://blog.particle.io/2014/08/06/control-the-connection/

# TODO
 * get another Roomba to handle input commands! 

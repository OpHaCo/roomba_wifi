# Description
Resurrect your old Roomba! Add Wifi connectivity and control it from anyway! 

Project has been done in Amiqual4Home Equipex Creativity Lab - https://amiqual4home.inria.fr/ 

# Prerequisities
 * Spark Core with latest firmware updates :
     
    ```
    sudo particle flash --usb cc3000
    sudo particle flash --usb deep_update_2014_06
    ``` 
 * Roomba - here model R3MOD24A

# Setup
## hardware 
* For this model, no input on external connector can be used to wake up roomba. Clean button is configured in pullup, when pressed it is grounded through 1k resistor. Connecting button input to D0 through 8k resistor gives control on it. 

<img src="https://raw.githubusercontent.com/Lahorde/roomba_wifi/master/img/roomba_hack_0.JPG" width="500">

* After soldering it, roomba can be closed... Modification is now invisible.

# Commands
    curl https://api.spark.io/v1/devices/SPARK_CORE_ID/roombaAPI -d access_token='YOUR_TOKEN' -d "params=CMD_NAME"
 
 * CMD_NAME = 
  * FORWARD
  * BACK
  * LEFT
  * RIGHT
  * STOP
  * SONG
  * VACUUMON
  * VACUUMOFF
  * VIBGYOR   : leds
  * GOHOME
  * GAINCONTROL
  * FREECONTROL
  * CLEAN
  * POWER

ex :
    
    curl https://api.spark.io/v1/devices/SPARK_CORE_ID/roombaAPI -d access_token='YOUR_TOKEN' -d "params=SONG"

# References
 * https://community.particle.io/t/sparkbot-spark-core-roomba/625
 * https://community.particle.io/t/getting-mac-address-of-unconnected-core/8473/5
 * http://blog.particle.io/2014/08/06/control-the-connection/

# TODO
 * Input commands : get mode, get battery state.
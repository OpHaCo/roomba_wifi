# About
# =====
#
# Control a roomba vacuum cleaner from MQTT using a spark photon
#
# Install
# -------
#
# Add something like this to you bash profile:
#

#export BROKER_IP=
#
# Commands:
#
#     f=forward, b=back, l=left, r=right, s=stop, e=song, p=vacuum on, o=vacuum off, l=leds, h=go home, g=full control mode
#     
#
# 

function rc_while() {
  while read -n 1 _c; do
    rc "$_c"
  done
}
 
function rc() {
  _command=''
  
  case "$1" in
  8) echo "Forward march!"
    _command="FORWARD"
     ;;
  2) echo "Come on back now"
    _command="BACK"
     ;;
  4) echo "Left"
    _command="LEFT"
     ;;
  6) echo "Right"
    _command="RIGHT"
     ;;
  5) echo "Stop!"
    _command="STOP"
     ;;
  e) echo "Play a song!"
    _command="SONG"
     ;;   
  p) echo "Vacuum!"
    _command="VACUUMON"
     ;;    
  o) echo "No Vacuum!"
    _command="VACUUMOFF"
     ;;        
  l) echo "VIBGYOR!"
    _command="VIBGYOR"
     ;;      
  h) echo "Go home, SparkBot!"
    _command="GOHO%E"
     ;;
  g) echo "Mind control..."
    _command="GAINCONTROL"
     ;;
  z) echo "Free control..."
    _command="FREECONTROL"
     ;;     
  c) echo "Clean..."
    _command="CLEAN"
     ;;
  a) echo "Power..."
    _command="POWERON"
     ;;  
    

  *) echo "Don't know what to do with $1 : 8=forward, 2=back, 4=left, 6=right, 5=stop, e=song, p=vacuum on, o=vacuum off, l=leds, h=go home, g=full control mode, z=free control, c=clean, a=power"
     return 
     ;;
  esac
  
  _command="mosquitto_pub -h $BROKER_IP -t roomba/roombaCmds -m $_command" 
  echo $_command 
  echo $_command | bash
}

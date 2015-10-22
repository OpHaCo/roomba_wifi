# About
# =====
#
# Control a Spark Core remote control car with a bash script. Why not?
#
# Link to the SparkBot post on the Spark Community page belo:
#
# https://community.sparkdevices.com/t/sparkbot-manually-automatically-vacuum-your-living-room/625
#
# Install
# -------
#
# Add something like this to you bash profile:
#

export SPARK_CORE_DEVICE_ID=55ff6d065075555319241887
export SPARK_CORE_ACCESS_TOKEN=b873f157588aa1ff4ec3a6b053154695355b929d

#     source ~/Downloads/spark_core_rc_car_bash_script.sh <<<- YOU WILL NEED TO CHANGE THIS!
#
# Source it:
#
#     source ~/.profile
#
# And Run
#
#     rc_while
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
  _base_command='curl https://api.spark.io/v1/devices/'"$SPARK_CORE_DEVICE_ID"'/myAPI -d access_token='"$SPARK_CORE_ACCESS_TOKEN"' -d "params=rc,__CMD__"'
  case "$1" in
  8) echo "Forward march!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/FORWARD/')
     ;;
  2) echo "Come on back now"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/BACK/')
     ;;
  4) echo "Hang a lu lu"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/LEFT/')
     ;;
  6) echo "Hang a rubarb"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/RIGHT/')
     ;;
  5) echo "Stop!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/STOP/')
     ;;
  e) echo "Play a song!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/SONG/')
     ;;   
  p) echo "Vacuum!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/VACUUMON/')
     ;;    
  o) echo "No Vacuum!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/VACUUMOFF/')
     ;;        
  l) echo "VIBGYOR!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/VIBGYOR/')
     ;;      
  h) echo "Go home, SparkBot!"
    _command=$(echo "${_base_command}" | sed 's/__CMD__/GOHOME/')
     ;;
  g) echo "Mind control..."
    _command=$(echo "${_base_command}" | sed 's/__CMD__/GAINCONTROL/')
     ;;
  z) echo "Free control..."
    _command=$(echo "${_base_command}" | sed 's/__CMD__/FREECONTROL/')
     ;;     
  c) echo "Clean..."
    _command=$(echo "${_base_command}" | sed 's/__CMD__/CLEAN/')
     ;;
  a) echo "Power..."
    _command=$(echo "${_base_command}" | sed 's/__CMD__/POWER/')
     ;;  
    

  *) echo "Don't know what to do with $1 : f=forward, b=back, l=left, r=right, s=stop, e=song, p=vacuum on, o=vacuum off, l=leds, h=go home, g=full control mode"
     ;;
  esac
 
  echo $_command
  echo $_command | bash
}

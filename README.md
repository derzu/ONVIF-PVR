# ONVIF-PVR
ONVIF multi device client PVR (Personal Video Recorder). It works show and record multiple ONVIF cameras using UDP or TCP.
Tested on Linux, should work easily on others OS's.

# Requirements
* OpenCV Library. Tested on version 3.4.1. Should work with any version greater than 2.
* On UDP mode needs the ffmpeg library.

# Compile
- make

# Run
- ./OnvifPVR configFile.txt
- ./OnvifPVR IP
  - Look for the IP of ONVIF cameras on local network.
- ./OnvifPVR rtsp://@192.168.15.2:554/onvif1 (udp)
  - Runs OnvifPVR for just one camera, udp optional.ß

# Config file
The actual version requires the rtsp link, if is to save or not, and if is udp or tcp. See this sample:
https://github.com/derzu/ONVIF-PVR/blob/master/config.txt

# Printscreen exemple


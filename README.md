# Wakey_Wakey_Wecker

## Web Page

You can reach the demo page [here]("https://mercyrion.github.io/Wakey_Wakey_Alarm/")

## Project description

Read our project description [here](https://github.com/Mercyrion/Wakey_Wakey_Alarm/tree/main/project%20description)

## Installation and set up

## Prerequirements:

We implemented our Arduino project for an [NodeMCU]("https://en.wikipedia.org/wiki/NodeMCU"). To try it out, a NodeMCU is necessary

<p align="center">
  <img src="https://github.com/Mercyrion/Wakey_Wakey_Alarm/assets/57272836/8ba3fa63-0773-434b-a4b5-20d02aff9405" width=500 />
</p>


Also, a smart LED bulb/strip such as the Amazon Alexa controlled RGB Smart LED bulb is needed


## Installation
 
First the Arduino IDE has to be installed. Instructions can be found [here]("https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE") 


Next, you have to install the ESP8266 Board in Arduino IDE. Follow the instructions [here]("https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/")


## Libraries

We used  the Arduino ESP8266 Filesystem Uploader and the NTP Client Library. Both has to be preinstalled

* Installing the NTP Client Library
  
  For installing the Arduino ESP8266 Filesystem Uploader, you just have to move the ESP8266FS folder from this repo into your ardunio tools folder to install the Arduino ESP8266 Filesystem Uploader and    to upload the index.html and data folder on your NodeMCU.

  In your Arduino folder, create a new folder called "tools" and put the ESP8266 folder inside. Restart your Arduino IDE after that.

  In your Arduino IDE click, connect to your NodeMCU and click: tools -> ESP8266 sketch data uplaod. Wait until it is finished.

  You can also follow these [instructions]("https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/") to install the ESP8266 Filesystem Uploader.

</br>

* Installing the NTP Client Library

  The NTP Client Library simplifies the process of retrieving time and date from an NTP server. 

  Navigate to Sketch > Include Library > Manage Libraries… Wait for the Library Manager to download the libraries index and update the list of installed libraries.

  Filter your search by entering ‘ntpclient’. Look for NTPClient by Fabrice Weinberg. Click on that entry and then choose Install.





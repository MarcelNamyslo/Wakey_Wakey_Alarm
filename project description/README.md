# Wakey Wakey Alarm
The Wakey Wakey Alarm is an implementation of an daylight alarm.

## Idea
The idea of daylight alarm is to smoothly wake someone without any hard interruption to feel refreshed and have a good start into the day.
Wakey Wakey extends the alarm to a smart light also used as general room lightning method and extended with some features.

## Concept
As ligths are used some warm white and cold white LED stripes to wake someone not just by increasing the intensity but also changing from a cosy warm/yellow white to a bright cool/blue daylight white. But also used as illumination, the ambient can be changed, chaning the mixture of the two whites.
During the day those stripes can be used as normal light using a dimmer and through an web page different alarms can be set, triggering the daylight alarm.

## Implementation
The NodeMCU controlls the power supply of the srtipes eitherwey directly or through a modem and hosts the website at the same time. With the correct router settings the IP address is also available from outside of the LAN.

## Extentions
There are multiple extensions/features,that can be added.
1. individual alarm types (brightness, duration, warm/cold mixture, blinking and so on) defined by the user.
2. second dimmer for changing the light mixture during normal usage
3. motion sensor to turn on a shade light after a long time of no motion (indicating entrance of the room/going to the toilet in the night to find the way), of cours customizable
4. second mode conrolling the brightness of the room and not the light (automatic adjustion of stripe brightness to keep a stable total brightness, considering other light sources)

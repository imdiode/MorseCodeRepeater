# MorseCodeRepeater
ESP32 based Morse code repeater

This is a morse code repeater designed for ESP32. You can send a sentance or a phrase over Wifi and it will keep repeating it until it is either reset or changed or turned off or broken(please don't break it).

It uses GPIO5 of ESP32 and ground pin. The circuit is fairly simple, attach a 1k resistor to the GPIO5 and connect the other end of the resistor to LED/Buzzer and connect the Gnd directly.

It is using multiple threads, one thread keeps the Morse running and the second one keeps scanning for any update from UART.

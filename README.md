
My goal was to control individually addressable LED strips from my phone.
I decided to use HomeKit, to leverage the Apple ecosystem and the Shortcut app.


HomeKit doesn't support individually addressable LED strips.
Instead, I first made the strip controllable like a simple lightbulb, with only one color for the entire strip.


I then tried to add some kind of drop-down menu to choose between different animation modes.
Unfortunately, I couldn't find any HomeKit device with such control. 
The best I came up with was to use a thermostat: change the temperature to change mode.


The LED animations used were taken from the DemoReel100.ino example from the FastLED library.


[WiFiManager Library](https://github.com/tzapu/WiFiManager) for easy connection setup</br>
[HomeKit Library](https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266) to comunicate with HomeKit</br>
[FastLED](https://fastled.io) to control the LED strip</br>

Once everything was up and running, I created shorcuts to easily switch between each mode.

//TODO add shortcut setup video link
//TODO add demo video link

Note: I had an issue where the first LED of the strip was randomly blinking. 
Had to downgrade to esp8266 2.7.4 (from Board Manager) to fix the issue.
(More info: https://github.com/FastLED/FastLED/issues/1260)

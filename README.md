# DolanClock
A Dolan Clock implemented on a TI MSP432P401R LaunchPad
By Myles Gavic, Thomas Harren, and Brian Mitchell

## To View Time
Time is shown by blinking the red LED every second for the current hour, up to 12. The minutes are shown by changing the color of the RGB LED every minute. The 00:00 time is shown by green, 00:20 by red, and 00:40 by blue. Every minute between these times is a fade to the next color.

This clock has hour and 1 minute accuracy.

## To Set Time
Press the first button to toggle between setting hours, setting minutes, and viewing the clock. A slow blink is shown on the red LED for setting the hours, and a fast blink of the red LED is shown for setting the minutes. Press the second button how ever many times you would like to set the hours or minutes. The RGB LED will flash white for each time the second button is pressed.

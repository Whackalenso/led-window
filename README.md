# LED Window

Code for an LED matrix that displays the day/night cycle from sunrise to sunset to night depending on potentiometer input.

As you turn the potentiometer from right to left, a 2x2 white square on the matrix appears in the bottom right corner of the matrix, then moves in an arc to the middle of the screen, then back down to the bottom left corner, and finally moves out of the screen. While this happens the background changes colors to mimic night turning into day then into night again with a sunset. It starts dark blue to represent night, then turns into an orange to blue gradient as the sun comes in. As the sun comes up, the gradient moves downward to transition into a light blue sky. The same thing happens in reverse to represent the sun coming down and turning into a sunset. 

After the "day" cycle, as you turn the potentiometer from left to right, 3 pixels representing a moon moves in a similar arc from the bottom left corner to the middle of the screen then to the bottom right corner of the screen. While this happens, the background stays a constant dark blue gradient. 

Each change on the screen happens based on if you move the potentiometer enough in a certain direction. If you move the potentiometer a different direction, you can move the sun/moon back and forth as you wish.

### Components and Wiring

![Wiring](Media/img.png)

The microcontroller used to control the LED matrix is an <b>Arduino Uno</b>. Connected to <b>analog pin 1</b> on the Arduino is a <b>potentiometer</b>. The LED matrix used is an RGB 8x8 <b>Adafruit NeoPixel NeoMatrix</b>. This is connected to the Arduino by <b>5V</b>, <b>GND</b>, and <b>analog pin 0</b> for data input. An external power supply is needed which is connected directly to the LED matrix.

### Code

The Arduino program uses the FastLED library for controlling the LED matrix. I made a function that takes in an position and a color and lights up the corresponding LED on the screen. There are also functions for drawing the sun and moon at an input position. In the Arduino loop function, the code takes the potentiometer reading and translates it into a specific phase on the screen. FastLED's blend() function is used to translate a specific value of where the state is currently in time to blend from one color to another, for transitioning the color of the background.

### Inspiration

This project was made for DESINV 23, a class at UC Berkeley, with the criteria to make something light up. There was the opportunity for me to use an LED matrix, so I knew I wanted to use that somehow instead of simple LEDs. In terms of what to actually display, I wanted to make something that would look cool/artistic as opposed to be interactive. I was inspired by sunsets and wanted to see if I could depict the vibrancy of a sunset using only an 8x8 grid. Originally, my idea was to turn the entire background into a solid orange color for the sunset, or even red, to be more representative and abstract. However, I felt like a gradient would make more sense for transitioning between the sunset and regular sky color. I added the potentiometer input so that the visuals are still interactive. It allows you to appreciate it at your own pace and have a little bit of fun with it.
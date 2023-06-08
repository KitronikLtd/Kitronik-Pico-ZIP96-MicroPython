# Kitronik Pico ZIP96 Retro Gamer
 
This repo contains the MicroPython library file for the [Kitronik ZIP96 Retro Gamer for Raspberry Pi Pico](https://kitronik.co.uk/5347).

To use the library you can save the `ZIP96Pico.py` file onto the Pico so it can be imported.

Also in this repo are some games in the `Games` folder which we have been created for the ZIP96 Retro Gamer.

These games include Snake written in MicroPython, and Run Along, Jump & Jump written in C.

We have also added a folder called `A-Mazing Game Lesson Plans` which contains a complete set of lesson plans for creating our A-Mazing Game in MicroPython for the ZIP96 Pico.

Below is a table to help you get started with each of these games. For the Run Along, Jump & Jump two player wireless version you will need two Kitronik ZIP96 Retro Gamers and two Raspberry Pi Pico Ws.

Game | Language | Files needed
-|-|-
Snake | MicroPython | <ul><li>Snake.py</li><li>ZIP96Pico.py</li></ul>
A-Mazing Game | MicroPython | <ul><li>A-Mazing Game folder</li><li>ZIP96Pico.py</li></ul>
Run Along, Jump & Jump | C | [The Kitronik-Pico-ZIP96-C repo](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-C)
<br/>

# How to use the ZIP96 for Pico
Below is a small section explaining how to use each component on the ZIP96 Retro Gamer.
- [Setup the ZIP96 Retro Gamer](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-MicroPython#setup-the-zip96-retro-gamer)
- [Detecting button presses](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-MicroPython#detecting-button-presses)
- [Using the buzzer](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-MicroPython#using-the-buzzer)
- [Using the vibration motor](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-MicroPython#using-the-vibration-motor)
- [Using the ZIPLED screen](https://github.com/KitronikLtd/Kitronik-Pico-ZIP96-MicroPython#using-the-zipled-screen)
<br/>

### Setup the ZIP96 Retro Gamer
To use the ZIP96 Retro Gamer for Pico we first need to import the library and setup our controller. To initialise our controller we can use the `KitronikZIP96` class which will setup all of our buttons and the buzzer for us.
``` python
from ZIP96Pico import *
# Setup ZIP96 Retro Gamer for Pico
controller = KitronikZIP96()
```
<br/>

### Detecting button presses
To detect when a button has been pressed on our Retro Gamer we can use the `pressed` function on each of our buttons. As we don't know when a button is going to be pressed it is best to continuously check for them in a loop.

Inside of the loop we can then use a series of `if` statements to check when each button is being pressed. Inisde of each `if` statement we can respond to a button press by executing some code, such as playing a tone on the buzzer.
``` python
while True:
    if controller.Up.pressed():
        # Respond to Up button being pressed
    if controller.Down.pressed():
        # Respond to Down button being pressed
    if controller.Left.pressed():
        # Respond to Left button being pressed
    if controller.Right.pressed():
        # Respond to Right button being pressed
    if controller.A.pressed():
        # Respond to A button being pressed
    if controller.B.pressed():
        # Respond to B button being pressed
```

The Retro Gamer has some of the GPIO pins broken out so we can access them and customise our controller. Two of these pins are at the top of the controller and could be used for adding left and right shoulder buttons.

To setup the two shoulder buttons we can use the `KitronikButton` class inside the Mini Controller library. When initialising the shoulder buttons we need to give the GPIO pin number as the input for where each of our buttons are connected to.
``` python
# Setup additional Left Shoulder button
LeftShoulder = KitronikButton(11)
# Setup additional Right Shoulder button
RightShoulder = KitronikButton(0)
```

We can now use the shoulder buttons like we would any of the default buttons on the Retro Gamer.
``` python
if LeftShoulder.pressed():
    # Respond to Left Shoulder button being pressed
if RightShoulder.pressed():
    # Respond to Right Shoulder button being pressed
```
<br/>

### Using the buzzer
To use the buzzer on our Retro Gamer we have two different ways of playing a tone.

The first is to use the `playTone` function, then wait for a period of time, and finally use the `stopTone` function. In this code we ask the buzzer to play a tone at 500 Hz. Then we use the `sleep` function to wait for 1 second. And finally we stop playing the tone after we have waited for 1 second.
``` python
# Play tone at frequency 500 Hz
controller.Buzzer.playTone(500)
# Sleep for 1 second
sleep(1)
# Stop tone playing
controller.Buzzer.stopTone()
```

The second is to use the `playTone_Length` function. In this code we ask the buzzer to again play a tone at 500 Hz, but we also ask it to only play the tone for 50 milliseconds.
``` python
# Play tone at frequency 500 Hz for 50 milliseconds
controller.Buzzer.playTone_Length(500, 50)
```
<br/>

### Using the vibration motor
To use the vibration motor on our Retro Gamer we can use the `Vibrate` object.

To start the motor vibrating we simply use the `vibrate` function. Then when we want to stop the vibration motor we use the `stop` function. We can add a delay in between these two functions to control how long the Retro Gamer is vibrating for.
``` python
# Start motor vibrating
controller.Vibrate.vibrate()
# Wait for 1 second
sleep(1)
# Stop the controller vibrating
controller.Vibrate.stop()
```
<br/>

### Using the ZIPLED screen
Also on the Retro Gamer is a screen made up from 96 ZIPLEDs. The screen has a width of 12 ZIPLEDs and a height of 8 ZIPLEDs. Each ZIPLED can be changed separately with over 16 million possible colours and 100 brightness settings.

To update all 96 of the ZIPLEDs on the Retro Gamer screen we can use the `fill` function. The `fill` function takes 1 input which is the RGB values we would like the ZIPLEDs to be. For example, below we set all the ZIPLEDs to green by setting red to 0, green to 255, and blue to 0.

After updating any of the ZIPLEDs values we need to use the `show` function. This will push any changes in RGB values to the ZIPLEDs, and actually update the colours on the screen. Having the `show` function allows us to make multiple changes to the ZIPLEDs colours and update the screen all at once.
``` python
# Sets a new RGB colour for all ZIPLEDs
controller.Screen.fill((0, 255, 0))
# Updates all ZIPLEDs on the screen with the new ZIPLED values
controller.Screen.show()
```

To change the colour of only one ZIPLED we can use the `setLEDMatrix` function. This takes 3 inputs being the X location of the ZIPLED, the Y location of the ZIPLED, and the RGB colour. The X and Y locations start in the top left corner with this pixel being at 0, 0. For example, below we set the ZIPLED in the bottom right corner to purple. And remember to use `show` to update the screen.
``` python
# Sets a new RGB colour for a ZIPLED, using the x, y location
controller.Screen.setLEDMatrix(11, 7, (180, 0, 255))
# Updates all ZIPLEDs on the screen with the new ZIPLED values
controller.Screen.show()
```

To change the brightness of the Retro Gamer screen we can use the `setBrightness` function. This function takes 1 input for the brightness level you would like the ZIPLEDs to be. The brightness can be set from 0% to 100%. Below we set the brightness to 10%. And remember to use `show` to update the screen.
``` python
# Sets a new brightness level for all ZIPLEDs
controller.Screen.setBrightness(10)
# Updates all ZIPLEDs on the screen with the new ZIPLED values
controller.Screen.show()
```

Rather than using the X and Y location to change a ZIPLED, we can give the actual index of the ZIPLED using the `setLED` function. The ZIPLEDs are numbered from 0 to 95 with the ZIPLED in the top left being index 0. The index then increases by 1 for each ZIPLED along the top row, until the top right ZIPLED is at index 11. At this point we then jump to the second row of ZIPLEDs and start numbering from 12 going left to right. This continues to the bottom row, where the final ZIPLED in the bottom right corner is numbered 95.

For example, if we wanted to set the first ZIPLED on the fourth row to cyan we would use an LED number of 36. And remember to use `show` to update the screen.
``` python
# Sets a new RGB colour for a ZIPLED
controller.Screen.setLED(36, (0, 255, 255))
# Updates all ZIPLEDs on the screen with the new ZIPLED values
controller.Screen.show()
```

We can also retrieve the current RBG colour value for a given ZIPLED using the `getLED` function. To do this we simply provide the LED number, like we did for the `setLED` function above, and the RGB value for the ZIPLED is returned.
``` python
# Gets the current RGB colour for a ZIPLED
colour = controller.Screen.getLED(LED)
```

To remove the RGB colour value from an ZIPLED we can use the `clear` function. This will set the ZIPLED's colour to be black. The `clear` function again uses the LED number like the `setLED` function above. And remember to use `show` to update the screen.
``` python
# Sets a ZIPLED to be empty (black)
controller.Screen.clear(LED)
# Updates all ZIPLEDs on the screen with the new ZIPLED values
controller.Screen.show()
```

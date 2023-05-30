import array
import os
from machine import Pin, PWM, ADC, time_pulse_us, I2C, RTC
from rp2 import PIO, StateMachine, asm_pio
from time import sleep, sleep_ms, sleep_us, ticks_ms, ticks_us

'''
Class for the Pico 96 LED retro gamer.    
'''

# Initialise the module with all outputs off
# Buzzer
buzzer = PWM(Pin(2))
buzzer.duty_u16(0)

# List of which StateMachines we have used
usedSM = [False, False, False, False, False, False, False, False]

# The KitronikButton class enable the use of the 2 user input buttons on the board
class KitronikButton:
    def __init__(self, WhichPin):
        self.theButton = Pin(WhichPin, Pin.IN, Pin.PULL_DOWN)
        
    def pressed(self):
        return self.theButton.value()
        
        
# The KitronikVibrate class enable the use of the vibration motor on the board
class KitronikVibrate:
    def __init__(self, WhichPin):
        self.theMotor = Pin(WhichPin, Pin.OUT)
        
    def vibrate(self):
        self.theMotor.value(1)
    
    def stop(self):
        self.theMotor.value(0)
    

# The KitronikBuzzer class enables control of the piezo buzzer on the board
class KitronikBuzzer:
    # Function is called when the class is initialised and sets the buzzer pin to GP2
    def __init__(self, WhichPin):
        self.buzzer = PWM(Pin(WhichPin))
        self.dutyCycle = 32767

    # Play a continous tone at a specified frequency
    def playTone(self, freq):
        if (freq < 30):
            freq = 30
        if (freq > 3000):
            freq = 3000
        self.buzzer.freq(freq)
        self.buzzer.duty_u16(self.dutyCycle)

    # Play a tone at a speciied frequency for a specified length of time in ms
    def playTone_Length(self, freq, length):
        self.playTone(freq)
        sleep_ms(length)
        self.stopTone()

    # Stop the buzzer producing a tone
    def stopTone(self):
        self.buzzer.duty_u16(0)

# The KitronikZIPLEDs class enables control of the ZIP LEDs on the board
class KitronikZIPLEDs:
    # We drive the ZIP LEDs using one of the PIO statemachines.         
    @asm_pio(sideset_init=PIO.OUT_LOW, out_shiftdir=PIO.SHIFT_LEFT, autopull=True, pull_thresh=24)
    def _ZIPLEDOutput():
        T1 = 2
        T2 = 5
        T3 = 3
        wrap_target()
        label("bitloop")
        out(x, 1)               .side(0)    [T3 - 1]
        jmp(not_x, "do_zero")   .side(1)    [T1 - 1]
        jmp("bitloop")          .side(1)    [T2 - 1]
        label("do_zero")
        nop()                   .side(0)    [T2 - 1]
        wrap()

    def __init__(self, num_zip_leds = 96, brightness = 0.2, ZIPPin = 7):
        self.num_zip_leds = num_zip_leds
        # Create  and start the StateMachine for the ZIPLeds
        for i in range(8): # StateMachine range from 0 to 7
            if usedSM[i]:
                continue # Ignore this index if already used
            try:
                self.ZIPLEDs = StateMachine(i, self._ZIPLEDOutput, freq=8_000_000, sideset_base=Pin(ZIPPin))
                usedSM[i] = True # Set this index to used
                break # Have claimed the SM, can leave now
            except ValueError:
                pass # External resouce has SM, move on
            if i == 7:
                # Cannot find an unused SM
                raise ValueError("Could not claim a StateMachine, all in use")
        
        self.theLEDs = array.array("I", [0 for _ in range(self.num_zip_leds)]) #an array for the LED colours.
        self.brightness = brightness
        self.ZIPLEDs.active(1)
            
        # Define some colour tuples for people to use.    
        self.BLACK = (0, 0, 0)
        self.RED = (255, 0, 0)
        self.YELLOW = (255, 150, 0)
        self.GREEN = (0, 255, 0)
        self.CYAN = (0, 255, 255)
        self.BLUE = (0, 0, 255)
        self.PURPLE = (180, 0, 255)
        self.WHITE = (255, 255, 255)
        self.COLOURS = (self.BLACK, self.RED, self.YELLOW, self.GREEN, self.CYAN, self.BLUE, self.PURPLE, self.WHITE)

    # Show pushes the current setup of the LEDS to the physical LEDS - it makes them visible.
    def show(self):
        brightAdjustedLEDs = array.array("I", [0 for _ in range(self.num_zip_leds)])
        for i,c in enumerate(self.theLEDs):
            r = int(((c >> 8) & 0xFF) * self.brightness)
            g = int(((c >> 16) & 0xFF) * self.brightness)
            b = int((c & 0xFF) * self.brightness)
            brightAdjustedLEDs[i] = (g<<16) + (r<<8) + b
        self.ZIPLEDs.put(brightAdjustedLEDs, 8)

    # Turn the LED off by setting the colour to black
    def clear(self, whichLED):
        self.setLED(whichLED, self.BLACK)
        
    # Sets the colour of an individual LED. Use show to make change visible
    def setLED(self, whichLED, whichColour):
        if(whichLED<0):
            raise Exception("INVALID LED:",whichLED," specified")
        elif(whichLED>(self.num_zip_leds - 1)):
            raise Exception("INVALID LED:",whichLED," specified")
        else:
            self.theLEDs[whichLED] = (whichColour[1]<<16) + (whichColour[0]<<8) + whichColour[2]

    def setLEDMatrix(self,X,Y,whichColour):
        #unpack X and Y into the single strip LED position
        whichLED = X + (Y * 12)
        self.setLED(whichLED,whichColour)
        
    # Gets the stored colour of an individual LED, which isnt nessecerily the colour on show if it has been changed, but not 'show'n
    def getLED(self, whichLED):
        if(whichLED<0):
            raise Exception("INVALID LED:",whichLED," specified")
        elif(whichLED>(self.num_zip_leds - 1)):
            raise Exception("INVALID LED:",whichLED," specified")
        else:
            return(((self.theLEDs[whichLED]>>8) & 0xff), ((self.theLEDs[whichLED]>>16)& 0xff) ,((self.theLEDs[whichLED])& 0xff))
    
    # Sets the colour of all LEDs to be the same.
    def fill(self, colour):
        for LED in range(self.num_zip_leds):
            self.setLED(LED, colour)
    
    # Takes 0-100 as a brightness value, brighness is applies in the'show' function
    def setBrightness(self, value):
        #cap to 0-100%
        if (value<0):
            value = 0
        elif (value>100):
            value=100
        self.brightness = value / 100


class KitronikZIP96:
    def __init__(self):
        self.Up = KitronikButton(14)
        self.Down = KitronikButton(12)
        self.Left = KitronikButton(13)
        self.Right= KitronikButton(15)
        self.A = KitronikButton(1)
        self.B = KitronikButton(2)
        
        self.Screen = KitronikZIPLEDs()
        self.Buzzer = KitronikBuzzer(5)
        self.Vibrate = KitronikVibrate(4)

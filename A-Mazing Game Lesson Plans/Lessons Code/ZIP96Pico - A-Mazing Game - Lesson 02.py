import ZIP96Pico
from utime import sleep

# Class to store our player functionality
class Player():
    # Player object constructor
    def __init__(self, x, y, colour, screen, screenWidth, screenHeight):
        self.x = x
        self.y = y
        self.colour = colour
        self.screen = screen
        self.screenWidth = screenWidth
        self.screenHeight = screenHeight
        # Draw the starting position
        self.draw()

    # Draw the current position on the screen
    def draw(self):
        self.screen.setLEDMatrix(self.x, self.y, self.colour)
    
    # Update the current position
    def move(self, x, y):
        # Change the current position by the x and y parameters
        self.x += x
        self.y += y
        
        # Update the new position on the screen
        self.draw()

# Setup ZIP96Pico variables
screenWidth = 12
screenHeight = 8
gamer = ZIP96Pico.KitronikZIP96()
screen = gamer.Screen

# Create an object for our player
player = Player(0, 1, gamer.Screen.YELLOW, screen, screenWidth, screenHeight)

# Show the created objects on the screen
screen.show()

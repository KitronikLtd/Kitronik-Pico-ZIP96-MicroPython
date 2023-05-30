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
    
    # Draw a blank space in the current position on the screen
    def drawEmpty(self):
        self.screen.setLEDMatrix(self.x, self.y, self.screen.BLACK)
    
    # Update the current position
    def move(self, x, y):
        # Reset the current position on the screen
        self.drawEmpty()
        
        # Change the current position by the x and y parameters
        self.x += x
        self.y += y
        
        # Check the new x position is valid (not off the edge of the screen)
        if (self.x < 0): self.x = self.screenWidth - 1
        if (self.x >= self.screenWidth): self.x = 0
        
        # Check the new y position is valid (not off the edge of the screen)
        if (self.y < 0): self.y = self.screenHeight - 1
        if (self.y >= self.screenHeight): self.y = 0
        
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

# Setup game variables
moveDelay = 0.5
moveX = 1
moveY = 0

# Start game loop
while 1:
    
    # Wait for moveDelay before moving
    sleep(moveDelay)
    
    # When up pressed, change player y position by -1
    if (gamer.Up.pressed()):
        moveX = 0
        moveY = -1
    
    # When down pressed, change player y position by 1
    if (gamer.Down.pressed()):
        moveX = 0
        moveY = 1
    
    # When left pressed, change player x position by -1
    if (gamer.Left.pressed()):
        moveX = -1
        moveY = 0
    
    # When right pressed, change player x position by 1
    if (gamer.Right.pressed()):
        moveX = 1
        moveY = 0
    
    # Update the player position using the move values
    player.move(moveX, moveY)
    
    # Show the updates on the screen
    screen.show()

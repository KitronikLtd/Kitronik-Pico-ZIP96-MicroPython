import ZIP96Pico
from utime import sleep
import _thread
from random import randint

# Class to store our player functionality
class Player():
    # Player object constructor
    def __init__(self, x, y, colour, walls, gems, screen, screenWidth, screenHeight):
        self.x = x
        self.y = y
        self.colour = colour
        self.walls = walls
        self.gems = gems
        self.foundGems = 0
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

        # Check each wall in our list
        for wall in self.walls:
            # If the player is colliding with a wall
            if wall.collision(self.x, self.y):
                # Undo the new position using the x and y parameters
                self.x -= x
                self.y -= y
                # Cannot collide with more than one wall so leave loop
                break

        # Check each Gem in our list
        for gem in self.gems:
            # If the Player is colliding with a Gem
            if gem.collision(self.x, self.y):
                # If the Gem hasn't been collected
                if not gem.collected:
                    # Collect the Gem
                    gem.collected = True
                    self.foundGems += 1
                # Cannot collide with more than one Gem so leave loop
                break
        
        # Update the new position on the screen
        self.draw()

# Class to store our Wall functionality
class Wall():
    # Wall object constructor
    def __init__(self, x, y, colour, screen):
        self.x = x
        self.y = y
        self.colour = colour
        self.screen = screen
        # Draw the starting position
        self.draw()
    
    # Draw the current position on the screen
    def draw(self):
        self.screen.setLEDMatrix(self.x, self.y, self.colour)

    # Check if the given x and y are the same as our current position
    def collision(self, x, y):
        return self.x == x and self.y == y

# Class to store our Gem functionality
class Gem():
    # Gem object constructor
    def __init__(self, x, y, colour, screen):
        self.x = x
        self.y = y
        self.colour = colour
        self.screen = screen
        self.collected = False
        # Draw the starting position
        self.draw()

    # Draw the current position on the screen
    def draw(self):
        # Only draw as a Gem if it hasn't been collected
        if self.collected:
            self.screen.setLEDMatrix(self.x, self.y, screen.BLACK)
        else:
            self.screen.setLEDMatrix(self.x, self.y, self.colour)

    # Check if the given x and y are the same as our current position
    def collision(self, x, y):
        return self.x == x and self.y == y

# Class to store our Enemy functionality
class Enemy():
    # Enemy object constructor
    def __init__(self, x, y, colour, walls, gems, screen, screenWidth, screenHeight):
        self.x = x
        self.y = y
        self.colour = colour
        self.walls = walls
        self.gems = gems
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
        
        # Check each Gem in our list
        for gem in self.gems:
            # If the Enemy is colliding with a Gem
            if gem.collision(self.x, self.y):
                # Redraw the Gem. Resets the LED to white if a gem is there
                gem.draw()
                break

    # Update the current position randomly
    def moveRandom(self):
        # Reset the current position on the screen
        self.drawEmpty()
        
        # Set the move parameters randomly
        x = 0
        y = 0
        random = randint(0, 3)
        if random == 0: x = 1
        elif random == 1: x = -1
        elif random == 2: y = 1
        else: y = -1
        
        # Change the current position by the x and y parameters
        self.x += x
        self.y += y
        
        # Check the new x position is valid (not off the edge of the screen)
        if (self.x < 0): self.x = self.screenWidth - 1
        if (self.x >= self.screenWidth): self.x = 0
        
        # Check the new y position is valid (not off the edge of the screen)
        if (self.y < 0): self.y = self.screenHeight - 1
        if (self.y >= self.screenHeight): self.y = 0
        
        # Check each Wall in our list
        for wall in self.walls:
            # If the Enemy is colliding with a Wall
            if wall.collision(self.x, self.y):
                # Undo the new position using the x and y parameters
                self.x -= x
                self.y -= y
                # Cannot collide with more than one Wall so leave loop
                break
        
        # Update the new position on the screen
        self.draw()

# Setup ZIP96Pico variables
screenWidth = 12
screenHeight = 8
gamer = ZIP96Pico.KitronikZIP96()
screen = gamer.Screen

# List to store the coordinates of our walls
wallsXY = [(0, 0), (1, 0), (2, 0), (3, 0), (4, 0), (5, 0), (6, 0), (7, 0), (8, 0), (9, 0), (10, 0), (11, 0),
                                   (3, 1),                         (7, 1),
           (0, 2), (1, 2),         (3, 2),         (5, 2),                         (9, 2),          (11, 2),
           (0, 3),                                 (5, 3), (6, 3),         (8, 3), (9, 3),          (11, 3),
           (0, 4),         (2, 4), (3, 4),         (5, 4), (6, 4),                                  (11, 4),
           (0, 5),         (2, 5),                         (6, 5),         (8, 5),         (10, 5), (11, 5),
                                           (4, 6),                         (8, 6),
           (0, 7), (1, 7), (2, 7), (3, 7), (4, 7), (5, 7), (6, 7), (7, 7), (8, 7), (9, 7), (10, 7), (11, 7)]
# List to store our wall objects
walls = []
# Loop to create wall objects using the wallsXY coordinates
for wallXY in wallsXY:
    # Add the wall objects to our walls list
    walls.append(Wall(wallXY[0], wallXY[1], gamer.Screen.BLUE, screen))

# List to store the coordinates of our gems
gemsXY = [(1, 1), (2, 1),         (4, 1), (5, 1), (6, 1),         (8, 1), (9, 1), (10, 1),
                  (2, 2),         (4, 2),                 (7, 2), (8, 2),         (10, 2),
          (1, 3), (2, 3), (3, 3), (4, 3),                 (7, 3),                 (10, 3),
          (1, 4),                 (4, 4),                 (7, 4), (8, 4), (9, 4),
          (1, 5),         (3, 5), (4, 5),                 (7, 5),         (9, 5),
          (1, 6), (2, 6), (3, 6),         (5, 6), (6, 6), (7, 6),         (9, 6), (10, 6)]
# List to store our Gem objects
gems = []
# Loop to create Gem objects using the gemsXY coordinates
for gemXY in gemsXY:
    # Add the Wall objects to our walls list
    gems.append(Gem(gemXY[0], gemXY[1], gamer.Screen.WHITE, screen))

# Create an object for our player
player = Player(0, 1, gamer.Screen.YELLOW, walls, gems, screen, screenWidth, screenHeight)
# List to store our Enemy objects
enemies = [Enemy(6, 2, gamer.Screen.RED, walls, gems, screen, screenWidth, screenHeight)]

# Show the created objects on the screen
screen.show()

# Setup game variables
moveDelay = 0.5
moveX = 1
moveY = 0
lives = 3

# Get input from the ZIP96Pico Gamer in a separate thread
def gamerInput(gamer, lives):
    global moveX, moveY
    
    buzzer = gamer.Buzzer
    haptic = gamer.Vibrate
    
    # Start game loop
    while lives > 0:
        # When up pressed, change player y position by -1
        if (gamer.Up.pressed()):
            buzzer.playTone(1000)
            haptic.vibrate()
            moveX = 0
            moveY = -1
            sleep(0.1)
        
        # When down pressed, change player y position by 1
        if (gamer.Down.pressed()):
            buzzer.playTone(1000)
            haptic.vibrate()
            moveX = 0
            moveY = 1
            sleep(0.1)
        
        # When left pressed, change player x position by -1
        if (gamer.Left.pressed()):
            buzzer.playTone(1000)
            haptic.vibrate()
            moveX = -1
            moveY = 0
            sleep(0.1)
        
        # When right pressed, change player x position by 1
        if (gamer.Right.pressed()):
            buzzer.playTone(1000)
            haptic.vibrate()
            moveX = 1
            moveY = 0
            sleep(0.1)
        
        buzzer.stopTone()
        haptic.stop()

# Start ZIP96Pico Gamer input logic in a separate thread
_thread.start_new_thread(gamerInput, (gamer, lives))

# Start game loop
while lives > 0:
    
    # Wait for moveDelay before moving
    sleep(moveDelay)
    
    # Update the player position using the move values
    player.move(moveX, moveY)
    # Check if the player has found all the gems
    if player.foundGems == len(gems):
        # Set the screen to green
        screen.fill(screen.GREEN)
        screen.show()
        # Set lives to zero to end the game loops
        lives = 0
        break

    # Check each Enemy in our list
    for i in range(len(enemies)):
        # First Enemy moves randomly
        if i == 0: enemies[i].moveRandom()
    
    # Show the updates on the screen
    screen.show()

import ZIP96Pico
from time import sleep_ms
import _thread
from random import randint

# Class to store our Snake functionality
class Snake():
    # Snake object constructor
    def __init__(self):
        # Store the head of the Snake
        self.head = [0, 0]
        # Store the rest of the Snake body
        self.body = []
        self.headColour = (0, 255, 0)
        self.bodyColour = (0, 255, 128)
    
    # Update the current position
    def move(self, direction, food):
        # Save the head's current position
        current = [self.head[0], self.head[1]]

        # Change the Snakes current position
        if direction == "Up":
            if self.head[1] == 0:
                self.head[1] = 7
            else:
                self.head[1] -= 1
        elif direction == "Down":
            if self.head[1] == 7:
                self.head[1] = 0
            else:
                self.head[1] += 1
        elif direction == "Left":
            if self.head[0] == 0:
                self.head[0] = 11
            else:
                self.head[0] -= 1
        elif direction == "Right":
            if self.head[0] == 11:
                self.head[0] = 0
            else:
                self.head[0] += 1
        
        # Check the Snake has collected the food
        eaten = self.head[0] == food.x and self.head[1] == food.y
        
        if eaten:
            # Set the first body part to be the head's last position
            self.body.insert(0, current)
        elif len(self.body) > 0:
            # Remove the last body part
            self.body.pop()
            # Set the first body part to be the head's last position
            self.body.insert(0, current)
        
        return eaten
    
    # Check the Snake's head has collided with a body part
    def collision(self):
        for part in self.body:
            if part[0] == self.head[0] and part[1] == self.head[1]:
                return True
        return False
    
    # Draw the current Snake on the screen
    def draw(self, screen):
        screen.setLEDMatrix(self.head[0], self.head[1], self.headColour)
        for part in self.body:
            screen.setLEDMatrix(part[0], part[1], self.bodyColour)

# Class to store our Food functionality
class Food():
    # Food object constructor
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.colour = (255, 255, 255)
    
    # Draw the current Food on the screen
    def draw(self, screen):
        screen.setLEDMatrix(self.x, self.y, self.colour)

# Get input from the ZIP96Pico Gamer in a separate thread
def gamerInput(gamer, buzzer, haptic):
    global alive, direction
    
    # Start game loop
    while alive:
        # When up pressed, change snake direction to Up
        if (gamer.Up.pressed()):
            haptic.vibrate()
            direction = "Up"
            sleep_ms(100)
        
        # When up pressed, change snake direction to Down
        if (gamer.Down.pressed()):
            haptic.vibrate()
            direction = "Down"
            sleep_ms(100)
        
        # When up pressed, change snake direction to Left
        if (gamer.Left.pressed()):
            haptic.vibrate()
            direction = "Left"
            sleep_ms(100)
        
        # When up pressed, change snake direction to Right
        if (gamer.Right.pressed()):
            haptic.vibrate()
            direction = "Right"
            sleep_ms(100)
        
        haptic.stop()
    
    buzzer.playTone(440)
    sleep_ms(500)
    buzzer.stopTone()

# Setup ZIP96Pico variables
gamer = ZIP96Pico.KitronikZIP96()
screen = gamer.Screen
screen.setBrightness(10)
buzzer = gamer.Buzzer
haptic = gamer.Vibrate

while True:
    screen.fill((0, 255, 0))
    screen.show()
    while True:
        # Wait for user input
        if (gamer.Up.pressed()
            or gamer.Down.pressed()
            or gamer.Left.pressed()
            or gamer.Right.pressed()
            or gamer.A.pressed()
            or gamer.B.pressed()):
            screen.fill((0, 0, 0))
            break
    
    # Setup game variables
    snake = Snake()
    alive = True
    direction = "Right"
    food = Food(randint(0, 11), randint(0, 7))

    # Start ZIP96Pico Gamer input logic in a separate thread
    _thread.start_new_thread(gamerInput, (gamer, buzzer, haptic))

    while alive:
        screen.show()
        sleep_ms(250)
        screen.fill((0, 0, 0))
        if snake.move(direction, food):
            # Generate a new random Food object
            food = Food(randint(0, 11), randint(0, 7))
        if snake.collision():
            # Head has collided with a body part
            alive = False
        snake.draw(screen)
        food.draw(screen)
    
    # Setup end game text to be displayed
    screen.fill((255, 0, 0))
    end_text = [
        ["                                                                               "],
        ["             ###    #    #   #  #####      ###  #   # ##### ####  #            "],
        ["            #   #  # #  # # # # #         #   # #   # #     #   # #            "],
        ["            #      # #  # # # # #         #   # #   # #     ####  #            "],
        ["            #  ## ##### #  #  # ####      #   #  # #  ####  # #   #            "],
        ["            #   # #   # #     # #         #   #  # #  #     #  #               "],
        ["             ###  #   # #     # #####      ###    #   ##### #   # #            "],
        ["                                                                               "]]
    
    screen.show()
    scroll = 0
    
    while scroll <= len(end_text[0][0]) - 12:
        for y in range(8):
            for x in range(12):
                if end_text[y][0][x + scroll] == "#":
                    screen.setLEDMatrix(x, y, (255, 255, 255))
                else:
                    screen.setLEDMatrix(x, y, (255, 0, 0))
        
        screen.show()
        sleep_ms(100)
        scroll += 1

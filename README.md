# falling-feast
* This is a program made in Raylib C++. It is a small, fun game made in C++.
* The game has two game states — Collecting Food, and Fighting.
* The user can choose between the two game states, by selecting what they want in the combobox and pressing the big Start button.
* In the Collecting Food mode, the Player holds a basket above their head to collect food.
* The Player has a nutrition meter that increases or decreases based on what types of foods the player decides to consume.
* The Player may come across many foods:
  - Cheese — 30 nutrition.
  - Apple — 40 nutrition.
  - Banana — 45 nutrition.
  - Pizza — 50 nutrition.
  - Yoghurt — 55 nutrition.
  - Potion — 100 nutrition.
* When the Player touches these foods that fall from the air, the foods get consumed by the player and the nutrition of the food gets added to the nutrition meter of the Player.
* For example, if a Player consumes an apple, their nutrition increases by 40.
* However, if a food looks spoiled, and the Player consumes it, the nutrition of the specific type of food gets subtracted from the nutrition meter of the Player.
* For example, if a Player consumes a spoiled apple, their nutrition decreases by 40.
* As the Player's nutrition meter increases, foods start to spawn faster and faster, and more spawn at a time.
* The Player has to try their best to avoid consuming spoiled food, and dodging it, and have to consume only fresh food, for they increase the Player's nutrition.
* If the Player's nutrition is more than 1000, the Purchase button enables.
* When the user presses on the Purchase button, the Player gets a Attraction effect for 25 seconds.
* When the Player has the Attraction effect, fresh foods start to move towards the Player's basket, and spoiled foods start to move away from it.
* The Attraction effect makes it easier for the Player to consume only fresh foods.
* The Player can select any background of their choice.
* Once the Player is done collecting enough nutrition, they may press the Title Screen button under the Change BG button, to return to the title screen.
* The user may now select the Fighting game state using the combobox and press Start.
* As soon as the Player enters the Fighting game state, they are greeted by five Enemies.
* An Enemy has 50HP, and five spawn in each round.
* An Enemy wields a bow and has infinite arrows. The Enemy can aim perfectly at the Player and shoot.
* The Enemy has a cooldown of 2–2.5 seconds each shot.
* An Enemy can do 5–10HP of damage to the Player.
* Once all the Enemies in a round die, more spawn again and again.
* The Player also wields a bow and infinite arrows.
* The Player can do a variable amount of damage, and the base is 10HP.
* The Player has to aim to the Enemy using their mouse pointer, and shoot.
* The Player has 100HP.
* The Player also has a level bar.
* In order to Level Up, the Player can press the Level Up Button below the Level bar.
* When the Level Up button is pressed, all the nutrition points of the Player are transferred to the Level bar, and 500 nutrition is equal to one level.
* For every level the Player has, they do 2 extra HP of damage from their bow.
* Once an Enemy dies, they drop a golden coin—the Player can pick it up.
* When the Player picks up a gold coin, the Player earns the coin.
* As the Player earns more and more coins from killing enemies, they can Purchase.
* The Purchase button is activated when the Player has more or equal to 15 coins in their possession.
* When the Purchase button is clicked by the Player—15 coins are taken, and the Player gets either effect—randomly:
  - Extra Speed: The Player moves extra fast.
  - Immunity: The Player is immune to Enemy arrows.
* The effects last for only 25 seconds.
* When the Player has more than 20 coins, the Buy Broccoli Buddy button gets activated.
* When the Player clicks on this button, 20 coins are deducted, and a sentient Broccoli spawns from the top of the screen.
* The Broccoli Buddy wields a bow with infinite arrows, and can shoot with a cooldown of 0.2—0.5 seconds.
* The Broccoli Buddy aims perfectly and shoots at the enemies, and does the same amount of damage as the Player.
* The Broccoli Buddy can aid the Player in battle and in times of need.
* The Broccoli Buddy disappears after 20 seconds.
* The Player can also return to the Title Screen and go to the Collecting Food game state, to collect more nutrition and level up to do more damage to Enemies.
* The Player's goal in this game state, is to dodge arrows, aim and shoot at the Enemies, wisely purchase Broccoli Buddies and effects using coins, and gain as many coins as possible.
* If the Player loses all of their health while fighting Enemies—all their stats get reset.
* In either game state, the Player may press the Change BG button.
* The Change BG button changes the background that the Player is using. It can be changed to:
  - Plains
  - Jungle
  - Desert
  - Tundra
  - Moon
* In either game state, the Player may also press the pause and play button to pause and play.
* The game also includes a debug mode, where the hitboxes and positions are shown on the game objects.
* The controls for the game are:
  - Move Up — Key W.
  - Move Down — Key S.
  - Move Left — Key A.
  - Move Right — Key D.
  - Shoot — Key Space.
  - Change Background — Key G.
  - Return to Title Screen — Key T.
  - Debug mode — Key Tab.
* The repository also has a builtin version of the game, where all the media such as fonts, audio, and images are built into the executable file.


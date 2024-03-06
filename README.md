# Pacman_Unreal_Engine_5.0
## ATTENTION! THIS REPOSITORY IS ARCHIVED. FEEL FREE TO FORK IF YOU WANT: GIVE CREDIT TO THE ORIGINAL PROJECT
Pacman_Unreal_Engine_5.0, a PacMan game clone made with Unreal.

![Pacman_Unreal_Engine_5.0](/images/PACMAN-SCREEn.jpg)

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
        <a href="#built-with">Built With</a>
   </li>
    <li>
        <a href="#prerequisites">Prerequisites</a>
   </li>
    <li>
        <a href="#creators">Creators</a>
    </li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

The game is based on one level that repeats untill player runs out of lives. 
The purpouse is to gain as many points as possible, eating dots and avoiding ghosts, that 
can be eaten only after eating a special food in the game.
Complete game rules of PacMan can be found here:
https://pacman.fandom.com/wiki/Pac-Man_(game)#Gameplay

**ATTENTION!!** Famous bugs of the original game have NOT been implemented, e.g.
the one at level 256 that destroys half of the map.
In this implementation, we have only ONE type of bonus fruit (cherries) that spawns 
around the ghost house under certain conditions throughout the game.

Here's the UML graph of classes used to realize the game:
![UML](/images/UML.png)

Specific game mechanics are explained in interested functions with comments.
Here's a general description of most important game features and respective 
implementation choices:

1) As every pawn (PacMan and ghosts) have different but also similar movements, 
  using OOP, we have implemented AGridPawn, base class that handles movement in the
  grid. For instance, it defines how to understand which node is accessible and which is not,
  how to handle a collision between pawns, and so on on. Specific feature of ghost and PacMan
  have been implemented in derived classes as each one has a different rule to chase PacMan.
  
3) We have defined different types of nodes (the tiles that create the grid) because it is helpful to 
   to link different meshes to each type when GridGenerator object creates the grid at the beginning 
   of the game. This is done by reading a matrix defined in GridGenerator class, according to a 
   specific semantics of symbols (see code for more info).
   In addition, we have also given specific attributes to each one, like isTunnel attribute to identify 
   TunnelNodes where teleport function is triggered.
   
4) GridPlayerController is the class that handles game binding with keyboard controls to play the game.

5) PacManWidget is used to handle game GUI.

6) PacmanGridGameModeBase and PacmanGrid are empty classes required by Unreal Engine in this project. TestGridGameMode handles the game state machine, 
   which is implemented with function defined in this class. Each state will trigger specific actions for every 
   pawn (defined in their classes). These states are defined in enum ECurrentState, except for RESPAWN state that is
   handled with a boolean since it affects only ghosts.
   
   Here's the list of game states:
   * CHASE, ghosts hunt PacMan.
   * SCATTER, this mode tells the ghosts to scatter to one of the four corners of the maze instead of chasing PacMan.
     Each ghost has their own corner.
   * FRIGHT, after PacMan has eaten PowerNodes, ghosts can be eaten and they change appearance and run away from pacman.
   * RESPAWN, after a ghost is eaten in FRIGHT state, it will enter this state and teleport to ghost house at the center
     of the map. After that, it will follow the rule to get out of there and keep playing according to the current state 
     of the game. 

7) GameInstance class is used to upload score and save data throughout levels during the game. 
   Otherwise, changing levels would cause a reset in score variable. Highscore data is saved 
   persistently in a txt file.

8) There are other 2 enums, to save specific properties in every node object of the grid:
   * EFood_Node_Type, used to distinguish PointNodes (normal food) or NOT eatable nodes from PowerNodes that will trigger transition to FRIGHT state.
   * EWalkableId, to check wheter the node is Walkable  or not.

9) There are 2 structs used to optimize certain operations:
   * FDirNode, that associates pointer to a specific node and a direction vector (FVector type). It is used by Ghosts to set target to
     chase, according to game state.
   * FGridPawn_ptr_Struct, reference with pointers to 5 pawns(PacMan and ghosts) used by TestGridGameMode for faster access to specific pawn object.

10) Audio effects have NOT been implemented.


<!-- BUILT WITH -->
## Built With

* [Unreal Engine](https://www.unrealengine.com/en-US)
* [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/vs/community/)

<!-- PREREQUISITES -->

## Prerequisites

* Install Unreal Engine: https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/
* Setting Up Visual Studio for Unreal Engine:
  https://docs.unrealengine.com/5.0/en-US/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine/
* Open the project in Unreal Engine
* Compile and Run pressing greeen arrow button to play the game

**ATTENTION!!**
  You could see that the GUI is NOT centered. To fix that, change padding setting for PacManWidget in Unreal Engine  project.

**ATTENTION!!**
  The target is Windows Desktop, remeber to change it to desired target configuration in Unreal Engine  project.
  
<!-- CREATORS -->
## Creators

* [CapoBoss23](https://github.com/CapoBoss23)
* [giorgiodaneri](https://github.com/giorgiodaneri)

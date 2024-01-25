# Maze Maverick
## Description
This C++ Maze Game offers a unique and engaging experience for both casual players and puzzle enthusiasts. At its core, the game allows users to generate, navigate, and solve mazes with varying levels of complexity. In the 'Easy' mode, mazes are generated with basic configuration, requiring only dimensions as input. For those seeking a more intricate challenge, the 'Hard' mode introduces advanced parameters, including path length and constraints on cell values and blocked paths. The game also tracks player history and progress, offering a leaderboard. With its player information management system, each user’s journey through different mazes is personalized and recorded. Whether you're looking to casually explore a maze or engage in a complex maze-solving challenge, this game offers a diverse range of options to suit all playstyles.

## Table of Contents
- [Description](#description)
- [Installation and Build Instructions](#installation-and-build-instructions)
  - [Prerequisites](#prerequisites)
  - [Clone the Repository](#clone-the-repository)
  - [Build the Project](#build-the-project)
  - [Run the Game](#run-the-game)
- [Features](#features)
- [Game Modes](#game-modes)
  - [Easy Mode](#easy-mode)
  - [Hard Mode](#hard-mode)
- [About `TextTable` Utility](#about-texttable-utility)
- [Authors](#authors)



## Installation and Build Instructions
### Prerequisites
- **C++17 Compiler**: This game requires a compiler that supports C++17 standards.
- **Operating System**: This game have two versions for Windows and Unix (Linux and MacOS). You can use global version for both Windows and Unix, and use other one for only Linux and MacOS.

### Clone the Repository
To get started, clone the repository to your local machine:
```bash
git clone https://github.com/yousofs/maze-maverick.git
```
### Build the Project
To build the **Global version** of the game, navigate to the project directory and run the following commands:
```bash
g++ -std=c++17 -fcolor-diagnostics -fansi-escape-codes -g maze_global.cpp -o maze_game_global
```
To build the **Unix version** of the game on **MacOS and Linux**, navigate to the project directory and run the following commands:
```bash
g++ -std=gnu++14 -std=c++17 -fcolor-diagnostics -fansi-escape-codes -g maze_unix.cpp -o maze_game_unix -lncurses
```
This will compile the game with the following flags:
- **`-std=c++17`**: Compile with C++17 standards.
- **`-fcolor-diagnostics`**: Enable color output in diagnostics (error and warning messages).
- **`-fansi-escape-codes`**: Allow ANSI escape codes in diagnostics.
- **`-g`**: Include debug information in the executable (useful for debugging).

### Run the Game
After building, you can start the game by running in file directory:
```bash
./maze_game
```

## Features
- **Two Maze Generation Modes**:
    - **Easy Mode**: Generates simpler mazes using only the width (`x`) and height (`y`) of the maze. Ideal for quick play and straightforward maze experiences.
    - **Hard Mode**: Creates complex mazes, requiring additional inputs like the maze's width (`x`), height (`y`), path length, and constraints on cell values (`a_l`, `a_u`) as well as the minimum and maximum number of blocked cells (`b_l`, `b_u`). This mode offers a more challenging and intricate maze-solving experience.
    - **Player Interaction**: Players can choose to navigate through the mazes, testing their problem-solving and navigational skills.
    - **Auto-Solve Feature**: The game can automatically solve the generated mazes, demonstrating the solution path.
    - **Player History and Profiles**: Maintains individual player profiles and tracks their game history, allowing for a personalized experience.
    - **Leaderboard**: Features a leaderboard that records and displays high scores or fastest completion times, adding a competitive element to the game.

## Game Modes
The Maze Game offers two distinct modes for creating mazes, each with different parameters and complexity levels:
### Easy Mode
- **Description**: Generates simpler mazes with basic configuration.
- **Parameters**:
    - `x`: Width of the maze.
    - `y`: Height of the maze.
- **Usage**: Ideal for quick play and for those who prefer straightforward maze layouts.

### Hard Mode
- **Description**:  Creates more complex mazes with additional configuration options.
- **Parameters**:
    - `x`: Width of the maze.
    - `y`: Height of the maze.
    - `Path Length`: Specifies the length of the path in the maze.
    - `a_l` and `a_u`: Minimum and maximum values for each cell in the maze.
    - `b_l` and `b_u`: Minimum and maximum number of '0' cells (blocked paths) in the 
- **Usage**: Suited for players who enjoy a more challenging maze-solving experience with customizable complexity.

Both modes offer unique experiences and cater to different preferences in maze complexity and gameplay style.

## About TextTable Utility
### Overview
The `TextTable` class, defined in `table.h`, is an integral part of the "Maze Maverick" project. This utility is designed for creating and managing text-based tables within the game. It's used primarily for displaying structured data like leaderboards and player statistics in a clear, tabular format.

### Features
- **Flexible Alignment**: Supports left, right, and center alignment of text, enhancing the readability of the data presented.
- **Multibyte Character Support**: Capable of handling multibyte character encodings, ensuring proper display of a wide range of characters.
- **Customizable Appearance**: Allows customization of table borders and separators, enabling a tailored look that fits the game's aesthetic.

### Usage in Maze Maverick
In our game, `TextTable` plays a crucial role in:

- Displaying the leaderboard with player names, scores, and rankings.
- Showing player progress and history in an organized manner.
- Any other specific uses in your game, such as displaying maze configurations or game statistics.

### Implementation Details
The class utilizes standard C++ libraries for its operations, ensuring compatibility across different platforms. It is designed with simplicity and flexibility in mind, making it a versatile tool for any feature requiring tabular data representation.

## Authors
- **[Yousof Shahrabi](https://github.com/yousofs)**
- **[Mohammad Mahdi Sharifbeigy](https://github.com/MohammadMahdi-Sharifbeigy)**

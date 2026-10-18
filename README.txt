Name: Timeo Ayme
SN: 25096683

1.SAVE THE PEOPLE!

A terminal-based rescue game written in C using `ncurses`.  
Dodge mines, weave through obstacles, grab powerups, and save the people to climb the leaderboard.

---

2.Game Overview

You control a little `@` character inside a rectangular arena filled with:

- People (`P`) to rescue  
- Mines (`X`) that explode if you step on them  
- Walls (`#`) forming a big cross in the middle  
- Powerups (`S`, `B`, `D`, `L`) that give you temporary abilities

To move, use the arrow keys. Press `q` to go back to the menu and press `I` to enable the AI.

Each rescued person increases your score. Every 5 rescues you level up:  
- The level increases  
- More mines are added  
- New powerups spawn  

Lose all your lives and it’s Game Over, but your score might make it onto the leaderboard.

---

3.Requirements

You’ll need:

- A C compiler (e.g. `gcc`)
- The `ncurses` library
- The `time` library
- The `stdlib`, `stdio` and `string` library
- The `math` library 

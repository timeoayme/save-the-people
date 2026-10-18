# Save the People!

A fast, terminal-based rescue game written in C with `ncurses`. Dodge mines, weave around a wall of obstacles, grab powerups, and rescue people to climb the leaderboard. There's even a built-in AI that will play for you.

```
 SAVE THE PEOPLE!
 SCORE: 300
 AI: OFF (press 'i' to toggle)
 Player: Alex          Lives: 3          Rescues: 3/5 (Level 1)
┌────────────────────────────────────────────────┐
│          X                          P          │
│                         #                      │
│     X                   #               X      │
│                         #                      │
│            #############@#############          │
│                         #                      │
│                  S      #        X             │
│                         #                      │
└────────────────────────────────────────────────┘
```

## Features

- Real-time arcade gameplay in your terminal, with colour and explosion animations
- Level progression: more mines and more powerups as you rescue more people
- Four powerups: shield, speed boost, mine detonator, extra life
- Persistent top-10 leaderboard saved to disk
- Toggleable autopilot you can switch on mid-game
- Zero dependencies beyond `ncurses` and the C standard library

## Getting Started

### Requirements

- A C compiler (e.g. `gcc` or `clang`)
- The `ncurses` development library
- A terminal with colour support, at least **90 columns × 26 rows**

Installing `ncurses`:

| Platform | Command |
| --- | --- |
| Debian / Ubuntu | `sudo apt install build-essential libncurses-dev` |
| Fedora | `sudo dnf install gcc ncurses-devel` |
| Arch | `sudo pacman -S base-devel ncurses` |
| macOS (Homebrew) | `brew install ncurses` |

### Build and run

```bash
gcc game.c -o game -lncurses -lm
./game
```

Run the game from the directory where you want `leaderboard.txt` to live, since scores are stored relative to your working directory.

## How to Play

You control the `@` character inside a walled arena. Reach the `P` to rescue them, avoid everything that hurts, and keep your lives up.

### Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move (you keep moving in the last direction until you change it or hit something) |
| `i` | Toggle the AI autopilot on or off |
| `q` | Quit the current game and return to the menu |
| `Enter` | Select a menu item |

### Arena legend

| Symbol | Meaning |
| --- | --- |
| `@` | You (the arrow-shaped head `^ v < >` shows your facing direction) |
| `P` | A person to rescue |
| `X` | Mine: costs you a life |
| `#` | Wall: the outer border and a large cross in the middle |
| `S` `B` `D` `L` | Powerups (see below) |

### Scoring and lives

| Event | Result |
| --- | --- |
| Rescue a person | +100 points |
| Clear a level (every 5 rescues) | +500 bonus, new arena, more mines |
| Hit a mine | -1 life, -50 points (score never drops below 0) |
| Hit a wall | -1 life |

You start with 3 lives. After a mine or wall hit you respawn at a random safe spot. Lose them all and it's game over; if your score is above 0 it's added to the leaderboard.

### Powerups

| Icon | Name | Effect |
| --- | --- | --- |
| `S` | Shield | Temporary invincibility to mines. Mines you touch are destroyed instead of hurting you, at the cost of some shield time. |
| `B` | Boost | Temporary double speed. |
| `D` | Detonate | Disarms every mine in the arena and gives +200 points. |
| `L` | Life | +1 life and +150 points (capped at 10 lives). |

The number of powerups per level grows as you progress (up to 3 at a time).

### Difficulty curve

- Level 1 starts with 6 mines; each level adds 2 more, up to a cap of 50.
- Mines are re-scattered every time you rescue someone, so memorising positions won't get you far.

### The AI

Press `i` during a game to hand control to the autopilot. It heads straight for the nearest person, sidestepping mines and walls where it can. It's deliberately simple, so it can get stuck behind the cross, so be ready to press `i` again and take over.

## Leaderboard

The top 10 scores are stored in plain text in `leaderboard.txt` (one `name score` pair per line). Spaces in player names are converted to underscores. Delete the file to reset the board.

## Project Structure

```
.
├── game.c            # Entire game: rendering, input, game logic, AI, leaderboard
├── leaderboard.txt   # Created/updated automatically at runtime
└── README.md
```

A quick map of `game.c`:

- **Game loop and state machine**: menu, name entry, instructions, playing, game over, leaderboard
- **World setup**: arena, cross obstacle, and random spawning of the player, people, mines, and powerups
- **Collision and rules**: rescue, mine, wall, and powerup handling plus level progression
- **Rendering**: HUD, layered drawing, and the radial explosion animation
- **AI**: greedy pathing toward the nearest person with a safe-move check
- **Persistence**: loading, sorting, and saving the leaderboard

## Ideas for the Future

- More people per level, or moving hazards
- A smarter AI with real pathfinding (e.g. BFS/A*)
- Resizable arena and terminal-size detection
- Difficulty settings

## Author

Timeo Ayme

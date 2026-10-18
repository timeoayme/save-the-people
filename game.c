// Compile the game with gcc game.c -o game -lncurses -lm  // Build command hint for the user

#include <ncurses.h>    // ncurses library for terminal graphics and input
#include <stdlib.h>     // Standard library (malloc, calloc, free, rand, etc.)
#include <stdio.h>      // Standard I/O (printf, FILE, fopen, etc.)
#include <time.h>       // Time functions (time for random seed)
#include <math.h>       // Math functions (sqrt, etc.)
#include <string.h>     // String handling functions (snprintf, etc.)

#define NUM_PEOPLE 1 // Number of people to rescue
#define CROSS_LENGTH 5 // Half-length of obstacle cross
#define CROSS_WIDTH 10 // Half-width of obstacle cross
#define MAX_OBSTACLES 100 // Safe buffer for obstacles
#define MAX_SCORES 10 // Max entries in leaderboard
#define LEADERBOARD_FILE "leaderboard.txt" // Leaderboard file name
#define PI 3.14159265  // Constant PI, currently unused but defined
#define EXP_COLOR_MINE 3  // Red color pair ID for mine explosion
#define EXP_COLOR_RESCUE 2  // Green color pair ID (unused currently)
#define MAX_POWERUPS 10  // Maximum number of powerups in the game

//Globals (Constants)
const int MAX_X = 50; // Arena width in characters
const int MAX_Y = 20; // Arena height in characters
const int BASE_MINES = 5;  // Base number of mines at level 1
const int MAX_MINES = 50; // Maximum number of mines allowed

//Typedefs

// Score Entry Struct for Leaderboard
typedef struct {
    char name[50]; // Player name for the leaderboard
    int score;     // Player score
} ScoreEntry;

// Game State Enum for managing different screens
typedef enum {
    STATE_MENU,         // Main menu screen
    STATE_PLAYING,      // Actual gameplay state
    STATE_EXIT,         // Exit request state
    STATE_ENTER_NAME,   // Screen to enter player name
    STATE_INSTRUCTIONS, // Instructions screen
    STATE_GAME_OVER,    // Game over screen
    STATE_LEADERBOARD   // Leaderboard screen
} GameState;

// Direction Enum for Player Movement
typedef enum {
    DIR_NONE,   // Not moving
    DIR_UP,     // Moving up
    DIR_DOWN,   // Moving down
    DIR_LEFT,   // Moving left
    DIR_RIGHT   // Moving right
} Direction;

// Player Struct for player attributes
typedef struct {
    int x_pos;        // Player body x-coordinate
    int y_pos;        // Player body y-coordinate
    char body;        // Player body character '@'
    char head;        // Player head character ('^', 'v', '<', '>')
    int head_dx;      // Head x offset from body
    int head_dy;      // Head y offset from body
    char name[50];    // Player name buffer
    int lives;        // Remaining lives
    int level;        // Current level number
    Direction dir;    // Current movement direction
    int shield_timer; // Frames left of shield (invincibility)
    int speed_timer;  // Frames left of speed boost
} Player;

// Arena Struct for arena dimensions
typedef struct {
    int top;    // screen row where arena starts
    int left;   // screen column where arena starts
    int height; // arena height in characters
    int width;  // arena width in characters
} Arena;

// Person Struct for people to rescue
typedef struct {
    int x_pos;   // Person x position
    int y_pos;   // Person y position
    char icon;   // Character used to draw the person (e.g. 'P')
    int rescued; // 1 = rescued, 0 = not rescued
} Person;

// Mine Struct for mines (bombs)
typedef struct {
    int x_pos;   // Mine x position
    int y_pos;   // Mine y position
    char icon;   // Mine character 'X'
    int active;  // 1 = active, 0 = inactive
} Mine;

// Obstacle Struct for walls
typedef struct {
    int x_pos;   // Obstacle x position
    int y_pos;   // Obstacle y position
    char icon;   // Obstacle character '#'
} Obstacle;

// Power-up type enum
typedef enum {
    PU_NONE,      // No powerup (unused)
    PU_SHIELD,    // Shield powerup
    PU_SPEED,     // Speed boost powerup
    PU_DETONATE,  // Detonate all mines powerup
    PU_LIFE,      // Extra life powerup
} PowerUpType;

// Power-up struct
typedef struct {
    int x_pos;        // Powerup x position
    int y_pos;        // Powerup y position
    char icon;        // Powerup icon character
    int active;       // 1 = present on map, 0 = not present
    PowerUpType type; // Type of powerup
} PowerUp;

// Game Struct to hold all game components such as player, arena, people, mines, obstacles, and game state
typedef struct {
    Player *player;      // Pointer to player struct
    Arena *arena;        // Pointer to arena struct
    Person *people;      // Array of people to rescue
    Mine *mines;         // Array of mines
    Obstacle *obstacles; // Array of obstacles

    int num_people;      // Number of people in the arena
    int max_mines;       // Capacity for mines array
    int max_obstacles;   // Capacity for obstacles array

    int current_mines;   // Number of active mines
    int obstacle_count;  // Number of active obstacles

    int score;           // Current player score
    int total_rescued;   // Total number of people rescued across levels

    GameState state;     // Current game state
    int menu_selection;  // Current selection index in the menu

    PowerUp *powerups;   // Array of powerups
    int current_powerups;// Number of active powerups
    int max_powerups;    // Capacity for powerups array

    int ai_active;       // 1 if AI is controlling the player, 0 if player controls
} Game;

//Functions (Refactored Signatures)
void init_arena(Game *game); // Initialize arena dimensions
void draw_arena_border(const Arena *arena); // Draw arena border
void init_player_in_arena(Game *game); // Initialize player position
void draw_menu(int selection); // Draw main menu
void handle_menu_input(Game *game, int ch); // Handle menu input
void spawn_people_in_arena(Game *game); // Spawn people in arena
void check_player_rescue(Game *game); // Check if player rescued any people
void spawn_mines_in_arena(Game *game); // Spawn mines in arena
void check_player_hit_mine(Game *game); // Check if player hit a mine
void game_over_screen(Game *game); // Display game over screen
void check_lives(Game *game); // Check player's lives
void handle_next_level(Game *game); // Handle level progression
void move_player_step(Game *game); // Move player one step
void spawn_cross_obstacle(Game *game); // Spawn cross-shaped obstacle
int is_head_blocked(const Game *game, int x, int y); // Check if head position is blocked
int is_occupied(const Game *game, int x, int y, int people_to_check, int mines_to_check, int powerups_to_check); // Check if a tile is occupied
int is_blocked(const Game *game, int x, int y); // Check if a tile is blocked
int is_safe_move(const Game *game, int x, int y); // Check if a move is safe (not hitting mine or wall)
void run_simple_ai(Game *game); // Simple AI for player movement
int compare_scores(const void *a, const void *b); // Compare function for qsort
void load_leaderboard(ScoreEntry scores[], int *count); // Load leaderboard from file
void save_leaderboard(ScoreEntry scores[], int count); // Save leaderboard to file
void display_leaderboard_screen(Game *game); // Display leaderboard screen
void process_new_high_score(Game *game); // Process new high score
void draw_game(Game *game); // Render all game elements
void draw_explosion_overlay(int cx, int cy, int frame, int max_frames, int color_pair); // Draw one frame of explosion overlay
void explosion_animation(Game *game, int ex, int ey, int max_frames, int color_pair); // Play explosion animation
void spawn_powerups_in_arena(Game *game); // Spawn powerups
void check_player_powerup(Game *game); // Handle player picking up powerups

int main(void) {  // Program entry point

    // 1. Start ncurses
    initscr(); // Initialize the screen and start ncurses mode
    cbreak(); // Disable line buffering so input is available immediately
    noecho(); // Don't show typed characters on screen
    keypad(stdscr, TRUE); // Enable arrow keys, F-keys, etc.
    nodelay(stdscr, TRUE); // Make getch() non-blocking (returns ERR if no key)
    curs_set(0);  // Hide the blinking cursor

    if (has_colors()) {       // Check if terminal supports colors
        start_color();        // Enable color functionality
        // Define color pairs: PAIR_ID, Foreground, Background
        init_pair(1, COLOR_BLUE, COLOR_BLACK);  // Player color
        init_pair(2, COLOR_GREEN, COLOR_BLACK); // People to rescue
        init_pair(3, COLOR_RED, COLOR_BLACK);   // Mines (Bombs)
        init_pair(4, COLOR_CYAN, COLOR_BLACK);  // Obstacles (Walls)
        init_pair(5, COLOR_YELLOW, COLOR_BLACK); // HUD / Arena Border (currently unused)
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);  // Power-ups
        init_pair(7, COLOR_WHITE, COLOR_BLACK);  // Shielded player glow
    }

    srand((unsigned)time(NULL)); // Seed random number generator with current time

    // 2. Initialize Game Struct
    Game *game = (Game *)calloc(1, sizeof(Game)); // Allocate zeroed memory for main game struct
    if (game == NULL) { // Check if allocation failed
        endwin(); // End ncurses mode before printing error
        printf("Error: Could not allocate memory for game.\n");
        return 1; // Exit with error
    }

    // Allocate all components *within* the game struct
    game->player = (Player *)calloc(1, sizeof(Player)); // Allocate memory for player
    game->arena = (Arena *)calloc(1, sizeof(Arena)); // Allocate memory for arena
    game->people = (Person *)calloc(NUM_PEOPLE, sizeof(Person)); // Allocate memory for people array
    game->mines = (Mine *)calloc(MAX_MINES, sizeof(Mine)); // Allocate memory for mines array
    game->obstacles = (Obstacle *)calloc(MAX_OBSTACLES, sizeof(Obstacle)); // Allocate memory for obstacles array
    game->powerups = (PowerUp *)calloc(MAX_POWERUPS, sizeof(PowerUp)); // Allocate memory for powerups array

    // Check for allocation failures
    if (game->player == NULL || game->arena == NULL || game->people == NULL ||
        game->mines == NULL || game->obstacles == NULL || game->powerups == NULL) { // If any allocation failed
        endwin(); // Exit ncurses mode
        printf("Error: Could not allocate memory for game components.\n");
        // Free what might have been allocated
        free(game->player);
        free(game->arena);
        free(game->people);
        free(game->mines);
        free(game->obstacles);
        free(game->powerups);
        free(game);
        return 1; // Exit with error
    }

    // Set initial game values
    snprintf(game->player->name, sizeof(game->player->name), "Player"); // Default player name
    game->num_people = NUM_PEOPLE;      // Set number of people to rescue
    game->max_mines = MAX_MINES;        // Set maximum mines capacity
    game->max_obstacles = MAX_OBSTACLES;// Set maximum obstacles capacity
    game->max_powerups = MAX_POWERUPS;  // Set maximum powerups capacity
    game->score = 0;                    // Initial score
    game->state = STATE_MENU;           // Start at main menu
    game->menu_selection = 0;           // First menu item selected

    int ch;                // Variable for keyboard input
    int running = 1;       // Main loop control flag
    game->ai_active = 0;   // AI off by default

    clear();                              // Clear the screen
    draw_menu(game->menu_selection);      // Draw initial menu
    refresh();                            // Refresh to show it

    while (running) {             // Main game loop
        ch = getch();             // Get key (non-blocking; may be ERR)

        switch (game->state) {    // Handle behavior based on current game state

        case STATE_MENU:          // When in the main menu
            // Only handle input if a key was actually pressed
            if (ch != ERR) {      // If there was some input
                handle_menu_input(game, ch); // Process menu input
            }

            clear();                          // Clear screen each frame
            draw_menu(game->menu_selection);  // Redraw the menu with updated selection
            refresh();                        // Show changes

            if (game->state == STATE_EXIT) {  // If user chose Exit
                running = 0;                  // Stop main loop
            }
            break;                            // End case STATE_MENU

        case STATE_ENTER_NAME:     // Name entry screen
            nodelay(stdscr, FALSE); // Make getch() blocking (wait for user input)
            echo();                 // Show typed characters

            clear(); // Clear screen
            mvprintw(5, 5, "Enter your name (No spaces allowed):"); // Prompt user
            // Read name
            getnstr(game->player->name, sizeof(game->player->name) - 1); // Read string into name buffer
            
            // Fix: If user entered nothing, default to "Player"
            if (game->player->name[0] == '\0') { // Check empty string
                snprintf(game->player->name, sizeof(game->player->name), "Player"); // Reset to default
            }

            noecho();               // Stop echoing typed characters
            nodelay(stdscr, TRUE);  // Return to non-blocking mode

            game->state = STATE_MENU; // Return to menu after name entry
            break;                    // End case STATE_ENTER_NAME

        case STATE_INSTRUCTIONS:   // Instructions screen
            clear(); // Clear screen
            mvprintw(5, 5, "Instructions:"); // Title
            mvprintw(7, 5, "Use arrow keys to move your character."); // Movement instructions
            mvprintw(8, 5, "Rescue all the people (P) in the arena."); // Goal instructions
            mvprintw(9, 5, "Press 'q' to return to the menu."); // Quit hint
            mvprintw(11, 5, "Press any key to return to the menu."); // Prompt to continue
            refresh(); // Show instructions

            nodelay(stdscr, FALSE); // Wait for user input
            getch();                // Block until key press
            nodelay(stdscr, TRUE);  // Back to non-blocking

            game->state = STATE_MENU; // Return to menu afterwards
            break;                    // End case STATE_INSTRUCTIONS

        case STATE_PLAYING:         // Active gameplay state
            if (ch != ERR) {        // If some key was pressed this frame
                if (ch == 'q') {    // If 'q' is pressed
                    game->state = STATE_MENU; // Go back to menu
                    game->ai_active = 0;      // Turn off AI when quitting
                    break;                    // Leave STATE_PLAYING case
                }

                if (ch == 'i') {                // If 'i' pressed, toggle AI
                    game->ai_active = !game->ai_active;  // Toggle AI flag
                    game->player->dir = DIR_NONE;        // Stop manual movement
                }

                if (!game->ai_active) { // Only listen to arrow keys if AI is off
                    switch (ch) {       // Check which arrow key
                    case KEY_UP:
                        game->player->dir = DIR_UP;    // Move up
                        break;
                    case KEY_DOWN:
                        game->player->dir = DIR_DOWN;  // Move down
                        break;
                    case KEY_LEFT:
                        game->player->dir = DIR_LEFT;  // Move left
                        break;
                    case KEY_RIGHT:
                        game->player->dir = DIR_RIGHT; // Move right
                        break;
                    }
                }
            }

            if (game->ai_active) {      // If AI is active
                run_simple_ai(game);    // Let AI choose direction
            }

            // Movement with speed boost
            if (game->player->dir != DIR_NONE) {                // If player is moving
                int steps = (game->player->speed_timer > 0 ? 2 : 1); // Double steps if speed boosted
                for (int s = 0; s < steps && game->state != STATE_GAME_OVER; s++) { // Move multiple steps
                    move_player_step(game);     // Move one step
                    check_player_rescue(game);  // Check rescue overlap
                    check_player_powerup(game); // Check powerup pickup
                    check_player_hit_mine(game);// Check mine collision
                    check_lives(game);          // Check if player still alive
                }
            }

            // Update timers (outside loop)
            if (game->player->shield_timer > 0) game->player->shield_timer--; // Decrease shield duration
            if (game->player->speed_timer > 0) game->player->speed_timer--;   // Decrease speed duration

            // Single-frame checks (for safety)
            check_player_rescue(game);      // Extra rescue check
            check_player_powerup(game);     // Extra powerup check
            check_player_hit_mine(game);    // Extra mine check
            check_lives(game);              // Check lives again

            if (game->people[0].rescued) {  // If the only person is rescued
                game->total_rescued++;      // Increment total rescues

                if (game->total_rescued % 5 == 0) { // Every 5 rescues, new level
                    handle_next_level(game);        // Advance level
                } else {
                    game->people[0].rescued = 0;    // Mark person as not rescued for new spawn
                    init_player_in_arena(game);     // Respawn player somewhere safe
                    spawn_people_in_arena(game);    // Spawn new person
                    spawn_mines_in_arena(game);     // Respawn mines
                }
            }

            check_player_hit_mine(game);    // Check mines again after spawns
            check_lives(game);              // Ensure game over triggered if needed

            if (game->state == STATE_GAME_OVER) { // If game over happened
                break;                             // Exit playing state loop
            }

            draw_game(game);  // Render entire current game frame

            refresh();        // Display it
            break;            // End case STATE_PLAYING

        case STATE_GAME_OVER: // Game over state
            game_over_screen(game); // Show game over screen and handle reset to menu
            break;                  // End case STATE_GAME_OVER

        case STATE_LEADERBOARD: // Leaderboard viewing state
            display_leaderboard_screen(game); // Show leaderboard and return to menu
            break;                            // End case STATE_LEADERBOARD

        case STATE_EXIT:       // Exit state
            running = 0;       // End main loop
            break;             // End case STATE_EXIT
        }
        napms(60); // Sleep for 60 milliseconds to control loop speed (~16 FPS)
    }

    // 4. End ncurses and free memory
    endwin(); // End ncurses mode and restore terminal

    free(game->player);    // Free player struct
    free(game->arena);     // Free arena struct
    free(game->people);    // Free people array
    free(game->mines);     // Free mines array
    free(game->obstacles); // Free obstacles array
    free(game->powerups);  // Free powerups array
    free(game);            // Free the main struct last

    return 0; // Exit success
}

void draw_menu(int selection) {                           // Draws the main menu, highlighting the current selection
    mvprintw(5, 10, "SAVE THE PEOPLE!");                  // Print game title at row 5, column 10

    // Start Game
    if (selection == 0) {                                 // If "Start Game" is the current selection
        attron(A_REVERSE);                                // Turn on reverse video attribute to highlight text
    }
    mvprintw(11, 10, "Start Game");                       // Print "Start Game" option at row 11, column 10
    if (selection == 0) {                                 // If we had highlighted this option
        attroff(A_REVERSE);                               // Turn off reverse video attribute
    }

    // Enter Name
    if (selection == 1) {                                 // If "Enter Player Name" is selected
        attron(A_REVERSE);                                // Highlight this option
    }
    mvprintw(12, 10, "Enter Player Name");                // Print "Enter Player Name" menu entry
    if (selection == 1) {                                 // If selection was 1
        attroff(A_REVERSE);                               // Disable highlight again
    }

    //Instructions
    if (selection == 2) {                                 // If "Instructions" is selected
        attron(A_REVERSE);                                // Highlight "Instructions"
        mvprintw(13, 10, "Instructions");                 // Print "Instructions" with highlight
        attroff(A_REVERSE);                               // Turn off highlight
    } else {                                              // If "Instructions" is not selected
        mvprintw(13, 10, "Instructions");                 // Print it normally
    }

    if (selection == 3) {                                 // If "Leaderboard" is selected
        attron(A_REVERSE);                                // Highlight "Leaderboard"
    }
    mvprintw(14, 10, "Leaderboard");                      // Print "Leaderboard" option on row 14
    if (selection == 3) {                                 // If we just highlighted it
        attroff(A_REVERSE);                               // Stop highlighting
    }

    // Quit (Shifted to selection 4, row 15)
    if (selection == 4) {                                 // If "Quit" is selected
        attron(A_REVERSE);                                // Highlight "Quit"
        mvprintw(15, 10, "Quit");                         // Print "Quit" with highlight
        attroff(A_REVERSE);                               // Disable highlight
    } else {                                              // If "Quit" not selected
        mvprintw(15, 10, "Quit");                         // Print it normally
    }
}

void handle_menu_input(Game *game, int ch) {              // Handles keyboard input when in the menu
    switch (ch) {                                         // Check which key was pressed
    case KEY_DOWN:                                        // Down arrow moves selection down
        if (game->menu_selection < 4) (game->menu_selection)++; // Increase selection but clamp at 4
        break;                                            // End KEY_DOWN handling

    case KEY_UP:                                          // Up arrow moves selection up
        if (game->menu_selection > 0) {                   // Ensure we don't go below 0
            (game->menu_selection)--;                     // Decrease selection index
        }
        break;                                            // End KEY_UP handling

    case '\n':                                            // Enter key confirms selection
        if (game->menu_selection == 0) {                  // If "Start Game" chosen
            game->state = STATE_PLAYING;                  // Switch to playing state

            // Fresh game stats
            game->player->lives = 3;                      // Reset lives to 3
            game->player->level = 1;                      // Reset level to 1
            game->score = 0;                              // Reset score to 0
            game->total_rescued = 0;                      // Reset total rescues

            init_arena(game);                             // Initialize arena dimensions

            // 1) Obstacles first: build the cross and set obstacle_count
            spawn_cross_obstacle(game);                   // Spawn central cross-shaped wall

            // 2) Player spawns on a free tile (avoids obstacles)
            init_player_in_arena(game);                   // Place player at a random valid position

            // 3) People avoid player + obstacles
            spawn_people_in_arena(game);                  // Spawn person at a valid, free tile

            // 4) Mines avoid player + people + obstacles
            // Use consistent formula
            game->current_mines = BASE_MINES + (game->player->level * 2 - 1); // Mine count based on level
            if (game->current_mines > game->max_mines) game->current_mines = game->max_mines; // Clamp to max

            spawn_mines_in_arena(game);                   // Spawn mines in safe locations
            spawn_powerups_in_arena(game);                // Spawn powerups appropriate for level
        }
        if (game->menu_selection == 1) {                  // If "Enter Player Name" chosen
            game->state = STATE_ENTER_NAME;               // Switch to name entry state
        }
        if (game->menu_selection == 2) {                  // If "Instructions" chosen
            game->state = STATE_INSTRUCTIONS;             // Switch to instructions state
        }
        
        if (game->menu_selection == 3) {                  // If "Leaderboard" chosen
            game->state = STATE_LEADERBOARD;              // Switch to leaderboard display
        }

        if (game->menu_selection == 4) {                  // If "Quit" chosen
            game->state = STATE_EXIT;                     // Set state to exit
        }
        break;                                            // End ENTER key handling
    default:                                              // Any other key is ignored
        break;                                            // Do nothing
    }
}

void init_arena(Game *game) {                             // Initialize arena dimensions in game struct
    game->arena->top = 5;                                 // Top border row index (leaves HUD above)
    game->arena->left = 0;                                // Left border column index
    game->arena->height = MAX_Y;                          // Arena height set to constant MAX_Y
    game->arena->width = MAX_X;                           // Arena width set to constant MAX_X
}

void draw_arena_border(const Arena *arena) {              // Draws the rectangular border of the arena
    mvaddch(arena->top, arena->left, ACS_ULCORNER);       // Draw upper-left corner
    mvaddch(arena->top, arena->left + arena->width - 1, ACS_URCORNER); // Draw upper-right corner
    mvaddch(arena->top + arena->height - 1, arena->left, ACS_LLCORNER); // Draw lower-left corner
    mvaddch(arena->top + arena->height - 1, arena->left + arena->width - 1, ACS_LRCORNER); // Draw lower-right

    // Top and bottom horizontal lines
    for (int x = arena->left + 1; x < arena->left + arena->width - 1; x++) { // Loop across top/bottom edges
        mvaddch(arena->top, x, ACS_HLINE);                // Draw top horizontal line
        mvaddch(arena->top + arena->height - 1, x, ACS_HLINE); // Draw bottom horizontal line
    }
    // Left and right vertical lines
    for (int y = arena->top + 1; y < arena->top + arena->height - 1; y++) { // Loop down vertical edges
        mvaddch(y, arena->left, ACS_VLINE);               // Draw left vertical border
        mvaddch(y, arena->left + arena->width - 1, ACS_VLINE); // Draw right vertical border
    }
}

void init_player_in_arena(Game *game) {                   // Place the player somewhere safe in the arena
    int x, y;                                             // Temporary coordinates

    // Find a random free tile that is not a wall or obstacle
    do {                                                  // Loop until a walkable tile is found
        x = game->arena->left + 1 + (rand() % (game->arena->width - 2)); // Random x inside border
        y = game->arena->top + 1 + (rand() % (game->arena->height - 2)); // Random y inside border
    } while (is_blocked(game, x, y));                     // Repeat if tile is blocked by wall or obstacle

    game->player->x_pos = x;                              // Set player's x position
    game->player->y_pos = y;                              // Set player's y position

    game->player->body = '@';                             // Player body character
    game->player->head = 'v';                             // Default head facing downward
    game->player->head_dx = 0;                            // Head offset x initially 0
    game->player->head_dy = 1;                            // Head offset y initially downwards

    game->player->dir = DIR_NONE;                         // Player initially not moving
}

void spawn_people_in_arena(Game *game) {                  // Spawn all people in valid positions
    for (int i = 0; i < game->num_people; i++) {          // Loop through each person to spawn
        int x, y;                                         // Temporary spawn coordinates
        do {                                              // Try random positions until free
            x = game->arena->left + 1 + (rand() % (game->arena->width - 2)); // Random x inside arena
            y = game->arena->top + 1 + (rand() % (game->arena->height - 2)); // Random y inside arena
        } while (is_occupied(game, x, y, i, 0, 0));       // Ensure not occupied by earlier people or obstacles

        game->people[i].icon = 'P';                       // Set display character for person
        game->people[i].x_pos = x;                        // Save x coordinate
        game->people[i].y_pos = y;                        // Save y coordinate
        game->people[i].rescued = 0;                      // Mark person as not rescued
    }
}

void check_player_rescue(Game *game) {                    // Check if player is on same tile as any person
    for (int i = 0; i < game->num_people; i++) {          // For each person
        if (!game->people[i].rescued &&                   // If person is not already rescued
            game->player->x_pos == game->people[i].x_pos && // And player shares same x
            game->player->y_pos == game->people[i].y_pos)   // And same y position
        {
            game->people[i].rescued = 1;                  // Mark this person as rescued
            game->score += 100;                           // Award 100 points for rescue
        }
    }
}

void spawn_mines_in_arena(Game *game) {                   // Spawn all mines for current level
    for (int i = 0; i < game->current_mines; i++) {       // Loop for number of mines to create
        int x, y;                                         // Temporary spawn coords
        do {                                              // Try until a free tile is found
            x = game->arena->left + 1 + (rand() % (game->arena->width - 2)); // Random x in arena
            y = game->arena->top + 1 + (rand() % (game->arena->height - 2)); // Random y in arena
        } while (is_occupied(game, x, y, game->num_people, i, game->current_powerups)); // Avoid overlaps

        game->mines[i].icon = 'X';                        // Character used to draw a mine
        game->mines[i].x_pos = x;                         // Store x position
        game->mines[i].y_pos = y;                         // Store y position
        game->mines[i].active = 1;                        // Mark mine as active
    }
}

void spawn_cross_obstacle(Game *game) {                   // Spawn central cross-shaped obstacle structure
    game->obstacle_count = 0;                             // Reset current obstacle count

    int cx = game->arena->left + game->arena->width / 2;  // X coordinate for cross center
    int cy = game->arena->top + game->arena->height / 2;  // Y coordinate for cross center

    // Vertical arm of the cross
    for (int dy = -CROSS_LENGTH; dy <= CROSS_LENGTH; dy++) { // Loop from top to bottom of vertical arm
        // Check for buffer overflow
        if (game->obstacle_count >= game->max_obstacles) return; // Stop if we hit obstacle capacity

        game->obstacles[game->obstacle_count].x_pos = cx;       // Set x at center for vertical arm
        game->obstacles[game->obstacle_count].y_pos = cy + dy;  // y varies around center
        game->obstacles[game->obstacle_count].icon = '#';       // Wall character
        (game->obstacle_count)++;                               // Increase count of obstacles
    }

    // Horizontal arm of the cross
    for (int dx = -CROSS_WIDTH; dx <= CROSS_WIDTH; dx++) { // Loop left-right for horizontal arm
        if (dx == 0) continue;                           // Skip center tile (already filled)

        // Check for buffer overflow
        if (game->obstacle_count >= game->max_obstacles) return; // Stop if no more space

        game->obstacles[game->obstacle_count].x_pos = cx + dx;   // x varies around center
        game->obstacles[game->obstacle_count].y_pos = cy;        // y fixed at center
        game->obstacles[game->obstacle_count].icon = '#';        // Wall character
        (game->obstacle_count)++;                               // Increment obstacles count
    }
}

void draw_game(Game *game) {                             // Render the full game frame
    clear();                                             // Clear screen before drawing

    // HUD (copy from main loop)
    mvprintw(0, 0, "SAVE THE PEOPLE!");                  // Draw game title at top
    mvprintw(1, 0, "SCORE: %d", game->score);            // Show current score
    mvprintw(2, 0, "AI: %s (press 'i' to toggle)",       // Show AI status (on/off)
            game->ai_active ? "ON" : "OFF");             // Ternary: print "ON" if AI active, else "OFF"
    mvprintw(3, 0, "Player: %s", game->player->name);    // Show player name
    mvprintw(3, 20, "Lives: %d", game->player->lives);   // Show remaining lives
    mvprintw(3, 40, "Rescues: %d/%d (Level %d)",         // Show rescues and level progress
            game->total_rescued, (game->player->level * 5), game->player->level);
    mvprintw(3, 70, "Shield:%d Boost:%d",                // Show timers for shield and speed boost
             game->player->shield_timer / 10, game->player->speed_timer / 10);

    draw_arena_border(game->arena);                      // Draw the arena border

    // People (green)
    for (int i = 0; i < game->num_people; i++) {         // Loop through all people
        if (!game->people[i].rescued) {                  // Draw only if not rescued
            attron(COLOR_PAIR(2));                       // Enable green color pair
            mvaddch(game->people[i].y_pos,               // Move to person's position (row)
                    game->people[i].x_pos,               // Person x position (column)
                    game->people[i].icon);               // Draw 'P'
            attroff(COLOR_PAIR(2));                      // Disable green color
        }
    }

    // Mines (red)
    for (int i = 0; i < game->current_mines; i++) {      // Loop over all mines
        if (game->mines[i].active) {                     // Only draw active mines
            attron(COLOR_PAIR(3));                       // Enable red color pair
            mvaddch(game->mines[i].y_pos,                // Row of mine
                    game->mines[i].x_pos,                // Column of mine
                    game->mines[i].icon);                // Draw 'X'
            attroff(COLOR_PAIR(3));                      // Disable red color
        }
    }

    // Obstacles first (background layer)
    for (int i = 0; i < game->obstacle_count; i++) {     // Loop through obstacles
        attron(COLOR_PAIR(4));                           // Enable cyan color for walls
        mvaddch(game->obstacles[i].y_pos,                // Row of obstacle
                game->obstacles[i].x_pos,                // Column of obstacle
                game->obstacles[i].icon);                // Draw '#'
        attroff(COLOR_PAIR(4));                          // Disable cyan color
    }

    // Power-ups (magenta, foreground but under player)
    for (int i = 0; i < game->current_powerups; i++) {   // Loop over powerups
        if (game->powerups[i].active) {                  // Only draw active ones
            attron(COLOR_PAIR(6) | A_BOLD);              // Enable magenta and bold
            mvaddch(game->powerups[i].y_pos,             // Row of powerup
                    game->powerups[i].x_pos,             // Column of powerup
                    game->powerups[i].icon);             // Draw its icon
            attroff(COLOR_PAIR(6) | A_BOLD);             // Disable magenta and bold
        }
    }

    // Player body (top layer, shielded effect)
    if (game->player->shield_timer > 0) {                // If shield is active
        attron(COLOR_PAIR(7) | A_BLINK | A_BOLD);        // White color, blinking, bold
        mvaddch(game->player->y_pos,                     // Player row
                game->player->x_pos,                     // Player column
                game->player->body);                     // Draw '@'
        attroff(COLOR_PAIR(7) | A_BLINK | A_BOLD);       // Turn off attributes
    } else {
        attron(COLOR_PAIR(1));                           // Normal player color (blue)
        mvaddch(game->player->y_pos,                     // Player row
                game->player->x_pos,                     // Player column
                game->player->body);                     // Draw '@'
        attroff(COLOR_PAIR(1));                          // Turn off color
    }

    // Player head
    int hx = game->player->x_pos + game->player->head_dx; // Compute head x coordinate
    int hy = game->player->y_pos + game->player->head_dy; // Compute head y coordinate
    if (hy > game->arena->top &&                          // Ensure head is inside arena vertically
        hy < game->arena->top + game->arena->height - 1 &&
        hx > game->arena->left &&                         // Ensure head is inside arena horizontally
        hx < game->arena->left + game->arena->width - 1) {
        if (!is_head_blocked(game, hx, hy)) {             // Do not draw head if blocked by entity
            int pair = (game->player->shield_timer > 0 ? 7 : 1); // Use shield color if shield active
            attron(COLOR_PAIR(pair));                     // Turn on chosen color
            mvaddch(hy, hx, game->player->head);          // Draw head character at computed position
            attroff(COLOR_PAIR(pair));                    // Turn off that color
        }
    }
}

int is_blocked(const Game *game, int x, int y) {          // Check if a tile is blocked by wall or obstacle
    // Block movement if hitting the arena walls
    if (y <= game->arena->top ||                          // If at or above top border
        y >= game->arena->top + game->arena->height - 1 ||// Or at/below bottom border
        x <= game->arena->left ||                         // Or at/left of left border
        x >= game->arena->left + game->arena->width - 1) {// Or at/right of right border
        return 1;                                         // Tile is blocked
    }

    // Block movement if hitting an obstacle (the cross)
    for (int i = 0; i < game->obstacle_count; i++) {      // Loop obstacles
        if (game->obstacles[i].x_pos == x &&              // If obstacle shares x
            game->obstacles[i].y_pos == y) {              // And shares y
            return 1;                                     // Tile is blocked
        }
    }

    return 0; // Tile is walkable                          // No block found, tile is free
}

void move_player_step(Game *game) {                       // Move player exactly one step based on direction
    int nx = game->player->x_pos;                         // Start with current x
    int ny = game->player->y_pos;                         // Start with current y

    switch (game->player->dir) {                          // Check current direction
    case DIR_UP:
        ny--;                                             // Move one cell up
        game->player->head = '^';                         // Head appears above body
        game->player->head_dx = 0;                        // Head x offset is 0
        game->player->head_dy = -1;                       // Head y offset is -1 (above)
        break;

    case DIR_DOWN:
        ny++;                                             // Move one cell down
        game->player->head = 'v';                         // Head appears below body
        game->player->head_dx = 0;                        // Head x offset is 0
        game->player->head_dy = 1;                        // Head y offset is 1 (below)
        break;

    case DIR_LEFT:
        nx--;                                             // Move one cell left
        game->player->head = '<';                         // Head appears left of body
        game->player->head_dx = -1;                       // Head offset x is -1
        game->player->head_dy = 0;                        // Head offset y is 0
        break;

    case DIR_RIGHT:
        nx++;                                             // Move one cell right
        game->player->head = '>';                         // Head appears right of body
        game->player->head_dx = 1;                        // Head offset x is +1
        game->player->head_dy = 0;                        // Head offset y is 0
        break;

    case DIR_NONE:
        return;                                           // If no direction, do nothing
    }

    if (is_blocked(game, nx, ny)) {                       // If new position hits wall/obstacle
        game->player->lives--;                            // Lose a life
        game->player->dir = DIR_NONE;                     // Stop moving
        init_player_in_arena(game);                       // Respawn at a safe place
        
        // Check if the game is over immediately
        if (game->player->lives <= 0) {                   // If no lives left
            game->state = STATE_GAME_OVER;                // Trigger game over state
        }
        
        return;                                           // Stop processing this move
    }

    game->player->x_pos = nx;                             // Update player x position
    game->player->y_pos = ny;                             // Update player y position
}

void check_player_hit_mine(Game *game) {                  // Detect collisions between player and mines
    for (int i = 0; i < game->current_mines; i++) {       // Loop all mines
        if (game->mines[i].active &&                      // Only check active mines
            game->player->x_pos == game->mines[i].x_pos &&// Same x as player
            game->player->y_pos == game->mines[i].y_pos) {// Same y as player
            
            if (game->player->shield_timer > 0) {         // If shield is active
                game->player->shield_timer -= 5;          // Slightly drain shield timer
                game->mines[i].active = 0;                // Deactivate the mine
                explosion_animation(game,                 // Play explosion at mine location
                                     game->mines[i].x_pos,
                                     game->mines[i].y_pos,
                                     6,                   // Number of frames
                                     EXP_COLOR_MINE);     // Red explosion color
                return;                                   // End after handling one collision
            }
            
            // Non-shielded hit (existing)
            game->mines[i].active = 0;                    // Deactivate the mine
            game->player->lives--;                        // Deduct one life
            game->player->dir = DIR_NONE;                 // Stop movement
            game->score -= 50;                            // Penalize score by 50
            if (game->score < 0) game->score = 0;         // Clamp score to non-negative
            explosion_animation(game,                     // Show explosion animation
                                 game->mines[i].x_pos,
                                 game->mines[i].y_pos,
                                 8,                       // More frames for unshielded hit
                                 EXP_COLOR_MINE);         // Explosion color
            init_player_in_arena(game);                   // Respawn the player somewhere safe
            return;                                       // Stop after processing hit
        }
    }
}

void check_lives(Game *game) {                            // Check if player has any lives left
    if (game->player->lives <= 0) {                       // If zero or negative lives
        game->state = STATE_GAME_OVER;                    // Switch state to game over
    }
}

void game_over_screen(Game *game) {                       // Handle game over display and flow
    process_new_high_score(game);                         // Try to record current score in leaderboard
    
    clear();                                              // Clear the screen
    mvprintw(10, 10, "GAME OVER!");                       // Print game over message
    mvprintw(12, 10, "Player: %s", game->player->name);   // Show player name
    mvprintw(13, 10, "Final Score: %d", game->score);     // Show final score
    mvprintw(14, 10, "You have no lives left.");          // Explain loss
    mvprintw(15, 10, "Press any key to return to the menu."); // Prompt user to continue
    refresh();                                            // Show the text

    nodelay(stdscr, FALSE);                               // Switch to blocking input
    getch();                                              // Wait for any key press
    nodelay(stdscr, TRUE);                                // Switch back to non-blocking
    game->player->lives = 3;                              // Reset lives for next game
    game->player->shield_timer = 0;                       // Reset shield timer
    game->player->speed_timer = 0;                        // Reset speed timer

    game->state = STATE_MENU;                             // Return to main menu state
}

void handle_next_level(Game *game) {                      // Progress to next level logic
    game->player->level++;                                // Increase level count
    
    game->score += 500;                                   // Award level completion bonus

    // Ensure the single person is set to 'not rescued' for the new level
    // (This is critical for the new spawn logic to work)
    game->people[0].rescued = 0;                          // Make sure person is considered unrescued

    // Rebuild the arena and reset positions
    init_arena(game);                                     // Reinitialize arena dimensions
    spawn_cross_obstacle(game);                           // Re-create central cross walls
    init_player_in_arena(game);                           // Respawn player at safe location

    // Spawn the new person
    spawn_people_in_arena(game);                          // Place new person

    // Mines scale and spawn
    game->current_mines = BASE_MINES + (game->player->level * 2 - 1); // Recalculate mine count
    if (game->current_mines > game->max_mines) game->current_mines = game->max_mines; // Clamp
    spawn_mines_in_arena(game);                           // Spawn mines
    spawn_powerups_in_arena(game);                        // Spawn powerups
}

int is_head_blocked(const Game *game, int x, int y) {     // Check if player's head tile conflicts with objects
    // People
    for (int i = 0; i < game->num_people; i++) {          // Loop all people
        if (!game->people[i].rescued &&                   // If not rescued
            game->people[i].x_pos == x &&                 // Same x coordinate
            game->people[i].y_pos == y) {                 // Same y coordinate
            return 1;                                     // Head is blocked by person
        }
    }

    // Mines
    for (int i = 0; i < game->current_mines; i++) {       // Loop mines
        if (game->mines[i].active &&                      // Only active mines
            game->mines[i].x_pos == x &&                  // Same x as head
            game->mines[i].y_pos == y) {                  // Same y as head
            return 1;                                     // Blocked by mine
        }
    }

    // Obstacles
    for (int i = 0; i < game->obstacle_count; i++) {      // Loop obstacles
        if (game->obstacles[i].x_pos == x &&              // Same x
            game->obstacles[i].y_pos == y) {              // Same y
            return 1;                                     // Blocked by wall
        }
    }

    // Power-ups (new)
    for (int i = 0; i < game->current_powerups; i++) {    // Loop powerups
        if (game->powerups[i].active &&                   // Only active ones
            game->powerups[i].x_pos == x &&               // Same x
            game->powerups[i].y_pos == y) {               // Same y
            return 1;                                     // Blocked by powerup (head not drawn over it)
        }
    }
    return 0;                                             // Head tile not blocked
}

int is_occupied(const Game *game, int x, int y,           // Check if a tile is occupied by certain entities
                int people_to_check, int mines_to_check, int powerups_to_check) {
    // Player body
    if (game->player != NULL &&                           // Ensure player exists
        x == game->player->x_pos && y == game->player->y_pos) { // Check same tile as player
        return 1;                                         // Tile is occupied
    }

    // People (check only those already spawned)
    for (int i = 0; i < people_to_check; i++) {           // Limit to given number of people
        if (!game->people[i].rescued &&                   // Ignore rescued ones
            x == game->people[i].x_pos &&                 // Check x
            y == game->people[i].y_pos) {                 // Check y
            return 1;                                     // Tile is occupied by a person
        }
    }

    // Mines (check only those already spawned)
    for (int i = 0; i < mines_to_check; i++) {            // Limit to given number of mines
        if (game->mines[i].active &&                      // Only active mines
            x == game->mines[i].x_pos &&                  // Same x
            y == game->mines[i].y_pos) {                  // Same y
            return 1;                                     // Tile occupied by mine
        }
    }

    // Obstacles (check all)
    for (int i = 0; i < game->obstacle_count; i++) {      // Loop all obstacles
        if (x == game->obstacles[i].x_pos &&              // Same x
            y == game->obstacles[i].y_pos) {              // Same y
            return 1;                                     // Tile occupied by wall
        }
    }

    for (int i = 0; i < powerups_to_check; i++) {         // Loop subset of powerups
        if (game->powerups[i].active &&                   // Only active
            x == game->powerups[i].x_pos &&               // Same x
            y == game->powerups[i].y_pos) {               // Same y
            return 1;                                     // Tile occupied by powerup
        }
    }
    return 0;                                             // Tile is not occupied by any checked entity
}

int is_safe_move(const Game *game, int x, int y) {        // Check if tile is safe for AI (not wall, not mine)
    // 1. Check for Walls/Obstacles
    if (is_blocked(game, x, y)) {                         // If tile is blocked
        return 0; // Unsafe because it's blocked          // Unsafe move
    }

    // 2. Check for Mines
    for (int i = 0; i < game->current_mines; i++) {       // Loop all mines
        if (game->mines[i].active &&                      // Only active
            game->mines[i].x_pos == x &&                  // Same x
            game->mines[i].y_pos == y) {                  // Same y
            return 0; // Unsafe because it's a mine       // Tile is unsafe
        }
    }

    return 1; // Safe                                     // No obstacles or mines here
}

void run_simple_ai(Game *game) {                          // Very simple AI to move toward the nearest person
    // 1. Find a target (the first un-rescued person)
    Person *target = NULL;                                // Pointer to target person
    for (int i = 0; i < game->num_people; i++) {          // Loop all people
        if (!game->people[i].rescued) {                   // If person is not rescued
            target = &game->people[i];                    // Use this as target
            break; // Found our target                    // Stop searching
        }
    }

    // 2. If no target, stop moving
    if (target == NULL) {                                 // If all people are rescued
        game->player->dir = DIR_NONE;                     // Don't move
        return;                                           // Exit AI function
    }

    // 3. Calculate distance to target
    int px = game->player->x_pos;                         // Player x
    int py = game->player->y_pos;                         // Player y
    int tx = target->x_pos;                               // Target x
    int ty = target->y_pos;                               // Target y

    int dx = tx - px; // Distance on X                    // Horizontal distance
    int dy = ty - py; // Distance on Y                    // Vertical distance

    // 4. If we are on the target, stop.
    if (dx == 0 && dy == 0) {                             // Overlapping the target
        game->player->dir = DIR_NONE;                     // Stop moving
        return;                                           // No further decisions
    }

    // 5. Determine primary and secondary moves
    Direction primary_dir = DIR_NONE;                     // Preferred direction
    Direction secondary_dir = DIR_NONE;                   // Backup direction

    if (abs(dx) > abs(dy)) {                              // If x distance is larger
        // X-axis is the primary move
        primary_dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;    // Move right or left
        if (dy != 0) {                                    // If there is y difference too
            secondary_dir = (dy > 0) ? DIR_DOWN : DIR_UP; // Use y as backup axis
        }
    } else {
        // Y-axis is the primary move
        primary_dir = (dy > 0) ? DIR_DOWN : DIR_UP;       // Move down or up
        if (dx != 0) {                                    // If x distance is not zero
            secondary_dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT; // x axis as backup
        }
    }

    // 6. Check if primary move is valid
    int nx = px, ny = py;                                 // Start from current position
    if (primary_dir == DIR_UP) ny--;                      // Try moving up
    if (primary_dir == DIR_DOWN) ny++;                    // Try moving down
    if (primary_dir == DIR_LEFT) nx--;                    // Try moving left
    if (primary_dir == DIR_RIGHT) nx++;                   // Try moving right

    if (primary_dir != DIR_NONE &&                        // If we have a primary direction
        is_safe_move(game, nx, ny)) {                     // And tile is safe
        game->player->dir = primary_dir;                  // Commit to that direction
        return; // Valid move found                       // Exit AI
    }

    // 7. Primary move was blocked, try secondary move
    nx = px; // Reset test coordinates                    // Reset test x
    ny = py;                                              // Reset test y
    if (secondary_dir == DIR_UP) ny--;                    // Try up as secondary
    if (secondary_dir == DIR_DOWN) ny++;                  // Try down as secondary
    if (secondary_dir == DIR_LEFT) nx--;                  // Try left as secondary
    if (secondary_dir == DIR_RIGHT) nx++;                 // Try right as secondary

    if (secondary_dir != DIR_NONE &&                      // If a secondary direction exists
        is_safe_move(game, nx, ny)) {                     // And is safe
        game->player->dir = secondary_dir;                // Use it
        return;                                           // Exit
    }

    // 8. Both moves are blocked (or invalid), AI is stuck
    game->player->dir = DIR_NONE;                         // Don't move anywhere
}

// Compares two scores for qsort (descending order)
int compare_scores(const void *a, const void *b) {        // Comparison function for qsort
    ScoreEntry *scoreA = (ScoreEntry *)a;                 // Cast first pointer to ScoreEntry
    ScoreEntry *scoreB = (ScoreEntry *)b;                 // Cast second pointer to ScoreEntry
    // Sort descending (higher score comes first)
    return scoreB->score - scoreA->score;                 // Negative if B < A, positive if B > A
}

// Loads scores from file into the ScoreEntry array
void load_leaderboard(ScoreEntry scores[], int *count) {  // Load up to MAX_SCORES scores from file
    FILE *file = fopen(LEADERBOARD_FILE, "r");            // Open leaderboard file for reading
    if (file == NULL) {                                   // If file can't be opened
        *count = 0; // File doesn't exist yet             // No scores available
        return;                                           // Exit function
    }

    *count = 0;                                           // Reset count to 0
    // Read up to MAX_SCORES entries from the file
    while (*count < MAX_SCORES &&                         // Ensure we don't exceed array
           fscanf(file, "%49s %d",                        // Read name and score
                  scores[*count].name,                    // Name field
                  &scores[*count].score) == 2) {          // Score field
        (*count)++;                                       // Increment count for each valid entry
    }
    fclose(file);                                         // Close file
}

// Saves the top scores to the file
void save_leaderboard(ScoreEntry scores[], int count) {   // Save 'count' scores to leaderboard file
    FILE *file = fopen(LEADERBOARD_FILE, "w");            // Open file for writing (overwrite)
    if (file == NULL) {                                   // If file can't be opened
        // Can't save, just return
        return;                                           // Give up quietly
    }

    for (int i = 0; i < count; i++) {                     // Loop through all scores
        fprintf(file, "%s %d\n",                          // Write "name score" per line
                scores[i].name, scores[i].score);         // Name then score
    }
    fclose(file);                                         // Close the file
}

void display_leaderboard_screen(Game *game) {             // Show leaderboard screen to player
    ScoreEntry scores[MAX_SCORES];                        // Local array to hold scores
    int count = 0;                                        // Number of scores read
    
    // Load the scores from the file
    load_leaderboard(scores, &count);                     // Fill scores[] and count

    clear();                                              // Clear screen
    mvprintw(5, 10, "--- TOP RESCUERS ---");              // Print leaderboard title
    
    if (count == 0) {                                     // No scores recorded
        mvprintw(7, 10, "No scores recorded yet.");       // Inform player
    } else {
        // Display the scores
        for (int i = 0; i < count; i++) {                 // Loop over stored scores
            mvprintw(7 + i, 10, "%2d. %-20s %d",          // Print index, name, and score
                i + 1,                                    // Human-readable rank (1-based)
                scores[i].name,                           // Player name
                scores[i].score                           // Score value
            );
        }
    }

    mvprintw(7 + count + 2, 10, "Press any key to return to the menu."); // Prompt to leave leaderboard
    refresh();                                            // Show leaderboard

    nodelay(stdscr, FALSE);                               // Wait for user input
    getch();                                              // Block until a key is pressed
    nodelay(stdscr, TRUE);                                // Switch back to non-blocking mode

    game->state = STATE_MENU;                             // Return to main menu after viewing leaderboard
}

void process_new_high_score(Game *game) {                 // Add current score to leaderboard if > 0
    if (game->score <= 0) {                               // If score is zero or negative
        return;                                           // Skip leaderboard processing
    }

    // Allocate space for MAX_SCORES + 1 (size 11)
    ScoreEntry scores[MAX_SCORES + 1];                    // Temporary array with extra slot
    int count = 0;                                        // Number of existing scores

    // 1. Load existing scores
    load_leaderboard(scores, &count);                     // Load from file

    // 2. Sanitize Name & Add current player to the end
    for(int i = 0; game->player->name[i]; i++) {          // Loop through player's name string
        if(game->player->name[i] == ' ')                  // If any space found
            game->player->name[i] = '_';                  // Replace with underscore (no spaces we store)
    }
    
    snprintf(scores[count].name,                          // Copy name into new entry slot
             sizeof(scores[0].name), "%s", game->player->name);
    scores[count].score = game->score;                    // Set score for new entry
    count++;                                              // Increment total count (existing + new)

    // 3. Sort the entire array (including the new player)
    // CRITICAL FIX: Pass sizeof(ScoreEntry) as the element size
    qsort(scores, count, sizeof(ScoreEntry), compare_scores); // Sort scores array descending

    // 4. Cap the count at MAX_SCORES
    if (count > MAX_SCORES) {                             // If more than allowed
        count = MAX_SCORES;                               // Keep only top MAX_SCORES
    }

    // 5. Save the file
    save_leaderboard(scores, count);                      // Write updated scores back to file
}

// Medium radial overlay: max 5-cell radius
void draw_explosion_overlay(int cx, int cy, int frame,    // Draw one frame of explosion visual effect
                            int max_frames, int color_pair) {
    int radius = frame;                                   // Explosion radius grows per frame
    if (radius > 5) radius = 5;                           // Clamp radius to maximum of 5
    const char *chars = " .:*!@#";                        // Characters representing intensity levels

    for (int dy = -radius; dy <= radius; dy++) {          // Loop vertical offset within radius
        for (int dx = -radius; dx <= radius; dx++) {      // Loop horizontal offset within radius
            int px = cx + dx;                             // Pixel x position relative to center
            int py = cy + dy;                             // Pixel y position relative to center
            // Arena bounds: interior only (left=1, right=MAX_X-2, top=6, bottom=24)
            if (px < 1 || px > MAX_X - 2 ||               // Outside horizontal visible area
                py < 6 || py > 24) continue;              // Outside vertical visible area

            double dist = sqrt(dx * dx + dy * dy);        // Euclidean distance from center
            if (dist > 0 && dist <= radius) {             // If point is inside ring area
                int intensity = (int)((1.0 - dist / radius) * 5); // Map distance to intensity index 0-5
                attron(COLOR_PAIR(color_pair) | A_BOLD);  // Turn on explosion color and bold
                if (frame >= max_frames / 2)              // If animation past halfway
                    attron(A_BLINK);                      // Add blinking effect
                mvaddch(py, px, chars[intensity]);        // Draw chosen intensity character
                attroff(COLOR_PAIR(color_pair) | A_BOLD | A_BLINK); // Turn off all attributes
            }
        }
    }
}

// Medium animation: 8 frames at 60ms (~480ms total)
void explosion_animation(Game *game, int ex, int ey,      // Animate an explosion effect at (ex, ey)
                         int max_frames, int color_pair) {
    beep();                                               // Trigger terminal beep sound
    flash();                                              // Trigger terminal screen flash

    for (int f = 1; f <= max_frames; f++) {               // Loop through each animation frame
        draw_game(game);                                  // Draw current game scene behind explosion
        draw_explosion_overlay(ex, ey, f, max_frames,     // Overlay explosion frame at center (ex, ey)
                               color_pair);
        refresh();                                        // Show frame
        napms(60);                                        // Delay ~60ms between frames
    }
}

void spawn_powerups_in_arena(Game *game) {                // Spawn powerups based on level
    // Deactivate existing
    for (int i = 0; i < game->max_powerups; i++) {        // Loop through all allocated powerup slots
        game->powerups[i].active = 0;                     // Mark them all inactive
    }
    
    // Level-based count (1-3)
    game->current_powerups = (game->player->level / 3) + 1; // Compute number of powerups based on level
    if (game->current_powerups > 3) game->current_powerups = 3; // Clamp to at most 3
    
    for (int i = 0; i < game->current_powerups; i++) {    // Loop through each powerup to spawn
        int x, y;                                         // Temporary coords
        do {                                              // Try random positions until free
            x = game->arena->left + 1 + (rand() % (game->arena->width - 2)); // Random x
            y = game->arena->top + 1 + (rand() % (game->arena->height - 2)); // Random y
        } while (is_occupied(game, x, y,                  // Ensure tile is not occupied
                             game->num_people,            // Check all people
                             game->current_mines,         // Check existing mines
                             i));                         // Check previously placed powerups
        
        game->powerups[i].type = (PowerUpType)(rand() % 4 + 1);  // Yields 1–4 (SHIELD/SPEED/DETONATE/LIFE)
        switch (game->powerups[i].type) {                // Choose icon based on type
            case PU_SHIELD:   game->powerups[i].icon = 'S'; break; // Shield powerup
            case PU_SPEED:    game->powerups[i].icon = 'B'; break; // Boost (speed) powerup
            case PU_DETONATE: game->powerups[i].icon = 'D'; break; // Detonate all mines
            case PU_LIFE:     game->powerups[i].icon = 'L'; break; // Extra life
            default:          game->powerups[i].icon = '?';        // Fallback icon (shouldn't occur)
        }
        game->powerups[i].x_pos = x;                    // Store chosen x coordinate
        game->powerups[i].y_pos = y;                    // Store chosen y coordinate
        game->powerups[i].active = 1;                   // Mark this powerup as active in the arena
    }
}

void check_player_powerup(Game *game) {                 // Check if player picked up a powerup
    for (int i = 0; i < game->current_powerups; i++) {  // Loop through active powerups
        if (game->powerups[i].active &&                 // If this powerup is active
            game->player->x_pos == game->powerups[i].x_pos && // Player shares x
            game->player->y_pos == game->powerups[i].y_pos) { // And shares y
            
            game->powerups[i].active = 0;               // Remove powerup from arena
            
            switch (game->powerups[i].type) {           // Apply effect based on type
                case PU_SHIELD:
                    game->player->shield_timer = 50;    // Give the player shield for 50 frames
                    break;
                case PU_SPEED:
                    game->player->speed_timer = 30;     // Give speed boost for 30 frames
                    break;
                case PU_DETONATE:
                    for (int m = 0; m < game->current_mines; m++) { // Loop all mines
                        game->mines[m].active = 0;      // Deactivate all mines
                    }
                    game->score += 200;                 // Reward player for detonating all mines
                    break;
                case PU_LIFE:  // New
                    if (game->player->lives < 10) {     // Only grant life if under cap of 10
                        game->player->lives++;          // Add one life
                        game->score += 150;             // Give 150 bonus points
                    }
                    break;
            }
            // Visual feedback
            explosion_animation(game,                    // Play small explosion animation
                                game->powerups[i].x_pos,
                                game->powerups[i].y_pos,
                                5,                       // Number of frames
                                6);                      // Use color pair 6 (magenta)
            return;  // One per frame                     // Only process one powerup per frame
        }
    }
}
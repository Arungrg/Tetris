#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pdcurses.h>

// Box dimensions
const int BOX_WIDTH = 10;
const int BOX_HEIGHT = 20;

int currentX; // Current X position of the tetromino
int currentY; // Current Y position of the tetromino

int **box;              // 2D array representing the game box
int **currentTetromino; // 2D array representing the current tetromino
int **nextTetromino;    // 2D array representing the next tetromino

int gameOver = 0; // Flag to indicate if the game is over
int score = 0;    // Player's score

// Tetromino shapes
const int shapes[7][4][4] = {
    {{0, 0, 0, 0},
     {1, 1, 1, 1},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {0, 1, 1, 0},
     {0, 1, 1, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {0, 1, 1, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {1, 1, 0, 0},
     {0, 1, 1, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {0, 1, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {1, 0, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0}},

    {{0, 0, 0, 0},
     {1, 1, 1, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0}}};

// Function to generate a random tetromino
int **generateTetromino()
{
    int **tetromino = malloc(4 * sizeof(int *));
    for (int i = 0; i < 4; i++)
    {
        tetromino[i] = malloc(4 * sizeof(int));
    }

    int shapeIndex = rand() % 7;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tetromino[i][j] = shapes[shapeIndex][i][j];
        }
    }

    return tetromino;
}

// Function to free memory allocated for a tetromino
void freeTetromino(int **tetromino)
{
    for (int i = 0; i < 4; i++)
    {
        free(tetromino[i]);
    }
    free(tetromino);
}

void printTetromino(int **tetromino)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (tetromino[i][j])
            {
                printf("# ");
            }
            else
            {
                printf("  ");
            }
        }
        printf("\n");
    }
}

void printBox()
{
    printf("+");
    for (int i = 0; i < BOX_WIDTH; i++)
    {
        printf("--");
    }
    printf("+\n");

    for (int i = 0; i < BOX_HEIGHT; i++)
    {
        printf("|");
        for (int j = 0; j < BOX_WIDTH; j++)
        {
            if (box[i][j])
            {
                printf("# ");
            }
            else
            {
                printf("  ");
            }
        }
        printf("|\n");
    }

    printf("+");
    for (int i = 0; i < BOX_WIDTH; i++)
    {
        printf("--");
    }
    printf("+\n");
}

void initializeBox()
{
    box = malloc(BOX_HEIGHT * sizeof(int *));
    for (int i = 0; i < BOX_HEIGHT; i++)
    {
        box[i] = calloc(BOX_WIDTH, sizeof(int));
    }
}

void removeCompletedRows()
{
    int rowsToRemove[4] = {-1, -1, -1, -1}; // Array to store rows to be removed
    int numRowsToRemove = 0;                // Number of rows to be removed

    for (int i = 0; i < 4; i++)
    {
        int row = currentY + i;
        if (row < BOX_HEIGHT)
        {
            int isRowComplete = 1;
            for (int j = 0; j < BOX_WIDTH; j++)
            {
                if (box[row][j] == 0)
                {
                    isRowComplete = 0;
                    break;
                }
            }

            if (isRowComplete)
            {
                rowsToRemove[numRowsToRemove] = row;
                numRowsToRemove++;
            }
        }
    }

    for (int i = 0; i < numRowsToRemove; i++)
    {
        int rowToRemove = rowsToRemove[i];
        for (int j = rowToRemove; j > 0; j--)
        {
            for (int k = 0; k < BOX_WIDTH; k++)
            {
                box[j][k] = box[j - 1][k];
            }
        }
    }
}

int isValidMove(int **tetromino, int newX, int newY)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (tetromino[i][j])
            {
                int x = newX + j;
                int y = newY + i;

                if (x < 0 || x >= BOX_WIDTH || y >= BOX_HEIGHT || (y >= 0 && box[y][x]))
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

int isTetrominoAtTop()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (currentTetromino[i][j])
            {
                int y = currentY + i;
                if (y < 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int **rotateTetromino(int **tetromino)
{
    int **rotatedTetromino = malloc(4 * sizeof(int *));
    for (int i = 0; i < 4; i++)
    {
        rotatedTetromino[i] = malloc(4 * sizeof(int));
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            rotatedTetromino[i][j] = tetromino[j][3 - i];
        }
    }

    return rotatedTetromino;
}

void placeTetromino()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (currentTetromino[i][j])
            {
                int x = currentX + j;
                int y = currentY + i;

                if (y >= 0)
                {
                    box[y][x] = 1;
                }
            }
        }
    }
}

void clearScreen()
{
    system("clear || cls");
}

void updateGame()
{
    clearScreen();

    // Check if the tetromino can move down
    if (isValidMove(currentTetromino, currentX, currentY + 1))
    {
        currentY++;
    }
    else
    {
        placeTetromino();
        removeCompletedRows();

        // Check if game over
        if (isTetrominoAtTop())
        {
            gameOver = 1;
        }
        else
        {
            // Generate new tetromino
            freeTetromino(currentTetromino);
            currentTetromino = nextTetromino;
            nextTetromino = generateTetromino();
            currentX = BOX_WIDTH / 2 - 2;
            currentY = 0;
        }
    }

    // Print game state
    printf("Score: %d\n", score);
    printf("Next Tetromino:\n");
    printTetromino(nextTetromino);
    printf("Game Box:\n");
    printBox();
}

int main()
{
    srand(time(NULL));

    initializeBox();
    currentTetromino = generateTetromino();
    nextTetromino = generateTetromino();
    currentX = BOX_WIDTH / 2 - 2;
    currentY = 0;

    while (!gameOver)
    {
        updateGame();

        // Add a delay of 200,000 microseconds (0.2 seconds)
        usleep(200000);

        // Check for key press
        if (_kbhit())
        {
            // Get the character input
            char input = getchar();

            // Move the tetromino based on the input
            switch (input)
            {
            case 'a': // Move left
                if (isValidMove(currentTetromino, currentX - 1, currentY))
                {
                    currentX--;
                }
                break;
            case 'd': // Move right
                if (isValidMove(currentTetromino, currentX + 1, currentY))
                {
                    currentX++;
                }
                break;
            case 's': // Move down
                if (isValidMove(currentTetromino, currentX, currentY + 1))
                {
                    currentY++;
                }
                break;
            case 'w': // Rotate
            {
                int **rotatedTetromino = rotateTetromino(currentTetromino);
                if (isValidMove(rotatedTetromino, currentX, currentY))
                {
                    freeTetromino(currentTetromino);
                    currentTetromino = rotatedTetromino;
                }
                else
                {
                    freeTetromino(rotatedTetromino);
                }
            }
            break;
            case 'q': // Quit game
                gameOver = 1;
                break;
            }
        }
    }

    // Game over
    clearScreen();
    printf("Game Over!\n");
    printf("Final Score: %d\n", score);

    // Free allocated memory
    freeTetromino(currentTetromino);
    freeTetromino(nextTetromino);
    for (int i = 0; i < BOX_HEIGHT; i++)
    {
        free(box[i]);
    }
    free(box);

    return 0;
}

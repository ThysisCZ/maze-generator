#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_WIDTH 840
#define SCREEN_HEIGHT 840
#define CELL_COUNT 100
#define CELL_SIZE 40
#define GRID_WIDTH 10
#define GRID_HEIGHT 10

typedef struct
{
    Vector2 pos;
    bool visited;
} Cell;

int stack_index = 0;
Cell stack[CELL_COUNT];
bool walls_broken[CELL_COUNT][4];

void create_cells(Cell *cells)
{
    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % GRID_WIDTH;
        int row = i / GRID_HEIGHT;
        float pos_x = CELL_SIZE + col * 2 * CELL_SIZE;
        float pos_y = CELL_SIZE + row * 2 * CELL_SIZE;

        cells[i].pos = (Vector2){pos_x, pos_y};
        cells[i].visited = false;

        // Initialize wall tracking
        for (int j = 0; j < 4; j++)
        {
            walls_broken[i][j] = false;
        }
    }
}

void draw_maze(Cell *cells)
{
    // Draw all cells
    for (int i = 0; i < CELL_COUNT; i++)
    {
        DrawRectangle(cells[i].pos.x, cells[i].pos.y, CELL_SIZE, CELL_SIZE, WHITE);
    }

    // Redraw broken walls
    for (int i = 0; i < CELL_COUNT; i++)
    {
        float start_pos_x = cells[i].pos.x;
        float start_pos_y = cells[i].pos.y;

        // Check next column position
        if (walls_broken[i][0])
        {
            DrawRectangle(start_pos_x + CELL_SIZE, start_pos_y, CELL_SIZE, CELL_SIZE, WHITE);
        }

        // Check next row position
        if (walls_broken[i][1])
        {
            DrawRectangle(start_pos_x, start_pos_y + CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
        }
    }
}

void break_wall(Cell *cells, int current_index)
{
    stack[stack_index++] = cells[current_index];
    cells[current_index].visited = true;

    int col = current_index % GRID_WIDTH;
    int row = current_index / GRID_HEIGHT;

    int neighbors[4] = {-1, -1, -1, -1};
    int directions[4] = {-1, -1, -1, -1};
    int neighbor_count = 0;

    // Check neighboring cells
    if (col < GRID_WIDTH - 1)
    {
        neighbors[neighbor_count] = current_index + 1;
        directions[neighbor_count] = 0; // right
        neighbor_count++;
    }

    if (row < GRID_HEIGHT - 1)
    {
        neighbors[neighbor_count] = current_index + GRID_WIDTH;
        directions[neighbor_count] = 1; // bottom
        neighbor_count++;
    }

    if (col > 0)
    {
        neighbors[neighbor_count] = current_index - 1;
        directions[neighbor_count] = 2; // left
        neighbor_count++;
    }

    if (row > 0)
    {
        neighbors[neighbor_count] = current_index - GRID_WIDTH;
        directions[neighbor_count] = 3; // top
        neighbor_count++;
    }

    // Try each unvisited neighbor
    while (neighbor_count > 0)
    {
        int random_nb = GetRandomValue(0, neighbor_count - 1);
        int next_index = neighbors[random_nb];
        int direction = directions[random_nb];

        if (next_index >= 0 && !cells[next_index].visited)
        {
            // Mark the wall as broken
            walls_broken[current_index][direction] = true;

            int opposite_direction = (direction + 2) % 4;
            walls_broken[next_index][opposite_direction] = true;

            break_wall(cells, next_index);
        }
        else
        {
            neighbor_count--;

            // Remove this neighbor from the list and try again
            neighbors[random_nb] = neighbors[neighbor_count];
            directions[random_nb] = directions[neighbor_count];
        }
    }

    // Backtrack after dead end
    stack_index--;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maze Generator");

    SetTargetFPS(60);

    Cell cells[CELL_COUNT];

    create_cells(&cells[0]);

    int start_index = GetRandomValue(0, CELL_COUNT - 1);
    break_wall(&cells[0], start_index);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_maze(&cells[0]);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
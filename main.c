#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define WIDTH 1050
#define HEIGHT 950
#define CELL_COUNT 100

typedef struct
{
    Vector2 pos;
} Cell;

void create_cells(Cell *cells, int start_cell)
{
    int cell_size = 50;
    float pos_x = 50;
    float pos_y = 50;

    for (int i = 1; i <= CELL_COUNT; i++)
    {
        if (i > 1)
        {
            pos_x += 2 * cell_size;
        }

        cells[i].pos = (Vector2){pos_x, pos_y};

        Color color;

        if (i == start_cell)
        {
            color = RED;
        }
        else
        {
            color = WHITE;
        }

        DrawRectangle(pos_x, pos_y, cell_size, cell_size, color);

        if (i % (CELL_COUNT / 10) == 0)
        {
            pos_x = -cell_size;
            pos_y += 100;
        }
    }
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Maze Generator");

    SetTargetFPS(60);

    Cell cells[CELL_COUNT];

    int start_cell = GetRandomValue(1, CELL_COUNT);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        create_cells(&cells[0], start_cell);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
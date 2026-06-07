#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define WIDTH 840
#define HEIGHT 840
#define CELL_COUNT 100

typedef struct
{
    Vector2 pos;
    bool visited;
} Cell;

void create_cells(Cell *cells)
{
    int cell_size = 40;
    float pos_x = 40;
    float pos_y = 40;

    for (int i = 0; i <= CELL_COUNT; i++)
    {
        if (i > 0)
        {
            pos_x += 2 * cell_size;
        }

        cells[i].pos = (Vector2){pos_x, pos_y};
        cells[i].visited = false;

        DrawRectangle(pos_x, pos_y, cell_size, cell_size, WHITE);

        if (i > 0 && i % (CELL_COUNT / 10) == 0)
        {
            pos_x = -cell_size;
            pos_y += 80;
        }
    }
}

void break_wall(Cell *cells, int start_index, int random_nb, bool breaked)
{
    int stack_index = 0;
    Cell stack[CELL_COUNT];

    stack[stack_index] = cells[start_index];
    cells[start_index].visited = true;

    int cell_size = 40;

    float start_pos_x = stack[0].pos.x;
    float start_pos_y = stack[0].pos.y;

    float nb_pos_x = 0;
    float nb_pos_y = 0;
    int wall_pos_x = 0;
    int wall_pos_y = 0;

    // Select a neighbouring cell
    switch (random_nb)
    {
    case 1:
        nb_pos_x = start_pos_x + 2 * cell_size;
        nb_pos_y = start_pos_y;
        wall_pos_x = -cell_size;
        break;
    case 2:
        nb_pos_x = start_pos_x - 2 * cell_size;
        nb_pos_y = start_pos_y;
        wall_pos_x = cell_size;
        break;
    case 3:
        nb_pos_x = start_pos_x;
        nb_pos_y = start_pos_y + 2 * cell_size;
        wall_pos_y = -cell_size;
        break;
    case 4:
        nb_pos_x = start_pos_x;
        nb_pos_y = start_pos_y - 2 * cell_size;
        wall_pos_y = cell_size;
        break;
    }

    for (int i = 0; !breaked; i++)
    {
        if (nb_pos_x <= WIDTH - cell_size && nb_pos_y <= HEIGHT - cell_size)
        {
            float cur_pos_x = cells[i].pos.x;
            float cur_pos_y = cells[i].pos.y;

            // Remove the wall between cells
            if (cur_pos_x == nb_pos_x && cur_pos_y == nb_pos_y)
            {
                DrawRectangle(nb_pos_x + wall_pos_x, nb_pos_y + wall_pos_y, cell_size, cell_size, WHITE);

                stack[stack_index] = cells[i];
                cells[i].visited = true;
                stack_index++;

                breaked = true;
            }
        }
        else
        {
            break;
        }
    }
}

int main()
{
    InitWindow(WIDTH, HEIGHT, "Maze Generator");

    SetTargetFPS(60);

    Cell cells[CELL_COUNT];

    int start_index = GetRandomValue(1, CELL_COUNT);

    int min_nb = 1;
    int max_nb = 4;
    int random_nb = GetRandomValue(min_nb, max_nb);
    bool wall_breaked = false;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        create_cells(&cells[0]);

        break_wall(&cells[0], start_index, random_nb, wall_breaked);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
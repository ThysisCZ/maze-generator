#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_WIDTH 840
#define SCREEN_HEIGHT 840
#define CELL_COUNT 100
#define CELL_SIZE 40
#define GRID_WIDTH 10
#define GRID_HEIGHT 10
#define PLAYER_RADIUS 15
#define PLAYER_SPEED 3
#define GAME_MAP_WIDTH 19
#define GAME_MAP_HEIGHT 19

typedef struct
{
    Vector2 pos;
    bool visited;
} Cell;

typedef struct
{
    Vector2 pos;
} Player;

typedef struct
{
    Vector2 pos;
} Space;

Cell cells[CELL_COUNT];
int stack_index = 0;
Cell stack[CELL_COUNT];
bool walls_broken[CELL_COUNT][4];
Space spaces[GAME_MAP_WIDTH * GAME_MAP_HEIGHT];
int wall_index = CELL_COUNT - 1;

void draw_player(float pos_x, float pos_y)
{
    DrawCircle(pos_x, pos_y, PLAYER_RADIUS, BLUE);
}

static bool point_in_spaces(float px, float py, Space *spaces, int max_index)
{
    for (int i = 0; i <= max_index; i++)
    {
        float sx = spaces[i].pos.x;
        float sy = spaces[i].pos.y;

        if (px >= sx && px <= sx + CELL_SIZE && py >= sy && py <= sy + CELL_SIZE)
        {
            return true;
        }
    }

    return false;
}

static bool player_in_spaces(Player *player, Space *spaces, int max_index)
{
    float cx = player->pos.x;
    float cy = player->pos.y;
    int point_count = 8;

    for (int i = 0; i < point_count; i++)
    {
        float angle = i * (2 * PI / point_count);
        float px = cx + PLAYER_RADIUS * cosf(angle);
        float py = cy + PLAYER_RADIUS * sinf(angle);

        if (!point_in_spaces(px, py, spaces, max_index))
        {
            return false;
        }
    }

    return true;
}

static bool point_in_finish(float px, float py, Space *spaces)
{
    int last_index = 99;
    float fx = spaces[last_index].pos.x;
    float fy = spaces[last_index].pos.y;

    if (px >= fx && px <= fx + CELL_SIZE && py >= fy && py <= fy + CELL_SIZE)
    {
        return true;
    }

    return false;
}

static bool player_in_finish(Player *player, Space *spaces)
{
    float cx = player->pos.x;
    float cy = player->pos.y;
    int point_count = 8;

    float angle = 2 * PI / point_count;
    float px = cx + PLAYER_RADIUS * cosf(angle);
    float py = cy + PLAYER_RADIUS * sinf(angle);

    if (!point_in_finish(px, py, spaces))
    {
        return false;
    }

    return true;
}

void create_cells(Cell *cells, Space *spaces)
{
    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % GRID_WIDTH;
        int row = i / GRID_WIDTH;
        float pos_x = CELL_SIZE + col * 2 * CELL_SIZE;
        float pos_y = CELL_SIZE + row * 2 * CELL_SIZE;
        int dir_count = 4;

        cells[i].pos = (Vector2){pos_x, pos_y};
        cells[i].visited = false;

        spaces[i].pos = (Vector2){pos_x, pos_y};

        // Initialize wall tracking
        for (int j = 0; j < dir_count; j++)
        {
            walls_broken[i][j] = false;
        }
    }
}

void break_wall(Cell *cells, int current_index, Space *spaces)
{
    stack[stack_index++] = cells[current_index];
    cells[current_index].visited = true;

    int col = current_index % GRID_WIDTH;
    int row = current_index / GRID_WIDTH;

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

            float pos_x = 0;
            float pos_y = 0;
            wall_index++;

            // Get broken wall position based on direction
            switch (direction)
            {
            case 0: // right
                pos_x = cells[current_index].pos.x + CELL_SIZE;
                pos_y = cells[current_index].pos.y;
                break;
            case 1: // bottom
                pos_x = cells[current_index].pos.x;
                pos_y = cells[current_index].pos.y + CELL_SIZE;
                break;
            case 2: // left
                pos_x = cells[current_index].pos.x - CELL_SIZE;
                pos_y = cells[current_index].pos.y;
                break;
            case 3: // top
                pos_x = cells[current_index].pos.x;
                pos_y = cells[current_index].pos.y - CELL_SIZE;
                break;
            }

            spaces[wall_index].pos = (Vector2){pos_x, pos_y};

            break_wall(cells, next_index, spaces);
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

void draw_maze(Cell *cells, Space *spaces)
{
    // Draw all cells
    for (int i = 0; i < CELL_COUNT; i++)
    {
        // Redraw spaces
        for (int i = 1; i < GAME_MAP_WIDTH * GAME_MAP_HEIGHT; i++)
        {
            float sx = spaces[i].pos.x;
            float sy = spaces[i].pos.y;

            if (sx && sy)
            {
                DrawRectangle(sx, sy, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }

        Color color;

        switch (i)
        {
        case 0:
            color = RED;
            break;
        case CELL_COUNT - 1:
            color = GREEN;
            break;
        default:
            color = WHITE;
            break;
        }

        DrawRectangle(cells[i].pos.x, cells[i].pos.y, CELL_SIZE, CELL_SIZE, color);
    }
}

void update_player(Player *player, Space *spaces)
{
    if (IsKeyDown(KEY_RIGHT))
    {
        player->pos.x += PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_DOWN))
    {
        player->pos.y += PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_LEFT))
    {
        player->pos.x -= PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_UP))
    {
        player->pos.y -= PLAYER_SPEED;
    }

    if (!player_in_spaces(player, spaces, wall_index))
    {
        player->pos.x = 3 * CELL_SIZE / 2;
        player->pos.y = 3 * CELL_SIZE / 2;
    }

    if (player_in_finish(player, spaces))
    {
        player->pos.x = 3 * CELL_SIZE / 2;
        player->pos.y = 3 * CELL_SIZE / 2;

        stack_index = 0;
        wall_index = CELL_COUNT - 1;

        create_cells(&cells[0], &spaces[0]);

        int start_index = GetRandomValue(0, CELL_COUNT - 1);
        break_wall(&cells[0], start_index, &spaces[0]);
    }

    draw_player(player->pos.x, player->pos.y);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maze Generator");

    SetTargetFPS(60);

    create_cells(&cells[0], &spaces[0]);

    int start_index = GetRandomValue(0, CELL_COUNT - 1);
    break_wall(&cells[0], start_index, &spaces[0]);

    float pos_x = cells[0].pos.x + CELL_SIZE / 2;
    float pos_y = cells[0].pos.y + CELL_SIZE / 2;

    Vector2 start_pos = {pos_x, pos_y};

    Player player = {start_pos};

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_maze(&cells[0], &spaces[0]);
        update_player(&player, &spaces[0]);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
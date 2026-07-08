#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_WIDTH 840
#define SCREEN_HEIGHT 840
#define NEIGHBOR_COUNT 100
#define WALL_COUNT NEIGHBOR_COUNT - 1
#define CELL_COUNT NEIGHBOR_COUNT + WALL_COUNT
#define MAX_DIRECTIONS 4
#define RIGHT 0
#define BOTTOM 1
#define LEFT 2
#define TOP 3
#define MAX_POINTS 8
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
    Color color;
} Cell;

typedef struct
{
    Vector2 pos;
} Player;

typedef struct
{
    Cell cells[CELL_COUNT];
    bool walls_broken[NEIGHBOR_COUNT][MAX_DIRECTIONS];
    int wall_index;
    int start_index;
    Player player;
} Maze;

void draw_player(float pos_x, float pos_y)
{
    DrawCircle(pos_x, pos_y, PLAYER_RADIUS, BLUE);
}

static bool point_in_spaces(float px, float py, Maze *maze)
{
    for (int i = 0; i <= maze->wall_index; i++)
    {
        float sx = maze->cells[i].pos.x;
        float sy = maze->cells[i].pos.y;

        if (px >= sx && px <= sx + CELL_SIZE && py >= sy && py <= sy + CELL_SIZE)
        {
            return true;
        }
    }

    return false;
}

static bool player_in_spaces(Maze *maze)
{
    float cx = maze->player.pos.x;
    float cy = maze->player.pos.y;

    for (int i = 0; i < MAX_POINTS; i++)
    {
        float angle = i * (2 * PI / MAX_POINTS);
        float px = cx + PLAYER_RADIUS * cosf(angle);
        float py = cy + PLAYER_RADIUS * sinf(angle);

        if (!point_in_spaces(px, py, maze))
        {
            return false;
        }
    }

    return true;
}

static bool point_in_finish(float px, float py, Maze *maze)
{
    const int last_index = 99;
    float fx = maze->cells[last_index].pos.x;
    float fy = maze->cells[last_index].pos.y;

    if (px >= fx && px <= fx + CELL_SIZE && py >= fy && py <= fy + CELL_SIZE)
    {
        return true;
    }

    return false;
}

static bool player_in_finish(Maze *maze)
{
    float cx = maze->player.pos.x;
    float cy = maze->player.pos.y;

    float angle = 2 * PI / MAX_POINTS;
    float px = cx + PLAYER_RADIUS * cos(angle);
    float py = cy + PLAYER_RADIUS * sin(angle);

    if (!point_in_finish(px, py, maze))
    {
        return false;
    }

    return true;
}

void init_grid(Maze *maze)
{
    for (int i = 0; i < NEIGHBOR_COUNT; i++)
    {
        int col = i % GRID_WIDTH;
        int row = i / GRID_WIDTH;
        float pos_x = CELL_SIZE + col * 2 * CELL_SIZE;
        float pos_y = CELL_SIZE + row * 2 * CELL_SIZE;

        maze->cells[i].pos = (Vector2){pos_x, pos_y};
        maze->cells[i].visited = false;
        maze->cells[i].color = WHITE;

        // Initialize wall tracking
        for (int j = 0; j < MAX_DIRECTIONS; j++)
        {
            maze->walls_broken[i][j] = false;
        }
    }
}

static Vector2 get_wall_position(int current_index, int direction, Maze *maze)
{
    float pos_x;
    float pos_y;

    // Get broken wall position based on direction
    switch (direction)
    {
    case RIGHT:
        pos_x = maze->cells[current_index].pos.x + CELL_SIZE;
        pos_y = maze->cells[current_index].pos.y;
        break;
    case BOTTOM:
        pos_x = maze->cells[current_index].pos.x;
        pos_y = maze->cells[current_index].pos.y + CELL_SIZE;
        break;
    case LEFT:
        pos_x = maze->cells[current_index].pos.x - CELL_SIZE;
        pos_y = maze->cells[current_index].pos.y;
        break;
    case TOP:
        pos_x = maze->cells[current_index].pos.x;
        pos_y = maze->cells[current_index].pos.y - CELL_SIZE;
        break;
    }

    return (Vector2){pos_x, pos_y};
}

static int get_neighbor_index(int current_index, int direction)
{
    int neighbor_index;

    switch (direction)
    {
    case RIGHT:
        neighbor_index = current_index + 1;
        break;
    case BOTTOM:
        neighbor_index = current_index + GRID_WIDTH;
        break;
    case LEFT:
        neighbor_index = current_index - 1;
        break;
    case TOP:
        neighbor_index = current_index - GRID_WIDTH;
        break;
    }

    return neighbor_index;
}

void carve_path(Maze *maze, int current_index)
{
    maze->cells[current_index].visited = true;

    int col = current_index % GRID_WIDTH;
    int row = current_index / GRID_WIDTH;

    int neighbors[MAX_DIRECTIONS] = {-1, -1, -1, -1};
    int directions[MAX_DIRECTIONS] = {-1, -1, -1, -1};
    int neighbor_count = 0;

    // Stack unvisited neighbor positions and directions
    if (col < GRID_WIDTH - 1)
    {
        int neighbor_index = get_neighbor_index(current_index, RIGHT);

        if (!maze->cells[neighbor_index].visited)
        {
            neighbors[neighbor_count] = get_neighbor_index(current_index, RIGHT);
            directions[neighbor_count] = RIGHT;
            neighbor_count++;
        }
    }

    if (row < GRID_HEIGHT - 1)
    {
        int neighbor_index = get_neighbor_index(current_index, BOTTOM);

        if (!maze->cells[neighbor_index].visited)
        {
            neighbors[neighbor_count] = get_neighbor_index(current_index, BOTTOM);
            directions[neighbor_count] = BOTTOM;
            neighbor_count++;
        }
    }

    if (col > 0)
    {
        int neighbor_index = get_neighbor_index(current_index, LEFT);

        if (!maze->cells[neighbor_index].visited)
        {
            neighbors[neighbor_count] = get_neighbor_index(current_index, LEFT);
            directions[neighbor_count] = LEFT;
            neighbor_count++;
        }
    }

    if (row > 0)
    {
        int neighbor_index = get_neighbor_index(current_index, TOP);

        if (!maze->cells[neighbor_index].visited)
        {
            neighbors[neighbor_count] = get_neighbor_index(current_index, TOP);
            directions[neighbor_count] = TOP;
            neighbor_count++;
        }
    }

    // Handle all current unvisited neighbors
    while (neighbor_count > 0)
    {
        // Pick random unvisited neighbor
        int current_nb = GetRandomValue(0, neighbor_count - 1);
        int next_index = neighbors[current_nb];

        // Recursively carve from an existing unvisited neighbor
        if (!maze->cells[next_index].visited)
        {
            int direction = directions[current_nb];
            int opposite_direction = (direction + 2) % MAX_DIRECTIONS;

            // Mark the wall as broken
            maze->walls_broken[current_index][direction] = true;
            maze->walls_broken[next_index][opposite_direction] = true;
            maze->wall_index++;

            Vector2 wall_pos = get_wall_position(current_index, direction, maze);
            maze->cells[maze->wall_index].pos = wall_pos;

            carve_path(maze, next_index);
        }

        // Remove the picked neighbor from the list
        neighbor_count--;

        neighbors[current_nb] = neighbors[neighbor_count];
        directions[current_nb] = directions[neighbor_count];
    }
}

void draw_maze(Maze *maze)
{
    // Draw all neighboring cells
    for (int i = 0; i < NEIGHBOR_COUNT; i++)
    {
        switch (i)
        {
        case 0:
            maze->cells[i].color = RED;
            break;
        case NEIGHBOR_COUNT - 1:
            maze->cells[i].color = GREEN;
            break;
        default:
            maze->cells[i].color = WHITE;
            break;
        }

        DrawRectangle(maze->cells[i].pos.x, maze->cells[i].pos.y, CELL_SIZE, CELL_SIZE, maze->cells[i].color);
    }

    // Redraw spaces
    for (int i = 0; i < NEIGHBOR_COUNT; i++)
    {
        for (int j = 0; j < MAX_DIRECTIONS; j++)
        {
            Vector2 wall_pos = get_wall_position(i, j, maze);

            if (maze->walls_broken[i][j])
            {
                DrawRectangle(wall_pos.x, wall_pos.y, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

void update_player(Maze *maze)
{
    if (IsKeyDown(KEY_RIGHT))
    {
        maze->player.pos.x += PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_DOWN))
    {
        maze->player.pos.y += PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_LEFT))
    {
        maze->player.pos.x -= PLAYER_SPEED;
    }

    if (IsKeyDown(KEY_UP))
    {
        maze->player.pos.y -= PLAYER_SPEED;
    }

    if (!player_in_spaces(maze))
    {
        maze->player.pos.x = 3 * CELL_SIZE / 2;
        maze->player.pos.y = 3 * CELL_SIZE / 2;
    }

    if (player_in_finish(maze))
    {
        maze->player.pos.x = 3 * CELL_SIZE / 2;
        maze->player.pos.y = 3 * CELL_SIZE / 2;
        maze->wall_index = WALL_COUNT;

        init_grid(maze);
        carve_path(maze, maze->start_index);
    }

    draw_player(maze->player.pos.x, maze->player.pos.y);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maze Generator");

    SetTargetFPS(60);

    Maze maze;

    maze.wall_index = WALL_COUNT;
    maze.start_index = 0;

    init_grid(&maze);
    carve_path(&maze, maze.start_index);

    float pos_x = maze.cells[0].pos.x + CELL_SIZE / 2;
    float pos_y = maze.cells[0].pos.y + CELL_SIZE / 2;

    Vector2 start_pos = {pos_x, pos_y};

    maze.player.pos = start_pos;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_maze(&maze);
        update_player(&maze);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
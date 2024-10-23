#include <raylib.h>
#include <math.h>
#include "raymath.h"

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define TILE_SIZE 20
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define FOV 90.0f
#define RAY_COUNT 120
#define FOVCHANGE (tan(FOV / 2))

int map[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

typedef struct {
    Vector2 position;
    float angle;
} Player;

void CastRay(Player player, float rayAngle, int xpos) {
    rayAngle = fmodf(rayAngle, 2 * PI);  // Normalize angle

    float dirX = cos(rayAngle), dirY = sin(rayAngle);

    // Calculate initial values
    int mapX = (int)(player.position.x / TILE_SIZE);
    int mapY = (int)(player.position.y / TILE_SIZE);

    float rayLengthX = (dirX == 0) ? 10000 : fabs(1 / dirX);
    float rayLengthY = (dirY == 0) ? 10000 : fabs(1 / dirY);

    int stepX = (dirX < 0) ? -1 : 1;
    int stepY = (dirY < 0) ? -1 : 1;

    float startDistX = fabs(mapX + (stepX < 0 ? 0 : 1) - player.position.x / TILE_SIZE ) * rayLengthX;
    float startDistY = fabs(mapY + (stepY < 0 ? 0 : 1) - player.position.y / TILE_SIZE ) * rayLengthY;
    float lineThickness = (float)SCREEN_WIDTH / RAY_COUNT;

    // Perform DDA
    int hit = 0;
    int side;
    float distance = 0, maxDistance = 10;
    while (!hit && (distance) < maxDistance) {
        if (startDistX < startDistY) {
            distance = startDistX;
            startDistX += rayLengthX;
            mapX += stepX;
            side = 0;
        } else {
            distance = startDistY;
            startDistY += rayLengthY;
            mapY += stepY;
            side = 1;
        }
        if (map[mapY][mapX] > 0) hit = 1;  // Wall hit
    }

    // Draw the ray as a line
    if (hit == 1)
    {
        float wallHeight = SCREEN_HEIGHT / (distance * cos(player.angle - rayAngle) * 2 * FOVCHANGE);
        float startPos = (SCREEN_HEIGHT >> 1) - wallHeight, endPos = startPos + wallHeight * 2;
        /*DrawRectangle(xpos * lineThickness, startPos, lineThickness + 1, wallHeight * 2, (side == 1) ? WHITE : LIGHTGRAY);*/
        int textureHeight = 32;
        float step =  textureHeight/(wallHeight * 2) ;
        for(int i = 0; i < (endPos - startPos); i+=step)
        {
            DrawRectangle(xpos * lineThickness, startPos + i * step, lineThickness + 1, step, (i % 2 == 0) ? BLACK : WHITE);
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycaster with DDA");

    Player player = {{TILE_SIZE * 1.5, TILE_SIZE * 1.5}, 0}, target;
    target = player;
    SetTargetFPS(60);

    float halfRayAngle = (FOV * DEG2RAD) / 2;
    float angleStep = (FOV * DEG2RAD) / RAY_COUNT;

    while (!WindowShouldClose()) {
        bool isThere = Vector2Distance(player.position, target.position) < 5;
        // Player controls
        if (IsKeyPressed(KEY_Q)) target.angle -= PI / 2;
        if (IsKeyPressed(KEY_E)) target.angle += PI / 2;

        int tsin = round(sin(target.angle));
        int tcos = round(cos(target.angle));

        int tposx = (int)target.position.x / TILE_SIZE;
        int tposy = (int)target.position.y / TILE_SIZE;
        if (IsKeyDown(KEY_W) && isThere && map[tposy + tsin][tposx + tcos] == 0) {
            target.position.x += tcos * TILE_SIZE;
            target.position.y += tsin * TILE_SIZE;
        }
        if (IsKeyDown(KEY_S) && isThere && map[tposy - tsin][tposx - tcos] == 0) {
            target.position.x -= tcos * TILE_SIZE;
            target.position.y -= tsin * TILE_SIZE;
        }
        if (IsKeyDown(KEY_A) && isThere && map[tposy - tcos][tposx + tsin] == 0) {
            target.position.x += tsin * TILE_SIZE;
            target.position.y -= tcos * TILE_SIZE;
        }
        if (IsKeyDown(KEY_D) && isThere && map[tposy + tcos][tposx - tsin] == 0) {
            target.position.x -= tsin * TILE_SIZE;
            target.position.y += tcos * TILE_SIZE;
        }

        player.position = Vector2Lerp(player.position, target.position, 0.1);
        player.angle = Lerp(player.angle, target.angle, 0.1);

        // Cast rays
        float rayAngle = player.angle - halfRayAngle;

        // Render
        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangle(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, GRAY);
        for (int i = 0; i < RAY_COUNT; i++, rayAngle += angleStep)
            CastRay(player, rayAngle, i);

        // Draw the map (for reference)
        /*DrawRectangle(0, 0, TILE_SIZE * MAP_WIDTH, TILE_SIZE * MAP_HEIGHT, BLACK);*/
        /*for (int y = 0; y < MAP_HEIGHT; y++)*/
        /*    for (int x = 0; x < MAP_WIDTH; x++)*/
        /*        if (map[y][x] == 1)*/
        /*            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);*/

        // Draw the player
        /*DrawCircleV(player.position, 5, RED);*/

        EndDrawing();
    }

    CloseWindow();
    return 0;
}


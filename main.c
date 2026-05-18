#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#define SW 1920
#define SH 1080
#define MAX_RADIUS 20.0
#define MIN_RADIUS 15.0

struct ParticleSystem {
    struct Vector2 *positions;
    struct Vector2 *oldPositions;
    float *radii;
    int count;
};

float GetRandomFloat(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );
}

void InitParticleSystem(struct ParticleSystem *particleSystem, int numParticles) {
    particleSystem->count = numParticles;
    particleSystem->positions = malloc(numParticles * sizeof(Vector2));
    particleSystem->oldPositions = malloc(numParticles * sizeof(Vector2));
    particleSystem->radii = malloc(numParticles * sizeof(float));

    for(int i = 0; i < numParticles; i++) {    
        Vector2 pos = {GetRandomValue(0, SW), GetRandomValue(0, SH)};
        particleSystem->radii[i] = GetRandomFloat(MIN_RADIUS, MAX_RADIUS);
        particleSystem->positions[i] = pos;
        particleSystem->oldPositions[i] = particleSystem->positions[i];
    }
}

void DrawParticles(struct ParticleSystem *particleSystem) {
    for(int i = 0; i < particleSystem->count; i++) {
        Vector2 curPos = particleSystem->positions[i];
        DrawCircle(curPos.x, curPos.y, particleSystem->radii[i], WHITE);
    }
}

void CheckBoundingBox(Vector2 *pos, float radius) {
    if (pos->x <= radius) {
        pos->x = radius;
    }
    else if (pos->x >= SW - radius) {
        pos->x = SW - radius;
    }
    if (pos->y <= radius) {
        pos->y = radius;
    }
    else if (pos->y >= SH - radius) {
        pos->y = SH - radius;
    }
}

void UpdatePos(struct ParticleSystem *particleSystem, float dt) {
    // formula: newPos = 2 * curPos - oldPos * accel * dt**2
    Vector2 gravity = {0.0, 32.174f};
    for (int i = 0; i < particleSystem->count; i++) {
        Vector2 scaledCur = Vector2Scale(particleSystem->positions[i], 2);
        Vector2 subbed = Vector2Subtract(scaledCur, particleSystem->oldPositions[i]);
        
        Vector2 scaledAccel = Vector2Scale(gravity, dt*dt);
        Vector2 newPos = Vector2Add(subbed, scaledAccel);
        CheckBoundingBox(&newPos, particleSystem->radii[i]);

        particleSystem->oldPositions[i] = particleSystem->positions[i];
        particleSystem->positions[i] = newPos;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./sim <num particles>\n");
        return -1;
    }

    char *p;
    srand((unsigned int)time(NULL));

    struct ParticleSystem system;
    InitParticleSystem(&system, strtol(argv[1], &p, 10));

    InitWindow(SW, SH, "particle sim");
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        
        UpdatePos(&system, GetFrameTime());
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawParticles(&system);
        EndDrawing();
    }

    CloseWindow();
    free(system.positions);
    free(system.oldPositions);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"

#define SW 1920
#define SH 1080
#define PARTICLE_RADIUS 20

struct ParticleSystem {
    struct Vector2 *positions;
    struct Vector2 *oldPositions;
    int count;
};

void InitParticleSystem(struct ParticleSystem *particleSystem, int numParticles) {
    particleSystem->count = numParticles;
    particleSystem->positions = malloc(numParticles * sizeof(Vector2));
    particleSystem->oldPositions = malloc(numParticles * sizeof(Vector2));

    for(int i = 0; i < numParticles; i++) {    
        Vector2 pos = {GetRandomValue(0, SW), 0.0f};

        particleSystem->positions[i] = pos;
        particleSystem->oldPositions[i] = particleSystem->positions[i];
    }
}

void DrawParticles(struct ParticleSystem *particleSystem) {
    for(int i = 0; i < particleSystem->count; i++) {
        Vector2 curPos = particleSystem->positions[i];
        DrawCircle(curPos.x, curPos.y, PARTICLE_RADIUS, WHITE);
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

        particleSystem->oldPositions[i] = particleSystem->positions[i];
        particleSystem->positions[i] = newPos;
    }
}

int main(int argc, char* argv) {
    if (argc != 2) {
        printf("Usage: ./sim <num particles>\n");
        return -1;
    }

    struct ParticleSystem system;
    InitParticleSystem(&system, argv[1]);

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

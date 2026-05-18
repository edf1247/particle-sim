#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#define SW 1920
#define SH 1080
#define MAX_RADIUS 10.0
#define MIN_RADIUS 5.0
#define MIN_ACCEL -20.0
#define MAX_ACCEL 20.0
#define MASS 2.0;

struct ParticleSystem {
    struct Vector2 *positions;
    struct Vector2 *velocities;
    struct Vector2 *accelerations;
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
    particleSystem->velocities = malloc(numParticles * sizeof(Vector2));
    particleSystem->accelerations = malloc(numParticles * sizeof(Vector2));
    particleSystem->radii = malloc(numParticles * sizeof(float));

    for(int i = 0; i < numParticles; i++) {    
        Vector2 initPos = {GetRandomValue(0, SW), GetRandomValue(0, SH)};
        float initRadius = GetRandomFloat(MIN_RADIUS, MAX_RADIUS);

        Vector2 initVel = {0.0f, 0.0f};
        Vector2 initAccel = {GetRandomFloat(MIN_ACCEL, MAX_ACCEL), GetRandomFloat(MIN_ACCEL, MAX_ACCEL)};

        particleSystem->radii[i] = initRadius;
        particleSystem->positions[i] = initPos;
        particleSystem->velocities[i] = initVel;
        particleSystem->accelerations[i] = initAccel;
    }
}

void DrawParticles(struct ParticleSystem *particleSystem) {
    for(int i = 0; i < particleSystem->count; i++) {
        Vector2 curPos = particleSystem->positions[i];
        DrawCircle(curPos.x, curPos.y, particleSystem->radii[i], WHITE);
    }
}

void CheckBoundingBox(Vector2 *pos, Vector2 *vel, float radius) {
    if (pos->x <= radius) {
        pos->x = radius;
        vel->x = -vel->x;
    }
    else if (pos->x >= SW - radius) {
        pos->x = SW - radius;
        vel->x = -vel->x;
    }
    if (pos->y <= radius) {
        pos->y = radius;
        vel->y = -vel->y;
    }
    else if (pos->y >= SH - radius) {
        pos->y = SH - radius;
        vel->y = -vel->y;
    }
}

void CollisionDetection(struct ParticleSystem *pS, int currentParticleIndex) {
    Vector2 currentPos = pS->positions[currentParticleIndex];
    float currentRadius = pS->radii[currentParticleIndex];
    Vector2 currentVel = pS->velocities[currentParticleIndex];

    for(int i = currentParticleIndex + 1; i < pS->count; i++) {
        Vector2 otherPos = pS->positions[i];
        float otherRadius = pS->radii[i];
        Vector2 otherVel = pS->velocities[i];
        
        float dx = otherPos.x - currentPos.x;
        float dy = otherPos.y - currentPos.y;
        
        float dist2 = dx*dx + dy*dy;
        float minDist = (otherRadius + currentRadius);

        if (dist2 == 0.0f) {
            continue;
        }
        
        if(dist2 <= minDist * minDist) {
            //collision
            pS->velocities[i] = currentVel;
            currentVel = otherVel;
            
            float dist = sqrtf(dist2);
            float overlap = minDist - dist;
            float nx = dx / dist;
            float ny = dy / dist;

            pS->positions[i].x += overlap * nx * 0.1f;
            pS->positions[i].y += overlap * nx * 0.1f;
            pS->positions[currentParticleIndex].x -= overlap * nx * 0.1f;
            pS->positions[currentParticleIndex].y -= overlap * ny * 0.1f;
        }
    }

    pS->velocities[currentParticleIndex] = currentVel;
}

void UpdatePos(struct ParticleSystem *pS, float dt) {
    for (int i = 0; i < pS->count; i++) {
        Vector2 accel = Vector2Scale(pS->accelerations[i], dt*dt*0.5f);
        Vector2 newPos = Vector2Add(pS->positions[i], Vector2Scale(pS->velocities[i], dt));
        newPos = Vector2Add(newPos, accel);

        pS->positions[i] = newPos;
        pS->velocities[i] = Vector2Add(pS->velocities[i], Vector2Scale(pS->accelerations[i], dt));
        
        CollisionDetection(pS, i);
        CheckBoundingBox(&pS->positions[i], &pS->velocities[i], pS->radii[i]);
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
    free(system.velocities);
    free(system.accelerations);
    free(system.radii);

    return 0;
}

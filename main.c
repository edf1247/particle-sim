#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "raylib.h"
#include "raymath.h"

#define SW 1920
#define SH 1080
#define MAX_RADIUS 10.0
#define MIN_RADIUS 5.0
#define MIN_ACCEL -30.0
#define MAX_ACCEL 30.0
#define MASS 2.0
#define NUM_BUCKETS 8192
#define MAX_PER_BUCKET 128
#define CELL_SIZE (MAX_RADIUS * 2.0f)
#define GRAVITY_MULT 4
#define RESTITUTION 0.65;
#define BLAST_RADIUS 100.0f
#define BLAST_STRENGTH 200.0f

struct Bucket {
    int particleIndicies[MAX_PER_BUCKET];
    int count;
};

struct HashTable {
    struct Bucket buckets[NUM_BUCKETS];
};

struct ParticleSystem {
    struct Vector2 *positions;
    struct Vector2 *velocities;
    struct Vector2 *accelerations;
    float *radii;
    int count;
};

int hashCell(int cx, int cy) {
    int x = (int)(cx * 92837111);
    int y = (int)(cy * 689287499);
    return abs(x ^ y) % NUM_BUCKETS;
}

int hash(Vector2 position) {
    int x = (int)(position.x / CELL_SIZE);
    int y = (int)(position.y / CELL_SIZE);
    return hashCell(x, y);
}

void CreateTable(struct ParticleSystem *ps, struct HashTable *ht) {
    for(int i = 0; i < ps->count; i++) {
        int bucket = hash(ps->positions[i]);
        int count = ht->buckets[bucket].count;
        
        if (count < MAX_PER_BUCKET) {
            ht->buckets[bucket].count++;
            ht->buckets[bucket].particleIndicies[count] = i;
        }
    }
}

void ClearBuckets(struct HashTable *ht) {
    for (int i = 0; i < NUM_BUCKETS; i++) {
        ht->buckets[i].count = 0;
        memset(ht->buckets[i].particleIndicies, 0, sizeof(ht->buckets[i].particleIndicies));
    }
}

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
        vel->y = -vel->y * RESTITUTION;
    }
}

void CollisionDetection(struct ParticleSystem *pS, struct HashTable *ht, int currentParticleIndex) {
    Vector2 currentPos = pS->positions[currentParticleIndex];
    float currentRadius = pS->radii[currentParticleIndex];
    Vector2 currentVel = pS->velocities[currentParticleIndex];
    
    int cellX = (int)(currentPos.x / CELL_SIZE);
    int cellY = (int)(currentPos.y / CELL_SIZE);

    for(int cx = cellX - 1; cx <= cellX + 1; cx++) {
        for(int cy = cellY - 1; cy <= cellY + 1; cy++) {
            struct Bucket curBucket = ht->buckets[hashCell(cx, cy)];
            
            for(int j = 0; j < curBucket.count; j++) {
                int i = curBucket.particleIndicies[j];
                
                Vector2 otherPos = pS->positions[i];
                float otherRadius = pS->radii[i];
                Vector2 otherVel = pS->velocities[i];
                
                float dx = otherPos.x - currentPos.x;
                float dy = otherPos.y - currentPos.y;
                
                float dist2 = dx*dx + dy*dy;
                float minDist = (otherRadius + currentRadius);

                if (i == currentParticleIndex || dist2 == 0.0f) {
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
                    pS->positions[i].y += overlap * ny * 0.1f;
                    pS->positions[currentParticleIndex].x -= overlap * nx * 0.1f;
                    pS->positions[currentParticleIndex].y -= overlap * ny * 0.1f;
                }
            }
        }
    }
    pS->velocities[currentParticleIndex] = currentVel;
}

void ApplyImpulse(struct ParticleSystem *pS, struct HashTable *ht) {
    if(IsMouseButtonPressed(0)) {
        Vector2 mousePos = GetMousePosition();
        int cellRange = (int)(BLAST_RADIUS / CELL_SIZE);
        int cellX = (int)(mousePos.x / CELL_SIZE);
        int cellY = (int)(mousePos.y / CELL_SIZE);

        for(int cx = cellX - cellRange; cx <= cellX + cellRange; cx++) {
            for(int cy = cellY - cellRange; cy <= cellY + cellRange; cy++) {
                struct Bucket curBucket = ht->buckets[hashCell(cx, cy)];
                for(int i = 0; i < curBucket.count; i++) {
                    int curP = curBucket.particleIndicies[i];
                    
                    Vector2 dir = Vector2Subtract(pS->positions[curP], mousePos);
                    float dist = Vector2Length(dir);

                    if(dist > 0 && dist < BLAST_RADIUS) {
                        Vector2 norm = Vector2Scale(dir, 1.0f / dist);
                        Vector2 impulse = Vector2Scale(norm, BLAST_STRENGTH);

                        pS->velocities[curP] = Vector2Add(pS->velocities[curP], impulse);
                    }
                }
            }
        }
    }
}

Vector2 ApplyForces(Vector2 accel) {
    if(IsKeyPressed(KEY_G)) {
        Vector2 newAccel = {0.0f, 9.81f * GRAVITY_MULT};
        return newAccel;
    }
    return accel;
}

void UpdatePos(struct ParticleSystem *pS, struct HashTable *ht, float dt) {
    for (int i = 0; i < pS->count; i++) {
        Vector2 oldAccel = pS->accelerations[i];
        Vector2 accelScaled = Vector2Scale(oldAccel, dt*dt*0.5f);
        Vector2 newPos = Vector2Add(pS->positions[i], Vector2Scale(pS->velocities[i], dt));
        newPos = Vector2Add(newPos, accelScaled);

        pS->positions[i] = newPos;
        pS->accelerations[i] = ApplyForces(oldAccel);
        pS->velocities[i] = Vector2Add(pS->velocities[i], Vector2Scale(Vector2Add(pS->accelerations[i], oldAccel), dt * 0.5f));
        
        CollisionDetection(pS, ht, i);
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
    
    struct HashTable ht;

    while(!WindowShouldClose()) {
        ClearBuckets(&ht);
        CreateTable(&system, &ht);
        ApplyImpulse(&system, &ht);
        UpdatePos(&system, &ht, GetFrameTime());
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawFPS(0, 0);
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

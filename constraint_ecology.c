/*
 * Constraint Ecology Simulation
 * Research-grade implementation for procedural generation experiments
 *
 * Comparison: Fixed constraints (baseline) vs Evolving constraints
 *
 * Compile with: gcc constraint_ecology.c -o constraint_ecology -lraylib -lm
 * Switch modes by changing EVOLVING_MODE (0=baseline, 1=evolving)
 */

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

#define EVOLVING_MODE 1 // 0=baseline (fixed), 1=evolving (adaptive)raints

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TARGET_FPS 60

#define AGENT_COUNT 50
#define TIMESTEPS_PER_EPISODE 1000
#define EPISODES_PER_RUN 10

#define FIELD_GRID_SIZE 50
#define AGENT_RADIUS 4.0f
#define RANDOM_WALK_STRENGTH 10.0f

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Agent;

typedef struct {
  Vector2 center;
  float radius;
  float densityThreshold;
  int effectType; // 0=repulsion, 1=attraction, 2=damping
  float strength;

  // Tracking for evolution
  int activationCount;
  float totalForceApplied;
} Constraint;

typedef struct {
  float **field;
  int width;
  int height;
  float cellSize;
} Environment;

// ============================================================================
// GLOBAL STATE
// ============================================================================

Agent agents[AGENT_COUNT];
Constraint *constraints = NULL;
int constraintCount = 0;
Environment env;

int currentTimestep = 0;
int currentEpisode = 0;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void InitEnvironment(void);
void FreeEnvironment(void);
void InitAgents(void);
void InitConstraints(void);
void FreeConstraints(void);
void ResetConstraintStats(void);
void EvolveConstraints(void);
void UpdateScalarField(void);
void ApplyConstraintForces(void);
void UpdateAgents(void);
void RenderSimulation(void);

// Episode-level metrics
float ComputeSpatialVariance(void); // FIX 4: Renamed from ComputeDiversity
float ComputeStability(void);
float ComputeEntropy(void);
float ComputeCollapse(void);
void LogEpisodeMetrics(FILE *logFile, int episodeNum);

// ============================================================================
// INITIALIZATION
// ============================================================================

void InitEnvironment(void) {
  env.width = FIELD_GRID_SIZE;
  env.height = FIELD_GRID_SIZE;
  env.cellSize = (float)WINDOW_WIDTH / FIELD_GRID_SIZE;

  env.field = (float **)malloc(env.height * sizeof(float *));
  for (int i = 0; i < env.height; i++) {
    env.field[i] = (float *)malloc(env.width * sizeof(float));
    for (int j = 0; j < env.width; j++) {
      env.field[i][j] = 0.0f;
    }
  }
}

void FreeEnvironment(void) {
  for (int i = 0; i < env.height; i++) {
    free(env.field[i]);
  }
  free(env.field);
}

void InitAgents(void) {
  for (int i = 0; i < AGENT_COUNT; i++) {
    agents[i].position = (Vector2){GetRandomValue(50, WINDOW_WIDTH - 50),
                                   GetRandomValue(50, WINDOW_HEIGHT - 50)};
    agents[i].velocity = Vector2Zero();
  }
}

void InitConstraints(void) {
  // Step 3: Add constraints
  constraintCount = 3;
  constraints = (Constraint *)malloc(constraintCount * sizeof(Constraint));

  // Constraint 1: Top-left repulsion
  constraints[0].center = (Vector2){200, 150};
  constraints[0].radius = 120.0f;
  constraints[0].densityThreshold = 2.0f;
  constraints[0].effectType = 0; // Repulsion
  constraints[0].strength = 50.0f;
  constraints[0].activationCount = 0;
  constraints[0].totalForceApplied = 0.0f;

  // Constraint 2: Center repulsion
  constraints[1].center = (Vector2){WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
  constraints[1].radius = 100.0f;
  constraints[1].densityThreshold = 3.0f;
  constraints[1].effectType = 0; // Repulsion
  constraints[1].strength = 80.0f;
  constraints[1].activationCount = 0;
  constraints[1].totalForceApplied = 0.0f;

  // Constraint 3: Bottom-right repulsion
  constraints[2].center = (Vector2){600, 450};
  constraints[2].radius = 90.0f;
  constraints[2].densityThreshold = 2.5f;
  constraints[2].effectType = 0; // Repulsion
  constraints[2].strength = 60.0f;
  constraints[2].activationCount = 0;
  constraints[2].totalForceApplied = 0.0f;
}

void FreeConstraints(void) {
  if (constraints != NULL) {
    free(constraints);
    constraints = NULL;
  }
}

void ResetConstraintStats(void) {
  // FIX 2: Reset activation tracking at episode start
  for (int c = 0; c < constraintCount; c++) {
    constraints[c].activationCount = 0;
    constraints[c].totalForceApplied = 0.0f;
  }
}

void EvolveConstraints(void) {
  // FIX 3 + STEP 3: Minimal evolution logic
  const int ACTIVATION_THRESHOLD = 100; // Tune based on episode length
  const float MIN_STRENGTH = 10.0f;
  const float MAX_STRENGTH = 200.0f;
  const float MUTATION_PROBABILITY = 0.1f;

  for (int c = 0; c < constraintCount; c++) {
    // Adjust strength based on activation
    if (constraints[c].activationCount < ACTIVATION_THRESHOLD) {
      constraints[c].strength *= 0.9f; // Reduce inactive constraints
    } else {
      constraints[c].strength *= 1.05f; // Boost active constraints
    }

    // Clamp strength to valid range
    if (constraints[c].strength < MIN_STRENGTH) {
      constraints[c].strength = MIN_STRENGTH;
    }
    if (constraints[c].strength > MAX_STRENGTH) {
      constraints[c].strength = MAX_STRENGTH;
    }

    // Random radius mutation with small probability
    float randVal = (float)GetRandomValue(0, 1000) / 1000.0f;
    if (randVal < MUTATION_PROBABILITY) {
      float mutationFactor = 1.0f + ((float)GetRandomValue(-10, 10) / 100.0f);
      constraints[c].radius *= mutationFactor;

      // Clamp radius to reasonable bounds
      if (constraints[c].radius < 50.0f)
        constraints[c].radius = 50.0f;
      if (constraints[c].radius > 200.0f)
        constraints[c].radius = 200.0f;
    }
  }
}

// ============================================================================
// SCALAR FIELD AND CONSTRAINT LOGIC
// ============================================================================

void UpdateScalarField(void) {
  // Clear field
  for (int i = 0; i < env.height; i++) {
    for (int j = 0; j < env.width; j++) {
      env.field[i][j] = 0.0f;
    }
  }

  // Add agent contributions (simple density)
  for (int a = 0; a < AGENT_COUNT; a++) {
    int gridX = (int)(agents[a].position.x / env.cellSize);
    int gridY = (int)(agents[a].position.y / env.cellSize);

    if (gridX >= 0 && gridX < env.width && gridY >= 0 && gridY < env.height) {
      env.field[gridY][gridX] += 1.0f;

      // Add influence to neighboring cells (Gaussian-like spread)
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int nx = gridX + dx;
          int ny = gridY + dy;
          if (nx >= 0 && nx < env.width && ny >= 0 && ny < env.height) {
            if (dx != 0 || dy != 0) {
              env.field[ny][nx] += 0.3f;
            }
          }
        }
      }
    }
  }
}

void ApplyConstraintForces(void) {
  for (int c = 0; c < constraintCount; c++) {
    // Calculate local density at constraint center
    int gridX = (int)(constraints[c].center.x / env.cellSize);
    int gridY = (int)(constraints[c].center.y / env.cellSize);

    float localDensity = 0.0f;
    if (gridX >= 0 && gridX < env.width && gridY >= 0 && gridY < env.height) {
      localDensity = env.field[gridY][gridX];
    }

    // Check if constraint should activate
    if (localDensity >= constraints[c].densityThreshold) {
      constraints[c].activationCount++;

      // Apply forces to agents within radius
      for (int a = 0; a < AGENT_COUNT; a++) {
        Vector2 toAgent =
            Vector2Subtract(agents[a].position, constraints[c].center);
        float distance = Vector2Length(toAgent);

        if (distance < constraints[c].radius && distance > 0.01f) {
          Vector2 direction = Vector2Normalize(toAgent);

          // Repulsion: push away from center
          if (constraints[c].effectType == 0) {
            float forceMagnitude = constraints[c].strength *
                                   (1.0f - distance / constraints[c].radius);
            Vector2 force = Vector2Scale(direction, forceMagnitude);
            agents[a].velocity = Vector2Add(agents[a].velocity, force);
            constraints[c].totalForceApplied += forceMagnitude;
          }
        }
      }
    }
  }
}

// ============================================================================
// UPDATE LOGIC
// ===========================================================================

void UpdateAgents(void) {
  // Update scalar field based on current agent positions
  UpdateScalarField();

  // Apply constraint forces
  ApplyConstraintForces();

  for (int i = 0; i < AGENT_COUNT; i++) {
    // Simple random walk
    Vector2 randomForce = {
        (float)(GetRandomValue(-100, 100)) / 100.0f * RANDOM_WALK_STRENGTH,
        (float)(GetRandomValue(-100, 100)) / 100.0f * RANDOM_WALK_STRENGTH};

    agents[i].velocity = Vector2Add(agents[i].velocity, randomForce);

    // Velocity damping
    agents[i].velocity = Vector2Scale(agents[i].velocity, 0.95f);

    // Update position
    agents[i].position = Vector2Add(
        agents[i].position, Vector2Scale(agents[i].velocity, GetFrameTime()));

    // Boundary clamping
    if (agents[i].position.x < AGENT_RADIUS) {
      agents[i].position.x = AGENT_RADIUS;
      agents[i].velocity.x *= -0.5f;
    }
    if (agents[i].position.x > WINDOW_WIDTH - AGENT_RADIUS) {
      agents[i].position.x = WINDOW_WIDTH - AGENT_RADIUS;
      agents[i].velocity.x *= -0.5f;
    }
    if (agents[i].position.y < AGENT_RADIUS) {
      agents[i].position.y = AGENT_RADIUS;
      agents[i].velocity.y *= -0.5f;
    }
    if (agents[i].position.y > WINDOW_HEIGHT - AGENT_RADIUS) {
      agents[i].position.y = WINDOW_HEIGHT - AGENT_RADIUS;
      agents[i].velocity.y *= -0.5f;
    }
  }
}

// ============================================================================
// RENDERING
// ============================================================================

void RenderSimulation(void) {
  BeginDrawing();
  ClearBackground(BLACK);

  // Draw constraints
  for (int c = 0; c < constraintCount; c++) {
    Color constraintColor;
    switch (constraints[c].effectType) {
    case 0:
      constraintColor = (Color){255, 80, 80, 100};
      break; // Repulsion: Red
    case 1:
      constraintColor = (Color){80, 80, 255, 100};
      break; // Attraction: Blue
    case 2:
      constraintColor = (Color){80, 255, 80, 100};
      break; // Damping: Green
    default:
      constraintColor = GRAY;
      break;
    }
    DrawCircleLines((int)constraints[c].center.x, (int)constraints[c].center.y,
                    constraints[c].radius, constraintColor);
    DrawCircleV(constraints[c].center, 3.0f, constraintColor);
  }

  // Draw agents
  for (int i = 0; i < AGENT_COUNT; i++) {
    DrawCircleV(agents[i].position, AGENT_RADIUS, WHITE);
  }

  // Draw HUD
  DrawText(TextFormat("Mode: %s", EVOLVING_MODE ? "EVOLVING" : "BASELINE"), 10,
           10, 20, RAYWHITE);
  DrawText(
      TextFormat("Episode: %d | Timestep: %d", currentEpisode, currentTimestep),
      10, 35, 20, RAYWHITE);
  DrawText(TextFormat("FPS: %d", GetFPS()), 10, 60, 20, RAYWHITE);

  EndDrawing();
}

// ============================================================================
// EPISODE-LEVEL METRICS
// ============================================================================

float ComputeSpatialVariance(void) {
  // FIX 4: Spatial variance of agent positions (renamed from diversity)
  float meanX = 0.0f, meanY = 0.0f;
  for (int i = 0; i < AGENT_COUNT; i++) {
    meanX += agents[i].position.x;
    meanY += agents[i].position.y;
  }
  meanX /= AGENT_COUNT;
  meanY /= AGENT_COUNT;

  float variance = 0.0f;
  for (int i = 0; i < AGENT_COUNT; i++) {
    float dx = agents[i].position.x - meanX;
    float dy = agents[i].position.y - meanY;
    variance += dx * dx + dy * dy;
  }
  return variance / AGENT_COUNT;
}

float ComputeStability(void) {
  // 1 - (velocity variance / mean velocity)
  float meanSpeed = 0.0f;
  for (int i = 0; i < AGENT_COUNT; i++) {
    meanSpeed += Vector2Length(agents[i].velocity);
  }
  meanSpeed /= AGENT_COUNT;

  if (meanSpeed < 0.01f)
    return 1.0f; // No movement = stable

  float variance = 0.0f;
  for (int i = 0; i < AGENT_COUNT; i++) {
    float speed = Vector2Length(agents[i].velocity);
    float diff = speed - meanSpeed;
    variance += diff * diff;
  }
  variance /= AGENT_COUNT;

  float stabilityScore = 1.0f - (variance / (meanSpeed * meanSpeed + 0.01f));
  return stabilityScore < 0.0f ? 0.0f : stabilityScore;
}

float ComputeEntropy(void) {
  // Shannon entropy of normalized scalar field
  float totalDensity = 0.0f;
  for (int i = 0; i < env.height; i++) {
    for (int j = 0; j < env.width; j++) {
      totalDensity += env.field[i][j];
    }
  }

  if (totalDensity < 0.01f)
    return 0.0f;

  float entropy = 0.0f;
  for (int i = 0; i < env.height; i++) {
    for (int j = 0; j < env.width; j++) {
      if (env.field[i][j] > 0.01f) {
        float p = env.field[i][j] / totalDensity;
        entropy -= p * log2f(p);
      }
    }
  }
  return entropy;
}

float ComputeCollapse(void) {
  // Proportion of agents in high-density clusters
  // Define high-density as cells with > 5 agents worth of density
  int clusteredAgents = 0;
  for (int a = 0; a < AGENT_COUNT; a++) {
    int gridX = (int)(agents[a].position.x / env.cellSize);
    int gridY = (int)(agents[a].position.y / env.cellSize);

    if (gridX >= 0 && gridX < env.width && gridY >= 0 && gridY < env.height) {
      if (env.field[gridY][gridX] > 5.0f) {
        clusteredAgents++;
      }
    }
  }
  return (float)clusteredAgents / AGENT_COUNT;
}

void LogEpisodeMetrics(FILE *logFile, int episodeNum) {
  // Update field one last time for final metrics
  UpdateScalarField();

  float spatialVariance = ComputeSpatialVariance(); // FIX 4: Renamed
  float stability = ComputeStability();
  float entropy = ComputeEntropy();
  float collapse = ComputeCollapse();

  float meanStrength = 0.0f;
  for (int c = 0; c < constraintCount; c++) {
    meanStrength += constraints[c].strength;
  }
  if (constraintCount > 0) {
    meanStrength /= constraintCount;
  }

  // Write CSV row:
  // run,episode,spatial_variance,stability,entropy,collapse,constraint_count,mean_strength
  fprintf(logFile, "0,%d,%.6f,%.6f,%.6f,%.6f,%d,%.6f\n", episodeNum,
          spatialVariance, stability, entropy, collapse, constraintCount,
          meanStrength);
  fflush(logFile);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

int main(void) {
  // Initialize random seed (FIXED for reproducibility)
  unsigned int seed = 12345; // Can be varied across runs or passed via argv
  srand(seed);
  SetRandomSeed(seed);

  // Initialize window
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Constraint Ecology Simulation");
  SetTargetFPS(TARGET_FPS);

  // Initialize simulation
  InitEnvironment();
  InitAgents();
  InitConstraints();

  // Open CSV log file
  FILE *episodeLog = fopen("episodes.csv", "w");
  if (episodeLog == NULL) {
    printf("ERROR: Could not create episodes.csv\n");
    return 1;
  }

  // Write CSV header
  fprintf(episodeLog, "run,episode,spatial_variance,stability,entropy,collapse,"
                      "constraint_count,mean_strength\n");
  fflush(episodeLog);

  printf("=== Constraint Ecology Simulation ===\n");
  printf("Mode: %s\n", EVOLVING_MODE ? "EVOLVING" : "BASELINE");
  printf("Agents: %d\n", AGENT_COUNT);
  printf("Episodes: %d\n", EPISODES_PER_RUN);
  printf("Timesteps per episode: %d\n", TIMESTEPS_PER_EPISODE);
  printf("Logging to: episodes.csv\n\n");

  // Main loop
  while (!WindowShouldClose()) {
    // Update simulation
    UpdateAgents();
    currentTimestep++;

    // Episode boundary
    if (currentTimestep >= TIMESTEPS_PER_EPISODE) {
      // Log episode metrics before resetting
      LogEpisodeMetrics(episodeLog, currentEpisode);
      printf("Completed episode %d\n", currentEpisode);

      currentTimestep = 0;
      currentEpisode++;

      // Reset agents for new episode
      InitAgents();

      // FIX 2: Reset constraint statistics
      ResetConstraintStats();

      // FIX 3: Evolve constraints if in evolving mode
      if (EVOLVING_MODE) {
        EvolveConstraints();
      }

      // End simulation after all episodes
      if (currentEpisode >= EPISODES_PER_RUN) {
        printf("\nSimulation complete!\n");
        printf("Results saved to episodes.csv\n");
        break;
      }
    }

    // Render
    RenderSimulation();
  }

  // Cleanup
  fclose(episodeLog);
  FreeConstraints();
  FreeEnvironment();
  CloseWindow();

  return 0;
}

#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define GAME_WIDTH 512.0f
#define GAME_HEIGHT 288.0f

typedef enum PlayerState PlayerState;
enum PlayerState {
  IDLE,
  RUN,
};

typedef enum PlayerDirection PlayerDirection;
enum PlayerDirection {
  LEFT,
  RIGHT,
};

typedef struct AnimationState AnimationState;
struct AnimationState {
  float duration;
  float timer;
  size_t frame;
  size_t frame_count;
};

typedef struct PlayerAnimations PlayerAnimations;
struct PlayerAnimations {
  AnimationState idle;
  AnimationState run;
  AnimationState attack;
  AnimationState jump;
  AnimationState hurt;
  AnimationState death;
};

typedef struct PlayerTextures PlayerTextures;
struct PlayerTextures {
  Texture2D idle;
  Texture2D run;
  Texture2D attack;
  Texture2D jump;
  Texture2D hurt;
  Texture2D death;
};

typedef struct Player Player;
struct Player {
  Vector2 pos;
  float velocity;
  PlayerState state;
  PlayerDirection direction;
  PlayerTextures texture;
  PlayerAnimations animation;
};

void input(float dt, Player *player) {
  player->state = IDLE;
  if (IsKeyDown(KEY_A)) {
    player->pos.x -= player->velocity * dt;
    player->state = RUN;
    player->direction = LEFT;
  }
  if (IsKeyDown(KEY_D)) {
    player->pos.x += player->velocity * dt;
    player->state = RUN;
    player->direction = RIGHT;
  }
}

void updateAnimationState(AnimationState *animation_state, float dt) {
  animation_state->timer += dt;
  if (animation_state->timer >= animation_state->duration) {
    animation_state->frame =
        (animation_state->frame + 1) % animation_state->frame_count;
    animation_state->timer = animation_state->timer - animation_state->duration;
  }
}

void drawIdleAnimation(float dt, Player *player) {
  updateAnimationState(&player->animation.idle, dt);
  Rectangle rect = {
      .x = player->animation.idle.frame * 30,
      .y = 0,
      .width = 30,
      .height = 30,
  };
  if (player->direction == LEFT) {
    rect.width = -rect.width;
  }
  DrawTextureRec(player->texture.idle, rect, player->pos, WHITE);
}

void drawRunAnimation(float dt, Player *player) {
  updateAnimationState(&player->animation.run, dt);
  Rectangle rect = {
      .x = player->animation.run.frame * 30,
      .y = 0,
      .width = 30,
      .height = 30,
  };
  if (player->direction == LEFT) {
    rect.width = -rect.width;
  }
  DrawTextureRec(player->texture.run, rect, player->pos, WHITE);
}

void initPlayerTexturesAndAnimations(Player *player) {
  player->texture.idle =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Idle_30x30_24f.png");
  player->texture.run =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Run_30x30_10f.png");
  player->animation.idle.duration = 0.1f;
  player->animation.idle.frame_count = 24;

  player->animation.run.duration = 0.075f;
  player->animation.run.frame_count = 10;
}

void drawPlayer(float dt, Player *player) {
  switch (player->state) {
  case IDLE: {
    drawIdleAnimation(dt, player);
  } break;
  case RUN: {
    drawRunAnimation(dt, player);
  } break;
  }
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(GAME_WIDTH, GAME_HEIGHT, "animations");
  SetTargetFPS(60);
  RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

  Texture2D background = LoadTexture(
      "/home/omar/Work/game_animations/assets/riverwake/1x/01_the_bank.png");

  Player player = {0};
  player.pos.x = 100;
  player.pos.y = 100;
  player.velocity = 150;
  player.state = IDLE;
  player.direction = RIGHT;
  initPlayerTexturesAndAnimations(&player);

  float dt = 0.0f;
  while (!WindowShouldClose()) {
    dt = GetFrameTime();
    input(dt, &player);
    BeginTextureMode(target);
    ClearBackground(WHITE);
    DrawTextureEx(background, (Vector2){0, 0}, 0.0f, 2.0f, WHITE);
    drawPlayer(dt, &player);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    float scale = fminf((float)GetScreenWidth() / GAME_WIDTH,
                        (float)GetScreenHeight() / GAME_HEIGHT);
    Rectangle src = {0, 0, GAME_WIDTH, -GAME_HEIGHT};
    Rectangle dst = {(GetScreenWidth() - GAME_WIDTH * scale) / 2.0f,
                     (GetScreenHeight() - GAME_HEIGHT * scale) / 2.0f,
                     GAME_WIDTH * scale, GAME_HEIGHT * scale};
    DrawTexturePro(target.texture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

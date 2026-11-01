#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define GAME_WIDTH 512.0f
#define GAME_HEIGHT 288.0f
#define VELOCITY 150
#define ATTACK_VELOCITY 20

typedef enum PlayerState PlayerState;
enum PlayerState {
  IDLE,
  RUN,
  ATTACK,
  JUMP,
  HURT,
  DEATH,
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
  size_t width;
  size_t height;
  Vector2 offset;
  Texture2D texture;
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

typedef struct Player Player;
struct Player {
  Vector2 pos;
  float velocity;
  Vector2 dimensions;
  Vector2 hitbox;
  PlayerState state;
  PlayerState prev_state;
  PlayerDirection direction;
  PlayerAnimations animation;
};

void input(float dt, Player *player) {
  player->prev_state = player->state;
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
  if (IsKeyDown(KEY_E)) {
    player->state = ATTACK;
    player->velocity = ATTACK_VELOCITY;
  } else {
    player->velocity = VELOCITY;
  }

  // if (IsKeyDown(KEY_I)) {
  //   player->prev_state = player->state;
  //   player->state = IDLE;
  // }
  // if (IsKeyDown(KEY_R)) {
  //   player->prev_state = player->state;
  //   player->state = RUN;
  // }
  // if (IsKeyDown(KEY_Q)) {
  //   player->prev_state = player->state;
  //   player->state = ATTACK;
  // }
  // if (IsKeyDown(KEY_J)) {
  //   player->prev_state = player->state;
  //   player->state = JUMP;
  // }
  // if (IsKeyDown(KEY_H)) {
  //   player->prev_state = player->state;
  //   player->state = HURT;
  // }
  // if (IsKeyDown(KEY_D)) {
  //   player->prev_state = player->state;
  //   player->state = DEATH;
  // }

  if (player->prev_state != player->state) {
    player->animation.idle.timer = 0;
    player->animation.idle.frame = 0;

    player->animation.run.timer = 0;
    player->animation.run.frame = 0;

    player->animation.attack.timer = 0;
    player->animation.attack.frame = 0;

    player->animation.jump.timer = 0;
    player->animation.jump.frame = 0;

    player->animation.hurt.timer = 0;
    player->animation.hurt.frame = 0;

    player->animation.death.timer = 0;
    player->animation.death.frame = 0;
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

void drawAnimation(float dt, AnimationState *animation_state, Player *player) {
  updateAnimationState(animation_state, dt);
  Rectangle src = {
      .x = animation_state->frame * animation_state->width,
      .y = 0,
      .width = animation_state->width,
      .height = animation_state->height,
  };

  Rectangle dst = {
      .x = player->pos.x - animation_state->width / 2.0f +
           animation_state->offset.x,
      .y = player->pos.y - animation_state->height + animation_state->offset.y,
      .width = animation_state->width,
      .height = animation_state->height,
  };
  if (player->direction == LEFT) {
    src.width = -src.width;
    dst.x = player->pos.x - animation_state->width / 2.0f -
            animation_state->offset.x;
  }
  DrawTexturePro(animation_state->texture, src, dst, (Vector2){0, 0}, 0.0f,
                 WHITE);
}

void drawIdleAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.idle, player);
}

void drawRunAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.run, player);
}

void drawAttackAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.attack, player);
}

void drawJumpAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.jump, player);
}

void drawHurtAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.hurt, player);
}

void drawDeathAnimation(float dt, Player *player) {
  drawAnimation(dt, &player->animation.death, player);
}

void initPlayerTexturesAndAnimations(Player *player) {
  player->animation.idle.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Idle_30x30_24f.png");
  player->animation.run.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Run_30x30_10f.png");

  player->animation.attack.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Attack1_90x50_8f.png");

  player->animation.jump.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_JumpFall_30x30_28f.png");

  player->animation.hurt.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Hurt_55x80_9f.png");

  player->animation.death.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Death_40x50_20f.png");

  player->animation.idle.duration = 0.1f;
  player->animation.idle.frame_count = 24;
  player->animation.idle.width = 30;
  player->animation.idle.height = 30;

  player->animation.run.duration = 0.075f;
  player->animation.run.frame_count = 10;
  player->animation.run.width = 30;
  player->animation.run.height = 30;

  player->animation.attack.duration = 0.075f;
  player->animation.attack.frame_count = 8;
  player->animation.attack.width = 90;
  player->animation.attack.height = 50;
  player->animation.attack.offset.y = 10;
  player->animation.attack.offset.x = 28;

  player->animation.jump.duration = 0.075f;
  player->animation.jump.frame_count = 28;
  player->animation.jump.width = 30;
  player->animation.jump.height = 30;

  player->animation.hurt.duration = 0.075f;
  player->animation.hurt.frame_count = 9;
  player->animation.hurt.width = 55;
  player->animation.hurt.height = 80;
  player->animation.hurt.offset.y = 25;

  player->animation.death.duration = 0.1f;
  player->animation.death.frame_count = 20;
  player->animation.death.width = 40;
  player->animation.death.height = 50;
}

void drawPlayer(float dt, Player *player) {

  Rectangle interactbox = {
      .x = player->pos.x - player->dimensions.x / 2.0f,
      .y = player->pos.y - player->dimensions.y,
      .width = player->dimensions.x,
      .height = player->dimensions.y,
  };
  Rectangle hitbox = {
      .x = player->pos.x - player->hitbox.x / 2.0f,
      .y = player->pos.y - player->hitbox.y,
      .width = player->hitbox.x,
      .height = player->hitbox.y,
  };
  DrawRectangleLinesEx(interactbox, 1.0f, BLUE);
  DrawRectangleLinesEx(hitbox, 1.0f, RED);
  switch (player->state) {
  case IDLE: {
    drawIdleAnimation(dt, player);
  } break;
  case RUN: {
    drawRunAnimation(dt, player);
  } break;
  case ATTACK: {
    drawAttackAnimation(dt, player);
  } break;
  case JUMP: {
    drawJumpAnimation(dt, player);
  } break;
  case HURT: {
    drawHurtAnimation(dt, player);
  } break;
  case DEATH: {
    drawDeathAnimation(dt, player);
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
  player.velocity = VELOCITY;
  player.dimensions.x = 25;
  player.dimensions.y = 30;
  player.hitbox.x = 15;
  player.hitbox.y = 25;
  player.state = IDLE;
  player.direction = LEFT;
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

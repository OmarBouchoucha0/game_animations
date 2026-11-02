#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define GAME_WIDTH 512.0f
#define GAME_HEIGHT 288.0f
#define SPEED 150
#define PLAYER_ID 1
#define MAX_ENTITIES 1000
#define GRAVITY_ACCELERATION 9.8f
#define GRAVITY 300.0f
#define JUMP_HEIGHT 90.0f
#define MAX_FALL_SPEED 600.0f
#define ATTACK_RANGE 55.0f

static size_t LAST_ENTITY_ID = 1;

typedef enum EntityState EntityState;
enum EntityState {
  IDLE,
  RUN,
  ATTACK,
  JUMP,
  FALL,
  HURT,
  DEATH,
};

typedef enum Direction Direction;
enum Direction {
  LEFT,
  RIGHT,
};

typedef enum EntityType EntityType;
enum EntityType {
  PLAYER,
  ENEMY,
  PLATFORM,
  NONE,
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

typedef struct EntityAnimations EntityAnimations;
struct EntityAnimations {
  AnimationState idle;
  AnimationState run;
  AnimationState attack;
  AnimationState jump;
  AnimationState fall;
  AnimationState hurt;
  AnimationState death;
};

typedef struct Entity Entity;
struct Entity {
  Vector2 pos;
  float speed;
  float weight;
  float attack_range;
  float health;
  float damage;
  Vector2 velocity;
  Vector2 dimensions;
  Vector2 hitbox;
  EntityState state;
  EntityState prev_state;
  Direction direction;
  EntityType entity_type;
  bool grounded;
  bool invurnable;
  EntityAnimations animation;
};

void addEntity(Entity *entities, Entity entity) {
  if (LAST_ENTITY_ID >= MAX_ENTITIES) {
    return;
  }
  entities[LAST_ENTITY_ID] = entity;
  LAST_ENTITY_ID++;
}

void input(float dt, Entity *player) {
  player->prev_state = player->state;
  player->velocity.x = 0;

  if (IsKeyDown(KEY_E)) {
    player->state = ATTACK;
  }
  if (IsKeyDown(KEY_A)) {
    player->velocity.x = -player->speed;
    player->direction = LEFT;
  }
  if (IsKeyDown(KEY_D)) {
    player->velocity.x = player->speed;
    player->direction = RIGHT;
  }

  if (IsKeyPressed(KEY_SPACE) && player->grounded) {
    player->velocity.y = -sqrtf(2.0f * GRAVITY * player->weight * JUMP_HEIGHT);
    player->grounded = false;
  }
}

void updateVelocity(float dt, Entity *entities, size_t id) {
  Entity *player = &entities[id];
  if (player->state == RUN || player->state == JUMP || player->state == FALL) {
    player->state = IDLE;
  }
  dt = fminf(dt, 1.0f / 30.0f);
  player->velocity.y += GRAVITY * player->weight * dt;
  if (player->velocity.y > MAX_FALL_SPEED)
    player->velocity.y = MAX_FALL_SPEED;
  Vector2 dp = {
      .x = player->velocity.x * dt,
      .y = player->velocity.y * dt,
  };
  float old_x = player->pos.x;
  Rectangle player_dimensions = {
      .x = player->pos.x + dp.x - player->hitbox.x / 2.0f,
      .y = player->pos.y - player->hitbox.y,
      .width = player->hitbox.x,
      .height = player->hitbox.y,
  };

  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    if (i != id) {
      Entity entity = entities[i];
      if (entity.entity_type == NONE) {
        continue;
      }
      Rectangle entity_dimensions = {0};
      if (entity.entity_type == PLATFORM) {
        entity_dimensions.x = entity.pos.x;
        entity_dimensions.y = entity.pos.y;
        entity_dimensions.width = entity.dimensions.x;
        entity_dimensions.height = entity.dimensions.y;
      }
      if (entity.entity_type == ENEMY) {
        entity_dimensions.x = entity.pos.x - entity.hitbox.x / 2;
        entity_dimensions.y = entity.pos.y - entity.hitbox.y;
        entity_dimensions.width = entity.hitbox.x;
        entity_dimensions.height = entity.hitbox.y;
      }
      if (CheckCollisionRecs(player_dimensions, entity_dimensions)) {
        if (dp.x > 0) {
          player_dimensions.x = entity_dimensions.x - entity_dimensions.width;
        } else if (dp.x < 0) {
          player_dimensions.x = entity_dimensions.x + entity_dimensions.width;
        }
      }
    }
  }
  player->pos.x = player_dimensions.x + player->hitbox.x / 2.0f;

  player_dimensions.y = player->pos.y + dp.y - player->hitbox.y;
  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    if (i != id) {
      Entity entity = entities[i];

      if (entity.entity_type == NONE) {
        continue;
      }

      Rectangle entity_dimensions = {0};
      if (entity.entity_type == PLATFORM) {
        entity_dimensions.x = entity.pos.x;
        entity_dimensions.y = entity.pos.y;
        entity_dimensions.width = entity.dimensions.x;
        entity_dimensions.height = entity.dimensions.y;
      }
      if (entity.entity_type == ENEMY) {
        entity_dimensions.x = entity.pos.x - entity.hitbox.x / 2;
        entity_dimensions.y = entity.pos.y - entity.hitbox.y;
        entity_dimensions.width = entity.hitbox.x;
        entity_dimensions.height = entity.hitbox.y;
      }
      if (CheckCollisionRecs(player_dimensions, entity_dimensions)) {
        if (dp.y > 0) {
          player_dimensions.y = entity_dimensions.y - player_dimensions.height;
          player->grounded = true;
        } else if (dp.y < 0) {
          player_dimensions.y = entity_dimensions.y + entity_dimensions.height;
        }
        player->velocity.y = 0;
      }
    }
  }
  player->pos.y = player_dimensions.y + player->hitbox.y;

  if (player->state != ATTACK) {
    if (!player->grounded) {
      if (player->velocity.y > 0) {
        player->state = FALL;
      } else {
        player->state = JUMP;
      }
    } else if (player->pos.x != old_x) {
      player->state = RUN;
    }
  }
}

void playerAttackCollision(Entity *entities) {
  Entity *player = &entities[PLAYER_ID];
  if (player->state != ATTACK) {
    return;
  }
  Rectangle attack_box = {
      .x = player->pos.x + player->dimensions.x / 2.0f,
      .y = player->pos.y - player->dimensions.y,
      .width = player->attack_range,
      .height = player->dimensions.y,
  };
  if (player->direction == LEFT) {
    attack_box.x =
        player->pos.x - player->dimensions.x / 2.0f - player->attack_range;
    attack_box.y = player->pos.y - player->dimensions.y;
  }
  for (size_t i = 2; i < LAST_ENTITY_ID; ++i) {
    Entity *entity = &entities[i];
    if (entity->entity_type == ENEMY) {
      Rectangle hitbox = {
          .x = entity->pos.x - entity->hitbox.x / 2.0f,
          .y = entity->pos.y - entity->hitbox.y,
          .width = entity->hitbox.x,
          .height = entity->hitbox.y,
      };
      if (CheckCollisionRecs(attack_box, hitbox)) {
        if (entity->invurnable == false) {
          entity->health -= player->damage;
          if (entity->health <= 0) {
            entity->state = DEATH;
            entity->prev_state = entity->state;
          } else {
            entity->state = HURT;
            entity->prev_state = entity->state;
          }
          entity->invurnable = true;
        }
      }
    }
  }
}

void resetState(Entity *entities) {
  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    Entity *entity = &entities[i];
    if (entity->entity_type == PLAYER || entity->entity_type == ENEMY) {
      if (entity->animation.attack.frame >=
          entity->animation.attack.frame_count - 1) {
        entity->prev_state = entity->state;
        entity->state = IDLE;
      }
      if (entity->animation.hurt.frame >=
          entity->animation.hurt.frame_count - 1) {
        entity->prev_state = entity->state;
        entity->state = IDLE;
        entity->invurnable = false;
      }
      if (entity->animation.death.frame >=
          entity->animation.death.frame_count - 1) {
        *entity = entities[0];
      }
    }
  }
}

void physics(float dt, Entity *entities) {
  resetState(entities);
  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    Entity entity = entities[i];
    if (entity.entity_type == PLAYER || entity.entity_type == ENEMY) {
      updateVelocity(dt, entities, i);
    }
  }
  playerAttackCollision(entities);
}

void updateAnimationState(AnimationState *animation_state, float dt) {
  animation_state->timer += dt;
  if (animation_state->timer >= animation_state->duration) {
    animation_state->frame =
        (animation_state->frame + 1) % animation_state->frame_count;
    animation_state->timer = animation_state->timer - animation_state->duration;
  }
}

void drawAnimation(float dt, AnimationState *animation_state, Entity *entity) {
  if (entity->prev_state != entity->state) {
    entity->animation.idle.timer = 0;
    entity->animation.idle.frame = 0;

    entity->animation.run.timer = 0;
    entity->animation.run.frame = 0;

    entity->animation.attack.timer = 0;
    entity->animation.attack.frame = 0;

    entity->animation.jump.timer = 0;
    entity->animation.jump.frame = 0;

    entity->animation.fall.timer = 0;
    entity->animation.fall.frame = 0;

    entity->animation.hurt.timer = 0;
    entity->animation.hurt.frame = 0;

    entity->animation.death.timer = 0;
    entity->animation.death.frame = 0;
  }
  updateAnimationState(animation_state, dt);
  Rectangle src = {
      .x = animation_state->frame * animation_state->width,
      .y = 0,
      .width = animation_state->width,
      .height = animation_state->height,
  };

  Rectangle dst = {
      .x = entity->pos.x - animation_state->width / 2.0f +
           animation_state->offset.x,
      .y = entity->pos.y - animation_state->height + animation_state->offset.y,
      .width = animation_state->width,
      .height = animation_state->height,
  };
  if (entity->direction == LEFT) {
    src.width = -src.width;
    dst.x = entity->pos.x - animation_state->width / 2.0f -
            animation_state->offset.x;
  }
  if (entity->entity_type == PLAYER) {
    DrawTexturePro(animation_state->texture, src, dst, (Vector2){0, 0}, 0.0f,
                   WHITE);
  } else {
    DrawTexturePro(animation_state->texture, src, dst, (Vector2){0, 0}, 0.0f,
                   GREEN);
  }
}

void drawIdleAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.idle, entity);
}

void drawRunAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.run, entity);
}

void drawAttackAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.attack, entity);
}

void drawJumpAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.jump, entity);
}

void drawFallAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.fall, entity);
}

void drawHurtAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.hurt, entity);
}

void drawDeathAnimation(float dt, Entity *entity) {
  drawAnimation(dt, &entity->animation.death, entity);
}

void initPlayerTexturesAndAnimations(Entity *player) {
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

  player->animation.fall.texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Fall_30x30_14f.png");

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

  player->animation.attack.duration = 0.05f;
  player->animation.attack.frame_count = 8;
  player->animation.attack.width = 90;
  player->animation.attack.height = 50;
  player->animation.attack.offset.y = 10;
  player->animation.attack.offset.x = 28;

  player->animation.jump.duration = 0.075f;
  player->animation.jump.frame_count = 14;
  player->animation.jump.width = 30;
  player->animation.jump.height = 30;

  player->animation.fall.duration = 0.075f;
  player->animation.fall.frame_count = 14;
  player->animation.fall.width = 30;
  player->animation.fall.height = 30;

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

void drawPlayer(float dt, Entity *player) {

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

  Rectangle attack_box = {
      .x = player->pos.x + player->dimensions.x / 2.0f,
      .y = player->pos.y - player->dimensions.y,
      .width = player->attack_range,
      .height = player->dimensions.y,
  };
  DrawRectangleLinesEx(interactbox, 1.0f, BLUE);
  DrawRectangleLinesEx(hitbox, 1.0f, RED);

  char info_buffer[1000];

  snprintf(info_buffer, sizeof(info_buffer), "hp : %.0f", player->health);
  int text_width = MeasureText(info_buffer, 4);
  DrawText(info_buffer, player->pos.x - text_width / 2,
           player->pos.y - player->dimensions.y - 8, 4, RAYWHITE);

  switch (player->state) {
  case IDLE: {
    drawIdleAnimation(dt, player);
  } break;
  case RUN: {
    drawRunAnimation(dt, player);
  } break;
  case ATTACK: {
    if (player->direction == LEFT) {
      attack_box.x =
          player->pos.x - player->dimensions.x / 2.0f - player->attack_range;
      attack_box.y = player->pos.y - player->dimensions.y;
    }
    DrawRectangleLinesEx(attack_box, 1.0f, GREEN);
    drawAttackAnimation(dt, player);
  } break;
  case JUMP: {
    drawJumpAnimation(dt, player);
  } break;
  case FALL: {
    drawFallAnimation(dt, player);
  } break;
  case HURT: {
    drawHurtAnimation(dt, player);
  } break;
  case DEATH: {
    drawDeathAnimation(dt, player);
  } break;
  }
}

void drawEnemies(float dt, Entity *entities) {
  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    if (entities[i].entity_type == ENEMY) {
      drawPlayer(dt, &entities[i]);
    }
  }
}

void spawnEnemy(Entity *entities, Entity enemy) {

  int enemy_count = 0;
  for (size_t i = 1; i < LAST_ENTITY_ID; ++i) {
    if (entities[i].entity_type == ENEMY) {
      enemy_count++;
    }
  }
  if (enemy_count <= 0) {
    addEntity(entities, enemy);
  }
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(GAME_WIDTH, GAME_HEIGHT, "animations");
  SetTargetFPS(60);
  RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

  Texture2D background = LoadTexture("/home/omar/Work/game_animations/"
                                     "assets/riverwake/1x/01_the_bank.png");

  Entity entities[MAX_ENTITIES] = {0};
  Entity none = {0};
  none.entity_type = NONE;
  entities[0] = none;

  Entity player = {0};
  player.pos.x = 20;
  player.pos.y = 100;
  player.speed = SPEED;
  player.attack_range = ATTACK_RANGE;
  player.weight = 3;
  player.health = 10;
  player.damage = 2;
  player.dimensions.x = 25;
  player.dimensions.y = 30;
  player.hitbox.x = 15;
  player.hitbox.y = 25;
  player.state = IDLE;
  player.direction = LEFT;
  player.grounded = false;
  player.entity_type = PLAYER;
  initPlayerTexturesAndAnimations(&player);

  Entity ground = {0};
  ground.pos.x = 0;
  ground.pos.y = 200;
  ground.dimensions.x = GAME_WIDTH;
  ground.dimensions.y = 50;
  ground.entity_type = PLATFORM;

  Entity enemy = {0};
  enemy.pos.x = 100;
  enemy.pos.y = 100;
  enemy.speed = SPEED;
  enemy.weight = 3;
  enemy.health = 5;
  enemy.damage = 1;
  enemy.hitbox.x = 15;
  enemy.hitbox.y = 25;
  enemy.dimensions.x = 25;
  enemy.dimensions.y = 30;
  enemy.entity_type = ENEMY;
  enemy.state = IDLE;
  enemy.prev_state = IDLE;
  initPlayerTexturesAndAnimations(&enemy);

  addEntity(entities, player);
  addEntity(entities, ground);

  Camera2D camera = {0};
  camera.target = (Vector2){player.pos.x + 20.0f, player.pos.y + 20.0f};
  camera.offset = (Vector2){GetRenderWidth() / 2.0f, GetRenderHeight() / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  float dt = 0.0f;
  char info_buffer[1000];

  while (!WindowShouldClose()) {
    dt = GetFrameTime();
    camera.target = (Vector2){
        entities[PLAYER_ID].pos.x + 20,
        entities[PLAYER_ID].pos.y + 20,
    };

    spawnEnemy(entities, enemy);
    input(dt, &entities[PLAYER_ID]);
    physics(dt, entities);
    BeginTextureMode(target);
    ClearBackground(WHITE);

    DrawTextureEx(background, (Vector2){0, 0}, 0.0f, 2.0f, WHITE);
    snprintf(info_buffer, sizeof(info_buffer), "player: pos{x:%.2f,y:%.2f}",
             entities[PLAYER_ID].pos.x, entities[PLAYER_ID].pos.y);
    DrawText(info_buffer, 2, 2, 8, RAYWHITE);
    switch (entities[PLAYER_ID].state) {
    case IDLE: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :IDLE");
    } break;
    case RUN: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :RUN");
    } break;
    case ATTACK: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :ATTACK");
    } break;
    case JUMP: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :JUMP");
    } break;
    case FALL: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :FALL");
    } break;
    case HURT: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :HURT");
    } break;
    case DEATH: {
      snprintf(info_buffer, sizeof(info_buffer), "player State :DEATH");
    } break;
    }
    DrawText(info_buffer, 2, 10, 8, RAYWHITE);

    snprintf(info_buffer, sizeof(info_buffer), "enemy: pos{x:%.2f,y:%.2f}",
             entities[3].pos.x, entities[3].pos.y);
    DrawText(info_buffer, 2, 20, 8, RAYWHITE);
    switch (entities[3].state) {
    case IDLE: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :IDLE");
    } break;
    case RUN: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :RUN");
    } break;
    case ATTACK: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :ATTACK");
    } break;
    case JUMP: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :JUMP");
    } break;
    case FALL: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :FALL");
    } break;
    case HURT: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :HURT");
    } break;
    case DEATH: {
      snprintf(info_buffer, sizeof(info_buffer), "enemy State :DEATH");
    } break;
    }
    DrawText(info_buffer, 2, 28, 8, RAYWHITE);

    BeginMode2D(camera);
    drawPlayer(dt, &entities[PLAYER_ID]);
    drawEnemies(dt, entities);
    DrawRectangleLinesEx(
        (Rectangle){
            .x = ground.pos.x,
            .y = ground.pos.y,
            .width = ground.dimensions.x,
            .height = ground.dimensions.y,
        },
        1.0f, RED);
    EndMode2D();
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

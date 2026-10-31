#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define GAME_WIDTH 640.0f
#define GAME_HEIGHT 360.0f

typedef struct AnimationState AnimationState;
struct AnimationState {
  float duration;
  float timer;
  size_t frame;
  size_t frame_count;
};

void updateAnimationState(AnimationState *animation_state, float dt) {
  animation_state->timer += dt;
  if (animation_state->timer >= animation_state->duration) {
    animation_state->frame =
        (animation_state->frame + 1) % animation_state->frame_count;
    animation_state->timer = animation_state->timer - animation_state->duration;
  }
}

void drawIdleAnimation(float dt, Texture2D idle_texture,
                       AnimationState *animation_state, Vector2 player_pos) {
  updateAnimationState(animation_state, dt);
  Rectangle rect = {
      .x = animation_state->frame * 30,
      .y = 0,
      .width = 30,
      .height = 30,
  };
  DrawTextureRec(idle_texture, rect, player_pos, WHITE);
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(GAME_WIDTH, GAME_HEIGHT, "animations");
  SetTargetFPS(60);
  RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

  Texture2D idle_texture =
      LoadTexture("/home/omar/Work/game_animations/assets/Seeker_Lite/"
                  "Spritesheets/Seeker_Void_Idle_30x30_24f.png");
  Vector2 player_pos = {
      .x = 100,
      .y = 100,
  };
  AnimationState idle_animation_state = {0};
  idle_animation_state.duration = 0.1f;
  idle_animation_state.frame_count = 24;

  float dt = 0.0f;
  while (!WindowShouldClose()) {
    dt = GetFrameTime();
    BeginTextureMode(target);
    ClearBackground(BLACK);
    drawIdleAnimation(dt, idle_texture, &idle_animation_state, player_pos);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    float scale = fminf((float)GetScreenWidth() / GAME_WIDTH,
                        (float)GetScreenHeight() / GAME_HEIGHT);
    Rectangle src = {0, 0, GAME_WIDTH, -GAME_HEIGHT};
    Rectangle dst = {0, 0, GAME_WIDTH * scale, GAME_HEIGHT * scale};
    DrawTexturePro(target.texture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
  }
  UnloadTexture(idle_texture);
  CloseWindow();
  return 0;
}

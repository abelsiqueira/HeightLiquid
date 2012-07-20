/* Copyright 2012 - Abel Soares Siqueira
 * 
 * This file is part of HeightLiquid.
 * 
 * HeightLiquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * HeightLiquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HeightLiquid.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

const unsigned int constFps = 60;

void Update (float **water, int **height, int w, int h) {
  water[1][1] += 1.0;

  for (int i = 1; i < h-1; i++) {
    for (int j = 1; j < w-1; j++) {
      float waterValue = water[j][i];
      int   heightValue = height[j][i];
      if (waterValue == 0)
        continue;
      for (int k = -1; k <= 1; k++) {
        for (int p = -1; p <= 1; p++) {
          if (p*p + k*k == 0)
            continue;
          float neighWaterValue = water[j+p][i+k];
          int neighHeightValue = height[j+p][i+k];
          if (neighWaterValue + neighHeightValue >= waterValue + heightValue)
            continue;
          if (neighWaterValue + neighHeightValue < waterValue + heightValue - 0.1) {
            float value = (waterValue + heightValue - 0.1 - neighWaterValue - neighHeightValue)/4;
            water[j+p][i+k] += value;
            water[j][i] -= value;
          }
        }
      }
    }
  }
}

int main () {
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *eventQueue;
  ALLEGRO_TIMER *timer;

  bool done = false;

  al_init();
  display = al_create_display(1280, 720);
  al_set_window_title(display, "Height Liquid");
  eventQueue = al_create_event_queue();
  timer = al_create_timer(1.0/constFps);

  al_init_primitives_addon();
  al_install_keyboard();
  al_init_image_addon();

  al_register_event_source(eventQueue, al_get_display_event_source(display));
  al_register_event_source(eventQueue, al_get_timer_event_source(timer));
  al_register_event_source(eventQueue, al_get_keyboard_event_source());

  ALLEGRO_BITMAP *floor;

  float tileSize = 10.0; 
  const int nTilesHoriz = 1280/tileSize;
  const int nTilesVert   = 720/tileSize;
  const ALLEGRO_COLOR heightColors[6] = {
    al_map_rgb(255, 240, 200),
    al_map_rgb(200, 190, 150),
    al_map_rgb(180, 170, 120),
    al_map_rgb(160, 150, 100),
    al_map_rgb(140, 130, 70),
    al_map_rgb(120, 110, 50)
  };

  srand(time(0));
  int   **height;
  float **water;

  height = new int*[nTilesHoriz];
  water  = new float*[nTilesHoriz];
  for (int i = 0; i < nTilesHoriz; i++) {
    height[i] = new int[nTilesVert];
    water[i]  = new float[nTilesVert];
  }

  for (int i = 0; i < nTilesVert; i++) {
    for (int j = 0; j < nTilesHoriz; j++) {
      int v = rand()%100 + 1;
      if (v > 99)
        v = 5;
      else if (v > 98)
        v = 4;
      else if (v > 97)
        v = 3;
      else if (v > 96)
        v = 2;
      else if (v > 95)
        v = 1;
      else
        v = 0;

      height[j][i] = v;
      water[j][i] = 0;
    }
  }

  floor = al_create_bitmap(1280, 720);
  al_set_target_bitmap(floor);
  for (int i = 0; i < nTilesVert; i++) {
    for (int j = 0; j < nTilesHoriz; j++) {
      al_draw_filled_rectangle(j*tileSize, i*tileSize, (j+1)*tileSize, (i+1)*tileSize,
          heightColors[height[j][i]]);
    }
  }
  al_set_target_bitmap(al_get_backbuffer(display));

//  int waterSourceX = 1280/2;
//  int waterSourceY = 720/2;

  bool redraw = false;

  al_clear_to_color(al_map_rgb(0, 0, 0));
  al_start_timer(timer);
  while (!done) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(eventQueue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;
      Update(water, height, nTilesHoriz, nTilesVert);
    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      done = true;
    } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      switch (ev.keyboard.keycode) {
        case ALLEGRO_KEY_Q:
        case ALLEGRO_KEY_ESCAPE:
          done = true;
          break;
      }
    }

    if (redraw && al_is_event_queue_empty(eventQueue)) {
      redraw = false;
      al_clear_to_color(al_map_rgb_f(0.0, 0.0, 0.0));

      al_draw_bitmap(floor, 0, 0, 0);
      for (int i = 0; i < nTilesVert; i++) {
        for (int j = 0; j < nTilesHoriz; j++) {
          float waterValue = water[j][i];
          if (waterValue > 5) waterValue = 5;
          if (waterValue > 0)
            al_draw_filled_rectangle(j*tileSize, i*tileSize,
                (j+1)*tileSize, (i+1)*tileSize, 
                al_map_rgba_f(0, 0, 1.0 - waterValue/10.0, 0.8));
        }
      }

      al_flip_display();
    }
  }

  for (int i = 0; i < nTilesHoriz; i++) {
    delete []height[i];
    delete []water[i];
  }
  delete []height;
  delete []water;

  al_destroy_bitmap(floor);
  al_destroy_timer(timer);
  al_destroy_event_queue(eventQueue);
  al_destroy_display(display);
}

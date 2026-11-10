#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "ray_tracer.h"
#include "demo.h"
#include "chess_balls.h"

#define IMAGE_WIDTH 500
#define IMAGE_HEIGHT 500

static color_t image[IMAGE_HEIGHT][IMAGE_WIDTH];

void put_pixel_func(screen_point p, color_t pixel)
{
    image[p.coords[1]][p.coords[0]] = pixel;
}

static void write_ppm(const char* filename)
{
    int i, j;
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "P3\n%d %d\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);
    for (j = 0; j < IMAGE_HEIGHT; ++j)
    {
        for (i = 0; i < IMAGE_WIDTH; ++i)
        {
            color_t c = image[j][i];
            fprintf(fp, " %d %d %d ", c.channels[0], c.channels[1], c.channels[2]);
        }
        fprintf(fp, "\n");
    }
    (void)fclose(fp);
}

static void render_scene(void (*scene_init)(scene_t*), void (*scene_destroy)(scene_t*), const char* filename)
{
    scene_t scene;
    scene_init(&scene);
    trace(&scene, IMAGE_WIDTH, IMAGE_HEIGHT, put_pixel_func);
    scene_destroy(&scene);
    write_ppm(filename);
}

int main(void)
{
    render_scene(demo_scene_init, demo_scene_destroy, "demo_scene.ppm");
    render_scene(chess_balls_scene_init, chess_balls_scene_destroy, "chess_balls_scene.ppm");
    return 0;
}

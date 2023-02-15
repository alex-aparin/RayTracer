#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "ray_tracer.h"


static color_t image[500][500];

void put_pixel_func(screen_point p, color_t pixel)
{
    image[p.coords[1]][p.coords[0]] = pixel;
}

int main(void)
{   
    trace(100, 100, put_pixel_func);
    {
        int i, j;
        FILE* fp = fopen("first.ppm", "w"); /* b - binary mode */
        fprintf(fp, "P3\n%d %d\n255\n", 500, 500);
        for (j = 0; j < 500; ++j)
        {
            for (i = 0; i < 500; ++i)
            {
                color_t c = image[j][i];
                fprintf(fp, " %d %d %d ", c.channels[0], c.channels[1], c.channels[2]);
            }
            fprintf(fp, "\n");
        }
        (void)fclose(fp);
    }
    return 0;
}
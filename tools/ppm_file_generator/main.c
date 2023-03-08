/*#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "ray_tracer.h"

#define IMAGE_WIDTH 500
#define IMAGE_HEIGHT 500

static color_t image[IMAGE_HEIGHT][IMAGE_WIDTH];

void put_pixel_func(screen_point p, color_t pixel)
{
    image[p.coords[1]][p.coords[0]] = pixel;
}
*/
int main(void)
{   
    trace(IMAGE_WIDTH, IMAGE_HEIGHT, put_pixel_func);
    {
        int i, j;
        FILE* fp = fopen("first.ppm", "w"); /* b - binary mode */
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
    return 0;
}
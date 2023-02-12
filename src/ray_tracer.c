#include "ray_tracer.h"

static int w = 100;
static int h = 100;
static float view_port_w = 100;
static float view_port_h = 100;

world_point viewport_point(screen_point p)
{
    world_point res;
    res.coords[0] = (p.coords[0] - w / 2.0f);
    res.coords[1] = (h / 2.0f - p.coords[1]);
    res.coords[2] = 1.0f;
    return res;
}

world_point from_viewport(world_point p)
{
    world_point res;
    res.coords[0] = p.coords[0] * view_port_w / w;
    res.coords[1] = p.coords[1] * view_port_h / h;
    res.coords[2] = 1.0f;
    return res;
}

void trace(const int canvas_width, const int canvas_height, put_pixel_callback put_pixel)
{
    w = canvas_width;
    h = canvas_height;
    world_sphere sphere;
    zero(&sphere.center);
    sphere.radius = 10;
    for (int row = 0; row < h; ++row)
    {
        for (int col = 0; col < w; ++col)
        {
            screen_point pixel_loc;
            pixel_loc.coords[0] = col;
            pixel_loc.coords[1] = row;
            world_point p = from_viewport(viewport_point(pixel_loc));
            world_line line;
            zero(&line.origin);
            line.dir = p;
            float roots[2];
            if (intersect_line_with_sphere(&line, &sphere, roots))
            {
                color c;
                c.channels[0] = 0;
                c.channels[1] = 255;
                c.channels[2] = 0;
                if (put_pixel)
                    put_pixel(pixel_loc, c);
                continue;
            }
            color c;
            c.channels[0] = 0;
            c.channels[1] = 0;
            c.channels[2] = 0;
            if (put_pixel)
                put_pixel(pixel_loc, c);
        }
    }
}
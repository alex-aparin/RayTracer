#include "ray_tracer.h"
#include "graphical_object.h"

static int w = 500;
static int h = 500;
static float view_port_w = 500;
static float view_port_h = 500;
#define GRAPHICAL_OBJECTS_COUNT 2
#define LIGHT_OBJECTS_COUNT 1

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

int find_nearest_object_intersection(const world_line line, graphic_object* objects, int count, float* t)
{
    int object_index = -1;
    for (int i = 0; i < count; ++i)
    {
        float roots[2];
        const int roots_count = (int)objects[i].intersect_func(objects[i].instance, &line, roots);
        if (roots_count == 0)
            continue;
        for (int root_index = 0; root_index < roots_count; ++root_index)
        {
            if (roots[root_index] < 1.0f)
                continue;
            if (object_index == -1 || roots[root_index] < *t)
            {
                object_index = i;
                *t = roots[root_index];
            }
        }
    }
    return object_index;
}

void trace(const int canvas_width, const int canvas_height, put_pixel_callback put_pixel)
{
    if (!put_pixel)
        return;
    w = canvas_width;
    h = canvas_height;
    light_object light_objects[LIGHT_OBJECTS_COUNT];
    {
        world_point location;
        zero(&location);
        location.coords[2] = 100;
        light_objects[0] = create_point_light(location, 0.8);
    }
    graphic_object graphical_objects[GRAPHICAL_OBJECTS_COUNT];
    {
        world_point sphere_center = { 0.0, 0.0, 10.005f };
        color_t sphere_color = { 0, 255, 0 };
        graphical_objects[0] = create_sphere_object(sphere_center, 200, sphere_color);
    }
    {
        world_point sphere_center = { 0.0, 0.0, 10.008f };
        color_t sphere_color = { 255, 0, 0 };
        graphical_objects[1] = create_sphere_object(sphere_center, 320, sphere_color);
    }
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
            float t = 0;
            int object_index = find_nearest_object_intersection(line,  graphical_objects, GRAPHICAL_OBJECTS_COUNT, &t);
            if (object_index != -1)
            {
                const material_t material = graphical_objects[object_index].material_func(graphical_objects[object_index].instance, line_point(line, t));
                const color_t color = mul_color_by_factor(material.color, compute_light_intensity(light_objects, LIGHT_OBJECTS_COUNT, material.normal, p));
                put_pixel(pixel_loc, color);
                continue;
            }
            color_t c;
            c.channels[0] = 0;
            c.channels[1] = 0;
            c.channels[2] = 0;
            if (put_pixel)
                put_pixel(pixel_loc, c);
        }
    }
    for (int i = 0; i < LIGHT_OBJECTS_COUNT; ++i)
    {
        light_objects[i].destroy_func(light_objects[i].instance);
    }
    for (int i = 0; i < GRAPHICAL_OBJECTS_COUNT; ++i)
    {
        graphical_objects[i].destroy_func(graphical_objects[i].instance);
    }
}
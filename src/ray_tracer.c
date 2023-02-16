#include "ray_tracer.h"
#include "graphical_object.h"

static float view_port_w = 1;
static float view_port_h = 1;
#define GRAPHICAL_OBJECTS_COUNT 3
#define LIGHT_OBJECTS_COUNT 2

world_point viewport_point(screen_point p, const int canvas_width, const int canvas_height)
{
    world_point res;
    res.coords[0] = (p.coords[0] - canvas_width / 2.0f);
    res.coords[1] = (canvas_height / 2.0f - p.coords[1]);
    res.coords[2] = 1.0f;
    return res;
}

world_point from_viewport(world_point p, const int canvas_width, const int canvas_height)
{
    world_point res;
    res.coords[0] = p.coords[0] * view_port_w / canvas_width;
    res.coords[1] = p.coords[1] * view_port_h / canvas_height;
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
    light_object light_objects[LIGHT_OBJECTS_COUNT];
    {
        light_objects[0] = create_ambient_light(0.2f);
    }
    {
        world_point location;
        zero(&location);
        location.coords[0] = 2;
        location.coords[1] = 1;
        location.coords[2] = 0;
        light_objects[1] = create_point_light(location, 0.6f);
    }
    graphic_object graphical_objects[GRAPHICAL_OBJECTS_COUNT];
    {
        world_point sphere_center = { 0.0, -1.0f, 3.0f };
        color_t sphere_color = { 255, 0, 0 };
        graphical_objects[0] = create_sphere_object(sphere_center, 1, sphere_color, 10);
    }
    {
        world_point sphere_center = { 2.0, 0.0, 4.0f };
        color_t sphere_color = { 0, 0, 255 };
        graphical_objects[1] = create_sphere_object(sphere_center, 1, sphere_color, 10);
    }
    {
        world_point sphere_center = { -2.0, 0.0, 4.0f };
        color_t sphere_color = { 0, 255, 0};
        graphical_objects[2] = create_sphere_object(sphere_center, 1, sphere_color, 10);
    }
    for (int row = 0; row < canvas_height; ++row)
    {
        for (int col = 0; col < canvas_width; ++col)
        {
            screen_point pixel_loc;
            pixel_loc.coords[0] = col;
            pixel_loc.coords[1] = row;
            world_point p = from_viewport(viewport_point(pixel_loc, canvas_width, canvas_height), canvas_width, canvas_height);
            world_line line;
            zero(&line.origin);
            line.dir = p;
            float t = 0;
            int object_index = find_nearest_object_intersection(line,  graphical_objects, GRAPHICAL_OBJECTS_COUNT, &t);
            if (object_index != -1)
            {
                const world_point surface_point = line_point(line, t);
                const material_t material = graphical_objects[object_index].material_func(graphical_objects[object_index].instance, surface_point);
                const float light_intensity = compute_light_intensity(light_objects, LIGHT_OBJECTS_COUNT, surface_point, material, p);
                const color_t color = mul_color_by_factor(material.color, light_intensity);
                put_pixel(pixel_loc, color);
                continue;
            }
            color_t c;
            c.channels[0] = 150;
            c.channels[1] = 150;
            c.channels[2] = 150;
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
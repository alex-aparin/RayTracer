#include "ray_tracer.h"
#include "graphical_object.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

static float view_port_w = 1;
static float view_port_h = 1;
#define GRAPHICAL_OBJECTS_COUNT 3
#define LIGHT_OBJECTS_COUNT 2
#define T_EPS 0.00001f

typedef struct _scene_t scene_t;

typedef float (*intensity_getter_func)(void* instance, scene_t* scene, const world_point point, const material_t material, const world_point view_vector);
typedef void(*destroy_light_instance_func)(void*);
typedef struct
{
    void* instance;
    intensity_getter_func intensity_func;
    destroy_light_instance_func destroy_func;
} light_object;

light_object create_ambient_light(float intensity);
light_object create_point_light(const world_point location, float intensity);
light_object create_directed_light(const world_point direction, float intensity);
int find_nearest_object_intersection(const world_line line, scene_t* scene, float tmin, float tmax, float* t);

struct _scene_t
{
    light_object light_objects[LIGHT_OBJECTS_COUNT];
    graphic_object graphical_objects[GRAPHICAL_OBJECTS_COUNT];
};

typedef struct
{
    float intensity;
} ambient_light_object;

typedef struct
{
    world_point location;
    float intensity;
} point_light_object;

typedef struct
{
    world_point direction;
    float intensity;
} directed_light_object;

void destroy_light_object(void* light_object)
{
    free(light_object);
}

float compute_diffuse_light(const world_point light_direction, const material_t material, const float intensity)
{
    const float cos_a = scalar_product(material.normal, light_direction) / length(material.normal) / length(light_direction);
    if (cos_a <= 0.0f)
        return 0.0f;
    return cos_a * intensity;
}

float compute_specular_light(const world_point light_direction, const material_t material, const float intensity, const world_point view_vector)
{
    if (material.specularity == -1)
        return 0.0f;
    const world_point reflected_light_dir = reflect(light_direction, material.normal);
    const float cos_a = scalar_product(reflected_light_dir, mul_by_factor(view_vector, -1.0f)) / length(reflected_light_dir) / length(view_vector);
    if (cos_a <= 0.0f)
        return 0.0f;
    return (float)pow(cos_a, (float)material.specularity) * intensity;
}

float ambient_light_intensity(void* instance, scene_t* scene, const world_point point, const material_t material, const world_point view_vector)
{
    return ((ambient_light_object*)instance)->intensity;
}

float point_light_intensity(void* instance, scene_t* scene, const world_point point, const material_t material, const world_point view_vector)
{
    const point_light_object* const point_light = (point_light_object*)instance;
    const world_point light_dir = sub(point_light->location, point);
    const float intensity = point_light->intensity;
    float t = 0;
    if (find_nearest_object_intersection(create_line(point, light_dir), scene, T_EPS, FLT_MAX, &t) != -1)
        return 0.0f;
    return compute_diffuse_light(light_dir, material, intensity) + compute_specular_light(light_dir, material, intensity, view_vector);
}

float directed_light_intensity(void* instance, scene_t* scene, const world_point point, const material_t material, const world_point view_vector)
{
    const directed_light_object* const directed_light = (directed_light_object*)instance;
    const world_point light_dir = mul_by_factor(directed_light->direction, -1);
    const float intensity = directed_light->intensity;
    float t = 0;
    if (find_nearest_object_intersection(create_line(point, light_dir), scene, T_EPS, FLT_MAX, &t) != -1)
        return 0.0f;
    return compute_diffuse_light(light_dir, material, intensity) + compute_specular_light(light_dir, material, intensity, view_vector);
}

light_object create_ambient_light(float intensity_value)
{
    light_object light;
    ambient_light_object* ambient_light = malloc(sizeof(ambient_light_object));
    ambient_light->intensity = intensity_value;
    light.instance = ambient_light;
    light.intensity_func = ambient_light_intensity;
    light.destroy_func = destroy_light_object;
    return light;
}

light_object create_point_light(const world_point location, float intensity)
{
    light_object light;
    point_light_object* point_light = malloc(sizeof(point_light_object));
    point_light->location = location;
    point_light->intensity = intensity;
    light.instance = point_light;
    light.intensity_func = point_light_intensity;
    light.destroy_func = destroy_light_object;
    return light;
}

light_object create_directed_light(const world_point direction, float intensity)
{
    light_object light;
    directed_light_object* directed_light = malloc(sizeof(directed_light_object));
    directed_light->direction = direction;
    directed_light->intensity = intensity;
    light.instance = directed_light;
    light.intensity_func = directed_light_intensity;
    light.destroy_func = destroy_light_object;
    return light;
}

float compute_light_intensity(scene_t* scene, const world_point point, const material_t material, const world_point view_vector)
{
    if (LIGHT_OBJECTS_COUNT == 0)
        return 1.0f;
    float intensity = 0.0f;
    for (int i = 0; i < LIGHT_OBJECTS_COUNT; ++i)
    {
        intensity += scene->light_objects[i].intensity_func(scene->light_objects[i].instance, scene, point, material, view_vector);
    }
    return MIN(intensity, 1.0f);
}


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

int find_nearest_object_intersection(const world_line line, scene_t* scene, float tmin, float tmax, float* t)
{
    int object_index = -1;
    for (int i = 0; i < GRAPHICAL_OBJECTS_COUNT; ++i)
    {
        float roots[2];
        const int roots_count = (int)scene->graphical_objects[i].intersect_func(scene->graphical_objects[i].instance, &line, roots);
        if (roots_count == 0)
            continue;
        for (int root_index = 0; root_index < roots_count; ++root_index)
        {
            if (roots[root_index] < tmin || tmax < roots[root_index])
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

void init_scene(scene_t* scene)
{
    {
        scene->light_objects[0] = create_ambient_light(0.2f);
    }
    {
        world_point location;
        zero(&location);
        location.coords[0] = 2;
        location.coords[1] = 1;
        location.coords[2] = 0;
        scene->light_objects[1] = create_point_light(location, 0.6f);
    }
    {
        world_point sphere_center = { 0.0, -1.0f, 3.0f };
        color_t sphere_color = { 255, 0, 0 };
        scene->graphical_objects[0] = create_sphere_object(sphere_center, 1, sphere_color, 500, 0.2f);
    }
    {
        world_point sphere_center = { 2.0, 0.0, 4.0f };
        color_t sphere_color = { 0, 0, 255 };
        scene->graphical_objects[1] = create_sphere_object(sphere_center, 1, sphere_color, 1000, 0.25f);
    }
    {
        world_point sphere_center = { -2.0, 0.0, 4.0f };
        color_t sphere_color = { 0, 255, 0 };
        scene->graphical_objects[2] = create_sphere_object(sphere_center, 1, sphere_color, 10, 0.3f);
    }
}

void destroy_scene(scene_t* scene)
{
    for (int i = 0; i < LIGHT_OBJECTS_COUNT; ++i)
    {
        scene->light_objects[i].destroy_func(scene->light_objects[i].instance);
    }
    for (int i = 0; i < GRAPHICAL_OBJECTS_COUNT; ++i)
    {
        scene->graphical_objects[i].destroy_func(scene->graphical_objects[i].instance);
    }
}

color_t trace_ray(scene_t* scene, const world_line ray, const float tmin, const float tmax, const int recursion_depth)
{
    float t = 0;
    int object_index = find_nearest_object_intersection(ray, scene, tmin, tmax, &t);
    if (object_index == -1)
    {
        color_t c;
        c.channels[0] = 0;
        c.channels[1] = 0;
        c.channels[2] = 0;
        return c;
    }
    const world_point surface_point = line_point(ray, t);
    const material_t material = scene->graphical_objects[object_index].material_func(scene->graphical_objects[object_index].instance, surface_point);
    const float light_intensity = compute_light_intensity(scene, surface_point, material, ray.dir);
    const color_t color = mul_color_by_factor(material.color, light_intensity);
    if (recursion_depth <= 0 || material.reflectivity <= 0 || material.reflectivity > 1)
        return color;
    const color_t reflected_color = trace_ray(scene, create_line(surface_point, reflect(mul_by_factor(ray.dir, -1.0f), material.normal)), 
        T_EPS, FLT_MAX, recursion_depth - 1);
    return lerp_color(color, reflected_color, material.reflectivity);
}

void trace(const int canvas_width, const int canvas_height, put_pixel_callback put_pixel)
{
    if (!put_pixel)
        return;
    scene_t scene;
    init_scene(&scene);
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
            const color_t c = trace_ray(&scene, line, 1.0f, FLT_MAX, 2);
            put_pixel(pixel_loc, c);
        }
    }
    destroy_scene(&scene);
}
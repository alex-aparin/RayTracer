#include "ray_tracer.h"
#include "objects.h"

typedef struct
{
    world_sphere sphere;
    material_t material;
} sphere_object_t;

static intersection_result intersect_sphere_object(void* instance, const world_line* const line, float* const roots)
{
    const sphere_object_t* const sphere_object = (const sphere_object_t*)instance;
    return intersect_line_with_sphere(line, &sphere_object->sphere, roots);
}

static material_t sphere_material_getter(void* instance, const world_point point)
{
    const sphere_object_t* const sphere_object = (const sphere_object_t*)instance;
    material_t material = sphere_object->material;
    material.normal = normalize(sub(point, sphere_object->sphere.center));
    return material;
}

static void destroy_sphere_object(void* instance)
{
    free(instance);
}

graphic_object create_sphere_object(world_point center, float radius, color_t color, int specularity, float reflectivity)
{
    graphic_object res;
    sphere_object_t* const sphere_object = malloc(sizeof(sphere_object_t));
    sphere_object->sphere.center = center;
    sphere_object->sphere.radius = radius;
    sphere_object->material.color = color;
    sphere_object->material.normal = center;
    sphere_object->material.specularity = specularity;
    sphere_object->material.reflectivity = reflectivity;
    res.instance = sphere_object;
    res.intersect_func = intersect_sphere_object;
    res.material_func = sphere_material_getter;
    res.destroy_func = destroy_sphere_object;
    return res;
}

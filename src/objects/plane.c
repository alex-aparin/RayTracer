#include "ray_tracer.h"
#include "objects.h"

typedef struct
{
    world_plane plane;
    material_t material;
} plane_object_t;

static intersection_result intersect_plane_object(void* instance, const world_line* const line, float* const roots)
{
    const plane_object_t* const plane_object = (const plane_object_t*)instance;
    return intersect_line_with_plane(line, &plane_object->plane, roots);
}

static material_t plane_material_getter(void* instance, const world_point point)
{
    const plane_object_t* const plane_object = (const plane_object_t*)instance;
    return plane_object->material;
}

static void destroy_plane_object(void* instance)
{
    free(instance);
}

graphic_object create_plane_object(world_point normal, float D, color_t color, int specularity, float reflectivity)
{
    graphic_object res;
    plane_object_t* const plane_object = malloc(sizeof(plane_object_t));
    plane_object->plane.normal = normal;
    plane_object->plane.D = D;
    plane_object->material.color = color;
    plane_object->material.normal = normal;
    plane_object->material.specularity = specularity;
    plane_object->material.reflectivity = reflectivity;
    res.instance = plane_object;
    res.intersect_func = intersect_plane_object;
    res.material_func = plane_material_getter;
    res.destroy_func = destroy_plane_object;
    return res;
}

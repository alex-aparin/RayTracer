#include "ray_tracer.h"
#include "demo.h"

#define GRAPHICAL_OBJECTS_COUNT 3
#define LIGHT_OBJECTS_COUNT 2

static light_object light_objects[LIGHT_OBJECTS_COUNT];
static graphic_object graphical_objects[GRAPHICAL_OBJECTS_COUNT];

static void destroy_scene_object(void* instance)
{
    free(instance);
}

typedef struct
{
    world_sphere sphere;
    int specularity;
    float reflectivity;
} gradient_sphere_t;

static intersection_result intersect_gradient_sphere(void* instance, const world_line* const line, float* const roots)
{
    const gradient_sphere_t* const sphere = (const gradient_sphere_t*)instance;
    return intersect_line_with_sphere(line, &sphere->sphere, roots);
}

static material_t gradient_sphere_material_getter(void* instance, const world_point point)
{
    gradient_sphere_t* const sphere = (gradient_sphere_t*)instance;
    const color_t top_color = {{ 255, 247, 196 }};
    const color_t bottom_color = {{ 207, 28, 83 }};
    const float t = (sphere->sphere.center.coords[1] + sphere->sphere.radius - point.coords[1]) / 2 / sphere->sphere.radius;
    material_t material;
    material.color = lerp_color(top_color, bottom_color, t);
    material.normal = normalize(sub(point, sphere->sphere.center));
    if (t < 0.8)
    {
        material.specularity = -1;
        material.reflectivity = 0;
    }
    else
    {
        material.specularity = sphere->specularity;
        material.reflectivity = sphere->reflectivity;
    }
    return material;
}

static graphic_object create_gradient_sphere(world_point center, float radius, int specularity, float reflectivity)
{
    graphic_object res;
    gradient_sphere_t* const sphere = malloc(sizeof(gradient_sphere_t));
    sphere->sphere.center = center;
    sphere->sphere.radius = radius;
    sphere->specularity = specularity;
    sphere->reflectivity = reflectivity;
    res.instance = sphere;
    res.intersect_func = intersect_gradient_sphere;
    res.material_func = gradient_sphere_material_getter;
    res.destroy_func = destroy_scene_object;
    return res;
}

typedef struct
{
    world_plane plane;
} earth_object_t;

static intersection_result intersect_earth_object(void* instance, const world_line* const line, float* const roots)
{
    earth_object_t* const earth = (earth_object_t*)instance;
    return intersect_line_with_plane(line, &earth->plane, roots);
}

static material_t earth_material_getter(void* instance, const world_point point)
{
    const float line_width = 0.02f;
    const float quad_width = 0.2f;
    const float width_channel = (float)fabs(point.coords[0]);
    const float height_channel = point.coords[2];
    float relative_w = width_channel / (quad_width + line_width);
    relative_w -= truncf(relative_w);
    float relative_h = height_channel / (quad_width + line_width);
    relative_h -= truncf(relative_h);
    material_t material;
    if (relative_h > (line_width / (quad_width + line_width)) && relative_w > (line_width / (quad_width + line_width)))
    {
        //	cell content
        color_t cell_color = {{ 18, 0, 98 }};
        material.color = cell_color;
    }
    else
    {
        //	border
        color_t border_color = {{ 209, 0, 133 }};
        material.color = border_color;
    }
    earth_object_t* earth = (earth_object_t*)(instance);
    material.normal = earth->plane.normal;
    material.specularity = 500;
    material.reflectivity = 0.5f;
    return material;
}

static graphic_object create_earth_object()
{
    graphic_object res;
    earth_object_t* const earth = malloc(sizeof(earth_object_t));
    earth->plane.D = 0.5;
    zero(&earth->plane.normal);
    earth->plane.normal.coords[1] = 1.0f;
    res.instance = earth;
    res.intersect_func = intersect_earth_object;
    res.material_func = earth_material_getter;
    res.destroy_func = destroy_scene_object;
    return res;
}

#define MOUNTAINS_VERTICES_COUNT 7
typedef struct
{
    world_point countour[MOUNTAINS_VERTICES_COUNT];
} mountains_t;

static intersection_result intersect_mountains_object(void* instance, const world_line* const line, float* const roots)
{
    mountains_t* const mountains = (mountains_t*)instance;
    return intersect_line_with_poly(line, mountains->countour, MOUNTAINS_VERTICES_COUNT, roots);
}

static material_t mountains_material_getter(void* instance, const world_point point)
{
    material_t material;
    color_t color = {{ 30, 0, 71 }};
    material.color = color;
    zero(&material.normal);
    material.normal.coords[2] = -1.0f;
    material.specularity = -1;
    material.reflectivity = 0.5f;
    return material;
}

static graphic_object create_mountains()
{
    graphic_object res;
    mountains_t* const mountains = malloc(sizeof(mountains_t));
    res.instance = mountains;
    res.intersect_func = intersect_mountains_object;
    res.material_func = mountains_material_getter;
    res.destroy_func = destroy_scene_object;

    //	Countour initialization
    float z_coord = 10;
    mountains->countour[0].coords[0] = -10.0f; mountains->countour[0].coords[1] = -0.51f; mountains->countour[0].coords[2] = z_coord;
    mountains->countour[1].coords[0] = 10.0f; mountains->countour[1].coords[1] = -0.51f; mountains->countour[1].coords[2] = z_coord;
    mountains->countour[2].coords[0] = 5.0f; mountains->countour[2].coords[1] = 0.7f; mountains->countour[2].coords[2] = z_coord;
    mountains->countour[3].coords[0] = 1.6f; mountains->countour[3].coords[1] = -0.1f; mountains->countour[3].coords[2] = z_coord;
    mountains->countour[4].coords[0] = 0.3f; mountains->countour[4].coords[1] = 0.1f; mountains->countour[4].coords[2] = z_coord;
    mountains->countour[5].coords[0] = -1.1f; mountains->countour[5].coords[1] = -0.1f; mountains->countour[5].coords[2] = z_coord;
    mountains->countour[6].coords[0] = -10.0f; mountains->countour[6].coords[1] = -0.3f; mountains->countour[6].coords[2] = z_coord;

    return res;
}

void demo_scene_init(scene_t* scene)
{
    scene->light_objects = light_objects;
    scene->lights_count = LIGHT_OBJECTS_COUNT;
    scene->graphical_objects = graphical_objects;
    scene->objects_count = GRAPHICAL_OBJECTS_COUNT;

    {
        light_objects[0] = create_ambient_light(0.2f);
    }
    {
        world_point location;
        zero(&location);
        location.coords[0] = 0;
        location.coords[1] = 2;
        location.coords[2] = 7;
        light_objects[1] = create_point_light(location, 0.8f);
    }
    {
        world_point sphere_center = {{ 0.0, 0.5f, 14.0f }};
        graphical_objects[0] = create_gradient_sphere(sphere_center, 1.5, 500, 0.2f);
    }
    {
        graphical_objects[1] = create_earth_object();
    }
    {
        graphical_objects[2] = create_mountains();
    }
}

void demo_scene_destroy(scene_t* scene)
{
    scene->objects_count = 0;
    scene->graphical_objects = 0;
    scene->lights_count = 0;
    scene->light_objects = 0;
    for (int i = 0; i < LIGHT_OBJECTS_COUNT; ++i)
    {
        light_objects[i].destroy_func(light_objects[i].instance);
    }
    for (int i = 0; i < GRAPHICAL_OBJECTS_COUNT; ++i)
    {
        graphical_objects[i].destroy_func(graphical_objects[i].instance);
    }
}

#include "ray_tracer.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

static float view_port_w = 1;
static float view_port_h = 1;
#define T_EPS 0.00001f

static float  det(const float a11, const float a12, const float a21, const float a22)
{
    return a11 * a22 - a12 * a21;
}

void zero(world_point* const p)
{
    if (!p)
        return;
    p->coords[0] = 0;
    p->coords[1] = 0;
    p->coords[2] = 0;
}

world_point sum(world_point const p1, world_point const p2)
{
    world_point res;
    res.coords[0] = p1.coords[0] + p2.coords[0];
    res.coords[1] = p1.coords[1] + p2.coords[1];
    res.coords[2] = p1.coords[2] + p2.coords[2];
    return res;
}

world_point sub(world_point const p1, world_point const p2)
{
    world_point res;
    res.coords[0] = p1.coords[0] - p2.coords[0];
    res.coords[1] = p1.coords[1] - p2.coords[1];
    res.coords[2] = p1.coords[2] - p2.coords[2];
    return res;
}

color_t mul_color_by_factor(color_t const color, float factor)
{
    color_t res = color;
    res.channels[0] = RAY_TRACER_MIN(255, res.channels[0] * factor);
    res.channels[1] = RAY_TRACER_MIN(255, res.channels[1] * factor);
    res.channels[2] = RAY_TRACER_MIN(255, res.channels[2] * factor);
    return res;
}

world_point mul_by_factor(world_point const p1, float factor)
{
    world_point res;
    res.coords[0] = p1.coords[0] * factor;
    res.coords[1] = p1.coords[1] * factor;
    res.coords[2] = p1.coords[2] * factor;
    return res;
}

world_point reflect(world_point dir, world_point normal)
{
    return sub(mul_by_factor(normal, 2.0f * scalar_product(dir, normal)), dir);
}

world_line create_line(world_point const origin, world_point const dir)
{
    world_line line;
    line.origin = origin;
    line.dir = dir;
    return line;
}

float length(const world_point p)
{
    return sqrt(scalar_product(p, p));
}

world_point normalize(const world_point p)
{
    return mul_by_factor(p, 1.0f / length(p));
}

world_point line_point(world_line line, float t)
{
    world_point res = sum(line.origin, mul_by_factor(line.dir, t));
    return res;
}

float scalar_product(world_point const p1, world_point const p2)
{
    const float* const arr1 = p1.coords;
    const float* const arr2 = p2.coords;
    return arr1[0] * arr2[0] + arr1[1] * arr2[1] + arr1[2] * arr2[2];
}

intersection_result intersect_line_with_sphere(const world_line* const line, world_sphere* const sphere, float* const t)
{
    if (!line || !sphere || !t)
        return NOT_INTERSECTED;
    const world_point delta = sub(line->origin, sphere->center);
    return solve_quadratic(scalar_product(line->dir, line->dir), scalar_product(line->dir, mul_by_factor(delta, 2.0f)), scalar_product(delta, delta) - sphere->radius * sphere->radius, t);
}

intersection_result intersect_line_with_plane(const world_line* const line, world_plane* const plane, float* const t)
{
    const float a = scalar_product(line->dir, plane->normal);
    const float b = scalar_product(line->origin, plane->normal) + plane->D;
    if (a == 0.0f)
    {
        if (a == b)
        {
            *t = 0.0f;
            return INTERSECTED;
        }
        return NOT_INTERSECTED;
    }
    *t = -b / a;
    return INTERSECTED;
}

intersection_result intersect_ray_with_line(const world_line* const ray, const world_line* const line2, float* const t)
{
    if (!ray || !line2)
        return NOT_INTERSECTED;
    const float a11 = ray->dir.coords[0];
    const float a12 = -line2->dir.coords[0];
    const float b1 = line2->origin.coords[0] - ray->origin.coords[0];
    const float a21 = ray->dir.coords[1];
    const float a22 = -line2->dir.coords[1];
    const float b2 = line2->origin.coords[1] - ray->origin.coords[1];
    const float main_det = det(a11, a12, a21, a22);
    if (main_det != 0.0f)
    {
        const float t1 = det(b1, a12, b2, a22) / main_det;
        const float t2 = det(a11, b1, a21, b2) / main_det;
        if (t1 >= 0.0f && 0.0f <= t2 && t2 <= 1.0f)
        {
            *t = t1;
            return INTERSECTED;
        }
    }
    return NOT_INTERSECTED;
}

intersection_result intersect_line_with_poly(const world_line* const line, const world_point* vertices, const int count, float* const t)
{
    if (!vertices || count < 3)
        return 0;
    world_plane poly_plane;
    zero(&poly_plane.normal);
    poly_plane.D = -vertices[0].coords[2];
    poly_plane.normal.coords[2] = 1.0f;
    if (!intersect_line_with_plane(line, &poly_plane, t))
        return NOT_INTERSECTED;
    world_line ray;
    zero(&ray.dir);
    ray.dir.coords[0] = 0.6f;
    ray.dir.coords[1] = 1.0f;
    ray.origin = line_point(*line, *t);
    float temp_t = 0;
    int res = 0;
    for (int i = 0; i < count; ++i)
    {
        world_line segment;
        segment.origin = vertices[i];
        segment.dir = sub(vertices[(i + 1) % count], vertices[i]);
        if (intersect_ray_with_line(&ray, &segment, &temp_t))
            ++res;
    }
    return res % 2;
}
int solve_quadratic(float a, float b, float c, float* const t)
{
    const float d = b * b - 4.0f * a * c;
    if (!t || d < 0)
        return 0;
    if (d == 0)
    {
        t[0] = -b / 2.0f / a;
        return 1;
    }
    const float sqrt_d = (float)sqrt(d);
    t[0] = (-b - sqrt_d) / 2.0f / a;
    t[1] = (-b + sqrt_d) / 2.0f / a;
    return 2;
}

color_t lerp_color(const color_t lhs, const color_t rhs, const float t)
{
    color_t res;
    res.channels[0] = lhs.channels[0] + t * (rhs.channels[0] - lhs.channels[0]);
    res.channels[1] = lhs.channels[1] + t * (rhs.channels[1] - lhs.channels[1]);
    res.channels[2] = lhs.channels[2] + t * (rhs.channels[2] - lhs.channels[2]);
    return res;
}

int find_nearest_object_intersection(const world_line line, scene_t* scene, float tmin, float tmax, float* t);


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
    return RAY_TRACER_MIN(intensity, 1.0f);
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

color_t trace_ray(scene_t* scene, const world_line ray, const float tmin, const float tmax, const int recursion_depth)
{
    float t = 0;
    int object_index = find_nearest_object_intersection(ray, scene, tmin, tmax, &t);
    if (object_index == -1)
    {
        color_t c = { 0, 0, 0 };
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
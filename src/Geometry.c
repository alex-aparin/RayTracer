#include  <math.h>
#include "Geometry.h"

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
    res.channels[0] = MIN(255, res.channels[0] * factor);
    res.channels[1] = MIN(255, res.channels[1] * factor);
    res.channels[2] = MIN(255, res.channels[2] * factor);
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
    if (!vertices || count <= 3)
        return 0;
    world_plane poly_plane;
    zero(&poly_plane.normal);
    poly_plane.D = vertices[0].coords[3];
    poly_plane.normal.coords[2] = 1.0f;
    if (!intersect_line_with_plane(line, &poly_plane, t))
        return NOT_INTERSECTED;
    world_line ray;
    zero(&ray.dir);
    ray.dir.coords[0] = 1.0f;
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
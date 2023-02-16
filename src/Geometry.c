#include  <math.h>
#include "Geometry.h"

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

intersection_result intersect(const world_line* const line, world_rect* const rect, float* const t)
{
    return INTERSECTED;
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
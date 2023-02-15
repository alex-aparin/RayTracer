#ifndef GEOMETRY_H_INCLUDED__
#define GEOMETRY_H_INCLUDED__

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef enum
{
    NOT_INTERSECTED = 0x0,
    INTERSECTED = 0x1
} intersection_result;

typedef struct
{
    unsigned char channels[3];
} color_t;

typedef struct
{
    float coords[3];
} world_point;

typedef struct
{
    color_t color;
    world_point normal;
} material_t;

typedef world_point world_vector;

typedef struct
{
    world_point origin;
    world_point dir;
} world_line;

typedef struct
{
    world_point center;
    float radius;
} world_sphere;

typedef struct
{
    world_point origin;
    world_point size;
} world_rect;

typedef struct
{
    int coords[2];
} screen_point;

void zero(world_point* const p);
world_point sum(world_point const p1, world_point const p2);
world_point sub(world_point const p1, world_point const p2);
color_t mul_color_by_factor(color_t const color, float factor);
world_point mul_by_factor(world_point const p1, float factor);
float length(const world_point p);
world_point normalize(const world_point p);
world_point line_point(world_line line, float t);
float scalar_product(world_point const p1, world_point const p2);
intersection_result intersect_line_with_sphere(const world_line* const line, world_sphere* const sphere, float* const t);
intersection_result intersect(const world_line* const line, world_rect* const rect, float* const t);
int solve_quadratic(float a, float b, float c, float* const t);

#endif
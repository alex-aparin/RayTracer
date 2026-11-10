#include "ray_tracer.h"
#include "objects.h"
#include "chess_balls.h"

#define GRAPHICAL_OBJECTS_COUNT 6
#define LIGHT_OBJECTS_COUNT 2

#define BOARD_LEVEL -0.5f
#define CELL_SIZE 0.5f
#define SKY_DISTANCE 25.0f

static light_object light_objects[LIGHT_OBJECTS_COUNT];
static graphic_object graphical_objects[GRAPHICAL_OBJECTS_COUNT];

typedef struct
{
    world_plane plane;
} chessboard_t;

static intersection_result intersect_chessboard(void* instance, const world_line* const line, float* const roots)
{
    chessboard_t* const board = (chessboard_t*)instance;
    return intersect_line_with_plane(line, &board->plane, roots);
}

static material_t chessboard_material_getter(void* instance, const world_point point)
{
    const int cell_x = (int)floorf(point.coords[0] / CELL_SIZE);
    const int cell_z = (int)floorf(point.coords[2] / CELL_SIZE);
    const color_t cell_color = ((cell_x + cell_z) & 1)
        ? (color_t){{ 238, 232, 214 }}
        : (color_t){{ 150, 140, 130 }};
    const color_t distant_color = {{ 194, 186, 172 }};
    //	fade the checkerboard into the average color with distance to suppress moire
    const float fade = fminf(fmaxf((length(point) - 8.0f) / 14.0f, 0.0f), 1.0f);
    material_t material;
    material.color = lerp_color(cell_color, distant_color, fade);
    material.normal = ((chessboard_t*)instance)->plane.normal;
    material.specularity = 200;
    material.reflectivity = 0.3f;
    return material;
}

static void destroy_chessboard(void* instance)
{
    free(instance);
}

static graphic_object create_chessboard(void)
{
    graphic_object res;
    chessboard_t* const board = malloc(sizeof(chessboard_t));
    board->plane.D = -BOARD_LEVEL;
    zero(&board->plane.normal);
    board->plane.normal.coords[1] = 1.0f;
    res.instance = board;
    res.intersect_func = intersect_chessboard;
    res.material_func = chessboard_material_getter;
    res.destroy_func = destroy_chessboard;
    return res;
}

#define SKY_VERTICES_COUNT 4
typedef struct
{
    world_point rect[SKY_VERTICES_COUNT];
} sky_t;

static intersection_result intersect_sky(void* instance, const world_line* const line, float* const roots)
{
    sky_t* const sky = (sky_t*)instance;
    return intersect_line_with_poly(line, sky->rect, SKY_VERTICES_COUNT, roots);
}

static material_t sky_material_getter(void* instance, const world_point point)
{
    const color_t horizon_color = {{ 255, 132, 78 }};
    const color_t mid_color = {{ 190, 70, 110 }};
    const color_t zenith_color = {{ 104, 74, 152 }};
    const float t = fminf(fmaxf((point.coords[1] + 0.5f) / 20.0f, 0.0f), 1.0f);
    material_t material;
    material.color = t < 0.5f
        ? lerp_color(horizon_color, mid_color, t * 2.0f)
        : lerp_color(mid_color, zenith_color, (t - 0.5f) * 2.0f);
    zero(&material.normal);
    material.normal.coords[2] = -1.0f;
    material.specularity = -1;
    material.reflectivity = 0.0f;
    return material;
}

static graphic_object create_sky(void)
{
    graphic_object res;
    sky_t* const sky = malloc(sizeof(sky_t));
    res.instance = sky;
    res.intersect_func = intersect_sky;
    res.material_func = sky_material_getter;
    res.destroy_func = destroy_chessboard;
    sky->rect[0].coords[0] = -20.0f; sky->rect[0].coords[1] = -0.5f; sky->rect[0].coords[2] = SKY_DISTANCE;
    sky->rect[1].coords[0] = 20.0f;  sky->rect[1].coords[1] = -0.5f; sky->rect[1].coords[2] = SKY_DISTANCE;
    sky->rect[2].coords[0] = 20.0f;  sky->rect[2].coords[1] = 15.0f;  sky->rect[2].coords[2] = SKY_DISTANCE;
    sky->rect[3].coords[0] = -20.0f; sky->rect[3].coords[1] = 15.0f;  sky->rect[3].coords[2] = SKY_DISTANCE;
    return res;
}

void chess_balls_scene_init(scene_t* scene)
{
    scene->light_objects = light_objects;
    scene->lights_count = LIGHT_OBJECTS_COUNT;
    scene->graphical_objects = graphical_objects;
    scene->objects_count = GRAPHICAL_OBJECTS_COUNT;

    {
        light_objects[0] = create_ambient_light(0.28f);
    }
    {
        world_point location = {{ -4.0f, 3.0f, 2.0f }};
        light_objects[1] = create_point_light(location, 0.8f);
    }
    {
        graphical_objects[0] = create_chessboard();
    }
    {
        graphical_objects[5] = create_sky();
    }
    {
        //	matte ball
        world_point center = {{ -1.6f, BOARD_LEVEL + 0.5f, 6.5f }};
        color_t color = {{ 196, 44, 44 }};
        graphical_objects[1] = create_sphere_object(center, 0.5f, color, -1, 0.0f);
    }
    {
        //	glossy ball
        world_point center = {{ -0.45f, BOARD_LEVEL + 0.4f, 8.5f }};
        color_t color = {{ 52, 160, 72 }};
        graphical_objects[2] = create_sphere_object(center, 0.4f, color, 250, 0.35f);
    }
    {
        //	mirror ball
        world_point center = {{ 0.9f, BOARD_LEVEL + 0.65f, 7.5f }};
        color_t color = {{ 228, 228, 232 }};
        graphical_objects[3] = create_sphere_object(center, 0.65f, color, 500, 0.75f);
    }
    {
        //	small metal ball in the foreground
        world_point center = {{ 0.15f, BOARD_LEVEL + 0.25f, 4.6f }};
        color_t color = {{ 72, 96, 200 }};
        graphical_objects[4] = create_sphere_object(center, 0.25f, color, 500, 0.6f);
    }
}

void chess_balls_scene_destroy(scene_t* scene)
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

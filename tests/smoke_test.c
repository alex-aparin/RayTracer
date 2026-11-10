#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ray_tracer.h"
#include "demo.h"
#include "chess_balls.h"

#define TEST_WIDTH 32
#define TEST_HEIGHT 24

static color_t frame[TEST_HEIGHT][TEST_WIDTH];

static void put_pixel(screen_point p, color_t pixel)
{
    frame[p.coords[1]][p.coords[0]] = pixel;
}

static void test_math(void)
{
    {
        const world_point p = {{ 3.0f, 4.0f, 0.0f }};
        assert(length(p) == 5.0f);
    }
    {
        const world_point n = normalize((world_point){{ 0.0f, 3.0f, 4.0f }});
        assert(length(n) > 0.999f && length(n) < 1.001f);
    }
    {
        const world_point r = reflect((world_point){{ 1.0f, -1.0f, 0.0f }}, (world_point){{ 0.0f, 1.0f, 0.0f }});
        assert(r.coords[0] == 1.0f && r.coords[1] == 1.0f && r.coords[2] == 0.0f);
    }
    {
        float t[2] = { 0, 0 };
        assert(solve_quadratic(1.0f, -3.0f, 2.0f, t) == 2);
        assert(t[0] == 1.0f && t[1] == 2.0f);
    }
    {
        //	ray from the origin along +z through a sphere at (0,0,5) with radius 1
        world_sphere sphere = {{{ 0.0f, 0.0f, 5.0f }}, 1.0f };
        world_point origin = {{ 0.0f, 0.0f, 0.0f }};
        world_line ray = create_line(origin, (world_point){{ 0.0f, 0.0f, 1.0f }});
        float t[2] = { 0, 0 };
        assert(intersect_line_with_sphere(&ray, &sphere, t) == 2);
        assert(t[0] == 4.0f && t[1] == 6.0f);
    }
}

static void test_scene(void (*scene_init)(scene_t*), void (*scene_destroy)(scene_t*))
{
    scene_t scene;
    scene_init(&scene);
    assert(scene.objects_count > 0 && scene.lights_count > 0);

    memset(frame, 0, sizeof(frame));
    trace(&scene, TEST_WIDTH, TEST_HEIGHT, put_pixel);

    int nonblack = 0;
    for (int j = 0; j < TEST_HEIGHT; ++j)
        for (int i = 0; i < TEST_WIDTH; ++i)
            if (frame[j][i].channels[0] || frame[j][i].channels[1] || frame[j][i].channels[2])
                ++nonblack;
    assert(nonblack > 0);

    //	rendering must be deterministic
    color_t first_frame[TEST_HEIGHT][TEST_WIDTH];
    memcpy(first_frame, frame, sizeof(frame));
    memset(frame, 0, sizeof(frame));
    trace(&scene, TEST_WIDTH, TEST_HEIGHT, put_pixel);
    assert(memcmp(first_frame, frame, sizeof(frame)) == 0);

    scene_destroy(&scene);
}

int main(void)
{
    test_math();
    test_scene(demo_scene_init, demo_scene_destroy);
    test_scene(chess_balls_scene_init, chess_balls_scene_destroy);
    printf("smoke_test: OK\n");
    return 0;
}

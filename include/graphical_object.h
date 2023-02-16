#ifndef GRAPHICAL_OBJECT_H_INCLUDED__
#define GRAPHICAL_OBJECT_H_INCLUDED__

#include "geometry.h"

typedef intersection_result(*intersect_with_line_func)(void*, const world_line* const, float* const t);
typedef material_t(*material_getter_func)(void*, const world_point);
typedef void(*destroy_instance_func)(void*);
typedef struct
{
	void* instance;
	intersect_with_line_func intersect_func;
	material_getter_func material_func;
	destroy_instance_func destroy_func;
} graphic_object;

typedef float (*intensity_getter_func)(void*, const world_point point, const material_t material, const world_point view_vector);
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

float compute_light_intensity(light_object* const lights, const int count, const world_point point, const material_t material, const world_point view_vector);

graphic_object create_sphere_object(world_point center, float radius, color_t color_value, int specularity);

#endif
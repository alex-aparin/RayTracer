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

graphic_object create_sphere_object(world_point center, float radius, color_t color_value, int specularity);

#endif
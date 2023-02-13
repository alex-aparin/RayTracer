#ifndef GRAPHICAL_OBJECT_H_INCLUDED__
#define GRAPHICAL_OBJECT_H_INCLUDED__

#include "geometry.h"

typedef intersection_result(*intersect_with_line_func)(void*, world_line* const, float* const t);
typedef void(*destroy_instance_func)(void*);
typedef struct
{
	void* instance;
	intersect_with_line_func intersect_func;
	destroy_instance_func destroy_func;
} graphic_object;

graphic_object create_sphere_object(world_point center, float radius, color color_value);

#endif
#include "graphical_object.h"
#include <stdlib.h>

typedef struct 
{
	world_sphere sphere;
	color color_value;
} SphereObject;


intersection_result intersect_sphere_object(void* instance, world_line* const line, float* const roots)
{
	SphereObject* sphere_object = (SphereObject*)(instance);
	return intersect_line_with_sphere(line, &sphere_object->sphere, roots);
}

void destroy_sphere_object(void* sphere_object)
{
	free(sphere_object);
}

graphic_object create_sphere_object(world_point center, float radius, color color_value)
{
	graphic_object res;
	SphereObject* sphere_object = (SphereObject*)malloc(sizeof(SphereObject));
	sphere_object->sphere.center = center;
	sphere_object->sphere.radius = radius;
	res.instance = sphere_object;
	res.intersect_func = intersect_sphere_object;
	res.destroy_func = destroy_sphere_object;
	return res;
}
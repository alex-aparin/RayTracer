#include "graphical_object.h"
#include <stdlib.h>

typedef struct 
{
	world_sphere sphere;
	color_t color;
	int specularity;
	float reflectivity;
} SphereObject;


intersection_result intersect_sphere_object(void* instance, const world_line* const line, float* const roots)
{
	SphereObject* sphere_object = (SphereObject*)(instance);
	return intersect_line_with_sphere(line, &sphere_object->sphere, roots);
}

material_t sphere_material_getter(void* instance, const world_point point)
{
	SphereObject* sphere_object = (SphereObject*)(instance);
	material_t material;
	material.color = sphere_object->color;
	material.normal = normalize(sub(point, sphere_object->sphere.center));
	material.specularity = sphere_object->specularity;
	material.reflectivity = sphere_object->reflectivity;
	return material;
}

void destroy_sphere_object(void* sphere_object)
{
	free(sphere_object);
}

graphic_object create_sphere_object(world_point center, float radius, color_t color, int specularity, float reflectivity)
{
	graphic_object res;
	SphereObject* sphere_object = malloc(sizeof(SphereObject));
	sphere_object->sphere.center = center;
	sphere_object->sphere.radius = radius;
	sphere_object->color = color;
	sphere_object->specularity = specularity;
	sphere_object->reflectivity = reflectivity;
	res.instance = sphere_object;
	res.intersect_func = intersect_sphere_object;
	res.material_func = sphere_material_getter;
	res.destroy_func = destroy_sphere_object;
	return res;
}
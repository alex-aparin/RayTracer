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

void destroy_base_object(void* base_graphical_object)
{
	free(base_graphical_object);
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
	res.destroy_func = destroy_base_object;
	return res;
}

typedef struct
{
	world_plane plane;
	color_t color;
} earth_object_t;

intersection_result intersect_earth_object(void* instance, const world_line* const line, float* const roots)
{
	earth_object_t* earth = (earth_object_t*)(instance);
	return intersect_line_with_plane(line, &earth->plane, roots);
}

material_t earth_material_getter(void* instance, const world_point point)
{
	earth_object_t* earth = (earth_object_t*)(instance);
	material_t material;
	material.color = earth->color;
	material.normal = earth->plane.normal;
	material.specularity = -1;
	material.reflectivity = 0.0f;
	return material;
}

graphic_object create_earth_object()
{
	graphic_object res;
	earth_object_t* earth = malloc(sizeof(earth_object_t));
	earth->color.channels[0] = 150;
	earth->color.channels[1] = 150;
	earth->color.channels[2] = 150;
	earth->plane.D = 3;
	zero(&earth->plane.normal);
	earth->plane.normal.coords[2] = 1.0f;
	res.instance = earth;
	res.intersect_func = intersect_earth_object;
	res.material_func = earth_material_getter;
	res.destroy_func = destroy_base_object;
	return res;
}
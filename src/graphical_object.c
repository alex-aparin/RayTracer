#include "graphical_object.h"
#include <stdlib.h>
#include <math.h>

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
	color_t top_color = { 255, 247, 196 };
	color_t bottom_color = { 207, 28, 83 };
	float t = (sphere_object->sphere.center.coords[1] + sphere_object->sphere.radius - point.coords[1]) / 2 / sphere_object->sphere.radius;
	material.color = lerp_color(top_color, bottom_color, t);
	material.normal = normalize(sub(point, sphere_object->sphere.center));
	
	if (t < 0.8)
	{
		material.specularity = -1;
		material.reflectivity = 0;
	}
	else
	{
		material.specularity = sphere_object->specularity;
		material.reflectivity = sphere_object->reflectivity;
	}
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
	float line_width = 0.05;
	float quad_width = 0.4;
	const float width_channel = fabs(point.coords[0]);
	const float height_channel = point.coords[2];
	float relative_w = width_channel / (quad_width + line_width);
	relative_w -= trunc(relative_w);
	float relative_h = height_channel / (quad_width + line_width);
	relative_h -= trunc(relative_h);
	material_t material;
	if (relative_h > (line_width / (quad_width + line_width)) && relative_w > (line_width / (quad_width + line_width)))
	{
		//	cell content
		material.color.channels[0] = 50;
		material.color.channels[1] = 50;
		material.color.channels[2] = 50;
	}
	else
	{
		//	border
		material.color.channels[0] = 177;
		material.color.channels[1] = 115;
		material.color.channels[2] = 242;
	}
	earth_object_t* earth = (earth_object_t*)(instance);
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
	earth->plane.D = 1;
	zero(&earth->plane.normal);
	earth->plane.normal.coords[1] = 1.0f;
	res.instance = earth;
	res.intersect_func = intersect_earth_object;
	res.material_func = earth_material_getter;
	res.destroy_func = destroy_base_object;
	return res;
}
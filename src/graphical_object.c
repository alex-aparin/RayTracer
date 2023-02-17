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
	float line_width = 0.02;
	float quad_width = 0.2;
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
		color_t cell_color = { 18, 0, 98 };
		material.color = cell_color;
		//material.color.channels[0] = 50;
		//material.color.channels[1] = 50;
		//material.color.channels[2] = 50;
	}
	else
	{
		//	border
		color_t border_color = { 209, 0, 133 };
		material.color = border_color;
		/*material.color.channels[0] = 177;
		material.color.channels[1] = 115;
		material.color.channels[2] = 242;*/
	}
	earth_object_t* earth = (earth_object_t*)(instance);
	material.normal = earth->plane.normal;
	material.specularity = 500;
	material.reflectivity = 0.5f;
	return material;
}

graphic_object create_earth_object()
{
	graphic_object res;
	earth_object_t* earth = malloc(sizeof(earth_object_t));
	earth->color.channels[0] = 150;
	earth->color.channels[1] = 150;
	earth->color.channels[2] = 150;
	earth->plane.D = 0.5;
	zero(&earth->plane.normal);
	earth->plane.normal.coords[1] = 1.0f;
	res.instance = earth;
	res.intersect_func = intersect_earth_object;
	res.material_func = earth_material_getter;
	res.destroy_func = destroy_base_object;
	return res;
}

#define MOUNTAINS_VERTICES_COUNT 7
typedef struct
{
	world_point countour[MOUNTAINS_VERTICES_COUNT];
} mountains_t;

intersection_result intersect_mountains_object(void* instance, const world_line* const line, float* const roots)
{
	mountains_t* mountains = (mountains_t*)(instance);
	return intersect_line_with_poly(line, mountains->countour, MOUNTAINS_VERTICES_COUNT, roots);
}

material_t mountains_material_getter(void* instance, const world_point point)
{
	material_t material;
	color_t color = { 30, 0, 71 };
	material.color = color;
	zero(&material.normal);
	material.normal.coords[2] = -1.0f;
	material.specularity = -1;
	material.reflectivity = 0.0f;
	return material;
}

graphic_object create_mountains()
{
	graphic_object res;
	mountains_t* mountains = malloc(sizeof(mountains_t));
	res.instance = mountains;
	res.intersect_func = intersect_mountains_object;
	res.material_func = mountains_material_getter;
	res.destroy_func = destroy_base_object;
	
	//	Countour initialization
	float z_coord = 10;
	mountains->countour[0].coords[0] = -10.0f; mountains->countour[0].coords[1] = -0.51f; mountains->countour[0].coords[2] = z_coord;
	mountains->countour[1].coords[0] = 10.0f; mountains->countour[1].coords[1] = -0.51f; mountains->countour[1].coords[2] = z_coord;
	mountains->countour[2].coords[0] = 5.0f; mountains->countour[2].coords[1] = 0.7f; mountains->countour[2].coords[2] = z_coord;
	mountains->countour[3].coords[0] = 1.6f; mountains->countour[3].coords[1] = -0.1f; mountains->countour[3].coords[2] = z_coord;
	mountains->countour[4].coords[0] = 0.3f; mountains->countour[4].coords[1] = 0.1f; mountains->countour[4].coords[2] = z_coord;
	mountains->countour[5].coords[0] = -1.1f; mountains->countour[5].coords[1] = -0.1f; mountains->countour[5].coords[2] = z_coord;
	mountains->countour[6].coords[0] = -10.0f; mountains->countour[6].coords[1] = -0.3f; mountains->countour[6].coords[2] = z_coord;

	return res;
}
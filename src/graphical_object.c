#include "graphical_object.h"
#include <stdlib.h>

typedef struct 
{
	world_sphere sphere;
	color_t color;
	int specularity;
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
	return material;
}

void destroy_sphere_object(void* sphere_object)
{
	free(sphere_object);
}

typedef struct
{
	float intensity;
} ambient_light_object;

typedef struct
{
	world_point location;
	float intensity;
} point_light_object;

typedef struct
{
	world_point direction;
	float intensity;
} directed_light_object;

void destroy_light_object(void* light_object)
{
	free(light_object);
}

float compute_diffuse_light(const world_point light_direction, const material_t material, const float intensity)
{
	const float cos_a = scalar_product(material.normal, light_direction) / length(material.normal) / length(light_direction);
	if (cos_a <= 0.0f)
		return 0.0f;
	return cos_a * intensity;
}

float compute_specular_light(const world_point light_direction, const material_t material, const float intensity, const world_point view_vector)
{
	if (material.specularity == -1)
		return 0.0f;
	const world_point reflected_light_dir = sub(mul_by_factor(material.normal, 2.0f * scalar_product(light_direction, material.normal)), light_direction);
	const float cos_a = scalar_product(reflected_light_dir, mul_by_factor(view_vector, -1.0f)) / length(reflected_light_dir) / length(view_vector);
	if (cos_a <= 0.0f)
		return 0.0f;
	return pow(cos_a, material.specularity) * intensity;
}

float ambient_light_intensity(void* instance, const world_point point, const material_t material, const world_point view_vector)
{
	return ((ambient_light_object*)instance)->intensity;
}

float point_light_intensity(void* instance, const world_point point, const material_t material, const world_point view_vector)
{
	const point_light_object* const point_light = (point_light_object*)instance;
	const world_point light_dir = sub(point_light->location, point);
	const float intensity = point_light->intensity;
	return compute_diffuse_light(light_dir, material, intensity) + compute_specular_light(light_dir, material, intensity, view_vector);
}

float directed_light_intensity(void* instance, const world_point point, const material_t material, const world_point view_vector)
{
	const directed_light_object* const directed_light = (directed_light_object*)instance;
	const world_point light_dir = mul_by_factor(directed_light->direction, -1);
	const float intensity = directed_light->intensity;
	return compute_diffuse_light(light_dir, material, intensity) + compute_specular_light(light_dir, material, intensity, view_vector);
}

light_object create_ambient_light(float intensity_value)
{
	light_object light;
	ambient_light_object* ambient_light = malloc(sizeof(ambient_light_object));
	ambient_light->intensity = intensity_value;
	light.instance = ambient_light;
	light.intensity_func = ambient_light_intensity;
	light.destroy_func = destroy_light_object;
	return light;
}

light_object create_point_light(const world_point location, float intensity)
{
	light_object light;
	point_light_object* point_light = malloc(sizeof(point_light_object));
	point_light->location = location;
	point_light->intensity = intensity;
	light.instance = point_light;
	light.intensity_func = point_light_intensity;
	light.destroy_func = destroy_light_object;
	return light;
}

light_object create_directed_light(const world_point direction, float intensity)
{
	light_object light;
	directed_light_object* directed_light = malloc(sizeof(directed_light_object));
	directed_light->direction = direction;
	directed_light->intensity = intensity;
	light.instance = directed_light;
	light.intensity_func = directed_light_intensity;
	light.destroy_func = destroy_light_object;
	return light;
}

float compute_light_intensity(light_object* const lights, const int count, const world_point point, const material_t material, const world_point view_vector)
{
	if (count == 0)
		return 1.0f;
	float intensity = 0.0f;
	for (int i = 0; i < count; ++i)
	{
		intensity += lights[i].intensity_func(lights[i].instance, point, material, view_vector);
	}
	return MIN(intensity, 1.0f);
}

graphic_object create_sphere_object(world_point center, float radius, color_t color, int specularity)
{
	graphic_object res;
	SphereObject* sphere_object = malloc(sizeof(SphereObject));
	sphere_object->sphere.center = center;
	sphere_object->sphere.radius = radius;
	sphere_object->color = color;
	sphere_object->specularity = specularity;
	res.instance = sphere_object;
	res.intersect_func = intersect_sphere_object;
	res.material_func = sphere_material_getter;
	res.destroy_func = destroy_sphere_object;
	return res;
}
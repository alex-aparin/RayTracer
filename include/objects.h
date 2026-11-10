#ifndef OBJECTS_H_INCLUDED__
#define OBJECTS_H_INCLUDED__

#include "ray_tracer.h"

graphic_object create_sphere_object(world_point center, float radius, color_t color, int specularity, float reflectivity);
graphic_object create_plane_object(world_point normal, float D, color_t color, int specularity, float reflectivity);

#endif

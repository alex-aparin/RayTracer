#include "Geometry.h"

typedef void (*put_pixel_callback)(screen_point point, color value);

void trace(const int canvas_width, const int canvas_height, put_pixel_callback put_pixel);
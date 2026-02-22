// Single compilation unit that provides stb_image, stb_image_write,
// and stb_image_resize2 implementations.
// This file must be compiled exactly once per binary target.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define DOOM_IMPLEMENT_PRINT
#define DOOM_IMPLEMENT_FILE_IO
#define DOOM_IMPLEMENT_MALLOC
#define DOOM_IMPLEMENT_GETTIME
#define DOOM_IMPLEMENT_EXIT
#define DOOM_IMPLEMENT_GETENV
#define DOOM_IMPLEMENTATION
#include "PureDOOM.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
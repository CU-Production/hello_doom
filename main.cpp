#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

#define DOOM_IMPLEMENT_PRINT
#define DOOM_IMPLEMENT_FILE_IO
#define DOOM_IMPLEMENT_MALLOC
#define DOOM_IMPLEMENT_GETTIME
#define DOOM_IMPLEMENT_EXIT
#define DOOM_IMPLEMENT_GETENV
#include "PureDOOM.h"

#include "stb_image_resize2.h"

doom_key_t sokol_keycode_to_doom_key(sapp_keycode keycode);
doom_button_t sokol_mousebutton_to_doom_button(sapp_mousebutton sokol_button);

constexpr uint32_t DOOM_WIDTH = 320;
constexpr uint32_t DOOM_HEIGHT = 200;
uint8_t frame_buffer[DOOM_WIDTH*DOOM_HEIGHT*4];

constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 600;

sg_pass_action pass_action = {0};
sg_image doom_img = {0};

void init() {
    sg_desc desc = {0};
    desc.context = sapp_sgcontext();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    sg_image_desc img_desc = {0};
    img_desc.width  = DOOM_WIDTH;
    img_desc.height = DOOM_HEIGHT;
    img_desc.label = "doom-frame-texture";
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.usage = SG_USAGE_STREAM;

    doom_img = sg_make_image(&img_desc);

    pass_action.colors[0] = (sg_color_attachment_action){ .action=SG_ACTION_CLEAR, .value={0.2f, 0.1f, 0.3f, 1.0f} };

    simgui_desc_t simgui_desc = {0};
    simgui_setup(&simgui_desc);
}

void frame() {
    const double dt = sapp_frame_duration();

    doom_update();

    const size_t buffer_stride = DOOM_WIDTH * 4;
    const size_t doom_stride = DOOM_WIDTH * 4;

    STBIR_RESIZE resize;
    stbir_resize_init(&resize, NULL, DOOM_WIDTH, DOOM_HEIGHT, doom_stride,  frame_buffer, 0, 0, buffer_stride,
                      STBIR_RGBA_NO_AW, STBIR_TYPE_UINT8_SRGB);

    resize.fast_alpha = 1;
    resize.horizontal_filter = STBIR_FILTER_BOX;
    resize.vertical_filter = STBIR_FILTER_BOX;
    resize.input_pixels = doom_get_framebuffer(4);
    resize.output_w = resize.output_subw= DOOM_WIDTH;
    resize.output_h = resize.output_subh = DOOM_HEIGHT;

    stbir_resize_extended(&resize);

    sg_image_data image_data = {0};
    image_data.subimage[0][0] = (sg_range){ .ptr=frame_buffer, .size=(DOOM_WIDTH * DOOM_HEIGHT * 4 * sizeof(uint8_t)) };
    sg_update_image(doom_img, &image_data);

    const int width = sapp_width();
    const int height = sapp_height();
    simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() });

    // imgui
    {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::Begin("FX")) {
            ImGui::Text("DOOM");
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
#define px(pos, color) draw_list->AddRectFilled(p + pos, ImVec2(2, 2) + p + pos, color)
            // draw_list->AddImage((ImTextureID)(uintptr_t)doom_img.id, p, ImVec2(DOOM_WIDTH, DOOM_HEIGHT) + p);
            // draw_list->AddRectFilled(p, p + ImVec2(DOOM_WIDTH, DOOM_HEIGHT), ImColor(128, 0, 0, 255));
            for (int x = 0; x < DOOM_WIDTH; x++) {
                for (int y = 0; y < DOOM_HEIGHT; y++) {
                    uint8_t r = frame_buffer[(x * DOOM_WIDTH + y) * 4 + 0];
                    uint8_t g = frame_buffer[(x * DOOM_WIDTH + y) * 4 + 1];
                    uint8_t b = frame_buffer[(x * DOOM_WIDTH + y) * 4 + 2];
                    uint8_t a = frame_buffer[(x * DOOM_WIDTH + y) * 4 + 3];
                    px(ImVec2(x, y), ImColor(r, g, b, a));
                }
            }
            ImGui::Dummy(ImVec2(DOOM_WIDTH, DOOM_HEIGHT));
            ImGui::End();
        }
    }

    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void cleanup() {
    simgui_shutdown();
    sg_shutdown();
}

void input(const sapp_event* event) {


    switch (event->type) {
        case SAPP_EVENTTYPE_KEY_DOWN: {
            doom_key_down(sokol_keycode_to_doom_key(event->key_code));
            break;
        }
        case SAPP_EVENTTYPE_KEY_UP: {
            doom_key_up(sokol_keycode_to_doom_key(event->key_code));
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            doom_button_down(sokol_mousebutton_to_doom_button(event->mouse_button));
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            doom_button_up(sokol_mousebutton_to_doom_button(event->mouse_button));
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            static float last_mouse_x = 0;
            doom_mouse_move((int)(event->mouse_x - last_mouse_x) * 15, 0);
            last_mouse_x = event->mouse_x;
            break;
        }
        default: break;
    }

    simgui_handle_event(event);
}

int main(int argc, char** args) {
    //-----------------------------------------------------------------------
    // Setup DOOM
    //-----------------------------------------------------------------------

    // Change default bindings to modern
    doom_set_default_int("key_up", DOOM_KEY_W);
    doom_set_default_int("key_down", DOOM_KEY_S);
    doom_set_default_int("key_strafeleft", DOOM_KEY_A);
    doom_set_default_int("key_straferight", DOOM_KEY_D);
    doom_set_default_int("key_use", DOOM_KEY_E);
    // doom_set_default_int("key_fire", DOOM_KEY_SPACE);
    doom_set_default_int("key_fire", DOOM_KEY_SPACE);
    doom_set_default_int("mouse_move", 0); // Mouse will not move forward

    // Setup resolution
    doom_set_resolution(DOOM_WIDTH, DOOM_HEIGHT);

    // Initialize doom
    doom_init(argc, args, DOOM_FLAG_MENU_DARKEN_BG);

    sapp_desc desc = {0};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup,
    desc.event_cb = input,
    desc.width  = SCREEN_WIDTH,
    desc.height = SCREEN_HEIGHT,
    desc.window_title = "sokol + puredoom",
    desc.icon.sokol_default = true,
    desc.logger.func = slog_func;
    sapp_run(&desc);

    return 0;
}

doom_key_t sokol_keycode_to_doom_key(sapp_keycode keycode) {
    switch (keycode) {
        case SAPP_KEYCODE_TAB: return DOOM_KEY_TAB;
        case SAPP_KEYCODE_ENTER: return DOOM_KEY_ENTER;
        case SAPP_KEYCODE_ESCAPE: return DOOM_KEY_ESCAPE;
        case SAPP_KEYCODE_SPACE: return DOOM_KEY_SPACE;
        case SAPP_KEYCODE_APOSTROPHE: return DOOM_KEY_APOSTROPHE;
        case SAPP_KEYCODE_KP_MULTIPLY: return DOOM_KEY_MULTIPLY;
        case SAPP_KEYCODE_COMMA: return DOOM_KEY_COMMA;
        case SAPP_KEYCODE_MINUS: return DOOM_KEY_MINUS;
        case SAPP_KEYCODE_PERIOD: return DOOM_KEY_PERIOD;
        case SAPP_KEYCODE_SLASH: return DOOM_KEY_SLASH;
        case SAPP_KEYCODE_0: return DOOM_KEY_0;
        case SAPP_KEYCODE_1: return DOOM_KEY_1;
        case SAPP_KEYCODE_2: return DOOM_KEY_2;
        case SAPP_KEYCODE_3: return DOOM_KEY_3;
        case SAPP_KEYCODE_4: return DOOM_KEY_4;
        case SAPP_KEYCODE_5: return DOOM_KEY_5;
        case SAPP_KEYCODE_6: return DOOM_KEY_6;
        case SAPP_KEYCODE_7: return DOOM_KEY_7;
        case SAPP_KEYCODE_8: return DOOM_KEY_8;
        case SAPP_KEYCODE_9: return DOOM_KEY_9;
        case SAPP_KEYCODE_SEMICOLON: return DOOM_KEY_SEMICOLON;
        case SAPP_KEYCODE_EQUAL: return DOOM_KEY_EQUALS;
        case SAPP_KEYCODE_LEFT_BRACKET: return DOOM_KEY_LEFT_BRACKET;
        case SAPP_KEYCODE_RIGHT_BRACKET: return DOOM_KEY_RIGHT_BRACKET;
        case SAPP_KEYCODE_A: return DOOM_KEY_A;
        case SAPP_KEYCODE_B: return DOOM_KEY_B;
        case SAPP_KEYCODE_C: return DOOM_KEY_C;
        case SAPP_KEYCODE_D: return DOOM_KEY_D;
        case SAPP_KEYCODE_E: return DOOM_KEY_E;
        case SAPP_KEYCODE_F: return DOOM_KEY_F;
        case SAPP_KEYCODE_G: return DOOM_KEY_G;
        case SAPP_KEYCODE_H: return DOOM_KEY_H;
        case SAPP_KEYCODE_I: return DOOM_KEY_I;
        case SAPP_KEYCODE_J: return DOOM_KEY_J;
        case SAPP_KEYCODE_K: return DOOM_KEY_K;
        case SAPP_KEYCODE_L: return DOOM_KEY_L;
        case SAPP_KEYCODE_M: return DOOM_KEY_M;
        case SAPP_KEYCODE_N: return DOOM_KEY_N;
        case SAPP_KEYCODE_O: return DOOM_KEY_O;
        case SAPP_KEYCODE_P: return DOOM_KEY_P;
        case SAPP_KEYCODE_Q: return DOOM_KEY_Q;
        case SAPP_KEYCODE_R: return DOOM_KEY_R;
        case SAPP_KEYCODE_S: return DOOM_KEY_S;
        case SAPP_KEYCODE_T: return DOOM_KEY_T;
        case SAPP_KEYCODE_U: return DOOM_KEY_U;
        case SAPP_KEYCODE_V: return DOOM_KEY_V;
        case SAPP_KEYCODE_W: return DOOM_KEY_W;
        case SAPP_KEYCODE_X: return DOOM_KEY_X;
        case SAPP_KEYCODE_Y: return DOOM_KEY_Y;
        case SAPP_KEYCODE_Z: return DOOM_KEY_Z;
        case SAPP_KEYCODE_BACKSPACE: return DOOM_KEY_BACKSPACE;
        case SAPP_KEYCODE_LEFT_ALT:
        case SAPP_KEYCODE_RIGHT_ALT: return DOOM_KEY_ALT;
        case SAPP_KEYCODE_LEFT_CONTROL:
        case SAPP_KEYCODE_RIGHT_CONTROL: return DOOM_KEY_CTRL;
        case SAPP_KEYCODE_LEFT: return DOOM_KEY_LEFT_ARROW;
        case SAPP_KEYCODE_UP: return DOOM_KEY_UP_ARROW;
        case SAPP_KEYCODE_RIGHT: return DOOM_KEY_RIGHT_ARROW;
        case SAPP_KEYCODE_DOWN: return DOOM_KEY_DOWN_ARROW;
        case SAPP_KEYCODE_LEFT_SHIFT:
        case SAPP_KEYCODE_RIGHT_SHIFT: return DOOM_KEY_SHIFT;
        case SAPP_KEYCODE_F1: return DOOM_KEY_F1;
        case SAPP_KEYCODE_F2: return DOOM_KEY_F2;
        case SAPP_KEYCODE_F3: return DOOM_KEY_F3;
        case SAPP_KEYCODE_F4: return DOOM_KEY_F4;
        case SAPP_KEYCODE_F5: return DOOM_KEY_F5;
        case SAPP_KEYCODE_F6: return DOOM_KEY_F6;
        case SAPP_KEYCODE_F7: return DOOM_KEY_F7;
        case SAPP_KEYCODE_F8: return DOOM_KEY_F8;
        case SAPP_KEYCODE_F9: return DOOM_KEY_F9;
        case SAPP_KEYCODE_F10: return DOOM_KEY_F10;
        case SAPP_KEYCODE_F11: return DOOM_KEY_F11;
        case SAPP_KEYCODE_F12: return DOOM_KEY_F12;
        default: return DOOM_KEY_UNKNOWN;
    }

    return DOOM_KEY_UNKNOWN;
}

doom_button_t sokol_mousebutton_to_doom_button(sapp_mousebutton sokol_button) {
    switch (sokol_button) {
        case SAPP_MOUSEBUTTON_LEFT: return DOOM_LEFT_BUTTON;
        case SAPP_MOUSEBUTTON_RIGHT: return DOOM_RIGHT_BUTTON;
        case SAPP_MOUSEBUTTON_MIDDLE: return DOOM_MIDDLE_BUTTON;
    }
    return (doom_button_t)3;
}

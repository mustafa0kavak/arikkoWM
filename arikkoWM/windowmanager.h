#pragma once
#include <X11/Xlib.h>
#include <iostream>

class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    void Run();

private:
    Display* dpy;
    Window root;
    bool is_dragging = false;
    Window dragged_window = None;
    int start_mouse_x, start_mouse_y;
    int start_frame_x, start_frame_y;

    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
};
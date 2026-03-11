#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <X11/Xlib.h>
#include <string>
#include <vector>



struct MenuItem {
    std::string label;
    void (*action)();
};

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void Run();
    void ShowMenu(int x, int y);

    static void openTerminal();
    static void refreshScreen();
    static void closeWM();
    std::vector<Window> hidden_windows;
    void minimizeWindow(Window w);

private:
    Display* dpy;
    Window root;
    Window current_menu;
    std::vector<MenuItem> menu_items;
    int item_height;
    bool is_dragging = false;
    Window dragged_window = None;
    Window focused_window = None;
    int start_mouse_x, start_mouse_y, start_frame_x, start_frame_y;


    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
};

#endif
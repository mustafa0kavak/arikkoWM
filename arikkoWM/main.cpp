#include "windowmanager.h"
#include <iostream>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

// BURADA NORMAL TANIMLIYORUZ (extern yok!)
Atom net_supported;
Atom net_client_list;
Atom net_active_window;
Atom net_wm_name;

void setup_atoms(Display* dpy) {
    net_supported = XInternAtom(dpy, "_NET_SUPPORTED", False);
    net_client_list = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
    net_active_window = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
}

void set_supported_properties(Display* dpy, Window root_win) {
    Atom supported[] = { net_supported, net_client_list, net_active_window, net_wm_name };
    XChangeProperty(dpy, root_win, net_supported, XA_ATOM, 32,
        PropModeReplace, (unsigned char*)supported, 4);
}

int main() {
    std::cout << "ArikkoWM Baslatiliyor..." << std::endl;
    Display* display_ptr = XOpenDisplay(NULL);
    if (!display_ptr) return 1;

    Window root_window = DefaultRootWindow(display_ptr);
    setup_atoms(display_ptr);
    set_supported_properties(display_ptr, root_window);

    WindowManager wm;
    wm.Run();

    XCloseDisplay(display_ptr);
    return 0;
}
#include "windowmanager.h"
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

extern Atom net_supported;
extern Atom net_client_list;
extern Atom net_active_window;
extern Atom net_wm_name;

void WindowManager::openTerminal() { system("DISPLAY=:1 xterm &"); }
void WindowManager::refreshScreen() { cout << "Sistem Yenilendi!" << endl; }
void WindowManager::closeWM() { exit(0); }

WindowManager::WindowManager() {
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) exit(1);
    root = DefaultRootWindow(dpy);
    current_menu = None;
    item_height = 30;
}


void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;

    XConfigureWindow(dpy, e.window, e.value_mask, &changes);
    XFlush(dpy);
}

WindowManager::~WindowManager() {
    if (dpy) XCloseDisplay(dpy);
}

void WindowManager::minimizeWindow(Window frame) {
    XUnmapWindow(dpy, frame);
    hidden_windows.push_back(frame);
    XFlush(dpy); 
}

void WindowManager::ShowMenu(int x, int y) {
    menu_items = {
        {"Open Terminal", WindowManager::openTerminal},
        {"Refresh", WindowManager::refreshScreen},
        {"New", WindowManager::closeWM}
    };

    int min_w = 100;
    XFontStruct* font_info = XQueryFont(dpy, XGContextFromGC(DefaultGC(dpy, 0)));
    for (const auto& item : menu_items) {
        int current_w = XTextWidth(font_info, item.label.c_str(), item.label.length());
        if (current_w > min_w) min_w = current_w;
    }
    int menu_width = min_w + 40;
    int menu_h = menu_items.size() * item_height;

    current_menu = XCreateSimpleWindow(dpy, root, x, y, menu_width, menu_h, 1, 0xFFFFFF, 0x222222);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, current_menu, CWOverrideRedirect, &attrs);
    XSelectInput(dpy, current_menu, ButtonPressMask | ExposureMask);

    XFlush(dpy);
    XMapRaised(dpy, current_menu);
    XFlush(dpy);

    GC gc = XCreateGC(dpy, current_menu, 0, NULL);
    XSetForeground(dpy, gc, 0xFFFFFF);
    for (int i = 0; i < (int)menu_items.size(); i++) {
        XDrawString(dpy, current_menu, gc, 15, (i + 1) * item_height - 10,
            menu_items[i].label.c_str(), menu_items[i].label.length());
    }
    XFreeGC(dpy, gc);
    if (font_info) XFreeFontInfo(NULL, font_info, 1);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    if (e.window == current_menu) {
        XMapWindow(dpy, e.window);
        XFlush(dpy);
        return;
    }

    XWindowAttributes attrs;
    if (XGetWindowAttributes(dpy, e.window, &attrs) && attrs.override_redirect) {
        XMapWindow(dpy, e.window);
        XFlush(dpy);
        return;
    }

    Window frame = XCreateSimpleWindow(dpy, root, attrs.x, attrs.y, attrs.width, attrs.height + 20, 2, 0xFFFFFF, 0x444444);

    XChangeProperty(dpy, root, net_client_list, XA_WINDOW, 32,
        PropModeAppend, (unsigned char*)&frame, 1);

    XSelectInput(dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask);

    XReparentWindow(dpy, e.window, frame, 0, 20);
    XMapWindow(dpy, e.window);
    XMapWindow(dpy, frame);
    XFlush(dpy);
}

void WindowManager::Run() {
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask);

    Cursor root_cursor = XCreateFontCursor(dpy, 68);
    XDefineCursor(dpy, root, root_cursor);
    XFreeCursor(dpy, root_cursor);

    XEvent ev;
    while (true) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
        case MapRequest: OnMapRequest(ev.xmaprequest); break;
        case ConfigureRequest: OnConfigureRequest(ev.xconfigurerequest); break;

        case ButtonPress: {
            if (current_menu != None) {
                if (ev.xbutton.window == current_menu) {
                    int clicked_index = ev.xbutton.y / item_height;
                    if (clicked_index >= 0 && clicked_index < (int)menu_items.size()) {
                        if (menu_items[clicked_index].action) menu_items[clicked_index].action();
                    }
                }
                XDestroyWindow(dpy, current_menu);
                current_menu = None;
                XFlush(dpy);
                break;
            }

            if (ev.xbutton.window == root && ev.xbutton.button == Button3) {
                ShowMenu(ev.xbutton.x_root, ev.xbutton.y_root);
                break;
            }

            XWindowAttributes attrs;
            if (!XGetWindowAttributes(dpy, ev.xbutton.window, &attrs)) break;

            if (focused_window != None) XSetWindowBorder(dpy, focused_window, 0x444444);
            focused_window = ev.xbutton.window;
            XRaiseWindow(dpy, focused_window);
            XSetWindowBorderWidth(dpy, focused_window, 2);
            XSetWindowBorder(dpy, focused_window, 0x0000FF);
            XSetInputFocus(dpy, focused_window, RevertToParent, CurrentTime);

            XChangeProperty(dpy, root, net_active_window, XA_WINDOW, 32,
            PropModeReplace, (unsigned char*)&focused_window, 1);

            if (ev.xbutton.button == Button1) {
                // Kapatma Butonu
                if (ev.xbutton.y < 20 && ev.xbutton.x >(attrs.width - 25)) {
                    XDestroyWindow(dpy, ev.xbutton.window);
                    focused_window = None;
                    break;
                }
                // Küçültme Butonu
                if (ev.xbutton.y < 20 && ev.xbutton.x >(attrs.width - 45)) {
                    minimizeWindow(ev.xbutton.window);
                    focused_window = None;
                    break;
                }

                is_dragging = true;
                dragged_window = ev.xbutton.window;
                start_mouse_x = ev.xbutton.x_root;
                start_mouse_y = ev.xbutton.y_root;
                start_frame_x = attrs.x;
                start_frame_y = attrs.y;
            }
        } break;

        case MotionNotify: {
            while (XCheckTypedEvent(dpy, MotionNotify, &ev));
            if (is_dragging && dragged_window != None) {
                XMoveWindow(dpy, dragged_window, start_frame_x + (ev.xmotion.x_root - start_mouse_x), start_frame_y + (ev.xmotion.y_root - start_mouse_y));
            }
        } break;

        case ButtonRelease:
            is_dragging = false;
            dragged_window = None;
            break;

        case Expose: {
            if (ev.xexpose.count != 0) break;

            if (current_menu != None && ev.xexpose.window == current_menu) break;
            GC gc = XCreateGC(dpy, ev.xexpose.window, 0, NULL);
            XWindowAttributes attrs;

            if (XGetWindowAttributes(dpy, ev.xexpose.window, &attrs) && attrs.height > 20) {
                char* window_name = NULL;
                Window root_return, parent_return, * children = NULL;
                unsigned int nchildren = 0;

                if (XQueryTree(dpy, ev.xexpose.window, &root_return, &parent_return, &children, &nchildren) && nchildren > 0) {
                    if (XFetchName(dpy, children[0], &window_name) == 0) {
                        XClassHint ch;
                        if (XGetClassHint(dpy, children[0], &ch)) {
                            window_name = strdup(ch.res_name);
                            XFree(ch.res_name);
                            XFree(ch.res_class);
                        }
                    }
                    if (children) XFree(children);
                }

                XSetForeground(dpy, gc, 0xFFFFFF); 
                if (window_name && strlen(window_name) > 0) {
                    XDrawString(dpy, ev.xexpose.window, gc, 10, 15, window_name, strlen(window_name));
                    free(window_name); 
                }
                else {
                    XDrawString(dpy, ev.xexpose.window, gc, 10, 15, "Uygulama", 8);
                }
                XSetForeground(dpy, gc, 0xFF0000);
                XFillRectangle(dpy, ev.xexpose.window, gc, attrs.width - 22, 4, 12, 12);
                XSetForeground(dpy, gc, 0xFFFF00);
                XFillRectangle(dpy, ev.xexpose.window, gc, attrs.width - 40, 4, 12, 12);
            }

            XFreeGC(dpy, gc);
        } break;
        }
    }
}
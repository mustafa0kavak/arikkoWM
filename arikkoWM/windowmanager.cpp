#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <iostream>
#include <cstdlib>
#include "windowmanager.h"
#include <cstring>

using namespace std;

WindowManager::WindowManager() {
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) exit(1);
    root = DefaultRootWindow(dpy);
}

WindowManager::~WindowManager() {
    if (dpy) XCloseDisplay(dpy);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    XWindowAttributes attrs;
    if (!XGetWindowAttributes(dpy, e.window, &attrs) || attrs.override_redirect) return;

    Window frame = XCreateSimpleWindow(
        dpy, root,
        attrs.x, attrs.y, attrs.width, attrs.height + 20,
        2, 0xFFFFFF, 0x444444
    );

    XSelectInput(dpy, frame, SubstructureRedirectMask | SubstructureNotifyMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask);

    // --- MOUSE İMLECİNİ SABİTLE ---
    Cursor cursor = XCreateFontCursor(dpy, 68); // Standart Ok
    XDefineCursor(dpy, frame, cursor);
    XDefineCursor(dpy, e.window, cursor);

    XReparentWindow(dpy, e.window, frame, 0, 20);
    XMapWindow(dpy, e.window);
    XMapWindow(dpy, frame);

    // Grab yaparken imleci hapsetmiyoruz (None), böylece akıcı kalıyor
    XGrabButton(dpy, Button1, 0, frame, False,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync, GrabModeAsync, None, None);

    XFreeCursor(dpy, cursor);
    XFlush(dpy);
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
    XWindowChanges changes;
    changes.x = e.x; changes.y = e.y;
    changes.width = e.width; changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;
    XConfigureWindow(dpy, e.window, e.value_mask, &changes);
}

Window focused_window = None;

void WindowManager::Run() {
    Cursor root_cursor = XCreateFontCursor(dpy, 68);
    XDefineCursor(dpy, root, root_cursor);
    XFreeCursor(dpy, root_cursor);

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);
    XEvent ev;

    while (true) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
        case MapRequest:
            OnMapRequest(ev.xmaprequest);
            break;

        case ConfigureRequest:
            OnConfigureRequest(ev.xconfigurerequest);
            break;

        case ButtonPress: {
            if (ev.xbutton.window == root) break;
            XWindowAttributes attrs;
            if (!XGetWindowAttributes(dpy, ev.xbutton.window, &attrs)) break;

            if (focused_window != None) {
                XSetWindowBorder(dpy, focused_window, 0x444444);
            }

            focused_window = ev.xbutton.window;

            focused_window = ev.xbutton.window;
            XRaiseWindow(dpy, focused_window);

            XSetWindowBorderWidth(dpy, focused_window, 2); 
            XSetWindowBorder(dpy, focused_window, 0x0000FF);

            XSetInputFocus(dpy, focused_window, RevertToParent, CurrentTime);

            if (ev.xbutton.button == Button1) {
                if (ev.xbutton.y < 20 && ev.xbutton.x >(attrs.width - 25)) {
                    XDestroyWindow(dpy, ev.xbutton.window);
                    focused_window = None;
                    XFlush(dpy);
                    break;
                }

                is_dragging = true;
                dragged_window = ev.xbutton.window;
                start_mouse_x = ev.xbutton.x_root;
                start_mouse_y = ev.xbutton.y_root;
                start_frame_x = attrs.x;
                start_frame_y = attrs.y;
                XRaiseWindow(dpy, dragged_window);
            }
        } break;

        case MotionNotify: {
            while (XCheckTypedEvent(dpy, MotionNotify, &ev));
            if (is_dragging && dragged_window != None) {
                XMoveWindow(dpy, dragged_window,
                    start_frame_x + (ev.xmotion.x_root - start_mouse_x),
                    start_frame_y + (ev.xmotion.y_root - start_mouse_y));
            }
        } break;

        case ButtonRelease:
            is_dragging = false;
            dragged_window = None;
            break;

        case Expose: {
            if (ev.xexpose.count != 0) break;
            Window win = ev.xexpose.window;
            GC gc = XCreateGC(dpy, win, 0, NULL);
            XWindowAttributes attrs;
            XGetWindowAttributes(dpy, win, &attrs);

            // --- SABİT İSİM ÇEKME (XGetClassHint) ---
            Window root_ret, parent_ret, * children;
            unsigned int nchildren;
            XClassHint ch;
            bool name_drawn = false;

            if (XQueryTree(dpy, win, &root_ret, &parent_ret, &children, &nchildren) && nchildren > 0) {
                if (XGetClassHint(dpy, children[0], &ch)) {
                    XSetForeground(dpy, gc, 0xFFFFFF);
                    XDrawString(dpy, win, gc, 10, 15, ch.res_class, strlen(ch.res_class));
                    XFree(ch.res_name);
                    XFree(ch.res_class);
                    name_drawn = true;
                }
                XFree(children);
            }

            if (!name_drawn) {
                XSetForeground(dpy, gc, 0xFFFFFF);
                XDrawString(dpy, win, gc, 10, 15, "Pencere", 7);
            }

            // BUTON ÇİZİMİ
            XSetForeground(dpy, gc, 0xFF0000);
            XFillRectangle(dpy, win, gc, attrs.width - 22, 4, 12, 12);
            XSetForeground(dpy, gc, 0xFFFFFF);
            XDrawRectangle(dpy, win, gc, attrs.width - 22, 4, 12, 12);

            XFreeGC(dpy, gc);
        } break;
        }
    }
}
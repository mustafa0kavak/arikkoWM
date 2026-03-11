#include "windowmanager.h"
#include <iostream>
#include <X11/Xutil.h>

int main() {
    std::cout << "wm Baslatiliyor..." << std::endl;

    WindowManager wm;
    wm.Run(); 

    return 0;
}
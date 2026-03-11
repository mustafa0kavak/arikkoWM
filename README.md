# arikkoWM

arikkoWM is a window manager just made for fun and learning. its written in c++ and x11 library is used.

## features at v1.0

+ dragging Windows  
+ closing  and unmapping windows  
+ A menu that appears when you right click the background (root)  

### installation
---
first of all i dont recommend you to USE this in your real pc but if you really want to try;  

if you want to use makefiles you only need to install the library

you should install libx11-dev  

```bash
sudo apt install libx11-dev
g++ main.cpp windowmanager.cpp -lX11 -o arikkoWM
./arikkoWM
```

or at the last ./arikkoWM part it can be

```bash
./arikkoWM.out
```


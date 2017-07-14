Mui
---

Mouse oriented ui library, which maintains a few states for each widget for squeezing the performance. It's very close to IMGUI, but with minimum states maintained to avoid unnecessary rendering. Creating UI with Mui is very fast (check out demo.cpp), customizing or writing your own widgets is very easy (check out widget implementations in mui.h).    

![image](mui.png "demo" {width=40px height=400px})

About Sui (sui.h, sui.c):

OpenCV's UI behaves differently for different widget frameworks. On Linux systems, there're always something inconsistent among different backends in Qt, GTK, GTK2.0 or Carbon, etc. Sui is created based on X11 to provide a consistent experience on Linux system. It's based on X11 directly which saves lots of memory and CPU time. I used it on some embded systems to provide . 


```
make demo # without Sui
```

```
make demo_sui # with Sui
```


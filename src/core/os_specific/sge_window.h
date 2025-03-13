//
// Created by Geisthardt on 27.02.2025.
//

#ifndef SGE_WINDOW_H
#define SGE_WINDOW_H
#include <stdbool.h>
#include <windows.h>


#ifdef WIN32
        typedef struct sge_platform_handle {
                HWND hwnd;
                HINSTANCE hinstance;
        } sge_platform_handle;
#elif UNIX
        typedef struct sge_platform_handle {
                //whatever unix needs
        } sge_platform_handle
#else

#endif

typedef struct sge_window {
        sge_platform_handle handle;
        int width;
        int height;
} sge_window;

sge_window *sge_window_create(const int width,const int height,const char *window_name);
void sge_window_destroy(sge_window *window);
void update_frame(const int target_fps, const DWORD start_time_ms, sge_window *window);
void hide_mouse();
void show_mouse();
bool window_should_close();
#endif //SGE_WINDOW_H

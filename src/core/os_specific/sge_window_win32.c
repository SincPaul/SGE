//
// Created by Geisthardt on 27.02.2025.
//

#include <stdio.h>
#include <windows.h>


#include "sge_window.h"
#include "../logging.h"
#include "../../utils/steintime.h"
#include "../input.h"
#include <windowsx.h>

#include "os_utils.h"
#include "../memory_control.h"
#include "../../renderer/sge_render.h"
#include "../../renderer/vulkan_renderer/vulkan_renderer.h"
#ifdef WIN32

DWORD g_delta_time = 0;
int is_window_open = 0;
int g_fps = 0;
bool is_resize = false;


void set_window_title(HWND hwnd, char *title, ...) {
        char title_message[256];
        va_list args;
        va_start(args, title);
        vsnprintf(title_message, sizeof(title_message), title, args);
        va_end(args);
        SetWindowTextA(hwnd, title_message);

}

LRESULT CALLBACK wndProc(const HWND hwnd, const UINT uMsg, const WPARAM wparam, const LPARAM lparam) {
        switch (uMsg) { https://learn.microsoft.com/de-de/windows/win32/winmsg
                case WM_CLOSE:
                        is_window_open = 0;
                        DestroyWindow(hwnd);
                        log_event(LOG_LEVEL_INFO, "Window got closed by close");
                        return 0;
                case WM_DESTROY:
                        is_window_open = 0;
                        PostQuitMessage(0);
                        log_event(LOG_LEVEL_INFO, "Window got closed by destroy");
                        return 0;
                case WM_KEYDOWN: { //https://learn.microsoft.com/de-de/windows/win32/inputdev/
                        //printf("KEY DOWN: %llu\n", wparam);
                        //printf("PARAMS KEY DOWN %llu\n", lparam);
                        key_states[wparam] = 1;
                } break;
                case WM_KEYUP: {
                        //printf("KEY UP: %llu\n", wparam);
                        //printf("PARAMS KEY UP %llu\n", lparam);
                        key_states[wparam] = 0;
                }
                case WM_MOUSEMOVE: {
                        const int x_pos = GET_X_LPARAM(lparam);
                        const int y_pos = GET_Y_LPARAM(lparam);
                        //printf("Mouse move: x = %d, y = %d\n", x_pos, y_pos);
                        last_mouse_pos.x = x_pos;
                        last_mouse_pos.y = y_pos;
                } break;
                case WM_LBUTTONDOWN: {
                        printf("PRESSED L MOUSEBUTTON\n");
                } break;
                case WM_LBUTTONDBLCLK: {
                        printf("DOUBLE CLICK WITH MOUSE\n");
                } break;
                case WM_SIZE: {
                        printf("RESIZE HAPPENED\n");
                        is_resize = true;
                }
        }
        return DefWindowProc(hwnd, uMsg, wparam, lparam);
}

sge_window *sge_window_create(const int width, const int height, const char *window_name) {
        const HINSTANCE hInstance = GetModuleHandleA(NULL);

        const char CLASS_NAME[] = "Sample Window Class";

        WNDCLASS wc = { };

        wc.lpfnWndProc = wndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        if (RegisterClass(&wc) == 0) {
                const DWORD last_error = GetLastError();
                char error_message[256];
                snprintf(error_message, sizeof(error_message), "Failed to Register window Class. Error. %dw", last_error);
                log_event(LOG_LEVEL_FATAL, error_message);
        }

        const HWND hwnd = CreateWindowEx(
                0,
                CLASS_NAME,
                window_name,
                WS_OVERLAPPEDWINDOW, //https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                width,
                height,
                NULL,
                NULL,
                hInstance,
                NULL
        );

        if (hwnd == NULL) {
                printf("hwnd is Null");
                const DWORD last_error = GetLastError();
                char error_message[256];
                snprintf(error_message, sizeof(error_message), "Failed to create Window. Error. %dw", last_error);
                log_event(LOG_LEVEL_FATAL, error_message);
        }

        //https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow?redirectedfrom=MSDN
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        log_event(LOG_LEVEL_INFO, "Created Window");
        is_window_open = 1;

        sge_platform_handle handle = {
                .hwnd = hwnd,
                .hinstance = hInstance
        };

        sge_window *window = allocate_memory(sizeof(sge_window), MEMORY_TAG_WINDOW);
        if (window == NULL) {
                log_event(LOG_LEVEL_FATAL, "Failed to allocate memory for window struct");
                return NULL;
        }
        window->handle = handle;
        window->height = height;
        window->width  = width;

        return window;
}

void update_frame(const int target_fps, const DWORD frame_start_ms, sge_window *window) {

        //printf("New Frame\n");
        update_key_states();

        MSG msg;
        while (PeekMessage(&msg, NULL, 0 ,0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        if (g_delta_time > 1000) {
                g_delta_time = 0;
                //printf("FPS: %d\n", g_fps);
                set_window_title(window->handle.hwnd, "Running with %d Fps", g_fps);
                g_fps = 0;
        } else {
                g_fps++;
        }



        if (target_fps > 0) {
                const DWORD target_frame_time = 1000 / target_fps;
                //printf("TARGET FRAME TIME: %d\n", target_frame_time);
                double frame_time;
                do  {
                        os_sleep(1);
                        frame_time = get_current_ms_time() - frame_start_ms;
                } while (frame_time < target_frame_time);
                //printf("FRAME TIME: %f\n", frame_time);
        }

        const DWORD frame_end_time = get_current_ms_time();
        g_delta_time += frame_end_time - frame_start_ms;
}

bool window_should_close() {
        if (is_window_open) {
                return 0;
        }
        return 1;
}

void sge_window_destroy(sge_window *window) {
        printf("DESTROYYYYYY RAWRRRRRRR\n");
}

#endif




//HWND hwnd = CreateWindowEx(
//0,                              // Optional window styles.
//CLASS_NAME,                     // Window class
//L"Learn to Program Windows",    // Window text
//WS_OVERLAPPEDWINDOW,            // Window style
//
//// Size and position
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//
//NULL,       // Parent window
//NULL,       // Menu
//hInstance,  // Instance handle
//NULL        // Additional application data
//);
//
//if (hwnd == NULL)
//{
//        return 0;
//}
//
// Created by Geisthardt on 26.02.2025.
//

#include <time.h>
#include <stdio.h>
#include "steintime.h"

#include <windows.h>

char *current_time_formatted(void) {
        char static current_time_formatted[50];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        strftime(current_time_formatted, sizeof(current_time_formatted), "%d.%m.%Y %H:%M:%S", tm_info);
        //printf("%s\n", current_time_formatted);

        return current_time_formatted;
}

int get_current_ms_time(void) {
        const DWORD current_ms = GetTickCount64();
        return current_ms;
}

int get_current_year(void) {
        const time_t now = time(NULL);
        const struct tm *tm_info = localtime(&now);

        return tm_info->tm_year + 1900;
}

int get_current_month(void) {
        const time_t now = time(NULL);
        const struct tm *tm_info = localtime(&now);

        return tm_info->tm_mon + 1;
}

int get_current_day(void) {
        const time_t now = time(NULL);
        const struct tm *tm_info = localtime(&now);

        return tm_info->tm_mday;
}

//
// Created by Geisthardt on 26.02.2025.
//

#ifndef LOGGING_H
#define LOGGING_H

typedef enum log_level {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} log_level;


int start_logger();
void log_event(log_level level, const char *message, ...);
int stop_logger();
#endif //LOGGING_H

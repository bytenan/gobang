#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include <time.h>

#define LOG(format, ...) do {\
    time_t t = time(nullptr);\
    struct tm *lt = localtime(&t);\
    char buf[64] = { 0 };\
    strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %H:%M:%S", lt);\
    fprintf(stdout, "[%s] %s:%d: " format "\n", buf, __FILE__, __LINE__, ##__VA_ARGS__);\
} while(0)

#endif //__LOGGER_H__
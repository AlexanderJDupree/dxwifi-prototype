/**
 * Logging API facade for DxWifi. 
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libdxwifi/details/logging.h>


enum dxwifi_log_level __user_log_level = DXWIFI_LOG_OFF;
dxwifi_logger __logger = 0;


const char* log_level_to_str(enum dxwifi_log_level level) {
    switch (level)
    {
    case DXWIFI_LOG_TRACE:
        return "TRACE";
    case DXWIFI_LOG_DEBUG:
        return "DEBUG";
    case DXWIFI_LOG_INFO:
        return "INFO";
    case DXWIFI_LOG_WARN:
        return "WARN";
    case DXWIFI_LOG_ERROR:
        return "ERROR";
    case DXWIFI_LOG_FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}


void init_logging(enum dxwifi_log_level log_level, dxwifi_logger logger) {
    __user_log_level = log_level;
    __logger = logger;
}


void set_log_level(enum dxwifi_log_level log_level) {
    __user_log_level = log_level;
}


void set_logger(dxwifi_logger logger) {
    __logger = logger;
}


void log_hexdump(uint8_t* data, int size) {
    // Hex dump is "Expensive" so we only enable it for trace logging
    #if DXWIFI_LOG_LEVEL < 6
    (void)data;(void)size;
    #else
    if(__logger && DXWIFI_LOG_TRACE <= __user_log_level) {

        int i           = 0;
        int nbytes      = 0;
        int location    = 0;

        char temp[16];
        char formatted_str[BUFSIZ];

        formatted_str[location++] = '\n';

        while (i < size) {
            nbytes = sprintf(temp, "%08x", i);

            memcpy(formatted_str + location, temp, nbytes);
            location += nbytes;

            for(int j = 0; j < 16 && i < size; ++i, ++j) {
                nbytes = sprintf(temp, " %02x", *(data + i));
                memcpy(formatted_str + location, temp, nbytes);
                location += nbytes;
            }
            formatted_str[location++] = '\n';
        }
        formatted_str[location] = '\0';

        __log(DXWIFI_LOG_TRACE, "%s", formatted_str);
    }
    #endif
}


void __log(enum dxwifi_log_level log_level, const char* fmt, ...) {
    if(__logger && log_level <= __user_log_level) {
        va_list args;
        va_start(args, fmt);
        __logger(log_level, fmt, args);
        va_end(args);
    }
}

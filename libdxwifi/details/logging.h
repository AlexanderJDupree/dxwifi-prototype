/**
 * Logging API facade for DxWifi. 
 * 
 * This is only a very basic API but it does support some key performance 
 * features, namely compile time and runtime log level configuration. Logging
 * under the compiler log level gets reduced to a noop. 
 * 
 * By default logging is disabled. To hook up your own logger simply define a 
 * function that fullfills the dxwifi_logger interface and bridge it to your own 
 * logging library. 
 * 
 */

#ifndef LIBDXWIFI_LOGGING_H
#define LIBDXWIFI_LOGGING_H

#include <stdarg.h>

enum dxwifi_log_level {
    DXWIFI_LOG_TRACE        = 6,
    DXWIFI_LOG_DEBUG        = 5,
    DXWIFI_LOG_INFO         = 4,
    DXWIFI_LOG_WARN         = 3,
    DXWIFI_LOG_ERROR        = 2,
    DXWIFI_LOG_FATAL        = 1,
    DXWIFI_LOG_OFF          = 0,
};


typedef void (*dxwifi_logger)(enum dxwifi_log_level, const char* fmt, va_list args);


const char* log_level_to_str(enum dxwifi_log_level level);

void init_logging(enum dxwifi_log_level log_level, dxwifi_logger logger);

void set_log_level(enum dxwifi_log_level log_level);

void set_logger(dxwifi_logger logger);

void log_hexdump(uint8_t* data, int size);


#if defined(LIBDXWIFI_DISABLE_LOGGING)
  #define DXWIFI_LOG_LEVEL 0
#elif defined(NDEBUG)
  #define DXWIFI_LOG_LEVEL 3
#else
  #define DXWIFI_LOG_LEVEL 6
#endif 

#if DXWIFI_LOG_LEVEL < 1
  #define log_fatal(fmt, ...)   (void)0
#else
  #define log_fatal(fmt, ...) __log(DXWIFI_LOG_FATAL, fmt, ##__VA_ARGS__)
#endif

#if DXWIFI_LOG_LEVEL < 2
  #define log_error(fmt, ...)   (void)0
#else
  #define log_error(fmt, ...) __log(DXWIFI_LOG_ERROR, fmt, ##__VA_ARGS__)
#endif

#if DXWIFI_LOG_LEVEL < 3
  #define log_warning(fmt, ...)   (void)0
#else
  #define log_warning(fmt, ...) __log(DXWIFI_LOG_WARN, fmt, ##__VA_ARGS__)
#endif

#if DXWIFI_LOG_LEVEL < 4
  #define log_info(fmt, ...)   (void)0
#else
  #define log_info(fmt, ...) __log(DXWIFI_LOG_INFO, fmt, ##__VA_ARGS__)
#endif

#if DXWIFI_LOG_LEVEL < 5
  #define log_debug(fmt, ...)   (void)0
#else
  #define log_debug(fmt, ...) __log(DXWIFI_LOG_DEBUG, fmt, ##__VA_ARGS__)
#endif

#if DXWIFI_LOG_LEVEL < 6
  #define log_trace(fmt, ...)   (void)0
#else
  #define log_trace(fmt, ...) __log(DXWIFI_LOG_TRACE, fmt, ##__VA_ARGS__)
#endif

void __log(enum dxwifi_log_level log_level, const char* fmt, ...);

#endif // LIBDXWIFI_LOGGING_H
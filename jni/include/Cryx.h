#ifndef CRYX_H
#define CRYX_H

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_CPU_CORES 16
#define CPU_STAT_PATH "/proc/stat"
#define MTK_BRIGHTNESS_PATH "/sys/class/leds/lcd-backlight/brightness"

#define LOOP_INTERVAL 600000
#define MAX_DATA_LENGTH 1024
#define MAX_COMMAND_LENGTH 600
#define MAX_OUTPUT_LENGTH 256
#define MAX_PATH_LENGTH 256
#define SLEEP_WHEN_OFF 3  

#define LOG_TAG "Cryx"

#define LOCK_FILE "/data/adb/.config/Cryx/.lock"
#define LOG_FILE "/data/adb/.config/Cryx/Cryx.log"
#define MODULE_PROP "/data/adb/modules/cryoblissopt/module.prop"
#define MODULE_UPDATE "/data/adb/modules/cryoblissopt/update"

#define MY_PATH                                                                                                                    \
    "PATH=/system/bin:/system/xbin:/data/adb/ap/bin:/data/adb/ksu/bin:/data/adb/magisk:/debug_ramdisk:/sbin:/sbin/su:/su/bin:/su/" \
    "xbin:/data/data/com.termux/files/usr/bin"


#define IS_AWAKE(state) (strcmp(state, "Awake") == 0 || strcmp(state, "true") == 0)
// Basic C knowledge: enum starts with 0

typedef enum : char {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;


extern char* custom_log_tag;
extern float prev_usage;
extern int prev_freq;
float get_cpu_usage(void);
int calculate_target_frequency(float usage);
void apply_frequency_all(void);
int is_screen_on(void);
extern bool screen_off;
void set_all_to_min_freq(void);
extern int global_min_freq;
void init_global_freq_bounds(void);
extern int global_max_freq;
int read_int_from_file(const char *path);
float get_usage_for_policy(const char *policy);
void apply_min_frequency_all(void);
void write_int_to_file(const char *path, int value);

/*
 * If you're here for function comments, you
 * are in the wrong place.
 */


void sighandler(const int signal);
char* trim_newline(char* string);
void is_kanged(void);
char* timern(void);
bool return_true(void);
bool return_false(void);

// Shell and Command execution
char* execute_command(const char* format, ...);
char* execute_direct(const char* path, const char* arg0, ...);
int systemv(const char* format, ...);

// Utilities
int create_lock_file(void);
int write2file(const char* filename, const bool append, const bool use_flock, const char* data, ...);


void log_zenith(LogLevel level, const char* message, ...);
void external_log(LogLevel level, const char* tag, const char* message);


#endif

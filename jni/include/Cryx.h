#ifndef AZENITH_H
#define AZENITH_H

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


#define LOOP_INTERVAL 15
#define MAX_DATA_LENGTH 1024
#define MAX_COMMAND_LENGTH 600
#define MAX_OUTPUT_LENGTH 256
#define MAX_PATH_LENGTH 256

#define LOG_TAG "Cryx"

#define LOCK_FILE "/data/adb/.config/Cryx/.lock"
#define LOG_FILE "/data/adb/.config/Cryx/Cryx.log"
#define MODULE_PROP "/data/adb/modules/cryoblissopt/module.prop"
#define MODULE_UPDATE "/data/adb/modules/cryoblissopt/update"

#define MY_PATH                                                                                                                    \
    "PATH=/system/bin:/system/xbin:/data/adb/ap/bin:/data/adb/ksu/bin:/data/adb/magisk:/debug_ramdisk:/sbin:/sbin/su:/su/bin:/su/" \
    "xbin:/data/data/com.termux/files/usr/bin"


// Basic C knowledge: enum starts with 0

typedef enum : char {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;


extern char* custom_log_tag;

/*
 * If you're here for function comments, you
 * are in the wrong place.
 */


void sighandler(const int signal);
char* trim_newline(char* string);
void notify(const char* message);
void toast(const char* message); 
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

// system
void log_preload(LogLevel level, const char *message, ...);
void log_zenith(LogLevel level, const char* message, ...);
void external_log(LogLevel level, const char* tag, const char* message);


#endif

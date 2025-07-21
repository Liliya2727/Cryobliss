/*
 * Copyright (C) 2024-2025 Zexshia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 

#include <Cryx.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

unsigned long long last_total = 0, last_idle = 0;


int global_min_freq = 0;
int global_max_freq = 0;
bool screen_off = false;

/***********************************************************************************
 * Function Name      : freq
 * Description        : check cpu max and minfreq
 ***********************************************************************************/

void init_global_freq_bounds(void) {
    DIR *dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (!dir) return;

    struct dirent *entry;
    global_min_freq = INT_MAX;
    global_max_freq = -1;

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "policy", 6) != 0) continue;

        char path_min[128], path_max[128];
        snprintf(path_min, sizeof(path_min),
                 "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_min_freq", entry->d_name);
        snprintf(path_max, sizeof(path_max),
                 "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_max_freq", entry->d_name);

        FILE *fp_min = fopen(path_min, "r");
        FILE *fp_max = fopen(path_max, "r");

        if (fp_min && fp_max) {
            int min_freq = 0, max_freq = 0;

            if (fscanf(fp_min, "%d", &min_freq) == 1 &&
                fscanf(fp_max, "%d", &max_freq) == 1) {

                // Log this policy's min/max
                log_zenith(LOG_INFO, "%s: min=%d max=%d", entry->d_name, min_freq, max_freq);

                if (min_freq < global_min_freq) global_min_freq = min_freq;
                if (max_freq > global_max_freq) global_max_freq = max_freq;
            }
        }

        if (fp_min) fclose(fp_min);
        if (fp_max) fclose(fp_max);
    }

    closedir(dir);

}
/***********************************************************************************
 * Function Name      : cpuusage
 * Description        : Check cpu usage
 ***********************************************************************************/
 
float get_cpu_usage(void) {
    FILE *fp = fopen(CPU_STAT_PATH, "r");
    if (!fp) return 0.0f;

    char buf[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    sscanf(buf, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long total_diff = total - last_total;
    unsigned long long idle_diff = idle - last_idle;

    last_total = total;
    last_idle = idle;

    if (total_diff == 0) return 0.0f;

    return (float)(total_diff - idle_diff) * 100.0f / total_diff;
}

/***********************************************************************************
 * Function Name      : Calculate 
 * Description        : Calculate target freq based kn usage
 ***********************************************************************************/
 
int calculate_target_frequency(int min_freq, int max_freq, float curr_usage) {
    if (min_freq <= 0 || max_freq <= 0 || max_freq < min_freq)
        return 0;

    if (curr_usage < 10.0f) return min_freq;
    if (curr_usage > 90.0f) return max_freq;

    float ratio = curr_usage / 100.0f;
    return min_freq + (int)((max_freq - min_freq) * ratio);
}

/***********************************************************************************
 * Function Name      : apply freq
 * Description        : apply scaling max and min freq
 ***********************************************************************************/

void apply_min_frequency_all(float curr_usage) {
    DIR *dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (!dir) {
        log_zenith(LOG_ERROR, "Failed to open cpufreq directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "policy", 6) != 0)
            continue;

        char path_min[128];
        snprintf(path_min, sizeof(path_min),
                 "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_min_freq", entry->d_name);
        int min_freq = read_int_from_file(path_min);
        if (min_freq <= 0)
            continue;

        // Use systemv with redirection via `tee` (which works inside sh -c)
        char set_min_cmd[256], set_max_cmd[256];
        snprintf(set_min_cmd, sizeof(set_min_cmd),
                 "echo %d | tee /sys/devices/system/cpu/cpufreq/%s/scaling_min_freq > /dev/null",
                 min_freq, entry->d_name);
        snprintf(set_max_cmd, sizeof(set_max_cmd),
                 "echo %d | tee /sys/devices/system/cpu/cpufreq/%s/scaling_max_freq > /dev/null",
                 min_freq, entry->d_name);

        if (systemv("%s", set_min_cmd) != 0 || systemv("%s", set_max_cmd) != 0) {
            log_zenith(LOG_ERROR, "Failed to apply min freq to %s", entry->d_name);
            continue;
        }

        log_zenith(LOG_INFO, "%s forced to min freq = %d kHz", entry->d_name, min_freq);
    }

    closedir(dir);
}

/***********************************************************************************
 * Function Name      : check screenon
 * Description        : check screen state
 ***********************************************************************************/
 
int is_screen_on(void) {
    FILE *fp = fopen(MTK_BRIGHTNESS_PATH, "r");
    if (!fp) return 1;  // assume screen is on if brightness can't be read

    int brightness = 0;
    fscanf(fp, "%d", &brightness);
    fclose(fp);

    return brightness > 0;
}

/***********************************************************************************
 * Function Name      : trim_newline
 * Description        : Set all cpu to minfreq
 ***********************************************************************************/
 
void apply_min_frequency_all(void) {
    DIR *dir = opendir("/sys/devices/system/cpu/cpufreq");
    if (!dir) {
        log_zenith(LOG_ERROR, "Failed to open cpufreq directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "policy", 6) != 0)
            continue;

        char path_min[128];
        snprintf(path_min, sizeof(path_min),
                 "/sys/devices/system/cpu/cpufreq/%s/cpuinfo_min_freq", entry->d_name);
        int min_freq = read_int_from_file(path_min);
        if (min_freq <= 0)
            continue;

        char set_min_cmd[256], set_max_cmd[256];
        snprintf(set_min_cmd, sizeof(set_min_cmd),
                 "echo %d | tee /sys/devices/system/cpu/cpufreq/%s/scaling_min_freq > /dev/null",
                 min_freq, entry->d_name);
        snprintf(set_max_cmd, sizeof(set_max_cmd),
                 "echo %d | tee /sys/devices/system/cpu/cpufreq/%s/scaling_max_freq > /dev/null",
                 min_freq, entry->d_name);

        if (systemv("%s", set_min_cmd) != 0 || systemv("%s", set_max_cmd) != 0) {
            log_zenith(LOG_ERROR, "Failed to set frequency for %s", entry->d_name);
            continue;
        }

        log_zenith(LOG_INFO, "%s forced to min freq = %d kHz", entry->d_name, min_freq);
    }

    closedir(dir);
}

void set_all_to_min_freq(void) {
    apply_min_frequency_all();
}


int read_int_from_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        log_zenith(LOG_ERROR, "Failed to open %s: %s", path, strerror(errno));
        return -1;
    }

    char buf[32];
    ssize_t len = read(fd, buf, sizeof(buf) - 1);
    if (len <= 0) {
        log_zenith(LOG_ERROR, "Failed to read from %s: %s", path, strerror(errno));
        close(fd);
        return -1;
    }

    buf[len] = '\0';
    close(fd);
    return atoi(buf);
}

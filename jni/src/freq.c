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
    int min_found = 0, max_found = 0;

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

                if (!min_found || min_freq < global_min_freq) {
                    global_min_freq = min_freq;
                    min_found = 1;
                }

                if (!max_found || max_freq > global_max_freq) {
                    global_max_freq = max_freq;
                    max_found = 1;
                }
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
 
int calculate_target_frequency(float usage) {
    if (global_min_freq == 0 || global_max_freq == 0) return 0;  // Sanity check

    if (usage < 10.0f) return global_min_freq;
    if (usage > 90.0f) return global_max_freq;

    float ratio = usage / 100.0f;
    return global_min_freq + (int)((global_max_freq - global_min_freq) * ratio);
}

/***********************************************************************************
 * Function Name      : apply freq
 * Description        : apply scaling max and min freq
 ***********************************************************************************/
 
void apply_frequency_all(int freq) {
    for (int i = 0; i < MAX_CPU_CORES; i++) {
        char path_max[128], path_min[128];

        snprintf(path_max, sizeof(path_max), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);
        snprintf(path_min, sizeof(path_min), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", i);

        FILE *fp_max = fopen(path_max, "w");
        FILE *fp_min = fopen(path_min, "w");

        if (fp_max) {
            fprintf(fp_max, "%d", freq);
            fclose(fp_max);
        }

        if (fp_min) {
            fprintf(fp_min, "%d", freq);  // Ensure it doesn't fall below
            fclose(fp_min);
        }
    }
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
void set_all_to_min_freq(void) {
    apply_frequency_all(global_min_freq);
}

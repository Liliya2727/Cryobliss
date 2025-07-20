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
#include <libgen.h>
#include <math.h>
float prev_usage = -1.0f;
int prev_freq = -1;
            
int main(int argc, char* argv[]) {
    // Handle case when not running on root
    if (getuid() != 0) {
        fprintf(stderr, "\033[31mERROR:\033[0m Please run this program as root\n");
        exit(EXIT_FAILURE);
    }

    // Expose logging interface for other modules
    char* base_name = basename(argv[0]);
    if (strcmp(base_name, "Cryx_log") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: Cryx_log <TAG> <LEVEL> <MESSAGE>\n");
            fprintf(stderr, "Levels: 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR, 4=FATAL\n");
            return EXIT_FAILURE;
        }

        // Parse log level
        int level = atoi(argv[2]);
        if (level < LOG_DEBUG || level > LOG_FATAL) {
            fprintf(stderr, "Invalid log level. Use 0-4.\n");
            return EXIT_FAILURE;
        }

        // Combine message arguments
        size_t message_len = 0;
        for (int i = 3; i < argc; i++) {
            message_len += strlen(argv[i]) + 1;
        }

        char message[message_len];
        message[0] = '\0';

        for (int i = 3; i < argc; i++) {
            strcat(message, argv[i]);
            if (i < argc - 1)
                strcat(message, " ");
        }

        external_log(level, argv[1], message);
        return EXIT_SUCCESS;
    }

    // Make sure only one instance is running
    if (create_lock_file() != 0) {
        fprintf(stderr, "\033[31mERROR:\033[0m Another instance of Daemon is already running!\n");
        exit(EXIT_FAILURE);
    }
    
    // Handle case when module modified by 3rd party
    is_kanged();
        

    

    // Daemonize service
    if (daemon(0, 0)) {
        log_zenith(LOG_FATAL, "Unable to daemonize service");
        exit(EXIT_FAILURE);
    }

    // Register signal handlers
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);


    log_zenith(LOG_INFO, "Daemon started as PID %d", getpid());
    
    init_global_freq_bounds();
    log_zenith(LOG_INFO, "Detected global min=%d max=%d", global_min_freq, global_max_freq);
        

while (1) {
    sleep(LOOP_INTERVAL);

    // Check if module was updated → exit for safety
    if (access(MODULE_UPDATE, F_OK) == 0) [[clang::unlikely]] {
        log_zenith(LOG_INFO, "Module update detected, exiting.");
        break;
    }

    // STEP 1: Get CPU usage
    float curr_usage = get_cpu_usage();
    if (curr_usage < 0) {
        log_zenith(LOG_WARN, "Failed to read CPU usage");
        continue;
    }

    // STEP 1.1: Is CPU usage different enough to act?
    if (fabs(curr_usage - prev_usage) < 1.0f) {
        log_zenith(LOG_DEBUG, "CPU usage unchanged: %.2f%%", curr_usage);
        continue;
    }

    prev_usage = curr_usage;

    // STEP 2: Calculate target frequency
    int target_freq = calculate_target_frequency(curr_usage);

    // STEP 2.1: Is target frequency same as current?
    if (target_freq == prev_freq) {
        log_zenith(LOG_DEBUG, "Frequency unchanged: %d kHz", target_freq);
        continue;
    }

    // STEP 2.2: Apply new frequency
    apply_frequency_all(target_freq);
    log_zenith(LOG_INFO, "CPU usage: %.2f%%, Applied freq: %d kHz", curr_usage, target_freq);
    prev_freq = target_freq;

    // STEP 3: Screen state check
    if (!is_screen_on()) {
        if (!screen_off) {
            log_zenith(LOG_INFO, "Screen off detected, setting lowest freq");
            set_all_to_min_freq();
            screen_off = 1;
        }

        // STEP 4: Wait until screen is on again
        while (!is_screen_on()) {
            sleep(SLEEP_WHEN_OFF);
        }

        log_zenith(LOG_INFO, "Screen on detected, resuming loop");
        screen_off = 0;
        prev_freq = -1;
        prev_usage = 0.0f;
    }
}

return 0;
}

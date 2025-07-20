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
    
    while (1) {
        sleep(LOOP_INTERVAL);

        // Handle case when module gets updated
        if (access(MODULE_UPDATE, F_OK) == 0) [[clang::unlikely]] {
            log_zenith(LOG_INFO, "Module update detected, exiting.");
            break;
        }

    }
    

    return 0;
}

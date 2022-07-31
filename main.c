#include "httpd.h"
#include <sys/stat.h>
#include <dirent.h>

#include "ecglib.h"

#define CHUNK_SIZE 64 // read 1024 bytes at a time

// Public directory settings
#define PUBLIC_DIR "./public"
#define INDEX_HTML "/index.html"
#define NOT_FOUND_HTML "/404.html"

int main(int c, char **v) {
    char *port = c == 1 ? "8000" : v[1];
    serve_forever(port);
    return 0;
}

int file_exists(const char *file_name) {
    struct stat buffer;
    int exists;

    exists = (stat(file_name, &buffer) == 0);

    return exists;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int read_ecg(const char *file_name) {
    char buf[CHUNK_SIZE];
    FILE *file;
    int err = 1;

    file = fopen(file_name, "r");

    if (file) {
        putc('[', stdout);
        while ((fgets(buf, sizeof(buf), file))) {
            replace_char(buf, '\n', '\0');
            replace_char(buf, '\r', '\0');
            replace_char(buf, '\t', '\0');
            replace_char(buf, ' ', '\0');
            printf("%s, ", buf);
        }
        puts("],");

        err = ferror(file);
        fclose(file);
    }
    return err;
}

void route() {
    ROUTE_START()

    GET("/list"){
            //char dirs[100][5] = {0};
            //printf("List of request headers:\n\n");

            DIR *dp;
            struct dirent *ep;
            unsigned ind = 0;

            dp = opendir ("./");
            if (dp != NULL)
            {
                HTTP_200;
                printf("[");
                while ((ep = readdir (dp))) {
                    unsigned len = strlen(ep->d_name);
                   if (len < 5)
                       continue;
                    if (!strcmp(&(ep->d_name)[len - 4], ".ecg"))
                        printf("\"%s\",", ep->d_name);
                }
                printf("]");
                (void) closedir (dp);
            }
            else
                HTTP_500;

        }


    GET(uri){
            char file_name[255];
            sprintf(file_name, "%s", &uri[1]);

            if (file_exists(file_name)) {
                HTTP_200;
                ecg_process_file(file_name);
                printf("{ \"errors\": %u, \"low_pressure\": %u, "
                       "\"irregular_heartbeat\": %u, "
                       "\"pulse_min\": %f, "
                       "\"pulse_max\": %f, "
                       "\"data\": ",
                       GLOBAL_STATS.errors,
                       GLOBAL_STATS.low_pressure,
                       GLOBAL_STATS.irregular_heartbeat,
                       GLOBAL_STATS.pulse_min,
                       GLOBAL_STATS.pulse_max
                       );

                read_ecg(file_name);
                printf("}\n");
            } else {
                HTTP_404;
            }
        }

        ROUTE_END()
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static char buf[256];

static inline int read_file(const char *path, char *dest, size_t size) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    
    size_t len = fread(dest, 1, size - 1, f);
    dest[len] = '\0';
    fclose(f);
    return len;
}

static void get_loadavg(void) {
    if (read_file("/proc/loadavg", buf, sizeof(buf)) > 0) {
        char *space = strchr(buf, ' ');
        if (space) *space = '\0';
        printf("%.2s ", buf);
    }
}

static void get_memory(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;
    
    unsigned long total = 0, avail = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "MemTotal:", 9) == 0)
            sscanf(buf + 9, "%lu", &total);
        else if (strncmp(buf, "MemAvailable:", 13) == 0)
            sscanf(buf + 13, "%lu", &avail);
    }
    fclose(f);
    
    if (total && avail) {
        unsigned int used_pct = ((total - avail) * 100) / total;
        printf("%u%% ", used_pct);
    }
}

static void get_datetime(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("%02d:%02d", tm->tm_hour, tm->tm_min);
}

int main(void) {
    get_loadavg();
    get_memory();
    get_datetime();
    putchar('\n');
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_CPUS 32
#define STAT_LINE_LEN 256

static char buf[256];

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_stat;

static inline int read_file(const char *path, char *dest, size_t size) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    size_t len = fread(dest, 1, size - 1, f);
    dest[len] = '\0';
    fclose(f);
    return len;
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
        printf("MEM:(%u%%) ", used_pct);
    }
}

static int read_cpu_stats(cpu_stat stats[], int max) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;

    int count = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, "cpu", 3) != 0 || buf[3] == ' ') continue;
        if (count >= max) break;
        sscanf(buf, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu",
               &count,
               &stats[count].user, &stats[count].nice, &stats[count].system,
               &stats[count].idle, &stats[count].iowait,
               &stats[count].irq, &stats[count].softirq,
               &stats[count].steal);
        count++;
    }

    fclose(f);
    return count;
}

static void get_cpu_usage(void) {
    cpu_stat prev[MAX_CPUS], curr[MAX_CPUS];
    int n = read_cpu_stats(prev, MAX_CPUS);
    usleep(100000); // 100ms

    read_cpu_stats(curr, MAX_CPUS);

    for (int i = 0; i < n; i++) {
        unsigned long long prev_idle = prev[i].idle + prev[i].iowait;
        unsigned long long curr_idle = curr[i].idle + curr[i].iowait;

        unsigned long long prev_total = prev[i].user + prev[i].nice + prev[i].system +
                                        prev[i].idle + prev[i].iowait + prev[i].irq +
                                        prev[i].softirq + prev[i].steal;

        unsigned long long curr_total = curr[i].user + curr[i].nice + curr[i].system +
                                        curr[i].idle + curr[i].iowait + curr[i].irq +
                                        curr[i].softirq + curr[i].steal;

        unsigned long long total_diff = curr_total - prev_total;
        unsigned long long idle_diff = curr_idle - prev_idle;

        if (total_diff == 0) total_diff = 1;

        unsigned int usage = (100 * (total_diff - idle_diff)) / total_diff;
        printf("CPU%d:(%u%%) ", i, usage);
    }
}

static void get_datetime(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("%02d:%02d", tm->tm_hour, tm->tm_min);
}

int main(void) {
    get_cpu_usage();
    get_memory();
    get_datetime();
    putchar('\n');
    return 0;
}

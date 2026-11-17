#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#define GB (1024.0 * 1024.0 * 1024.0)

typedef struct {
    double cpu;
    double ram;
    double ram_used;
    double ram_total;
    double disk;
    double disk_used;
    double disk_total;
} PCStatus;

unsigned long long to_number(FILETIME t) {
    return ((unsigned long long)t.dwHighDateTime << 32) | t.dwLowDateTime;
}

int measure_cpu(double *result) {
    FILETIME idle1, kernel1, user1;
    FILETIME idle2, kernel2, user2;

    if (!GetSystemTimes(&idle1, &kernel1, &user1)) return 0;

    Sleep(1000);

    if (!GetSystemTimes(&idle2, &kernel2, &user2)) return 0;

    unsigned long long idle_a = to_number(idle1);
    unsigned long long idle_b = to_number(idle2);
    unsigned long long kernel_a = to_number(kernel1);
    unsigned long long kernel_b = to_number(kernel2);
    unsigned long long user_a = to_number(user1);
    unsigned long long user_b = to_number(user2);

    if (idle_b < idle_a || kernel_b < kernel_a || user_b < user_a)
        return 0;

    unsigned long long idle_diff = idle_b - idle_a;
    unsigned long long total_diff =
        (kernel_b - kernel_a) + (user_b - user_a);

    if (total_diff == 0 || idle_diff > total_diff) return 0;

    *result = 100.0 * (double)(total_diff - idle_diff) / (double)total_diff;
    return 1;
}

int measure_pc(PCStatus *pc) {
    MEMORYSTATUSEX mem = {0};
    ULARGE_INTEGER disk_total, disk_free;

    if (!measure_cpu(&pc->cpu)) return 0;

    mem.dwLength = sizeof(mem);
    if (!GlobalMemoryStatusEx(&mem)) return 0;
    if (mem.ullTotalPhys == 0) return 0;

    pc->ram_total = (double)mem.ullTotalPhys / GB;
    pc->ram_used = (double)(mem.ullTotalPhys - mem.ullAvailPhys) / GB;
    pc->ram = 100.0 * pc->ram_used / pc->ram_total;

    if (!GetDiskFreeSpaceExW(L"C:\\", NULL, &disk_total, &disk_free))
        return 0;
    if (disk_total.QuadPart == 0) return 0;

    pc->disk_total = (double)disk_total.QuadPart / GB;
    pc->disk_used = (double)(disk_total.QuadPart - disk_free.QuadPart) / GB;
    pc->disk = 100.0 * pc->disk_used / pc->disk_total;

    return 1;
}

void print_pc(const PCStatus *pc) {
    SYSTEMTIME now;
    GetLocalTime(&now);

    printf("\n------ PC STATUS (%04d-%02d-%02d %02d:%02d:%02d) ------\n",
           now.wYear, now.wMonth, now.wDay,
           now.wHour, now.wMinute, now.wSecond);
    printf("CPU   : %.1f %%\n", pc->cpu);
    printf("RAM   : %.1f %% (%.1f / %.1f GB)\n",
           pc->ram, pc->ram_used, pc->ram_total);
    printf("Disk C: %.1f %% (%.1f / %.1f GB)\n",
           pc->disk, pc->disk_used, pc->disk_total);
}

int main(void) {
    int menu;
    PCStatus pc;

    while (1) {
        printf("\n===== PC STATUS ANALYZER =====\n");
        printf("1. Check PC status\n");
        printf("2. Measure 5 times and show averages\n");
        printf("0. Exit\n");
        printf("Select: ");

        if (scanf("%d", &menu) != 1) {
            printf("Please enter a number.\n");
            break;
        }
        if (menu == 0) break;

        if (menu == 1) {

            if (measure_pc(&pc)) print_pc(&pc);
            else printf("Failed to get PC information.\n");
        } else if (menu == 2) {

            double cpu_sum = 0.0;
            double ram_sum = 0.0;
            int count = 0;

            for (int i = 0; i < 5; i++) {
                printf("\nMeasuring %d/5 ...\n", i + 1);
                if (!measure_pc(&pc)) {
                    printf("Measurement failed.\n");
                    break;
                }
                print_pc(&pc);
                cpu_sum += pc.cpu;
                ram_sum += pc.ram;
                count++;
            }

            if (count > 0) {
                printf("\n--- AVERAGE (%d measurements) ---\n", count);
                printf("CPU average: %.1f %%\n", cpu_sum / count);
                printf("RAM average: %.1f %%\n", ram_sum / count);
            }
        } else {
            printf("Please choose 0, 1, or 2.\n");
        }
    }

    printf("Program finished.\n");
    return 0;
}

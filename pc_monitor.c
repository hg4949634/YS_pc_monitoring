#include <windows.h>
#include <stdio.h>

typedef struct {
    double cpu_percent;
    double memory_percent;
    double memory_used_gb;
    double memory_total_gb;
    double disk_percent;
    double disk_used_gb;
    double disk_total_gb;
} PCStatus;

int get_cpu_usage(double *percent) {

}

int get_pc_status(PCStatus *status) {
    
}

int main() {
    int menu;
    PCStatus status;
}
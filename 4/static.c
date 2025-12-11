#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "integral.h"
#include "area.h"

int main() {
    char buffer[1024];
    char cmd[10];
    int len;
    while (1) {
        len = snprintf(buffer, sizeof(buffer), 
                 "Enter command (1 a b e / 2 a b / exit): ");
        write(STDOUT_FILENO, buffer, len);
        fflush(stdout);
        scanf("%s", cmd);
        
        if (strcmp(cmd, "exit") == 0) break;
        
        if (strcmp(cmd, "1") == 0) {
            float a, b, e;
            scanf("%f %f %f", &a, &b, &e);
            float result = sin_integral_rect(a, b, e);
            len = snprintf(buffer, sizeof(buffer),
                    "Integral sin(x) on [%.2f, %.2f]: %.6f\n", 
                    a, b, result);
            write(STDOUT_FILENO, buffer, len);
        } 
        else if (strcmp(cmd, "2") == 0) {
            float a, b;
            scanf("%f %f", &a, &b);
            float result = area_rect(a, b);
            len = snprintf(buffer, sizeof(buffer),
                    "Area (%.2f x %.2f): %.6f\n", 
                    a, b, result);
            write(STDOUT_FILENO, buffer, len);
        }
        else {
            len = snprintf(buffer, sizeof(buffer), "Unknown command\n");
            write(STDOUT_FILENO, buffer, len);
        }
    }
    
    return 0;
}
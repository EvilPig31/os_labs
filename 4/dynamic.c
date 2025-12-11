#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef float (*integral_func)(float, float, float);
typedef float (*area_func)(float, float);

int main() {
    char buffer[1024];
    int len;
    void *lib = dlopen("./libmath.so", RTLD_LAZY);
    if (!lib) {
        len = snprintf(buffer, sizeof(buffer), 
                "Error lin loading: %s\n", dlerror());
        write(STDOUT_FILENO, buffer, len);
        return 1;
    }
    
    integral_func sin_integral = (integral_func)dlsym(lib, "sin_integral_rect");
    area_func area = (area_func)dlsym(lib, "area_rect");
    int impl = 1;
    
    char cmd[10];
    
    while (1) {
        len = snprintf(buffer, sizeof(buffer), "\nCurrent realisation: %d\n", impl);
        write(STDOUT_FILENO, buffer, len);
        
        len = snprintf(buffer, sizeof(buffer),
                "Enter command (0 / 1 a b e / 2 a b / exit): ");
        write(STDOUT_FILENO, buffer, len);
        fflush(stdout);
        scanf("%s", cmd);
        
        if (strcmp(cmd, "exit") == 0) break;
        
        if (strcmp(cmd, "0") == 0) {
            impl = (impl == 1) ? 2 : 1;
            
            if (impl == 1) {
                sin_integral = (integral_func)dlsym(lib, "sin_integral_rect");
                area = (area_func)dlsym(lib, "area_rect");
            } else {
                sin_integral = (integral_func)dlsym(lib, "sin_integral_trap");
                area = (area_func)dlsym(lib, "area_triangle");
            }
            
            len = snprintf(buffer, sizeof(buffer), 
                    "Switched to realisation %d\n", impl);
            write(STDOUT_FILENO, buffer, len);
            continue;
        }
        
        if (strcmp(cmd, "1") == 0) {
            float a, b, e;
            scanf("%f %f %f", &a, &b, &e);
            float result = sin_integral(a, b, e);
            len = snprintf(buffer, sizeof(buffer), "Result: %.6f\n", result);
            write(STDOUT_FILENO, buffer, len);
        } 
        else if (strcmp(cmd, "2") == 0) {
            float a, b;
            scanf("%f %f", &a, &b);
            float result = area(a, b);
            len = snprintf(buffer, sizeof(buffer), "Result: %.6f\n", result);
            write(STDOUT_FILENO, buffer, len);
        }
        else {
            len = snprintf(buffer, sizeof(buffer), "Unknown command\n");
            write(STDOUT_FILENO, buffer, len);
        }
    }
    
    dlclose(lib);
    return 0;
}
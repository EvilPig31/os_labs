#include "integral.h"
#include <math.h>

float sin_integral_rect(float a, float b, float e) {
    float result = 0.0;
    float step = (e > 0) ? e : 0.001f;
    
    for (float x = a; x < b; x += step) {
        float next_x = (x + step < b) ? x + step : b;
        float mid = (x + next_x) / 2.0f;
        result += sinf(mid) * (next_x - x);
    }
    
    return result;
}

float sin_integral_trap(float a, float b, float e) {
    float result = 0.0;
    float step = (e > 0) ? e : 0.001f;
    
    for (float x = a; x < b; x += step) {
        float next_x = (x + step < b) ? x + step : b;
        result += (sinf(x) + sinf(next_x)) * (next_x - x) / 2.0f;
    }
    
    return result;
}
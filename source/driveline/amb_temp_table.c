
#include <stdbool.h>
#include <stdint.h>

/* Precomputed AMB Temperature Table */
typedef struct {
    float ohms;
    float celsius;
} amb_temp_point_t;

static const amb_temp_point_t amb_temp_table[] = {
    { 101770.0f, -40.0f },
    { 2820.0f,   25.0f },
    { 988.1f,   50.0f },
    { 179.6f,  100.0f },
    { 89.11f,  125.0f },

};
#define AMB_TEMP_TABLE_LEN (sizeof(amb_temp_table) / sizeof(amb_temp_table[0]))

float amb_temp_ohms_to_celsius(float ohms) {
    if (ohms >= amb_temp_table[0].ohms)
        return amb_temp_table[0].celsius;
    if (ohms <= amb_temp_table[AMB_TEMP_TABLE_LEN - 1].ohms)
        return amb_temp_table[AMB_TEMP_TABLE_LEN - 1].celsius;

    for (int i = 0; i < AMB_TEMP_TABLE_LEN - 1; i++) {
        if (ohms <= amb_temp_table[i].ohms && ohms >= amb_temp_table[i + 1].ohms) {
            float r0 = amb_temp_table[i].ohms,     t0 = amb_temp_table[i].celsius;
            float r1 = amb_temp_table[i + 1].ohms, t1 = amb_temp_table[i + 1].celsius;
            return t0 + (t1 - t0) * ((ohms - r0) / (r1 - r0));
        }
    }
    return 0.0f; 
}
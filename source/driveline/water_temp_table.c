#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float ohms;
    float celsius;
} water_temp_point_t;

static const water_temp_point_t water_temp_table[] = {
    { 94331.0f, -40.0f },
    { 68175.0f, -35.0f },
    { 49796.0f, -30.0f },
    { 36743.0f, -25.0f },
    { 27377.0f, -20.0f },
    { 20590.0f, -15.0f },
    { 15626.0f, -10.0f },
    { 11960.0f,  -5.0f },
    {  9231.0f,   0.0f },
    {  7180.0f,   5.0f },
    {  5628.0f,  10.0f },
    {  4444.0f,  15.0f },
    {  3533.0f,  20.0f },
    {  2828.0f,  25.0f },
    {  2278.0f,  30.0f },
    {  1847.0f,  35.0f },
    {  1506.0f,  40.0f },
    {  1235.0f,  45.0f },
    {  1018.0f,  50.0f },
    {   844.0f,  55.0f },
    {   703.0f,  60.0f },
    {   589.0f,  65.0f },
    {   495.0f,  70.0f },
    {   418.0f,  75.0f },
    {   355.0f,  80.0f },
    {   303.0f,  85.0f },
    {   259.0f,  90.0f },
    {   222.0f,  95.0f },
    {   192.0f, 100.0f },
};

#define WATER_TEMP_TABLE_LEN (sizeof(water_temp_table) / sizeof(water_temp_table[0]))

float water_temp_ohms_to_celsius(float ohms) {
    if (ohms >= water_temp_table[0].ohms)
        return water_temp_table[0].celsius;
    if (ohms <= water_temp_table[WATER_TEMP_TABLE_LEN - 1].ohms)
        return water_temp_table[WATER_TEMP_TABLE_LEN - 1].celsius;

    for (int i = 0; i < WATER_TEMP_TABLE_LEN - 1; i++) {
        if (ohms <= water_temp_table[i].ohms && ohms >= water_temp_table[i + 1].ohms) {
            float r0 = water_temp_table[i].ohms,     t0 = water_temp_table[i].celsius;
            float r1 = water_temp_table[i + 1].ohms, t1 = water_temp_table[i + 1].celsius;
            return t0 + (t1 - t0) * ((ohms - r0) / (r1 - r0));
        }
    }
    return 0.0f;
}
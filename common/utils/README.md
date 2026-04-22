# PER Utility Functions

`max.h` provides inline functions to compute the maximum of 2, 3, or 4 integers or floats.
`min.h` provides inline functions to compute the minimum of 2, 3, or 4 integers or floats.

Example usage:
```c
int a = 5, b = 3, c = 10, d = 7;
int max = MAXOF(a, b); // max will be 5
max = MAXOF(a, b, c); // max will be 10
max = MAXOF(a, b, c, d); // max will be 10

float x = 1.5f, y = 2.3f, z = 0.8f;
float minf = MINOF(x, y); // minf will be 1.5
minf = MINOF(x, y, z); // minf will be 0.8
```

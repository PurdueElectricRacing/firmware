#include "SFS.h" 

// calibration
#define MAG_CALIBRATION 1.0
#define ACC_CALIBRATION 1.0
#define GYRO_CALIBRATION 1.0
#define VEL_CALIBRATION 0.001
#define POS_DEG_CALIBRATION 0.0000001
#define POS_H_CALIBRATION 0.001

// clamping
#define MIN_MAG 200.0
#define MAX_MAG -200.0

#define MIN_GYRO -2.0
#define MAX_GYRO 2.0

#define MIN_ACC -25.0
#define MAX_ACC 25.0

#define MIN_POS -300.0
#define MAX_POS 300.0

#define MIN_VEL -30.0
#define MAX_VEL 30.0

void SFS_pp(ExtU* rtU);
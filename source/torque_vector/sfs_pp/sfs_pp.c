#include "sfs_pp.h"
#include "SFS.h"
#include "can_parse.h"
#include "common_defs.h"

void SFS_pp(ExtU *rtU)
{

    if (0)
    {
        rtU->pos[0] = CLAMP(0, MIN_POS, MAX_POS);
        rtU->pos[1] = CLAMP(0, MIN_POS, MAX_POS);
        rtU->pos[2] = CLAMP(0, MIN_POS, MAX_POS);

        rtU->vel[0] = CLAMP(0, MIN_VEL, MAX_VEL);
        rtU->vel[1] = CLAMP(0, MIN_VEL, MAX_VEL);
        rtU->vel[2] = CLAMP(0, MIN_VEL, MAX_VEL);
    }
    else
    {
    }
}

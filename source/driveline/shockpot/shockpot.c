#include "shockpot.h"
#include "can_parse.h"
#include "common/phal_L4/dma/dma.h"
#include "force.h"

extern q_handle_t q_tx_can;

static int shockPots [2][N_REAR] = {0};         // 0 - left, 1 - right
// TODO: convert to using the data struct
volatile raw_shock_pots_t raw_shock_pots;
int start = 0;

// float n_rear_left;
// float n_rear_right;
// float n_front_left;
// float n_front_right;

float pot_speed_r;
float pot_speed_l;

float n_rear_left;
float n_rear_right;
float n_front_left;
float n_front_right;

void shockpotInit()
{

}


void shockpot1000Hz()
{
    shockPots[0][start] = raw_shock_pots.pot_left;
    shockPots[1][start] = raw_shock_pots.pot_right;
    // float pot_speed_r = pot_speed(shockPots[0], RESOLUTION_FRONT, DELTA_FRONT, 10, start);
    // float pot_speed_l = pot_speed(shockPots[1], RESOLUTION_FRONT, DELTA_FRONT, 10, start);

    force_rear(&n_rear_left, &n_rear_right, shockPots[1], shockPots[1], start);
    // n_rear (shockPots[0], shockPots[1], &n_rear_left, &n_rear_right, start);
    start = (start + N_REAR - 1) % N_REAR;
  //  SEND_FRONT_WHEEL_DATA(q_tx_can, 0, 0, n_rear_left, n_rear_right);
}


// #include "amk/amk2.h"
// #include "common/can_library/faults_common.h"
// #include "common/can_library/generated/MAIN_MODULE.h"
// #include "common/can_library/generated/can_types.h"

// static constexpr uint32_t MIN_BUZZING_TIME_MS = 2500;

// typedef struct {
//     CarState_t current_state;
//     CarState_t next_state;
//     AMK_t amk_front_right;
//     AMK_t amk_front_left;
//     AMK_t amk_rear_left;
//     AMK_t amk_rear_right;

//     uint32_t buzzer_start_time;

//     bool is_regen_enabled;
//     bool last_start_button_state;
//     bool brake_light;
//     bool is_sdc_closed;
//     bool buzzer;
// } car_t;

// car_t g_car;

// void init_periodic();
// void idle_periodic();
// void precharge_periodic();
// void energized_periodic();
// void buzzing_periodic();
// void ready2drive_periodic();
// void error_periodic();

// static inline bool is_SDC_open() {
//     return true;
// }

// static inline bool is_init_complete() {
//     return true;
// }

// static inline bool is_TSMS_high() {
//     return true;
// }

// static inline bool is_precharge_complete() {
//     return true;
// }

// static inline bool is_start_button_pressed() {
//     if (can_data.start_button.stale) {
//         return false;
//     }

//     bool current_state = can_data.start_button.start;
//     bool edge = current_state && !g_car.last_start_button_state;

//     g_car.last_start_button_state = current_state;
//     return edge;
// }

// static inline bool is_buzzing_time_elapsed() {
//     return (OS_TICKS - g_car.buzzer_start_time >= MIN_BUZZING_TIME_MS);
// }

// void buzzing_periodic() {
    
// }

// /* BRAKE LIGHT CONFIG */
// #define BRAKE_LIGHT_ON_THRESHOLD  (170)
// #define BRAKE_LIGHT_OFF_THRESHOLD (70)

// /**
// * Brake Light Control
// * The on threshold is larger than the off threshold to
// * behave similar to a Shmitt-trigger, preventing blinking
// * during a transition
// */
// void set_brake_light() {
//     if (can_data.filt_throttle_brake.brake > BRAKE_LIGHT_ON_THRESHOLD) {
//         if (!g_car.brake_light) {
//             g_car.brake_light = true;
//         }
//     } else if (can_data.filt_throttle_brake.brake < BRAKE_LIGHT_OFF_THRESHOLD) {
//         if (g_car.brake_light) {
//             g_car.brake_light = false;
//         }
//     }
// }

// void car_init() {
//     // enter INIT at n_reset
//     g_car.current_state = CARSTATE_INIT;
//     g_car.next_state    = CARSTATE_INIT;
// }

// void fsm_periodic() {
//     g_car.current_state = g_car.next_state;
//     g_car.next_state    = g_car.current_state; // explicit self loop

//     // check SDC before doing anything else
//     if (is_SDC_open()) {
//         g_car.next_state = CARSTATE_FATAL;
//         return;
//     }

//     set_brake_light();

//     switch (g_car.current_state) {
//         case CARSTATE_INIT: {
//             init_periodic();

//             if (is_init_complete()) {
//                 g_car.next_state = CARSTATE_IDLE;
//             }
//             break;
//         }
//         case CARSTATE_IDLE: {
//             idle_periodic();

//             if (is_TSMS_high()) {
//                 g_car.next_state = CARSTATE_PRECHARGING;
//             }
//             break;
//         }
//         case CARSTATE_PRECHARGING: {
//             precharge_periodic();

//             if (is_precharge_complete()) {
//                 g_car.next_state = CARSTATE_ENERGIZED;
//             }
//             break;
//         }
//         case CARSTATE_ENERGIZED: {
//             energized_periodic();

//             if (is_start_button_pressed()) {
//                 g_car.buzzer_start_time = OS_TICKS;
//                 g_car.next_state = CARSTATE_BUZZING;
//             }
//             break;
//         }
//         case CARSTATE_BUZZING: {
//             buzzing_periodic();

//             if (is_buzzing_time_elapsed()) {
//                 g_car.next_state = CARSTATE_READY2DRIVE;
//             }
//             break;
//         }
//         case CARSTATE_READY2DRIVE: {
//             ready2drive_periodic();

//             if (is_start_button_pressed()) {
//                 g_car.next_state = CARSTATE_IDLE;
//             }
//             break;
//         }
//         case CARSTATE_FATAL: {
//             error_periodic();

//             if (!is_SDC_open()) {
//                 g_car.next_state = CARSTATE_IDLE;
//             }
//             break;
//         }
//         default: { // should never reach here
//             g_car.next_state = CARSTATE_FATAL;
//             break;
//         }
//     }
// }
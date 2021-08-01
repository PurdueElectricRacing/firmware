/** @file pedals.h
 * 
 * @brief Perform ADC conversion on the 4 pedal chanels and allows for access to this data. 
 *
 */ 


#ifndef PEDALS_H_
#define PEDALS_H_

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct
{
    float t1;   // Throttle 1
    float t2;   // Throttle 2
    float b1;   // Break 1
    float b2;   // Break 2
} pedals_positions_S;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

// None

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

// None

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

// None

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/**
 * @brief Initilize the Pedals module. This should be called once at startup
 *   or whenever you want to re-initilize this module.
 * 
 */
void pedals_Init();

/**
 * @brief Perform ADC conversion on the pedals and update the inner module state.
 * 
 */
void pedals_UpdatePositions();

/**
 * @brief Get the most recent Pedal positions copied into the proveded struct.
 * 
 * @param positions 
 */
void pedals_GetPositions(pedals_positions_S* positions);

#endif
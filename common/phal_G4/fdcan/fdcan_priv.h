
#ifndef __PHAL_G4_FDCAN_PRIV_H__
#define __PHAL_G4_FDCAN_PRIV_H__

#define RCC_FDCANCLKSOURCE_HSE   0x00000000U
#define RCC_FDCANCLKSOURCE_PLL   RCC_CCIPR_FDCANSEL_0
#define RCC_FDCANCLKSOURCE_PCLK1 RCC_CCIPR_FDCANSEL_1

#define FDCAN_ACCEPT_IN_RX_FIFO0 ((uint32_t)0x00000000U) /*!< Accept in Rx FIFO 0 */
#define FDCAN_ACCEPT_IN_RX_FIFO1 ((uint32_t)0x00000001U) /*!< Accept in Rx FIFO 1 */
#define FDCAN_REJECT             ((uint32_t)0x00000002U) /*!< Reject              */

#define SRAMCAN_FLS_NBR (28U) /* Max. Filter List Standard Number      */
#define SRAMCAN_FLE_NBR (64U) /* Max. Filter List Extended Number      */
#define SRAMCAN_RF0_NBR (3U) /* RX FIFO 0 Elements Number             */
#define SRAMCAN_RF1_NBR (3U) /* RX FIFO 1 Elements Number             */
#define SRAMCAN_TEF_NBR (3U) /* TX Event FIFO Elements Number         */
#define SRAMCAN_TFQ_NBR (3U) /* TX FIFO/Queue Elements Number         */

#define SRAMCAN_FLS_SIZE (1U * 4U) /* Filter Standard Element Size in bytes */
#define SRAMCAN_FLE_SIZE (2U * 4U) /* Filter Extended Element Size in bytes */
#define SRAMCAN_RF0_SIZE (18U * 4U) /* RX FIFO 0 Elements Size in bytes      */
#define SRAMCAN_RF1_SIZE (18U * 4U) /* RX FIFO 1 Elements Size in bytes      */
#define SRAMCAN_TEF_SIZE (2U * 4U) /* TX Event FIFO Elements Size in bytes  */
#define SRAMCAN_TFQ_SIZE (18U * 4U) /* TX FIFO/Queue Elements Size in bytes  */

#define SRAMCAN_FLSSA ((uint32_t)0) /* Filter List Standard Start
                                                                                            Address                  */
#define SRAMCAN_FLESA ((uint32_t)(SRAMCAN_FLSSA + (SRAMCAN_FLS_NBR * SRAMCAN_FLS_SIZE))) /* Filter List Extended Start
                                                                                            Address                  */
#define SRAMCAN_RF0SA ((uint32_t)(SRAMCAN_FLESA + (SRAMCAN_FLE_NBR * SRAMCAN_FLE_SIZE))) /* Rx FIFO 0 Start Address  */
#define SRAMCAN_RF1SA ((uint32_t)(SRAMCAN_RF0SA + (SRAMCAN_RF0_NBR * SRAMCAN_RF0_SIZE))) /* Rx FIFO 1 Start Address  */
#define SRAMCAN_TEFSA ((uint32_t)(SRAMCAN_RF1SA + (SRAMCAN_RF1_NBR * SRAMCAN_RF1_SIZE))) /* Tx Event FIFO Start
                                                                                            Address */
#define SRAMCAN_TFQSA ((uint32_t)(SRAMCAN_TEFSA + (SRAMCAN_TEF_NBR * SRAMCAN_TEF_SIZE))) /* Tx FIFO/Queue Start
                                                                                            Address                  */
#define SRAMCAN_SIZE  ((uint32_t)(SRAMCAN_TFQSA + (SRAMCAN_TFQ_NBR * SRAMCAN_TFQ_SIZE))) /* Message RAM size         */

#endif // __PHAL_G4_FDCAN_PRIV_H__

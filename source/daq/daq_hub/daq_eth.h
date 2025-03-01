#ifndef __DAQ_ETH_H__
#define __DAQ_ETH_H__

// W5500
#define ETH_PHY_VERSION_ID      0x04
#define ETH_PHY_RESET_PERIOD_MS   10

// W5500 has 8 sockets internally
#define DAQ_SOCKET_UDP_BROADCAST   0
#define DAQ_SOCKET_TCP             1
#define DAQ_SOCKET_FTP_CTRL0       2  // FTP uses 3 sockets
#define DAQ_SOCKET_FTP_DATA        3
#define DAQ_SOCKET_FTP_CTRL1       4

typedef enum
{
    ETH_LINK_IDLE        = 0,
    ETH_LINK_STARTING    = 1,
    ETH_LINK_DOWN        = 2,
    ETH_LINK_UP          = 3,
    ETH_LINK_FAIL        = 4,
} eth_state_t;

typedef enum
{
    ETH_TCP_IDLE         = 0,
    ETH_TCP_LISTEN       = 1,
    ETH_TCP_ESTABLISHED  = 2,
    ETH_TCP_FAIL         = 3,
} eth_tcp_state_t;

typedef enum
{
    ETH_ERROR_NONE       = 0,
    ETH_ERROR_INIT       = 1,
    ETH_ERROR_VERS       = 2,
    ETH_ERROR_UDP_SOCK   = 3,
    ETH_ERROR_UDP_SEND   = 4,
    ETH_ERROR_TCP_SOCK   = 5,
    ETH_ERROR_TCP_LISTEN = 6,
    ETH_ERROR_TCP_SEND   = 7,
} eth_error_t;

typedef enum
{
    TCP_CMD_HANDSHAKE    = 0,
    TCP_CMD_CAN_FRAME    = 1,
    TCP_CMD_UDS_FRAME    = 2,
} tcp_cmd_t;

typedef enum __attribute__ ((__packed__))
{
    DAQ_FRAME_CAN_RX     = 0, //!< RX to DAQ over CAN
    DAQ_FRAME_TCP2CAN    = 1, //!< RX to DAQ over TCP, relay to other nodes on CAN
    DAQ_FRAME_TCP2DAQ    = 2, //!< RX to DAQ over TCP, message intended for DAQ
    DAQ_FRAME_TCP_TX     = 3, //!< TX from DAQ over TCP
    DAQ_FRAME_UDP_TX     = 4, //!< TX from DAQ over UDP
} daq_frame_type_t;
static_assert(sizeof(daq_frame_type_t) == sizeof(uint8_t));

void eth_update_periodic(void);

#endif // __DAQ_ETH_H__

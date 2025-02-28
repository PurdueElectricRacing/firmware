#ifndef _FTPD_H_
#define _FTPD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
* Wiznet.
* (c) Copyright 2002, Wiznet.
*
* Filename	: ftpd.h
* Version	: 1.0
* Programmer(s)	:
* Created	: 2003/01/28
* Description   : Header file of FTP daemon. (AVR-GCC Compiler)
*/

#include <stdint.h>

#define F_FILESYSTEM // If your target support a file system, you have to activate this feature and implement.

#if defined(F_FILESYSTEM)
#include "ff.h"
#endif

#define F_APP_FTP

#define _FTP_BUF_SIZE 512//8192
// #define _FTP_DEBUG_


#define LINELEN		100
//#define DATA_BUF_SIZE	100
#if !defined(F_FILESYSTEM)
#define _MAX_SS		512
#endif

#define CTRL_SOCK	DAQ_SOCKET_FTP_CTRL0
#define DATA_SOCK	DAQ_SOCKET_FTP_DATA
#define CTRL_SOCK1	DAQ_SOCKET_FTP_CTRL1


#define	IPPORT_FTPD	20	/* FTP Data port */
#define	IPPORT_FTP	21	/* FTP Control port */

#define HOSTNAME	"PER_DAQ"
#define VERSION		"1.0"

/* FTP commands */
enum ftp_cmd {
	USER_CMD,
	ACCT_CMD,
	PASS_CMD,
	TYPE_CMD,
	LIST_CMD,
	CWD_CMD,
	DELE_CMD,
	NAME_CMD,
	QUIT_CMD,
	RETR_CMD,
	STOR_CMD,
	PORT_CMD,
	NLST_CMD,
	PWD_CMD,
	XPWD_CMD,
	MKD_CMD,
	XMKD_CMD,
	XRMD_CMD,
	RMD_CMD,
	STRU_CMD,
	MODE_CMD,
	SYST_CMD,
	XMD5_CMD,
	XCWD_CMD,
	FEAT_CMD,
	PASV_CMD,
	SIZE_CMD,
	MLSD_CMD,
	APPE_CMD,
	NO_CMD,
};

enum ftp_type {
	ASCII_TYPE,
	IMAGE_TYPE,
	LOGICAL_TYPE
};

enum ftp_state {
	FTPS_NOT_LOGIN,
	FTPS_LOGIN
};

enum datasock_state{
	DATASOCK_IDLE,
	DATASOCK_READY,
	DATASOCK_START
};

enum datasock_mode{
	PASSIVE_MODE,
	ACTIVE_MODE
};

enum ftp_use_status{
	STATUS_USED,
	STATUS_NOT_USED
};

struct ftpd {
	uint8_t control;			/* Control stream */
	uint8_t data;			/* Data stream */

	enum ftp_type type;		/* Transfer type */
	enum ftp_state state;

	enum ftp_cmd current_cmd;

	enum datasock_state dsock_state;
	enum datasock_mode dsock_mode;

	enum ftp_use_status ID_Enable;
	enum ftp_use_status PW_Enable;

	char username[LINELEN];		/* Arg to USER command */
	char userpassword[LINELEN];
	char workingdir[LINELEN];
	char filename[LINELEN];

#if defined(F_FILESYSTEM)
	FIL fil;	// FatFs File objects
	FRESULT fr;	// FatFs function common result code
#endif

};

#ifndef un_I2cval
typedef union _un_l2cval {
	uint32_t	lVal;
	uint8_t		cVal[4];
}un_l2cval;
#endif

void ftpd_init(uint8_t * src_ip);
uint8_t ftpd_run(uint8_t * dbuf);
char proc_ftpd(uint8_t sn, char * buf);

char ftplogin(uint8_t sn,char * pass);

int pport(char * arg);

int sendit(char * command);
int recvit(char * command);

long sendfile(uint8_t s, char * command);
long recvfile(uint8_t s);

#if defined(F_FILESYSTEM)
void print_filedsc(FIL *fil);
#endif

#ifdef __cplusplus
}
#endif

#endif // _FTPD_H_

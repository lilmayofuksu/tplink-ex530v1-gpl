/*
 * GSM 07.10 Implementation with User Space Serial Ports
 *
 * Code heavily based on gsmMuxd written by
 * Copyright (C) 2003 Tuukka Karvonen <tkarvone@iki.fi>
 * Modified November 2004 by David Jander <david@protonic.nl>
 * Modified January 2006 by Tuukka Karvonen <tkarvone@iki.fi>
 * Modified January 2006 by Antti Haapakoski <antti.haapakoski@iki.fi>
 * Modified March 2006 by Tuukka Karvonen <tkarvone@iki.fi>
 * Modified October 2006 by Vasiliy Novikov <vn@hotbox.ru>
 *
 * Copyright (C) 2008 M. Dietrich <mdt@emdete.de>
 * Modified January 2009 by Ulrik Bech Hald <ubh@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* If compiled with the MUX_ANDROID flag this mux will be enabled to run under Android */

/**************************/
/* INCLUDES                          */
/**************************/
#include <errno.h>
#include <fcntl.h>
//#include <features.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define CMUX_DRIVER_VERSION "Quectel_Linux_CMUX_Driver_V2.0.3"

#define MAX_BUF_SIZE    256
//#define XINGWANG_DEBUG
#ifdef XINGWANG_DEBUG
//#include "ct_linuxframwork_log.h"
#define ct_linuxframwork_radiolog(lvl, tag, f, ...) do {{fprintf(stderr,"%d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);}}while(0)
#endif

#ifdef MUX_ANDROID //full path is required on some boards, i donot know why
#define MUX_CHN_DIR "/data/misc/rild/"
int ql_system(const char *string) {
    if (system(string)) {
        char system_bin[MAX_BUF_SIZE] = {0};
        strcpy(system_bin, "/system/bin/");
        strncat(system_bin, string, MAX_BUF_SIZE - strlen(system_bin));
        return system(system_bin);
    }
    return 0;
}
#else
#define MUX_CHN_DIR "/dev/chn/"
#define ql_system system
#endif

/**************************/
/* DEFINES                            */
/**************************/
/*Logging*/
#ifndef MUX_ANDROID
#include <syslog.h>
//  #define LOG(lvl, f, ...) do{if(lvl<=syslog_level)syslog(lvl,"%s:%d:%s(): " f "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);}while(0)
#ifdef XINGWANG_DEBUG
#define DEBUG
#ifdef DEBUG
#define MUX_TAG "gsm0710"
#define LOGMUX(lvl,f,...) do {\
    if (lvl <= LOG_ERR) {ct_linuxframwork_radiolog(CT_LOG_ERROR,MUX_TAG,f,##__VA_ARGS__);}\
    else if (lvl <= LOG_WARNING) {ct_linuxframwork_radiolog(CT_LOG_WARN,MUX_TAG,f,##__VA_ARGS__);}\
    else if (lvl <= LOG_INFO) {ct_linuxframwork_radiolog(CT_LOG_INFO,MUX_TAG,f,##__VA_ARGS__);}\
    else if (lvl <= LOG_DEBUG) {ct_linuxframwork_radiolog(CT_LOG_DEBUG,MUX_TAG,f,##__VA_ARGS__);}\
    else {};\
} while(0)

#define LOGMUXE(f,...) ct_linuxframwork_radiolog(CT_LOG_ERROR,MUX_TAG,f,##__VA_ARGS__)
#define LOGMUXW(f,...) ct_linuxframwork_radiolog(CT_LOG_WARN,MUX_TAG,f,##__VA_ARGS__)
#define LOGMUXD(f,...) ct_linuxframwork_radiolog(CT_LOG_DEBUG,MUX_TAG,f,##__VA_ARGS__)
#define LOGMUXI(f,...) ct_linuxframwork_radiolog(CT_LOG_INFO,MUX_TAG,f,##__VA_ARGS__)
#else
#define LOGMUX(lvl,f,...) do {} while(0)
#endif
#else
#ifndef MUX_ANDROID
static const char * cmux_time(void);
#endif
#define LOGMUX(lvl,f,...) do{if((lvl<=syslog_level) || ql_cmux_debug){\
                                if (logtofile){\
                                  fprintf(muxlogfile,"%s %d:%s(): " f "\n", cmux_time(), __LINE__, __FUNCTION__, ##__VA_ARGS__);\
                                  fflush(muxlogfile);}\
                                else\
                                  fprintf(stderr,"%s %d:%s(): " f "\n", cmux_time(), __LINE__, __FUNCTION__, ##__VA_ARGS__);\
                                }\
                            }while(0)
#endif
#else //will enable logging using android logging framework (not to file)
#define LOG_NDEBUG 0
#define LOG_TAG "MUXD"
#include "../ql-log.h"
#ifdef USE_NDK
#define LOGMUX(lvl,f,...) do{if((lvl<=syslog_level) || ql_cmux_debug) {\
                                __android_log_print(android_log_lvl_convert[lvl], "RIL MUXD", "%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);}\
                          }while(0)
#else
#define LOGMUX(lvl,f,...) do{if((lvl<=syslog_level) || ql_cmux_debug) {\
                                __android_log_buf_print(LOG_ID_RADIO, android_log_lvl_convert[lvl],LOG_TAG,"MUXD %d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);}\
                          }while(0)
#endif



//just dummy defines since were not including syslog.h.
#define LOG_EMERG    0
#define LOG_ALERT    1
#define LOG_CRIT    2
#define LOG_ERR        3
#define LOG_WARNING    4
#define LOG_NOTICE    5
#define LOG_INFO    6
#define LOG_DEBUG    7

/* Android's log level are in opposite order of syslog.h */
int android_log_lvl_convert[8]={ANDROID_LOG_SILENT, /*8*/
                                ANDROID_LOG_SILENT, /*7*/
                                ANDROID_LOG_FATAL, /*6*/
                                ANDROID_LOG_ERROR,/*5*/
                                ANDROID_LOG_WARN,/*4*/
                                ANDROID_LOG_INFO,/*3*/
                                ANDROID_LOG_DEBUG,/*2*/
                                ANDROID_LOG_VERBOSE};/*1*/

#endif /*MUX_ANDROID*/

#define SYSCHECK(c) do{if((c)<0){LOGMUX(LOG_ERR,"%s %d system-error: '%s' (code: %d)", __func__, __LINE__, strerror(errno), errno);\
                        return -1;}\
                    }while(0)

/*MUX defines */
#define GSM0710_FRAME_FLAG 0xF9// basic mode flag for frame start and end
#define GSM0710_FRAME_ADV_FLAG 0x7E// advanced mode flag for frame start and end
#define GSM0710_FRAME_ADV_ESC 0x7D// advanced mode escape symbol
#define GSM0710_FRAME_ADV_ESC_COPML 0x20// advanced mode escape complement mask
#define GSM0710_FRAME_ADV_ESCAPED_SYMS { GSM0710_FRAME_ADV_FLAG, GSM0710_FRAME_ADV_ESC, 0x11, 0x91, 0x13, 0x93 }// advanced mode escaped symbols: Flag, Escape, XON and XOFF
// bits: Poll/final, Command/Response, Extension
#define GSM0710_PF 0x10//16
#define GSM0710_CR 0x02//2
#define GSM0710_EA 0x01//1
// type of frames
#define GSM0710_TYPE_SABM 0x2F//47 Set Asynchronous Balanced Mode
#define GSM0710_TYPE_UA 0x63//99 Unnumbered Acknowledgement
#define GSM0710_TYPE_DM 0x0F//15 Disconnected Mode
#define GSM0710_TYPE_DISC 0x43//67 Disconnect
#define GSM0710_TYPE_UIH 0xEF//239 Unnumbered information with header check
#define GSM0710_TYPE_UI 0x03//3 Unnumbered Acknowledgement
// control channel commands
#define GSM0710_CONTROL_PN (0x80|GSM0710_EA)//?? DLC parameter negotiation
#define GSM0710_CONTROL_CLD (0xC0|GSM0710_EA)//193 Multiplexer close down
#define GSM0710_CONTROL_PSC (0x40|GSM0710_EA)//??? Power Saving Control
#define GSM0710_CONTROL_TEST (0x20|GSM0710_EA)//33 Test Command
#define GSM0710_CONTROL_MSC (0xE0|GSM0710_EA)//225 Modem Status Command
#define GSM0710_CONTROL_NSC (0x10|GSM0710_EA)//17 Non Supported Command Response
#define GSM0710_CONTROL_RPN (0x90|GSM0710_EA)//?? Remote Port Negotiation Command
#define GSM0710_CONTROL_RLS (0x50|GSM0710_EA)//?? Remote Line Status Command
#define GSM0710_CONTROL_SNC (0xD0|GSM0710_EA)//?? Service Negotiation Command
// V.24 signals: flow control, ready to communicate, ring indicator,
// data valid 
#define GSM0710_SIGNAL_FC 0x02
#define GSM0710_SIGNAL_RTC 0x04
#define GSM0710_SIGNAL_RTR 0x08
#define GSM0710_SIGNAL_IC 0x40//64
#define GSM0710_SIGNAL_DV 0x80//128
#define GSM0710_SIGNAL_DTR 0x04
#define GSM0710_SIGNAL_DSR 0x04
#define GSM0710_SIGNAL_RTS 0x08
#define GSM0710_SIGNAL_CTS 0x08
#define GSM0710_SIGNAL_DCD 0x80//128
//
#define GSM0710_COMMAND_IS(type, command) ((type & ~GSM0710_CR) == command)
#define GSM0710_FRAME_IS(type, frame) ((frame->control & ~GSM0710_PF) == type)
#ifndef min
#define min(a,b) ((a < b) ? a :b)
#endif

#define cmux_N1 127 //lots of modem only support short frame, like quectel 2G modules
#define cmux_FRAME (cmux_N1+6) //lots of modem only support short frame, like quectel 2G modules
#define GSM0710_MAX_CHANNELS 4
#define QUECTEL_CACHE_FRAMES 20
#define GSM0710_BUFFER_SIZE (2*QUECTEL_CACHE_FRAMES*cmux_FRAME)

static int write_frame(
    int channel,
    const unsigned char *input,
    int length,
    unsigned char type);


/**************************/
/* TYPES                                */
/**************************/
typedef struct GSM0710_Frame
{
    unsigned char channel;
    unsigned char control;
    int length;
    unsigned char *data;
} GSM0710_Frame;

typedef struct GSM0710_Buffer
{
    unsigned char data[GSM0710_BUFFER_SIZE];
    unsigned char *readp;
    unsigned char *writep;
    unsigned char *endp;
    unsigned int datacount;
    unsigned int max_count;
    int flag_found;// set if last character read was flag
    unsigned long received_count;
    unsigned long dropped_count;
} GSM0710_Buffer;

typedef struct Channel // Channel data
{
    int id; // gsm 07 10 channel id
    char* devicename;
    int fd;
    int opened;
    int frame_allowed;
    unsigned char v24_signals;
    char* ptsname;
    char* origin;
    int remaining;
    unsigned char recv_buf[QUECTEL_CACHE_FRAMES*cmux_N1];
    unsigned char cache_buf[QUECTEL_CACHE_FRAMES*cmux_N1];
    int cache_frames;
    int cache_offset;
    int reopen;
    int disc_ua_pending;
} Channel;

typedef enum MuxerStates
{
    MUX_STATE_OPENING,
    MUX_STATE_INITILIZING,
    MUX_STATE_MUXING,
    MUX_STATE_CLOSING,
    MUX_STATE_OFF,
    MUX_STATES_COUNT // keep this the last
} MuxerStates;

typedef struct Serial
{
    const char *devicename;
    int fd;
    MuxerStates state;
    GSM0710_Buffer *in_buf;// input buffer
    unsigned char cache_buf[QUECTEL_CACHE_FRAMES*cmux_FRAME];
    int cache_frames;
    int cache_offset;
    time_t frame_receive_time;
    int ping_number;
} Serial;

/* Struct is used for passing fd, read function and read funtion arg to a device polling thread */
typedef struct Poll_Thread_Arg
{
int fd;
void * read_function_arg;
}Poll_Thread_Arg;


/**************************/
/* FUNCTION PROTOTYPES      */
/**************************/
/* increases buffer pointer by one and wraps around if necessary */
//void gsm0710_buffer_inc(GSM0710_Buffer *buf, void&* p);
#define gsm0710_buffer_inc(readp,datacount) do { readp++; datacount--; \
                                       if (readp == buf->endp) readp = buf->data; \
                                     } while (0)

/* Tells how many chars are saved into the buffer. */
//int gsm0710_buffer_length(GSM0710_Buffer *buf);
//#define gsm0710_buffer_length(buf) ((buf->readp > buf->writep) ? (GSM0710_BUFFER_SIZE - (buf->readp - buf->writep)) : (buf->writep-buf->readp))
#define gsm0710_buffer_length(buf) (buf->datacount)


/* tells how much free space there is in the buffer */
//int gsm0710_buffer_free(GSM0710_Buffer *buf);
//#define gsm0710_buffer_free(buf) ((buf->readp > buf->writep) ? ((buf->readp - buf->writep)-1) : (GSM0710_BUFFER_SIZE - (buf->writep-buf->readp))-1)
#define gsm0710_buffer_free(buf) (GSM0710_BUFFER_SIZE - buf->datacount)

static int watchdog(Serial * serial);
static int close_devices();
static int serial_device_read(Serial * serial);
static int serial_device_write(Serial *channel, unsigned char *data, int length);
static int pseudo_device_read(Channel* channel);
static int pseudo_device_write(Channel *channel, unsigned char *data, int length);
static void* pseudo_poll_thread(void* vargp);
static int create_thread(pthread_t * thread_id, void * thread_function, void * thread_function_arg );
static void set_main_exit_signal(int signal);
static int restart_pty_interface(Channel* channel);


/**************************/
/* CONSTANTS & GLOBALS     */
/**************************/
static unsigned char close_channel_cmd[] = { GSM0710_CONTROL_CLD | GSM0710_CR, GSM0710_EA | (0 << 1) };
static unsigned char test_channel_cmd[] = { GSM0710_CONTROL_TEST | GSM0710_CR, GSM0710_EA | (6 << 1), 'P', 'I', 'N', 'G', '\r', '\n', };
//static unsigned char psc_channel_cmd[] = { GSM0710_CONTROL_PSC | GSM0710_CR, GSM0710_EA | (0 << 1), };
//static unsigned char wakeup_sequence[] = { GSM0710_FRAME_FLAG, GSM0710_FRAME_FLAG, };
/* crc table from gsm0710 spec */
static const unsigned char r_crctable[] = {//reversed, 8-bit, poly=0x07
//{{{
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED,
    0x7C, 0x09, 0x98, 0xEA, 0x7B, 0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A,
    0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67, 0x38,
    0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44,
    0x31, 0xA0, 0xD2, 0x43, 0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0,
    0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F, 0x70, 0xE1,
    0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79,
    0xE8, 0x9A, 0x0B, 0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
    0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17, 0x48, 0xD9, 0xAB,
    0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0,
    0xA2, 0x33, 0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A,
    0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F, 0xE0, 0x71, 0x03, 0x92,
    0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A,
    0x9B, 0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63,
    0x11, 0x80, 0xF5, 0x64, 0x16, 0x87, 0xD8, 0x49, 0x3B, 0xAA, 0xDF,
    0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29,
    0xB8, 0xCD, 0x5C, 0x2E, 0xBF, 0x90, 0x01, 0x73, 0xE2, 0x97, 0x06,
    0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB, 0x8C,
    0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0,
    0x85, 0x14, 0x66, 0xF7, 0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C,
    0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3, 0xB4, 0x25,
    0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD,
    0x2C, 0x5E, 0xCF, 
//}}}
};

// config stuff
static int no_daemon = 0;
static int pin_code = -1;
static int use_ping = 0;
static int use_timeout = 0;
static int logtofile = 0;
static int syslog_level = LOG_INFO;
static int ql_cmux_debug = 1;
static FILE * muxlogfile;
static int vir_ports = 3; /* number of virtual ports to create */
/*misc global vars */
static int main_exit_signal=0;  /* 1:main() received exit signal */
static int uih_pf_bit_received = 0;
static unsigned int pts_reopen=0; /*If != 0,  signals watchdog that one cahnnel needs to be reopened */
static int hardware_flow_control = 0; /*1:open hardware flow control;0:close hardware flow control*/

/*pthread */
static pthread_t ser_read_thread;
static pthread_t pseudo_terminal[GSM0710_MAX_CHANNELS]; // -1 because control channel cannot be mapped to pseudo-terminal /dev/pts/*
static pthread_attr_t thread_attr;
static pthread_mutex_t syslogdump_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t write_frame_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t main_exit_signal_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t pts_reopen_lock = PTHREAD_MUTEX_INITIALIZER;

// serial io
static Serial serial;
// muxed io channels
static Channel channellist[GSM0710_MAX_CHANNELS+1]; // remember: [0] is not used acticly because it's the control channel
// some state
// +CMUX=<mode>[,<subset>[,<port_speed>[,<N1>[,<T1>[,<N2>[,<T2>[,<T3>[,<k>]]]]]]]]
static int cmux_mode = 0;
static int cmux_subset = 0;
static int cmux_port_speed = 5; //115200 baud rate
static int cmux_port_speed_default = 5; //115200 baud rate

/*
 * The following arrays must have equal length and the values must
 * correspond. also it has to correspond to the gsm0710 spec regarding
 * baud id of CMUX the command.
 */
static int baud_rates[] = {
    0, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 1500000, 2000000, 3000000, 4000000
};
static speed_t baud_bits[] = {
    0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600, B1500000, B2000000, B3000000, B4000000
};
static int quectel_speeds[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 20, 23, 26
};

#ifndef MUX_ANDROID
static const char * cmux_time(void) {
    static char time_buf[50];
    struct timeval  tv;
    time_t time;
    suseconds_t millitm;
    struct tm *ti;

    gettimeofday (&tv, NULL);

    time= tv.tv_sec;
    millitm = (tv.tv_usec + 500) / 1000;

    if (millitm == 1000) {
        ++time;
        millitm = 0;
    }

    ti = localtime(&time);
    sprintf(time_buf, "[%02d-%02d_%02d:%02d:%02d:%03d]", ti->tm_mon+1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, (int)millitm);
    return time_buf;
}
#endif

#if 0 //the easy way is directly drop data, hanlde FlowControl naybe make modem goto wrong state
/**************************/
/* MAIN CODE                        */
/**************************/
static int MSC_flag = 0;

#define CTRL_UIH_MSC            0xE0        // modem status command
#define SET_CR_MASK             0x02
#define SET_EA_MASK             0x01
#define CTRL_MSC_MSG_LENGTH     2
#define CTRL_MSC_FC_BIT         0x02

//send MSC
int send_modem_status(int bDlci, int fcOnOrOff)
{
//{{{
    unsigned char bUIHData[4];  
    unsigned char v24sig = 0;
    bUIHData[0] = CTRL_UIH_MSC | SET_CR_MASK | SET_EA_MASK;//0xE3
    bUIHData[1] = (CTRL_MSC_MSG_LENGTH << 1) | SET_EA_MASK;//0x05
    bUIHData[2] = (bDlci << 2) | SET_CR_MASK | SET_EA_MASK;
    if(fcOnOrOff)
        bUIHData[3] = v24sig | CTRL_MSC_FC_BIT | SET_EA_MASK;//0x03
    else
        bUIHData[3] = v24sig | SET_EA_MASK;//0x01


    LOGMUX(LOG_INFO,"MSC_flag=%x, bDlci=%d, fcOnOrOff=%d", MSC_flag, bDlci, fcOnOrOff);

    if((MSC_flag & (1 << (bDlci - 1))) && fcOnOrOff == 1)
    {
        return 0;
    }
    else if((MSC_flag & (1 << (bDlci - 1))) && fcOnOrOff == 0)
    {
        write_frame(0, bUIHData, sizeof(bUIHData),GSM0710_TYPE_UIH);
        MSC_flag &= ~(1 << (bDlci - 1));
        return 0;
    }
//                                if(fcOnOrOff && ptr_channel_status[bDlci-1]==CHANNEL_STATE_SUSPEND)
//                                            return 0;
    write_frame(0, bUIHData, sizeof(bUIHData),GSM0710_TYPE_UIH);
    MSC_flag |= 1 << (bDlci - 1);
//}}}
    return 0;
}

//{{{ list buffer

typedef struct _DataLeft
{
    char *data;
    char *data_head;
    unsigned int length;
//    struct _DateLeft *pre;
    struct _DataLeft *next;
}DataLeft;

static DataLeft data_left[GSM0710_MAX_CHANNELS];

int init_list()
{
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    int i = 0;
    for(;i <GSM0710_MAX_CHANNELS;i++)
    {
        data_left[i].data = NULL;
        data_left[i].data_head = NULL;
        data_left[i].length = 0;
//        data_left[i].pre = NULL;
        data_left[i].next = NULL;
    }
    LOGMUX(LOG_DEBUG,"%s[%d]:Leave",__FUNCTION__,__LINE__);
    return 0;
}

int list_add(unsigned char *data,unsigned int length,DataLeft *data_left_list)
{
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    DataLeft *p,*tmp;

    tmp = data_left_list;
    p = (DataLeft *)malloc(sizeof(DataLeft));
    p->data = (char *)malloc(length);
    p->data_head = p->data;
    p->next = NULL;
    p->length = 0;

    LOGMUX(LOG_DEBUG,"%s[%d]",__FUNCTION__,__LINE__);
    if(p == NULL || p->data == NULL)
    {
        return -1;
    }
    LOGMUX(LOG_DEBUG,"%s[%d]",__FUNCTION__,__LINE__);

    while(tmp->next != NULL)
    {
        LOGMUX(LOG_DEBUG,"%s[%d]",__FUNCTION__,__LINE__);
        tmp = tmp->next;
    }
    LOGMUX(LOG_DEBUG,"%s[%d]",__FUNCTION__,__LINE__);

    memcpy(p->data,data,length);
    p->length = length;
    
    LOGMUX(LOG_DEBUG,"%s[%d]",__FUNCTION__,__LINE__);
    tmp->next = p;
    
    LOGMUX(LOG_DEBUG,"%s[%d]:Leave",__FUNCTION__,__LINE__);
    return 0;
}

int fill_in_list(int channel,unsigned char *data,int length)
{
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    int ret;

    ret = list_add(data,length,&data_left[channel - 1]);

    LOGMUX(LOG_DEBUG,"%s[%d]:Leave",__FUNCTION__,__LINE__);
    return ret;
}

int free_list_node(DataLeft *node)
{
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    if(node->data != NULL)
    {
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
        free(node->data_head);
        node->data = NULL;
        node->data_head = NULL;
    }
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    node->length = 0;
    node->next = NULL;
    free(node);
    LOGMUX(LOG_DEBUG,"%s[%d]:Leave",__FUNCTION__,__LINE__);
    return 0;
}

/* write list buffer to PTS*/
int send_left_data(int port_num)
{
    LOGMUX(LOG_DEBUG,"%s[%d]:Enter",__FUNCTION__,__LINE__);
    int i = 0;
    int n;
    for(;i < port_num;i++)
    {
        if(data_left[i].next != NULL)
        {
            DataLeft *p = &data_left[i];
            p = p->next;
            do
            {
                if (channellist[i + 1].fd > 0) //reopening, discard the data
                n = write(channellist[i + 1].fd,p->data,p->length);
                else {
                    LOGMUX(LOG_INFO,"channel %d closed, discard the data",i + 1);
                    n = p->length;
                }
                LOGMUX(LOG_DEBUG,"write %d/%d to pts",n,p->length);
                if(n <= 0)
                {
                    break;
                }
                else if(n < (int)p->length)
                {
                    p->data += n;
                    p->length -=n;
                    break;
                }
                else
                {
#if 1
                    DataLeft *tmp;
                    tmp = p;
                    p = p->next;
                    free_list_node(tmp);
#else               
                    p = p->next;
                    //free_list_node(data_left[i].next);
#endif
                    data_left[i].next = p;
                }
            }while(p != NULL);
        }
    }
    for(i = 0;i < port_num;i++)
    {
        if(data_left[i].next == NULL && (MSC_flag & (1 << i)) != 0)
        {
            send_modem_status(i + 1,0);
            //MSC_flag &= ~(1 << i);
            LOGMUX(LOG_DEBUG,"%s[%d]:clear MSC_flag bit",__FUNCTION__,__LINE__);
        }
    }
    LOGMUX(LOG_DEBUG,"%s[%d]:Leave",__FUNCTION__,__LINE__);

    return 0;
}
//}}}
#endif

/*
* Purpose:  Determine baud-rate index for CMUX command
* Input:        baud_rate - baud rate (eg. 460800)
* Return:    -1 if fail, i - baud rate index if success
*/
static int baud_rate_index(int baud_rate)
{
    unsigned int i = 0;
    for (i = 0; i < sizeof(baud_rates) / sizeof(*baud_rates); ++i)
        if (baud_rates[i] == baud_rate)
            return i;

    return -1;
}

/*
* Purpose:  Calculates frame check sequence from given characters.
* Input:        input - character array
*                length - number of characters in array (that are included)
* Return:    frame check sequence
*/
static unsigned char frame_calc_crc(
    const unsigned char *input,
    int length)
{
//{{{
    unsigned char fcs = 0xFF;
    int i;
    for (i = 0; i < length; i++)
        fcs = r_crctable[fcs ^ input[i]];
    return 0xFF - fcs;
//}}}
}

/*
* Purpose:  ascii/hexdump a byte buffer 
* Input:        prefix - string to printed before hex data on every line
*                ptr - the string to be dumped
*                length - the length of the string to be dumped
* Return:    0
*/
static int syslogdump(
    const char *prefix,
    const unsigned char *ptr,
    unsigned int length)
{
//{{{
    char buffer[100];
    unsigned int offset = 0l;
    int i;

    if ((LOG_DEBUG>syslog_level) && (ql_cmux_debug == 0)) /*No need for all frame logging if it's not to be seen */
        return 0;

    pthread_mutex_lock(&syslogdump_lock);     //new lock
    while (offset < length)
    {
        int off;
        strcpy(buffer, prefix);
        off = strlen(buffer);
        SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%08x: ", offset));
        off = strlen(buffer);
        for (i = 0; i < 16; i++)
        {
            if (offset + i < length){
                SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%02x%c", ptr[offset + i], i == 7 ? '-' : ' '));
                }
            else{
                SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " .%c", i == 7 ? '-' : ' '));
                }
            off = strlen(buffer);
        }
        SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " "));
        off = strlen(buffer);
        for (i = 0; i < 16; i++)
            if (offset + i < length)
            {
                SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%c", (ptr[offset + i] < ' ') ? '.' : ptr[offset + i]));
                off = strlen(buffer);
            }
        offset += 16;
        LOGMUX(LOG_DEBUG,"%s", buffer);
    }
    pthread_mutex_unlock(&syslogdump_lock);/*new lock*/

    return 0;
//}}}
}

/*
* Purpose:  Writes a frame to a logical channel. C/R bit is set to 1. 
*                Doesn't support FCS counting for GSM0710_TYPE_UI frames.
* Input:        channel - channel number (0 = control)
*                input - the data to be written
*                length - the length of the data
*                type - the type of the frame (with possible P/F-bit)
* Return:    number of characters written
*/
static int write_frame(
    int channel,
    const unsigned char *input,
    int length,
    unsigned char type)
{
//{{{
    /* new lock */
    pthread_mutex_lock(&write_frame_lock);
    LOGMUX(LOG_DEBUG, "write_frame Enter");
/* flag, GSM0710_EA=1 C channel, frame type, length 1-2 */
    unsigned char prefix[5] = { GSM0710_FRAME_FLAG, GSM0710_EA | GSM0710_CR, 0, 0, 0 };
    unsigned char postfix[2] = { 0xFF, GSM0710_FRAME_FLAG };
    ssize_t prefix_length = 4;
    int c;
    unsigned char tmp[cmux_FRAME];
    
    LOGMUX(LOG_DEBUG, "Sending frame to channel %d", channel);
 
/* GSM0710_EA=1, Command, let's add address */
    prefix[1] = prefix[1] | ((63 & (unsigned char) channel) << 2);
/* let's set control field */
    prefix[2] = type;
    if ((type ==  GSM0710_TYPE_UIH || type ==  GSM0710_TYPE_UI) &&
        uih_pf_bit_received == 1 &&
        GSM0710_COMMAND_IS(input[0],GSM0710_CONTROL_MSC) ){
      prefix[2] = prefix[2] | GSM0710_PF; //Set the P/F bit in Response if Command from modem had it set
      uih_pf_bit_received = 0; //Reset the variable, so it is ready for next command
        LOGMUX(LOG_INFO,"WRITE MSC");
        //sleep(1);
    }
/* let's not use too big frames */
    length = min(cmux_N1, length);
    if (!cmux_mode)//basic
    {
/* Modified acording PATCH CRC checksum */
/* postfix[0] = frame_calc_crc (prefix + 1, prefix_length - 1); */
/* length */
        if (length > 127)
        {
            prefix_length = 5;
            prefix[3] = (0x007F & length) << 1;
            prefix[4] = (0x7F80 & length) >> 7;
        }
        else
            prefix[3] = 1 | (length << 1);
        postfix[0] = frame_calc_crc(prefix + 1, prefix_length - 1);

        memcpy(tmp, prefix, prefix_length);

        if (length > 0)
        {
            memcpy(tmp + prefix_length, input, length);
        }

        memcpy(tmp + prefix_length + length, postfix, 2);
        c = prefix_length + length + 2;
        syslogdump(">s ", tmp, c);

        serial_device_write(&serial, tmp, c);
    }
    LOGMUX(LOG_DEBUG, "Leave");
    
    pthread_mutex_unlock(&write_frame_lock);
    return length;
}

/*
* Purpose:  Handles received data from pseudo terminal device (application)
* Input:        buf - buffer, which contains received data
*                len - the length of the buffer channel
*                channel - logical channel id where data was received
* Return:    The number of remaining bytes in partial packet
*/
static int handle_channel_data(
    unsigned char *buf,
    int len,
    int channel)
{
    int written = 0;
    int last = 0;
    
    while (written < len)
    {
        last = write_frame(channel, buf + written, len - written, GSM0710_TYPE_UIH);
        if (last > 0)
            written += last;
        else
            break;
    }
    
    if (written < len)
        LOGMUX(LOG_WARNING, "Couldn't write data to channel %d. Wrote only %d bytes, when should have written %d",
                channel, written, len);

    return len - written;
}

/* 
* Purpose:  Close mux logical channel
* Input:      channel - logical channel struct
* Return:    0
*/
static int logical_channel_close(Channel* channel)
{
//{{{
    if (channel->fd >= 0)
        close(channel->fd);
    channel->fd = -1;
    if (channel->ptsname != NULL)
        free(channel->ptsname);
    channel->ptsname = NULL;
    if (channel->origin != NULL)
        free(channel->origin);
    channel->origin = NULL;
    channel->opened = 0;
    channel->v24_signals = 0;
    channel->remaining = 0;
    return 0;
//}}}
}

/* 
* Purpose:  Initialize mux logical channel
* Input:      channel - logical channel struct
*                id - logical channel id number
* Return:    0
*/
static int logical_channel_init(Channel* channel, int id)
{
//{{{
    channel->id = id; // connected channel-id
    channel->devicename = id?"/dev/ptmx":NULL; // TODO do we need this to be dynamic anymore?
    channel->fd = -1;
    channel->ptsname = NULL;
    channel->origin = NULL;
    channel->reopen = 0;
    channel->disc_ua_pending = 0;
    channel->cache_frames = channel->cache_offset = 0;
    return logical_channel_close(channel);
//}}}
}

/* 
* Purpose:  Restart pseudo terminal device and reading thread, connect it to existing mux channel
* Input:      pointer to Channel struct
* Return:    1 if fail, 0 if success
*/
int restart_pty_interface(Channel* channel)
{
//{{{
    if (channel->fd < 0) // is this channel free?
                {
                    SYSCHECK(channel->fd = open(channel->devicename, O_RDWR | O_NONBLOCK)); //open pseudo terminal devices from /dev/ptmx master
        char pts_r[64] = {0};
        char* pts = NULL;
        if (ptsname_r(channel->fd, pts_r, sizeof(pts_r)) == 0)
            pts = pts_r;                                        
                    if (pts == NULL) SYSCHECK(-1);
                    
                    //joe for new
                    LOGMUX(LOG_DEBUG,"PTS: %s",pts);
                    char link_name[64];
                    char command[64+6];
                    sprintf(link_name, MUX_CHN_DIR "%d",channel->id);
                    snprintf(command,sizeof(command),"rm %s -f",link_name);
                    ql_system(command);

                    if(symlink(pts,link_name) != 0)
                    {
                        LOGMUX(LOG_ERR,"Create link %s Error : %d (%s)",link_name,errno, strerror(errno));
                        return 1;
                    }

                    channel->ptsname = strdup(pts);
                    struct termios options;
                    memset(&options, 0, sizeof(options));
                    tcgetattr(channel->fd, &options); //get the parameters
                    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
                    options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
                    options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
                    tcsetattr(channel->fd, TCSANOW, &options);
                    if (!strcmp(channel->devicename, "/dev/ptmx"))
                    {
                        //Otherwise programs cannot access the pseudo terminals
                        SYSCHECK(grantpt(channel->fd));
                        SYSCHECK(unlockpt(channel->fd));
                    }
                    channel->v24_signals = GSM0710_SIGNAL_DV | GSM0710_SIGNAL_RTR | GSM0710_SIGNAL_RTC | GSM0710_EA;
                    channel->frame_allowed = 1;
                    //create thread
                    LOGMUX(LOG_INFO, "Reopened %s, channel number: %d fd: %d ",channel->ptsname, channel->id, channel->fd);
                    Poll_Thread_Arg* poll_thread_arg = (Poll_Thread_Arg*) malloc(sizeof(Poll_Thread_Arg)); //iniitialize pointer to thread args
                    if(NULL == poll_thread_arg) {
                        LOGMUX(LOG_ERR, "%s:%d malloc failed", __func__, __LINE__);
                        return 1;
                    }
                    poll_thread_arg->fd = channel->fd;
                    poll_thread_arg->read_function_arg = (void *) (channel);
                    if(create_thread(&pseudo_terminal[channel->id], pseudo_poll_thread,(void*) poll_thread_arg)!=0){ //create thread for reading input from virtual port
                      LOGMUX(LOG_ERR,"Could not restart thread for listening on %s", channel->ptsname);
                      return 1;
                    }
                    LOGMUX(LOG_INFO, "Restarted thread listening on %s", channel->ptsname);                
        }
    return 0;
//}}}
}

static int pseudo_device_read(Channel* channel) {
    int len;
    
    LOGMUX(LOG_DEBUG, "Enter");

    len = read(channel->fd, channel->recv_buf + channel->remaining, sizeof(channel->recv_buf) - channel->remaining);

    if (len > 0) {
        LOGMUX(LOG_DEBUG, "Data from channel %d, %d bytes", channel->id, len);
        
        if (len + channel->remaining > 0) {
            int new_remaining = handle_channel_data(channel->recv_buf, len + channel->remaining, channel->id);
        
            if (new_remaining > 0)
                memmove(channel->recv_buf, channel->recv_buf + (len + channel->remaining) - new_remaining, new_remaining);
            channel->remaining = new_remaining;
        }
    } 
    else
    {
        LOGMUX(LOG_ERR,"pseudo_device_read error,  errno: %d (%s)", errno, strerror(errno));
    }
    
    LOGMUX(LOG_DEBUG, "Leave");
    return len;
}

static int pseudo_device_write(Channel* channel, unsigned char *data, int length) {
    if (length > 0) {
        memcpy(channel->cache_buf + channel->cache_offset, data, length);
        channel->cache_frames++;
        channel->cache_offset += length;
    }

    if (channel->cache_offset == 0)
        return 0;
    
    if (length < cmux_N1 || channel->cache_frames == QUECTEL_CACHE_FRAMES) { //too larger cache szie can make uart speed become low?
        int cur = 0;

#if 1
        cur = write (channel->fd, channel->cache_buf+cur, channel->cache_offset-cur);
#else        
        while (cur < channel->cache_offset) {
            struct pollfd pollfds[] = {{channel->fd, POLLOUT, 0}};
            int ret, written;

            written = write (channel->fd, channel->cache_buf+cur, channel->cache_offset-cur);

            if (written <= 0 && errno != EAGAIN) {
                LOGMUX(LOG_WARNING, "write = %d/%d, errno: %d (%s)",  written, channel->cache_offset-cur, errno, strerror(errno));
                break;
            } else if (written > 0) {
                cur += written;
            }

            if (cur >= channel->cache_offset)
                break;

            do {
                ret = poll(pollfds, 1, 1000);
            } while ((ret < 0 ) && (errno == EINTR));

            if ((ret < 0) || (pollfds[0].revents & (POLLERR|POLLHUP))) {
                LOGMUX(LOG_ERR, "Device polling ret = %d, revents: %x, errno: %d (%s)", ret, pollfds[0].revents, errno, strerror(errno));
                break;
            }
        }
#endif

        if (cur < channel->cache_offset) {
            LOGMUX(LOG_WARNING, "Couldn't write the whole data to the virtual port %d. Wrote only %d/%d bytes, errno: %d (%s)",
                                channel->id, cur, channel->cache_offset, errno, strerror(errno));
        }

        channel->cache_frames = channel->cache_offset = 0;
    }

    return length;
}

/* 
* Purpose:  Allocate a channel and corresponding virtual port and a start a reading thread on that port
* Input:      origin - string to define origin of allocation
* Return:    1 if fail, 0 if success
*/
static int c_alloc_channel(const char* origin, pthread_t * thread_id)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    int i;
    if (serial.state == MUX_STATE_MUXING)
        for (i=1;i<GSM0710_MAX_CHANNELS;i++)
            if (channellist[i].fd < 0) // is this channel free?
            {
                LOGMUX(LOG_DEBUG, "Found free channel %d fd %d on %s", i, channellist[i].fd, channellist[i].devicename);
                channellist[i].origin = strdup(origin);
                SYSCHECK(channellist[i].fd = open(channellist[i].devicename, O_RDWR | O_NONBLOCK)); //open pseudo terminal devices from /dev/ptmx master
                char pts_r[64] = {0};
                char* pts = NULL;
                if (ptsname_r(channellist[i].fd, pts_r, sizeof(pts_r)) == 0)
                    pts = pts_r;                     
                if (pts == NULL) SYSCHECK(-1);
                //joe for new
                LOGMUX(LOG_DEBUG,"PTS: %s",pts);
                if (access(MUX_CHN_DIR, O_RDWR) && mkdir(MUX_CHN_DIR,S_IRWXU|S_IRWXG|S_IRWXO) != 0)
                {
                    LOGMUX(LOG_ERR,"Create " MUX_CHN_DIR " Dir Error : %d (%s)",errno, strerror(errno));
                    return 1;
                }
                else
                {
                    LOGMUX(LOG_DEBUG,"Create" MUX_CHN_DIR " Dir Success");
                }
                char link_name[64];
                sprintf(link_name,MUX_CHN_DIR "%d",i);
                char command[64+6];
                snprintf(command,sizeof(command),"rm %s -f",link_name);
                ql_system(command);
                //if(symlink(pts,link_name) != 0)
                if(symlink(pts,link_name) != 0)
                {
                    LOGMUX(LOG_ERR, "Create link %s Error : %d (%s)", link_name, errno, strerror(errno));
                    return 1;
                }
                else
                {
                    LOGMUX(LOG_DEBUG,"=====================================");
                    LOGMUX(LOG_DEBUG,"Create link %s Success",link_name);
                }
                channellist[i].ptsname = strdup(pts);
                struct termios options;
                memset(&options, 0, sizeof(options));
                tcgetattr(channellist[i].fd, &options); //get the parameters
                options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
                options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
                options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
                tcsetattr(channellist[i].fd, TCSANOW, &options);
                if (!strcmp(channellist[i].devicename, "/dev/ptmx"))
                {
                    //Otherwise programs cannot access the pseudo terminals
                    SYSCHECK(grantpt(channellist[i].fd));
                    SYSCHECK(unlockpt(channellist[i].fd));
                }
                channellist[i].v24_signals = GSM0710_SIGNAL_DV | GSM0710_SIGNAL_RTR | GSM0710_SIGNAL_RTC | GSM0710_EA;
                channellist[i].frame_allowed = 1;
                //create thread
                LOGMUX(LOG_DEBUG, "New channel properties: number: %d fd: %d device: %s", i, channellist[i].fd, channellist[i].devicename);
                Poll_Thread_Arg* poll_thread_arg = (Poll_Thread_Arg*) malloc(sizeof(Poll_Thread_Arg)); //iniitialize pointer to thread args
                if(NULL == poll_thread_arg) {
                    LOGMUX(LOG_ERR, "%s:%d malloc failed", __func__, __LINE__);
                    return 1;
                }
                poll_thread_arg->fd = channellist[i].fd;
                poll_thread_arg->read_function_arg = (void *) (channellist+i);
                  if(create_thread(thread_id, pseudo_poll_thread,(void*) poll_thread_arg)!=0){ //create thread for reading input from virtual port
                    LOGMUX(LOG_ERR,"Could not create thread for listening on %s", channellist[i].ptsname);
                    return 1;
                  }
                  LOGMUX(LOG_DEBUG, "Thread is running and listening on %s", channellist[i].ptsname);

                write_frame(i, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF); //should be moved?? messy
                LOGMUX(LOG_INFO, "Connecting %s to virtual channel %d for %s on %s",
                    channellist[i].ptsname, channellist[i].id, channellist[i].origin, serial.devicename);
                return 0;
            }
    LOGMUX(LOG_ERR, "Not muxing or no free channel found");
    return 1;
//}}}
}

/* 
* Purpose:  Allocates memory for a new buffer and initializes it.
* Input:      -
* Return:    pointer to a new buffer
*/
static GSM0710_Buffer *gsm0710_buffer_init()
{
    GSM0710_Buffer *buf = (GSM0710_Buffer*)malloc(sizeof(GSM0710_Buffer));

    if (NULL == buf) {
        LOGMUX(LOG_ERR, "%s:%d malloc failed", __func__, __LINE__);
        return NULL;
    }

    memset(buf, 0, sizeof(GSM0710_Buffer));
    buf->readp = buf->data;
    buf->writep = buf->data;
    buf->endp = buf->data + GSM0710_BUFFER_SIZE;

    serial.cache_frames = serial.cache_offset = 0;
    return buf;
}

/* 
* Purpose:  Destroys the buffer (i.e. frees up the memory
* Input:      buf - buffer to be destroyed
* Return:    -
*/
static void gsm0710_buffer_destroy(
    GSM0710_Buffer* buf)
{
//{{{
    free(buf);
//}}}
}

/* 
* Purpose:  Gets a complete basic mode frame from buffer. You have to remember to free this frame
*                when it's not needed anymore
* Input:      buf - the buffer, where the frame is extracted
* Return:    frame or null, if there isn't ready frame with given index
*/
static GSM0710_Frame* gsm0710_base_buffer_get_frame(
    GSM0710_Buffer * buf, GSM0710_Frame *frame)
{
//{{{
    int end;
    unsigned int length_needed = 5;// channel, type, length, fcs, flag
    unsigned char fcs = 0xFF;
    unsigned char *local_readp;
    unsigned int local_datacount, local_datacount_backup;
    LOGMUX(LOG_DEBUG, "Enter");

/*Find start flag*/
    while (!buf->flag_found && gsm0710_buffer_length(buf) > 0)
    {
        if (*buf->readp == GSM0710_FRAME_FLAG)
            buf->flag_found = 1;
        gsm0710_buffer_inc(buf->readp,buf->datacount); 
    }
    
    if (!buf->flag_found)// no frame started
    {
        LOGMUX(LOG_DEBUG, "Leave. No start frame 0xf9 found in bytes stored in GSM0710 buffer");
        return NULL;
    }
    
/*skip empty frames (this causes troubles if we're using DLC 62) - skipping frame start flags*/
    while (gsm0710_buffer_length(buf) > 0 && (*buf->readp == GSM0710_FRAME_FLAG))
    {
        gsm0710_buffer_inc(buf->readp,buf->datacount);
    }
/* Okay, we're ready to analyze a proper frame header */

    /*Make local copy of buffer pointer and data counter. They are shared between 2 threads, so we want to update them only after a frame extraction */
    /*From now on, only operate on these local copies */
    local_readp = buf->readp;
    local_datacount = local_datacount_backup = buf->datacount; /* current no. of stored bytes in buffer */

    if (local_datacount >= length_needed) /* enough data stored for 0710 frame header+footer? */
    {
        if (frame != NULL)
        {
            frame->channel = ((*local_readp & 252) >> 2); /*frame header address-byte read*/
            if (frame->channel > vir_ports ) /* Field Sanity check if channel ID actually exists */
            {
                LOGMUX(LOG_WARNING, "Dropping frame: Corrupt! Channel Addr. field indicated %d, which does not exist",frame->channel);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }        
            fcs = r_crctable[fcs ^ *local_readp];
            gsm0710_buffer_inc(local_readp,local_datacount);
            length_needed--;
            frame->control = *local_readp; /*frame header type-byte read*/
            fcs = r_crctable[fcs ^ *local_readp];
            gsm0710_buffer_inc(local_readp,local_datacount);
            length_needed--;
            frame->length = (*local_readp & 254) >> 1; /*Frame header 1st length-byte read*/
            fcs = r_crctable[fcs ^ *local_readp];
        }
        
        if ((*local_readp & 1) == 0)/*if frame payload length byte extension bit not set, a 2nd length byte is in header*/
        {
            //Current spec (version 7.1.0) states these kind of
            //frames to be invalid Long lost of sync might be
            //caused if we would expect a long frame because of an
            //error in length field.
            gsm0710_buffer_inc(local_readp,local_datacount);
            frame->length += (*local_readp*128); /*Frame header 2nd length-byte read*/
            fcs = r_crctable[fcs^*local_readp];
        }
        
        length_needed += frame->length; /*length_needed : 1 length byte + payload + 1 fcs byte + 1 end frame flag */
        LOGMUX(LOG_DEBUG, "length_needed: %d, available in local_datacount: %d",length_needed,local_datacount);

        if (frame->length > cmux_N1) /* Field Sanity check if payload is bigger than the max size negotiated in +CMUX */
        {
            LOGMUX(LOG_WARNING, "Dropping frame: Corrupt! Length field indicated %d. Max %d allowed",frame->length, cmux_N1);
            buf->flag_found = 0;
            buf->dropped_count++;
            goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
        }
        if (!(local_datacount >= length_needed))
        {
            LOGMUX(LOG_DEBUG, "Leave, frame extraction cancelled. Frame not completely stored in re-assembly buffer yet");
            return NULL;
        }        
        gsm0710_buffer_inc(local_readp,local_datacount);

/*Okay, done with the frame header. Start extracting the payload data */
        if (frame->length > 0)
        {
            {
                end = buf->endp - local_readp;
                if (frame->length > end) /*wrap-around necessary*/
                {
                    frame->data = local_readp;
                    local_readp = buf->data + (frame->length - end);
                    local_datacount -= frame->length;
                }
                else
                {
                    frame->data = local_readp;
                    local_readp += frame->length;
                    local_datacount -= frame->length;
                    if (local_readp == buf->endp)
                        local_readp = buf->data;
                }
                if (GSM0710_FRAME_IS(GSM0710_TYPE_UI, frame))
                {
                    for (end = 0; end < frame->length; end++)
                        fcs = r_crctable[fcs ^ (frame->data[end])];
                }
            }
        }
/*Okay, check FCS*/
        if (r_crctable[fcs ^ (*local_readp)] != 0xCF)
        {
            gsm0710_buffer_inc(local_readp,local_datacount);
            if (*local_readp != GSM0710_FRAME_FLAG) /* the FCS didn't match, but the next byte may not even be an end-frame-flag*/
            {
                LOGMUX(LOG_WARNING, "Dropping frame: Corrupt! End flag not present and FCS mismatch.");
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
            else 
            {
            LOGMUX(LOG_WARNING, "Dropping frame: FCS doesn't match");
            buf->flag_found = 0;
            buf->dropped_count++;
            goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
            }
        }
        else
        {
/*Okay, check end flag */
            gsm0710_buffer_inc(local_readp,local_datacount);
            if (*local_readp != GSM0710_FRAME_FLAG)
            {
                LOGMUX(LOG_WARNING, "Dropping frame: End flag not present. Instead: %d", *local_readp);
                buf->flag_found = 0;
                buf->dropped_count++;
                goto update_buffer_dropping_frame;
            }
            else
                buf->received_count++;
            gsm0710_buffer_inc(local_readp,local_datacount); /* prepare readp for next frame extraction */
        }
    }
    else 
    {
        LOGMUX(LOG_DEBUG, "Leave, not enough bytes stored in buffer for header information yet");
        return NULL;
    }

/* Everything went fine, update GSM0710 buffer pointer and counter */
    buf->readp = local_readp;
    buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
    buf->flag_found = 0; /* prepare for any future frame processing*/
    LOGMUX(LOG_DEBUG, "Leave, frame found");
    return frame;

update_buffer_dropping_frame:
    /*Update GSM0710 buffer pointer and counter */
    buf->readp = local_readp;
    buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
    return gsm0710_base_buffer_get_frame(buf, frame);    /*continue extracting more frames if any*/
//}}}
}

/* 
* Purpose:  Compares two strings.
*                strstr might not work because WebBox sends garbage before the first OK
*                when it's not needed anymore
* Input:      haystack - string to check
*                length - length of string to check
*                needle - reference string to compare to. must be null-terminated.
* Return:    1 if comparison was success, else 0
*/
static int memstr(
    const char *haystack,
    int length,
    const char *needle)
{
//{{{
    int i;
    int j = 0;
    if (needle[0] == '\0')
        return 1;
    for (i = 0; i < length; i++)
        if (needle[j] == haystack[i])
        {
            j++;
            if (needle[j] == '\0') // Entire needle was found
                return 1;
        }
        else
            j = 0;
    return 0;
//}}}
}

/* 
* Purpose:  Sends an AT-command to a given serial port and waits for reply.
* Input:      fd - file descriptor
*                cmd - command
*                to - how many seconds to wait for response
* Return:   0 on success (OK-response), -1 otherwise
*/
static int chat(
    int serial_device_fd,
    char *cmd,
    int to)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    unsigned char buf[256];
    int sel;
    int len;
    int wrote = 0;
    syslogdump(">s ", (unsigned char *) cmd, strlen(cmd));
    SYSCHECK(wrote = write(serial_device_fd, cmd, strlen(cmd)));
    LOGMUX(LOG_DEBUG, "Wrote %d bytes", wrote);

    LOGMUX(LOG_DEBUG, "CHAT++++++++++++");

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(serial_device_fd, &rfds);
    struct timeval timeout;
    timeout.tv_sec = to;
    timeout.tv_usec = 0;
    do
    {
        SYSCHECK(sel = select(serial_device_fd + 1, &rfds, NULL, NULL, &timeout));
        LOGMUX(LOG_DEBUG, "Selected %d", sel);
        if (FD_ISSET(serial_device_fd, &rfds))
        {
            memset(buf, 0, sizeof(buf));
            usleep(100*1000); //wait all at response, on some SOC, "ok" is split
            len = read(serial_device_fd, buf, sizeof(buf));
            SYSCHECK(len);
            LOGMUX(LOG_DEBUG, "Read %d bytes from serial device", len);
            syslogdump("<s ", buf, len);
            errno = 0;
            if (memstr((char *) buf, len, "OK"))
            {
                LOGMUX(LOG_DEBUG, "Received OK");
                return 0;
            }
            if (memstr((char *) buf, len, "ERROR"))
            {
                LOGMUX(LOG_DEBUG, "Received ERROR");
                return -1;
            }
        }
    } while (sel);
    return -1;
//}}}
}

/* 
* Purpose:  Handles commands received from the control channel.
* Input:      frame - the mux frame struct
* Return:   0
*/
static int handle_command(
    GSM0710_Frame * frame)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    unsigned char type, signals;
    int length = 0, i, type_length, channel, supported = 1;
    unsigned char *response = NULL;
//struct ussp_operation op;
    if (frame->length > 0)
    {
        type = frame->data[0];// only a byte long types are handled now skip extra bytes
        for (i = 0; (frame->length > i && (frame->data[i] & GSM0710_EA) == 0); i++);
        i++;
        type_length = i;
        if ((type & GSM0710_CR) == GSM0710_CR)
        {
//command not ack extract frame length
            while (frame->length > i)
            {
                length = (length * 128) + ((frame->data[i] & 254) >> 1);
                if ((frame->data[i] & 1) == 1)
                    break;
                i++;
            }
            i++;
            switch ((type & ~GSM0710_CR))
            {
            case GSM0710_CONTROL_CLD:
                LOGMUX(LOG_INFO, "The mobile station requested mux-mode termination");
                serial.state = MUX_STATE_CLOSING;
                break;
            case GSM0710_CONTROL_PSC:
                LOGMUX(LOG_INFO, "Power Service Control command: ***");
                LOGMUX(LOG_INFO, "Frame->data = %s / frame->length = %d", frame->data + i, frame->length - i);
            break;
            case GSM0710_CONTROL_TEST:
                LOGMUX(LOG_INFO, "Test command: ");
                LOGMUX(LOG_INFO, "Frame->data = %s / frame->length = %d", frame->data + i, frame->length - i);
                //serial->ping_number = 0;
                break;
            case GSM0710_CONTROL_MSC:
                LOGMUX(LOG_INFO,"GET MSC");
                if (i + 1 < frame->length)
                {
                    channel = ((frame->data[i] & 252) >> 2);
                    i++;
                    signals = (frame->data[i]);
//op.op = USSP_MSC;
//op.arg = USSP_RTS;
//op.len = 0;
                    LOGMUX(LOG_INFO, "Modem status command on channel %d", channel);
                    channellist[channel].frame_allowed = ((signals & GSM0710_SIGNAL_FC) != GSM0710_SIGNAL_FC);
                    if ((signals & GSM0710_SIGNAL_FC) == GSM0710_SIGNAL_FC)
                        LOGMUX(LOG_INFO, "No frames allowed");
                    else
                    {
//op.arg |= USSP_CTS;
                        LOGMUX(LOG_INFO, "Frames allowed");
                    }
                    if ((signals & GSM0710_SIGNAL_RTC) == GSM0710_SIGNAL_RTC)
                    {
//op.arg |= USSP_DSR;
                        LOGMUX(LOG_INFO, "Signal RTC");
                    }
                    if ((signals & GSM0710_SIGNAL_IC) == GSM0710_SIGNAL_IC)
                    {
//op.arg |= USSP_RI;
                        LOGMUX(LOG_INFO, "Signal Ring");
                    }
                    if ((signals & GSM0710_SIGNAL_DV) == GSM0710_SIGNAL_DV)
                    {
//op.arg |= USSP_DCD;
                        LOGMUX(LOG_INFO, "Signal DV");
                    }
                }
                else
                    LOGMUX(LOG_ERR, "Modem status command, but no info. i: %d, len: %d, data-len: %d",
                        i, length, frame->length);
                break;
            default:
                LOGMUX(LOG_ERR,"Unknown command (%d) from the control channel", type);
                if ((response = malloc(sizeof(char) * (2 + type_length))) != NULL)
                {
                    i = 0;
                    response[i++] = GSM0710_CONTROL_NSC;
                    type_length &= 127; //supposes that type length is less than 128
                    response[i++] = GSM0710_EA | (type_length << 1);
                    while (type_length--)
                    {
                        response[i] = frame->data[i - 2];
                        i++;
                    }
                    write_frame(0, response, i, GSM0710_TYPE_UIH);
                    free(response);
                    response = NULL;
                    supported = 0;
                }
                else
                    LOGMUX(LOG_ERR,"Out of memory, when allocating space for response");
                break;
            }
            if (supported)
            {
//acknowledge the command
                frame->data[0] = frame->data[0] & ~GSM0710_CR;
                LOGMUX(LOG_INFO, "response MSC...");
                write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);
                LOGMUX(LOG_INFO, "response MSC");
#if 0
                switch ((type & ~GSM0710_CR)){
                  case GSM0710_CONTROL_MSC:
                    if (frame->control & GSM0710_PF){ //Check if the P/F var needs to be set again (cleared in write_frame)
                      uih_pf_bit_received = 1;
                    }
                    LOGMUX(LOG_DEBUG, "Sending 1st MSC command App->Modem");
                    frame->data[0] = frame->data[0] | GSM0710_CR; //setting the C/R bit to "command"
                    write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);
                    break;
                  default:
                    break;
                }
#endif
            }
        }
        else
        {
//received ack for a command
            if (GSM0710_COMMAND_IS(type, GSM0710_CONTROL_NSC))
                LOGMUX(LOG_ERR, "The mobile station didn't support the command sent");
            else if(GSM0710_COMMAND_IS(type,GSM0710_CONTROL_MSC))
            {
                LOGMUX(LOG_INFO,"Channel:%d FC:%d",(frame->data[i] & 252) >> 2,frame->data[0]&0x2);
                LOGMUX(LOG_INFO,"\n\nGET ACK FOR MSC\n\n");
                //sleep(1);
            }
            else
                LOGMUX(LOG_INFO, "Command acknowledged by the mobile station");
        }
    }
    return 0;
//}}}
}

#if 0
#define CTRL_UIH_MSC            0xE0        // modem status command
#define SET_CR_MASK             0x02
#define SET_EA_MASK             0x01
#define CTRL_MSC_MSG_LENGTH     2
#define CTRL_MSC_FC_BIT         0x02

int send_modem_status(int bDlci, int fcOnOrOff)
{
//{{{
    unsigned char bUIHData[4];  
    unsigned char v24sig = 0;
    bUIHData[0] = CTRL_UIH_MSC | SET_CR_MASK | SET_EA_MASK;//0xE3
    bUIHData[1] = (CTRL_MSC_MSG_LENGTH << 1) | SET_EA_MASK;//0x05
    bUIHData[2] = (bDlci << 2) | SET_CR_MASK | SET_EA_MASK;
    if(fcOnOrOff)
        bUIHData[3] = v24sig | CTRL_MSC_FC_BIT | SET_EA_MASK;//0x03
    else
        bUIHData[3] = v24sig | SET_EA_MASK;//0x01

    if(MSC_flag & (1 << (bDlci - 1)))
    {
        return 0;
    }
//                                if(fcOnOrOff && ptr_channel_status[bDlci-1]==CHANNEL_STATE_SUSPEND)
//                                            return 0;
    write_frame(0, bUIHData, sizeof(bUIHData),GSM0710_TYPE_UIH);
    MSC_flag |= (1 << (bDlci - 1));
    LOGMUX(LOG_DEBUG,"+++++++");
//}}}
}
#endif

/* 
* Purpose:  Extracts and assembles frames from the mux GSM0710 buffer
* Input:      buf - the receiver buffer
* Return:   number of frames extracted
*/
static int extract_frames(GSM0710_Buffer* buf)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    int frames_extracted = 0;
    GSM0710_Frame local_frame;
    GSM0710_Frame *frame = &local_frame;
    
    while (gsm0710_base_buffer_get_frame(buf, frame) != NULL)
    {
        frames_extracted++;

        if ((GSM0710_FRAME_IS(GSM0710_TYPE_UI, frame) || GSM0710_FRAME_IS(GSM0710_TYPE_UIH, frame)))
        {
            LOGMUX(LOG_DEBUG, "Frame is UI or UIH");
            if (frame->control & GSM0710_PF){
              uih_pf_bit_received = 1;
            }

            if (frame->channel > 0)
            {
                LOGMUX(LOG_DEBUG, "Writing %d byte frame received on channel %d to %s",frame->length,frame->channel, channellist[frame->channel].ptsname);
//data from logical channel
                //syslogdump("Frame:", frame->data, frame->length);
                int write_result;

                if (channellist[frame->channel].fd > 0) {//reopening, discard the data
                    write_result = pseudo_device_write(&channellist[frame->channel], frame->data, frame->length);
                } else {
                    LOGMUX(LOG_INFO,"channel %d closed, discard the frame", frame->channel);
                    write_result = frame->length;
                }
                
                if (write_result < frame->length)
                {
                    LOGMUX(LOG_INFO, "write() returned. Written %d/%d bytes of frame to %s, errno: %d (%s)",
                        write_result,frame->length,channellist[frame->channel].ptsname, errno, strerror(errno));

                    //send_modem_status(frame->channel,1);
                    //fill_in_list(frame->channel,frame->data + write_result,frame->length - write_result);   
                    //MSC_flag |=  1 << (frame->channel - 1);
                }
            }
            else
            {
//control channel command
                LOGMUX(LOG_DEBUG, "Frame channel == 0, control channel command");
                handle_command(frame);
            }
        }
        else
        {
//not an information frame
            LOGMUX(LOG_DEBUG, "Not an information frame");
            switch ((frame->control & ~GSM0710_PF))
            {
            case GSM0710_TYPE_UA:
                LOGMUX(LOG_DEBUG, "Frame is UA");
                if (channellist[frame->channel].opened)
                {
                    SYSCHECK(logical_channel_close(channellist+frame->channel));
                    LOGMUX(LOG_INFO, "Logical channel %d for %s closed",
                        frame->channel, channellist[frame->channel].origin);
                }
                else
                {
                    if(channellist[frame->channel].disc_ua_pending == 0){
                        channellist[frame->channel].opened = 1;
                        if (frame->channel == 0)
                        {
                            LOGMUX(LOG_DEBUG, "Control channel opened");
                        }
                        else
                            LOGMUX(LOG_INFO, "Logical channel %d opened", frame->channel);
                    }
                    else {
                        LOGMUX(LOG_INFO, "UA to acknowledgde DISC on channel %d received", frame->channel);
                        channellist[frame->channel].disc_ua_pending = 0; 
                    }
                }
                if (frame->channel == vir_ports)
                    ql_cmux_debug = 0;
                break;
            case GSM0710_TYPE_DM:
                if (channellist[frame->channel].opened)
                {
                    SYSCHECK(logical_channel_close(channellist+frame->channel));
                    LOGMUX(LOG_INFO, "DM received, so the channel %d for %s was already closed",
                        frame->channel, channellist[frame->channel].origin);
                }
                else
                {
                    if (frame->channel == 0)
                    {
                        LOGMUX(LOG_INFO, "Couldn't open control channel.\n->Terminating");
                        serial.state = MUX_STATE_CLOSING;
//close channels
                    }
                    else
                        LOGMUX(LOG_INFO, "Logical channel %d for %s couldn't be opened", frame->channel, channellist[frame->channel].origin);
                }
                break;
            case GSM0710_TYPE_DISC:
                if (channellist[frame->channel].opened)
                {
                    channellist[frame->channel].opened = 0;
                    write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
                    if (frame->channel == 0)
                    {
                        serial.state = MUX_STATE_CLOSING;
                        LOGMUX(LOG_INFO, "Control channel closed");
                    }
                    else
                        LOGMUX(LOG_INFO, "Logical channel %d for %s closed", frame->channel, channellist[frame->channel].origin);
                }
                else
                {
//channel already closed
                    LOGMUX(LOG_WARNING, "Received DISC even though channel %d for %s was already closed",
                            frame->channel, channellist[frame->channel].origin);
                    write_frame(frame->channel, NULL, 0, GSM0710_TYPE_DM | GSM0710_PF);
                }
                break;
            case GSM0710_TYPE_SABM:
//channel open request
                if (channellist[frame->channel].opened)
                {
                    if (frame->channel == 0)
                        LOGMUX(LOG_INFO, "Control channel opened");
                    else
                        LOGMUX(LOG_INFO, "Logical channel %d for %s opened",
                            frame->channel, channellist[frame->channel].origin);
                }
                else
//channel already opened
                    LOGMUX(LOG_WARNING, "Received SABM even though channel %d for %s was already closed",
                        frame->channel, channellist[frame->channel].origin);
                channellist[frame->channel].opened = 1;
                write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
                if (frame->channel == vir_ports)
                    ql_cmux_debug = 0;
                break;
            }
        }
    }
    
    LOGMUX(LOG_DEBUG, "Leave");

    return frames_extracted;
//}}}
}

/* 
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
static void signal_treatment(
    int param)
{
//{{{
    switch (param)
    {
    case SIGPIPE:
        exit(0);
    break;
    case SIGHUP:
//reread the configuration files
    break;
    case SIGINT:
    case SIGTERM:
    case SIGUSR1:
        //exit(0);
//sig_term(param);

             set_main_exit_signal(1);
    break;
    case SIGKILL:
    default:
        exit(0);
    break;
    }
//}}}
}

static const unsigned int autosuspend_delay_ms = 0; //2000;
static unsigned int autosuspend_idle_ms = 0;
static unsigned int autosuspend_gpio_dtr = 0;
static unsigned int autosuspend_suspend_state = 0; // 0 ~ active, 1 ~ suspend
static pthread_mutex_t autosuspend_lock = PTHREAD_MUTEX_INITIALIZER;

static inline void autosuspend_change_dtr(const Serial* serial, int Set) {
    if (autosuspend_gpio_dtr) {
        static int dtr_fd = 0;

        if (dtr_fd == 0) {
            char vaule_str[64];

            snprintf(vaule_str, sizeof(vaule_str), "/sys/class/gpio%u/value", autosuspend_gpio_dtr);
            dtr_fd = open(vaule_str, O_RDWR | O_NOCTTY | O_NONBLOCK);
        }

        if (dtr_fd != -1) {
            write(dtr_fd, Set ? "1" : "0", 2);
        }
    }
    else if (serial->fd != -1) {
        int modembits = TIOCM_DTR;

        ioctl(serial->fd, Set ? TIOCMBIS : TIOCMBIC, &modembits);
    }
}

static inline void autosuspend_mark_last_busy(const Serial* serial) {
    if (!autosuspend_delay_ms)
        return;

    pthread_mutex_lock(&autosuspend_lock);
    if (autosuspend_idle_ms) {
        autosuspend_idle_ms = 0;
    }
    if (autosuspend_suspend_state) {
        autosuspend_suspend_state = 0;
        autosuspend_change_dtr(serial, 1);
        LOGMUX(LOG_INFO, "low power mode exit");
        usleep(20*1000);
    }
    pthread_mutex_unlock(&autosuspend_lock);
}

static inline void autosuspend_add_idle_ms(const Serial* serial, unsigned int idle_ms) {
    if (!autosuspend_delay_ms)
        return;

    pthread_mutex_lock(&autosuspend_lock);
    autosuspend_idle_ms += idle_ms;
    if (autosuspend_idle_ms >= autosuspend_delay_ms) {
        autosuspend_idle_ms = 0;
        autosuspend_suspend_state = 1;
        autosuspend_change_dtr(serial, 0);
        LOGMUX(LOG_INFO, "low power mode enter");
    }
    pthread_mutex_unlock(&autosuspend_lock);
}

/* 
* Purpose:  Poll a device without select(). read() will do the blocking if VMIN=1.
*                call a reading function for the particular device
* Input:      vargp - a pointer to a Poll_Thread_Arg struct.
* Return:    NULL if error
*/
static void* serial_poll_thread(void *vargp) {
    Poll_Thread_Arg* poll_thread_arg = (Poll_Thread_Arg*)vargp;
    Serial *serial_p = poll_thread_arg->read_function_arg;

    LOGMUX(LOG_DEBUG,"Enter");

    while (1)
    {
        int ret;
        struct pollfd pollfds[] ={{poll_thread_arg->fd, POLLIN, 0}};
        unsigned int idle_ms = -1;

        if (autosuspend_delay_ms && !autosuspend_suspend_state)
            idle_ms = 500;

        do {
            ret = poll(pollfds, 1, /*(MSC_flag != 0) ? 1000 : */idle_ms); //poll channle X to get POLLOUT is more better
        } while ((ret < 0 ) && (errno == EINTR));    

        //LOGMUX(LOG_DEBUG, "poll revents = %x, ret = %d, MSC_flag = %d", pollfds[0].revents, ret, MSC_flag);
        //if ((MSC_flag != 0))
        //{
        //    send_left_data(vir_ports);
        //}

        if ((ret < 0) || (pollfds[0].revents & (POLLERR|POLLHUP))) {
            LOGMUX(LOG_ERR, "Device polling ret = %d, revents: %x, errno: %d (%s)", ret, pollfds[0].revents, errno, strerror(errno));
            break;
        }
    
        if (!(pollfds[0].revents & POLLIN)) {
            if (ret == 0) autosuspend_add_idle_ms(serial_p, idle_ms);
            continue;
        }

       autosuspend_mark_last_busy(serial_p);
       if (serial_device_read(serial_p) > 0)
        {
            if (gsm0710_buffer_length(serial_p->in_buf) > 0)
            {
                if (extract_frames(serial_p->in_buf)) {
                    int vir_port = 1;
                    for (vir_port = 1; vir_port <= vir_ports; vir_port++) {
                        pseudo_device_write(&channellist[vir_port], NULL, 0); //flush all data to virtual port
                    }
                }

                if (serial_p->in_buf->readp != serial_p->in_buf->data) { //relayout data in cache_buf
                    if (gsm0710_buffer_length(serial_p->in_buf)) {
                        //LOGMUX(LOG_DEBUG, "memmove(0, %ld, %d)", (long)(serial_p->in_buf->readp - serial_p->in_buf->data), gsm0710_buffer_length(serial_p->in_buf));
                        memmove(serial_p->in_buf->data, serial_p->in_buf->readp, gsm0710_buffer_length(serial_p->in_buf));
                    }
                    serial_p->in_buf->readp = serial_p->in_buf->data;
                    serial_p->in_buf->writep = serial_p->in_buf->data + gsm0710_buffer_length(serial_p->in_buf);
                }
            }
        }
    }

    LOGMUX(LOG_ERR, "Device polling thread terminated");
    free(poll_thread_arg);  //free the memory allocated for the thread args before exiting thread
    return NULL;
}

static int serial_device_read(Serial * serial)
{
    int length = 0;

    switch (serial->state)
    {
        case MUX_STATE_MUXING:
            length = min(gsm0710_buffer_free(serial->in_buf), (serial->in_buf->endp - serial->in_buf->writep));

            if (length > 0 ) /*available space in buffer (not locked since we want to utilize all available space)*/
            {
                length = read(serial->fd, serial->in_buf->writep, length);
                if (length > 0)
                {
                    LOGMUX(LOG_DEBUG,"READ SIZE : %d", length);
                    syslogdump("<s ", serial->in_buf->writep, length);
                    serial->in_buf->writep += length;
                    if (serial->in_buf->writep == serial->in_buf->endp)
                        serial->in_buf->writep = serial->in_buf->data;

                    serial->in_buf->datacount += length; /*updating the data-not-yet-read counter*/
                    LOGMUX(LOG_DEBUG,"GSM0710 buffer (up-to-date): written %d, free %d, stored %d",
                        length,gsm0710_buffer_free(serial->in_buf),gsm0710_buffer_length(serial->in_buf));
                    
                    if (length > serial->in_buf->max_count) {
                        LOGMUX(LOG_INFO,"MAX READ SIZE : %d", length);
                        LOGMUX(LOG_INFO,"GSM0710 buffer (up-to-date): written %d, free %d, stored %d",
                            length,gsm0710_buffer_free(serial->in_buf),gsm0710_buffer_length(serial->in_buf)); 
                        serial->in_buf->max_count = length;
                    }
                }
                else
                {
                    LOGMUX(LOG_INFO, "READ SIZE : %d, errno: %d (%s)", length, errno, strerror(errno));
                }
            }
            else
            {
                LOGMUX(LOG_DEBUG,"Free space is not enough");
            }
        break;
        
        default:
            LOGMUX(LOG_WARNING, "Don't know how to handle reading in state %d", serial->state);
        break;
    }
    
    return length;
}

static int serial_device_write(Serial *channel, unsigned char *data, int length) {
    if (length > 0) {
        memcpy(channel->cache_buf + channel->cache_offset, data, length);
        channel->cache_frames++;
        channel->cache_offset += length;
    }

    if (channel->cache_offset == 0)
        return 0;
    
    if (length < cmux_FRAME || channel->cache_frames == QUECTEL_CACHE_FRAMES) {
        int cur = 0;
        
        autosuspend_mark_last_busy(&serial);
        while (cur < channel->cache_offset) {
            struct pollfd pollfds[] = {{channel->fd, POLLOUT, 0}};
            int ret, written;

            written = write (channel->fd, channel->cache_buf+cur, channel->cache_offset-cur);

            if (written <= 0 && errno != EAGAIN) {
                LOGMUX(LOG_WARNING, "write = %d/%d, errno: %d (%s)",  written, channel->cache_offset-cur, errno, strerror(errno));
                break;
            } else if (written > 0) {
                cur += written;
            }

            if (cur >= channel->cache_offset)
                break;

            do {
                ret = poll(pollfds, 1, 1000);
            } while ((ret < 0 ) && (errno == EINTR));

            if ((ret < 0) || (pollfds[0].revents & (POLLERR|POLLHUP))) {
                LOGMUX(LOG_ERR, "Device polling ret = %d, revents: %x, errno: %d (%s)", ret, pollfds[0].revents, errno, strerror(errno));
                break;
            }
        }

        if (cur < channel->cache_offset) {
            LOGMUX(LOG_WARNING, "Couldn't write the whole data to the serial port. Wrote only %d/%d bytes, errno: %d (%s)",
                cur, channel->cache_offset, errno, strerror(errno));
        }

        channel->cache_frames = channel->cache_offset = 0;
    }

    return length;
}

/* 
* Purpose:  Open and initialize the serial device used.
* Input:      serial - the serial struct
* Return:    0 if port successfully opened, else 1.
*/
int open_serial_device(Serial* serial, int port_speed, int ctsrts)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    unsigned int i;
    for (i=0;i<GSM0710_MAX_CHANNELS;i++)
        SYSCHECK(logical_channel_init(channellist+i, i));
/* open the serial port */
    SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));

    LOGMUX(LOG_INFO, "Opened serial port");

    struct termios t;
    memset(&t, 0, sizeof(t));
    tcgetattr(serial->fd, &t);
    cfmakeraw(&t);
    t.c_cflag |= CLOCAL; 
    if(ctsrts == 1)
        t.c_cflag |= CRTSCTS; //enable the flow control on dev board
    else
        t.c_cflag &= ~(CRTSCTS);//disable the flow control on dev board
    speed_t speed = baud_bits[port_speed];
    cfsetispeed(&t, speed);
    cfsetospeed(&t, speed);
    SYSCHECK(tcsetattr(serial->fd, TCSANOW, &t));
    int status = TIOCM_DTR | TIOCM_RTS;
    ioctl(serial->fd, TIOCMBIS, &status);
    autosuspend_mark_last_busy(serial);
    LOGMUX(LOG_INFO, "Configured serial device");
    serial->ping_number = 0;
    time(&serial->frame_receive_time); //get the current time
    serial->state = MUX_STATE_INITILIZING;
    LOGMUX(LOG_DEBUG, "Switched Mux state to %d ",serial->state);

    //enable qualcomm high speed uart clock, or will lost rx data
    if (!strncmp(serial->devicename, "/dev/ttyHS", strlen("/dev/ttyHS"))) {
        char *uart_clock = NULL;

        asprintf(&uart_clock, "/sys/class/tty/%s/device/clock", &serial->devicename[strlen("/dev/")]);
        if (!access(uart_clock, W_OK)) {
            #define MSM_ENABLE_UART_CLOCK 0x5441
            ioctl(serial->fd, MSM_ENABLE_UART_CLOCK, NULL);
        }
        free(uart_clock);
    }
    return 0;
//}}}
}

/* 
* Purpose:  Initialize mux connection with modem.
* Input:      serial - the serial struct
* Return:    0
*/
int start_muxer(
    Serial* serial
    )
{
//{{{
    LOGMUX(LOG_INFO, "Configuring modem");
    char gsm_command[100];
    //check if communication with modem is online
    if (chat(serial->fd, "AT\r\n", 1) < 0)
    {
        LOGMUX(LOG_WARNING, "Modem does not respond to AT commands, trying close mux mode");
        //if (cmux_mode) we do not know now so write both
            write_frame(0, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR);
        //else
            write_frame(0, close_channel_cmd, 2, GSM0710_TYPE_UIH);
        if ((chat(serial->fd, "AT\r\n", 1) < 0) && (cmux_port_speed_default != cmux_port_speed)) { //reopen serialfd
            close(serial->fd);
            sleep(1);
            cmux_port_speed_default = cmux_port_speed;
            if (open_serial_device(serial, cmux_port_speed_default, 0) != 0){
                LOGMUX(LOG_WARNING, "Could not re-open serial device and continue muxer");
                return 1;
            }
        }
        SYSCHECK(chat(serial->fd, "AT\r\n", 1));
    }

    if(hardware_flow_control == 1)
    {
        SYSCHECK(chat(serial->fd, "AT+IFC=2,2\r\n", 2)); // enable flow control function for module
    }
    else
    {
        SYSCHECK(chat(serial->fd, "AT+IFC=0,0\r\n", 2)); // disable flow control function for module
    }

    if (cmux_port_speed != 5) {
        snprintf(gsm_command, sizeof(gsm_command), "AT+IPR=%d\r", baud_rates[cmux_port_speed]);
        SYSCHECK(chat(serial->fd, gsm_command, 2));
    }
        
    if ((cmux_port_speed != 5) || hardware_flow_control) { //reopen serialfd
        close(serial->fd);
        sleep(1);
        if (open_serial_device(serial, cmux_port_speed, hardware_flow_control) != 0){
            LOGMUX(LOG_WARNING, "Could not re-open serial device and continue muxer");
            return 1;
        }
    }

    //SYSCHECK(chat(serial->fd, "ATZ\r\n", 3)); //mdm9x07 will reset AT+IFC/AT+IPR by 20170914
    SYSCHECK(chat(serial->fd, "ATE0\r\n", 1));
    
    if (pin_code >= 0)
    {
        LOGMUX(LOG_DEBUG, "send pin %04d", pin_code);
//Some modems, such as webbox, will sometimes hang if SIM code is given in virtual channel
        SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CPIN=%04d\r\n", pin_code));
        SYSCHECK(chat(serial->fd, gsm_command, 10));
    }

    if (autosuspend_delay_ms) {
/*
 * https://ticket.quectel.com/browse/FAE-6368
 * for M66, Enable CMUX enter to sleep via DTR pin change to high.
*/
        /*SYSCHECK*/(chat(serial->fd, "AT+QMUXCFG=1,3,1\r\n", 3));
        SYSCHECK(chat(serial->fd, "AT+QSCLK=1\r\n", 3));
    }
       
    if (cmux_mode){
        SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CMUX=1\r\n"));
    }
    else {
        SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CMUX=%d,%d,%d,%d\r\n"
            , cmux_mode
            , cmux_subset
            , quectel_speeds[cmux_port_speed]
            , cmux_N1
            ));
    }

    LOGMUX(LOG_INFO, "Starting mux mode");
    SYSCHECK(chat(serial->fd, gsm_command, 3));
    serial->state = MUX_STATE_MUXING;
    LOGMUX(LOG_DEBUG, "Switched Mux state to %d ",serial->state);
    LOGMUX(LOG_INFO, "Waiting for mux-mode");
    //joe
    //fix bug:cmux init cost too much time
    usleep(1000*150);
    LOGMUX(LOG_INFO, "Init control channel");
    return 0;
//}}}
}

/* 
* Purpose:  Close all devices, send mux termination frames
* Input:      -
* Return:    0
*/
static int close_devices()
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    int i;
    for (i=1;i<GSM0710_MAX_CHANNELS;i++)
    {
//terminate command given. Close channels one by one and finaly close
//the mux mode
        if (channellist[i].fd >= 0)
        {
            if (channellist[i].opened)
            {
                LOGMUX(LOG_INFO, "Closing down the logical channel %d", i);
                if (cmux_mode)
                    write_frame(i, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR);
                else
                    write_frame(i, close_channel_cmd, 2, GSM0710_TYPE_UIH);
                SYSCHECK(logical_channel_close(channellist+i));
            }
            LOGMUX(LOG_INFO, "Logical channel %d closed", channellist[i].id);
        }
    }
    if (serial.fd >= 0)
    {
        if (cmux_mode)
            write_frame(0, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR);
        else
            write_frame(0, close_channel_cmd, 2, GSM0710_TYPE_UIH);
#ifdef MUX_ANDROID
        /* tcdrain not available on ANDROID */
        ioctl(serial.fd, TCSBRK, 1); //equivalent to tcdrain(). perhaps permanent replacement?
#else
        tcdrain(serial.fd);
#endif
        SYSCHECK(close(serial.fd));
        serial.fd = -1;
    }
    serial.state = MUX_STATE_OFF;
    return 0;
//}}}
}
#if 0/*before start muxer,send DSCI frame to close down DLC 0*/
static int close_DLC0(Serial * serial)
{
    char buff[10] = {0xf9,0x03,0xEF,0x05,0xc3,0x01,0xF2,0xF9};
    int ret;
    int try = 0;
    
    tcflush(serial->fd, TCIFLUSH);
    
    ret = write(serial->fd, buff, strlen(buff));
    LOGMUX(LOG_INFO, "Close_DLC0 write return = %d", ret);
    usleep(500 * 1000);
    tcflush(serial->fd, TCIFLUSH);    //clear recv queue
    while(try ++ < 5)
    {
        ret = chat(serial->fd, "AT\r\n", 1);
        if(ret == 0) //succcess
        {
            LOGMUX(LOG_INFO, "Close_DLC0 success!");
            tcflush(serial->fd, TCIFLUSH);    //clear recv queue
            return 0;
        }
    }
    LOGMUX(LOG_INFO , "Close_DLC0 failed, may be risky!");
    return -1;
}
#endif 
/* 
* Purpose:  The watchdog state machine restarted every x seconds
* Input:      serial - the serial struct
* Return:    1 if error, 0 if success
*/
static int watchdog(Serial * serial)
{
//{{{
    LOGMUX(LOG_DEBUG, "Enter");
    int i;

    LOGMUX(LOG_DEBUG, "Serial state is %d", serial->state);
    switch (serial->state)
    {
    case MUX_STATE_OPENING:
        if (open_serial_device(serial, 5, 0) != 0) { //first use default settings to open serial
            LOGMUX(LOG_WARNING, "Could not open serial device and start muxer");
                     return 1;
              }
        LOGMUX(LOG_INFO, "Watchdog started");
    case MUX_STATE_INITILIZING:
#if 0 /*before start muxer,send DSCI frame to close down DLC 0*/
        close_DLC0(serial);
#endif    
            if (start_muxer(serial) < 0) {
                  LOGMUX(LOG_WARNING, "Could not open all devices and start muxer errno=%d", errno);
                return 1;
            }

              /* Create thread for polling on serial device (mux input) and writing to GSM0710 mux buffer */
              Poll_Thread_Arg *poll_thread_arg = (Poll_Thread_Arg*) malloc(sizeof(Poll_Thread_Arg)); //iniitialize pointer to thread args ;
              if (NULL == poll_thread_arg) {
                  LOGMUX(LOG_ERR, "%s:%d malloc failed", __func__, __LINE__);
                  return 1;
              }
              poll_thread_arg->fd = serial->fd;
              poll_thread_arg->read_function_arg = (void *) serial;
        if(create_thread(&ser_read_thread, serial_poll_thread,(void*) poll_thread_arg)!=0){ //create thread for reading input from serial device
          LOGMUX(LOG_ERR,"Could not create thread for listening on %s", serial->devicename);
          return 1;
        }
           LOGMUX(LOG_DEBUG, "Thread is running and listening on %s", serial->devicename); //listening on serial port
           write_frame(0, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF); //need to move? messy

        //tempary solution. call to allocate virtual port(s)
        if (vir_ports<GSM0710_MAX_CHANNELS){         
            for (i=1;i<=vir_ports;i++) {
                int open_timeout = 500;
                LOGMUX(LOG_INFO, "Allocating logical channel %d/%d ",i,vir_ports);
                if((c_alloc_channel("watchdog_init", &pseudo_terminal[i])) != 0) {
                    set_main_exit_signal(1); //exit main function if channel couldn't be allocated
                    return 1;
                } 
                //joe
                //fix bug:cmux init cost too much time
                do {
                    usleep(1000*10);
                } while (open_timeout-- > 0 && channellist[i].opened == 0);
                if (channellist[i].opened == 0) {
                    LOGMUX(LOG_ERR,"Cannot open virtual port %d", i);
                    LOGMUX(LOG_ERR,"If you are using EC20, the RTS pin must pull down to low level!");
                    return 1;
                }
            }
        }
        else{
            LOGMUX(LOG_ERR,"Cannot allocate %d virtual ports", vir_ports);
            return 1;
        }
        LOGMUX(LOG_INFO, "Multiplexing started..");
    break;
    case MUX_STATE_MUXING:
        /* Re-establish previously closed logical channel and pseudo terminal */
        if (pts_reopen==1){
            for(i=1;i<=vir_ports;i++){
                if (channellist[i].reopen == 1){
                  if(restart_pty_interface(&channellist[i]) != 0)
                      {
                      set_main_exit_signal(1); //exit main function if channel couldn't be allocated
                      }
                    else {
                    channellist[i].reopen = 0;
                    pthread_mutex_lock(&pts_reopen_lock);
                     pts_reopen=0;
                    pthread_mutex_unlock(&pts_reopen_lock);
                        }
                }
                }
            }    
        if (use_ping)
        {
            if (serial->ping_number > use_ping)
            {
                LOGMUX(LOG_DEBUG, "no ping reply for %d times, resetting modem", serial->ping_number);
                serial->state = MUX_STATE_CLOSING;
                LOGMUX(LOG_DEBUG, "Switched Mux state to %d ",serial->state);
            }
            else
            {
                LOGMUX(LOG_DEBUG, "Sending PING to the modem");
                //write_frame(0, psc_channel_cmd, sizeof(psc_channel_cmd), GSM0710_TYPE_UI);
                write_frame(0, test_channel_cmd, sizeof(test_channel_cmd), GSM0710_TYPE_UI);
                serial->ping_number++;
            }
        }
        if (use_timeout)
        {
            time_t current_time;
            time(&current_time); //get the current time
            if (current_time - serial->frame_receive_time > use_timeout)
            {
                LOGMUX(LOG_DEBUG, "timeout, resetting modem");
                serial->state = MUX_STATE_CLOSING;
                LOGMUX(LOG_DEBUG, "Switched Mux state to %d ",serial->state);
            }
        }

    break;
    case MUX_STATE_CLOSING:
        close_devices();
        serial->state = MUX_STATE_OPENING;
        LOGMUX(LOG_DEBUG, "Switched Mux state to %d ",serial->state);
    break;
    default:
        LOGMUX(LOG_WARNING, "Don't know how to handle state %d", serial->state);
    break;
    }
    return 0;
//}}}
}

/* 
* Purpose:  shows how to use this program
* Input:      name - string containing name of program
* Return:    -1
*/
static int usage(
    char *_name)
{
//{{{
    fprintf(stderr, "\tUsage: %s [options]\n", _name);
    fprintf(stderr, "Options:\n");
    // process control
    fprintf(stderr, "\t-d: Fork, get a daemon [%s]\n", no_daemon?"no":"yes");
    fprintf(stderr, "\t-v: Set verbose logging level. 0 (Silent) - 7 (Debug) [%d]\n",syslog_level);
    // modem control
    fprintf(stderr, "\t-s <serial port name>: Serial port device to connect to [%s]\n", serial.devicename);
    fprintf(stderr, "\t-c <hardware flow control>:  Hardware flow control [%s]\n", hardware_flow_control?"enabled":"disabled");
    fprintf(stderr, "\t-t <timeout>: reset modem after this number of seconds of silence [%d]\n", use_timeout);
    fprintf(stderr, "\t-P <pin-code>: PIN code to unlock SIM [%d]\n", pin_code);
    fprintf(stderr, "\t-p <number>: use ping and reset modem after this number of unanswered pings [%d]\n", use_ping);
    // legacy - will be removed
    fprintf(stderr, "\t-b <baudrate>: mode baudrate [%d]\n", baud_rates[cmux_port_speed]);
    //fprintf(stderr, "\t-m <modem>: Mode (basic, advanced) [%s]\n", cmux_mode?"advanced":"basic");
    //fprintf(stderr, "\t-f <framsize>: Frame size [%d]\n", cmux_N1);
    fprintf(stderr, "\t-n <number of ports>: Number of virtual ports to create, must be in range 1-31 [%d]\n", vir_ports);
    fprintf(stderr, "\t-o <output log to file>: Output log to /tmp/gsm0710muxd.log [%s]\n", logtofile?"yes":"no");
    //
    fprintf(stderr, "\t-h: Show this help message and show current settings.\n");
    return -1;
//}}}
}

static void set_main_exit_signal(int signal){
    pthread_mutex_lock(&main_exit_signal_lock);
    main_exit_signal = signal;
    pthread_mutex_unlock(&main_exit_signal_lock);
}

static int create_thread(pthread_t * thread_id, void * thread_function, void * thread_function_arg ){
    LOGMUX(LOG_DEBUG,"Enter");

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg)!=0){
        LOGMUX(LOG_ERR,"Could not create thread! errno : %d (%s)", errno, strerror(errno));
        set_main_exit_signal(1); //exit main function if thread couldn't be created
        return 1;
    }
    pthread_attr_destroy(&thread_attr); /* Not strictly necessary */
    
    return 0; //thread created successfully
}

static void* pseudo_poll_thread(void *vargp) {
    Poll_Thread_Arg* poll_thread_arg = (Poll_Thread_Arg*)vargp;
    Channel* channel = (Channel*)(poll_thread_arg->read_function_arg);

    LOGMUX(LOG_DEBUG,"Enter");

    if (poll_thread_arg->fd== -1 ) {
        LOGMUX(LOG_ERR, "Serial port not initialized");
        goto terminate;
    }

    while (1) {
        int ret;
        struct pollfd pollfds[] ={{poll_thread_arg->fd, POLLIN, 0}};

        do {
            ret = poll(pollfds, 1, -1);
        } while ((ret < 0 ) && (errno == EINTR));    

        if ((ret < 0) || (pollfds[0].revents & (POLLERR|POLLHUP))) {
            LOGMUX(LOG_ERR, "Device polling ret = %d, revents: %x, errno: %d (%s)", ret, pollfds[0].revents, errno, strerror(errno));
            goto terminate;
        }
    
        if (!(pollfds[0].revents & POLLIN))
            continue;

        if (!channel->opened) {
            LOGMUX(LOG_WARNING, "Write to a channel which wasn't acked to be open.");
            write_frame(channel->id, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF);
            LOGMUX(LOG_DEBUG, "Leave");
            goto terminate;
        }

        if (!channel->frame_allowed) {
            LOGMUX(LOG_WARNING, "Write to a channel which wasn't frame allowed.");
            usleep(100*1000);
            continue;
        }
    
        if (pseudo_device_read(channel) <= 0 ) {
            LOGMUX(LOG_WARNING, "Device read function returned error, errno: %d (%s)", errno, strerror(errno));
            goto terminate;
        }

        pthread_mutex_lock(&write_frame_lock);
        serial_device_write(&serial, NULL, 0); //flush all data to serial port
        pthread_mutex_unlock(&write_frame_lock);
  }

terminate:
    LOGMUX(LOG_INFO, "Appl. dropped connection, device %s shutting down. Set to be reopened", channel->ptsname);
    /*disconnect channel from pty*/
    close(channel->fd);
    channel->fd = -1;
    free(channel->ptsname);
    channel->ptsname = NULL;
    pthread_mutex_lock(&pts_reopen_lock);
    pts_reopen = 1; /*global flag to signal at least one channel needs to be reopened */
    pthread_mutex_unlock(&pts_reopen_lock);
    channel->reopen = 1; /* set channel to be reopened. this will not be cleared when doing a channel close */
    
    LOGMUX(LOG_ERR, "Device polling thread terminated");
    free(poll_thread_arg);  //free the memory allocated for the thread args before exiting thread
    return NULL;
}

/* 
* Purpose:  The main program loop
* Input:      argc - number of input arguments
*                argv - array of strings (input arguments)
* Return:    0
*/
#ifdef LINUX_RIL_SHLIB
int gsm0710muxd(int argc,char *argv[])
#else
int main(int argc,char *argv[])
#endif
{
//{{{
    ql_cmux_debug = 1;
    LOGMUX(LOG_INFO, "Quectel CMUX Revision: %s", CMUX_DRIVER_VERSION);
    LOGMUX(LOG_DEBUG, "Enter");

    int opt;
//for fault tolerance
    if (!serial.devicename)
        serial.devicename = "/dev/ttyS0";
    
    while ((opt = getopt(argc, argv, "dov:s:c:t:p:f:n:h?m:b:P:")) > 0)
    {
        switch (opt)
        {
        case 'v':
            syslog_level = atoi(optarg);
            if ((syslog_level>LOG_DEBUG) || (syslog_level < 0)){
              usage(argv[0]);
              exit(0);
            #ifdef MUX_ANDROID
            //syslog_level=android_log_lvl_convert[syslog_level];
            #endif
            }
            break;
        case 'o':
            logtofile = 1;
            if ((muxlogfile=fopen("/tmp/gsm0710muxd.log", "w+")) == NULL){
                 fprintf(stderr, "Error: %s.\n", strerror(errno));
                 usage(argv[0]);
                 exit(0);
                 }
            else
                fprintf(stderr, "gsm0710muxd log is output to /tmp/gsm0710muxd.log\n");
            break;            
        case 'd':
            no_daemon = !no_daemon;
            break;
        case 's':
            serial.devicename = optarg;
            break;
        case 'c':
            hardware_flow_control = atoi(optarg);
            break;
        case 't':
            use_timeout = atoi(optarg);
            break;
        case 'p':
            use_ping = atoi(optarg);
            break;
        case 'P':
            pin_code = atoi(optarg);
            break;
        case 'f':
            //cmux_N1 = atoi(optarg); //quectel force use 127
            break;
        case 'n':
            vir_ports = atoi(optarg);
            if ((vir_ports>GSM0710_MAX_CHANNELS-1) || (vir_ports < 1)){
              usage(argv[0]);
              exit(0);
            }
            break;
        case 'm':
            if (!strcmp(optarg, "basic"))
                cmux_mode = 0;
            else if (!strcmp(optarg, "advanced"))
                ;//cmux_mode = 1;
            else
                cmux_mode = 0;
            break;
        case 'b': {
            int rateIndex = baud_rate_index(atoi(optarg));
            if(-1 != rateIndex) {
                cmux_port_speed = rateIndex;
            }
            break;
        }
        default:
        case '?':
        case 'h':
            usage(argv[0]);
            exit(0);
            break;
        }
    }

    umask(0);
//signals treatment
    signal(SIGHUP, signal_treatment);
    signal(SIGPIPE, signal_treatment);
    signal(SIGKILL, signal_treatment);
    signal(SIGINT, signal_treatment);
    signal(SIGUSR1, signal_treatment);
    signal(SIGTERM, signal_treatment);

#ifndef MUX_ANDROID
    if (no_daemon)
        openlog(argv[0], LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_LOCAL0);
    else
        openlog(argv[0], LOG_NDELAY | LOG_PID, LOG_LOCAL0);
#endif
//allocate memory for data structures
    serial.in_buf = gsm0710_buffer_init();
    if (serial.in_buf == NULL)
    {
        LOGMUX(LOG_ERR,"Out of memory");
        exit(-1);
    }

//Initialize modem and virtual ports
    serial.state = MUX_STATE_OPENING;

    LOGMUX(LOG_INFO,"Called with following options:");
    LOGMUX(LOG_INFO,"\t-d: Fork, get a daemon [%s]", no_daemon?"no":"yes");
    LOGMUX(LOG_INFO,"\t-v: Set verbose logging level. 0 (Silent) - 7 (Debug) [%d]",syslog_level);
    LOGMUX(LOG_INFO,"\t-s <serial port name>: Serial port device to connect to [%s]", serial.devicename);
    LOGMUX(LOG_INFO,"\t-c <hardware flow control>:  Hardware flow control [%s]", hardware_flow_control?"enabled":"disabled");
    LOGMUX(LOG_INFO,"\t-t <timeout>: reset modem after this number of seconds of silence [%d]", use_timeout);
    LOGMUX(LOG_INFO,"\t-P <pin-code>: PIN code to unlock SIM [%d]", pin_code);
    LOGMUX(LOG_INFO,"\t-p <number>: use ping and reset modem after this number of unanswered pings [%d]", use_ping);
    LOGMUX(LOG_INFO,"\t-b <baudrate>: mode baudrate [%d]", baud_rates[cmux_port_speed]);
    LOGMUX(LOG_INFO,"\t-m <modem>: Mode (basic, advanced) [%s]", cmux_mode?"advanced":"basic");
    LOGMUX(LOG_INFO,"\t-f <framsize>: Frame size [%d]", cmux_N1);
    LOGMUX(LOG_INFO,"\t-n <number of ports>: Number of virtual ports to create, must be in range 1-4 [%d]", vir_ports);
    LOGMUX(LOG_INFO,"\t-o <output log to file>: Output log to /tmp/gsm0710muxd.log [%s]", logtofile?"yes":"no");

    while((main_exit_signal==0) && (watchdog(&serial)==0)){
        LOGMUX(LOG_INFO, "GSM0710 buffer. Stored %d", gsm0710_buffer_length(serial.in_buf));
        LOGMUX(LOG_INFO, "Frames received/dropped: %ld/%ld",serial.in_buf->received_count,serial.in_buf->dropped_count);
        sleep(5);
    }


//finalize everything
    SYSCHECK(close_devices());
    gsm0710_buffer_destroy(serial.in_buf);
    LOGMUX(LOG_INFO, "Received %ld frames and dropped %ld received frames during the mux-mode",
        serial.in_buf->received_count, serial.in_buf->dropped_count);
    LOGMUX(LOG_DEBUG, "%s finished", argv[0]);

#ifndef MUX_ANDROID    
    closelog();// close syslog
#endif

if (logtofile)
  fclose(muxlogfile);    

return 0;
//}}}
}

#ifdef MUX_ANDROID
static int qkill(pid_t pid, int signo) {
    int ret = kill(pid, signo);
    //LOGMUX(LOG_INFO, "kill(%d, %d)=%d", pid, signo, ret);
    return ret;
}
static pid_t gsm0710muxd_pid = 0;
int gsm0710muxd(const char *serialname, int speed, int ctsrts) {
    int child_pid = 0;
    int i = 0;
    LOGMUX(LOG_INFO, "%s serialname = %s, gsm0710muxd_pid = %d", __func__, serialname, gsm0710muxd_pid);
    if (gsm0710muxd_pid && !qkill(gsm0710muxd_pid, 0)) {
        qkill(gsm0710muxd_pid, SIGKILL);
        while (!waitpid(gsm0710muxd_pid, NULL, WNOHANG))
            sleep(1);
        LOGMUX(LOG_INFO, "kill gsm0710muxd finished!");
    }

    for (i = 1; i <= vir_ports; i++) {
        char link_name[64];
        char command[64];
        sprintf(link_name,MUX_CHN_DIR "%d", i);
        sprintf(command,"rm %s -f", link_name);
        ql_system(command);
    }

    memset(&serial, 0x00, sizeof(serial));
    memset(&channellist, 0x00, sizeof(channellist));
    serial.devicename = serialname;
    cmux_port_speed = baud_rate_index(speed);
    hardware_flow_control = ctsrts;

    gsm0710muxd_pid = 0;
    child_pid = fork();
    if (child_pid == 0) {
        char *argv[] = {"gsm0710muxd", NULL};
        //int ret = prctl(PR_SET_NAME,"gsm0710muxd",NULL,NULL,NULL);
        //LOGMUX(LOG_INFO, "prctl = %d, errno: %d (%s)", ret, errno, strerror(errno));
        exit(main(1, argv));
    } else if(child_pid > 0) {
        gsm0710muxd_pid = child_pid;
        do {
            sleep(1);
        } while (!qkill(child_pid, 0) && access(MUX_CHN_DIR "3", O_RDWR));
        
        if (!qkill(child_pid, 0)) {
            if (!access(MUX_CHN_DIR "3", O_RDWR)) {
                LOGMUX(LOG_INFO, "%s gsm0710muxd_pid = %d", __func__, gsm0710muxd_pid);
                return 0;
            }
        } else {
            gsm0710muxd_pid = 0;
        }
    }
    return -1;
}
#endif

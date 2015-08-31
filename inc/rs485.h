#ifndef __RS485_H__
#define __RS485_H__

#define XFER_PARM    0x01
#define XFER_PDAM    0x02
#define XFER_ZARM    0x03
#define XFER_ZDAM    0x04
#define XFER_ZBP     0x05
#define XFER_ZBPR    0x06
#define XFER_PBP     0x07
#define XFER_PBPR    0x08
#define XFER_ZALT    0x09
#define XFER_ZALR    0x0a
#define XFER_ZFLT    0x0b
#define XFER_ZFLR    0x0c

#define XFER_FAIL    0x0d

#define XFER_PGE     0x20
#define XFER_PGX     0x21
#define XFER_APF     0x22
#define XFER_APR     0x23
#define XFER_BL      0x24
#define XFER_BLR     0x25
#define XFER_BF      0x26
#define XFER_BFR     0x27

extern void  rs485_init(void);
extern void  rs485_uninit(void);
extern void  rs485_start(void);
extern void  rs485_exit(void);
extern void  uart_xfer_realt(int id, int t);
extern void  uart_xfer_print(int id, int t);
extern void  uart_xfer_zstat(int id);


#endif


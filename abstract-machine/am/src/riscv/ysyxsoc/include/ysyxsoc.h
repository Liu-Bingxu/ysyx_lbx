#ifndef __YSYXSOC_H__
#define __YSYXSOC_H__

#define SERIAL_BASE 0x10000000
#define SPI_BASE    0x10001000

#define SERIAL_ADDR         (SERIAL_BASE + 0x000)
#define SERIAL_DLL_ADDR     (SERIAL_BASE + 0x000)
#define SERIAL_DLM_ADDR     (SERIAL_BASE + 0x001)
#define SERIAL_LCR_ADDR     (SERIAL_BASE + 0x003)
#define SERIAL_LSR_ADDR     (SERIAL_BASE + 0x005)

#define SPI_RX0_ADDR     (SPI_BASE + 0x000)
#define SPI_RX1_ADDR     (SPI_BASE + 0x004)
#define SPI_RX2_ADDR     (SPI_BASE + 0x008)
#define SPI_RX3_ADDR     (SPI_BASE + 0x00C)
#define SPI_TX0_ADDR     (SPI_BASE + 0x000)
#define SPI_TX1_ADDR     (SPI_BASE + 0x004)
#define SPI_TX2_ADDR     (SPI_BASE + 0x008)
#define SPI_TX3_ADDR     (SPI_BASE + 0x00C)
#define SPI_CTRL_ADDR    (SPI_BASE + 0x010)
#define SPI_DIV_ADDR     (SPI_BASE + 0x014)
#define SPI_SS_ADDR      (SPI_BASE + 0x018)

// #define TIMER_ADDR          (SERIAL_BASE + 0x100)

#endif
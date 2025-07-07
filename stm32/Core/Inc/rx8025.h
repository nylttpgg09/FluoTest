#include "main.h"

#define u8 uint8_t
#define u16 uint16_t

typedef  struct time{
	u16 year;
	u8 month;
	u8 day;
	u8 week;
	u8 hour;
	u8 minute;
	u8 second;
}TIME;

u8 rx8025_init(void);
u8 get_rx8025_time(TIME *t);
u8 set_rx8025_time(u16 year,u8 month,u8 day,u8 week,u8 hour,u8 minute,u8 second);
u8 rx8025_write_data(u8 addr, u8 *buf, u8 len);
u8 rx8025_write_data1(u8 addr, u8 *buf, u8 len);
u8 rx8025_read_data(u8 addr,u8 *buf,u8 len);
u8 set_rx8025_time_t(TIME t);
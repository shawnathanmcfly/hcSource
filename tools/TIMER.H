#ifndef _TIMER_H
#define _TIMER_H

unsigned short timer = 0;

void set_timerISR( void );
void reset_timerISR( void );
void set_freq( unsigned char freq );

#endif

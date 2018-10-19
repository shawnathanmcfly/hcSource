#ifndef _TMR_H
#define _TMR_H

unsigned short timer = 0;

void reset_timerISR( void);
void set_timerISR( void );
void set_freq( unsigned char freq_div );

#endif


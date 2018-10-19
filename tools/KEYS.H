#ifndef _KEYS_H
#define _KEYS_H

unsigned char keybuff[128];
unsigned short old_off, old_seg;

void set_keyboard( void );
void reset_keyboard( void );

#endif

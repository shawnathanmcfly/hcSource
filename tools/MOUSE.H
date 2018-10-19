#ifndef _MOUSE_H
#define _MOUSE_H

struct MOUSE{
        
    unsigned char left_button, middle_button, right_button;
    unsigned short x, y;
    
}mouse;

unsigned short const MOUSE_ON = 1, MOUSE_OFF = 2;

void mouse_init( unsigned short stat ){
    
    _AX = stat;
    asm int 33H
}

void mouse_list( void ){
    
    unsigned char t;
    
    asm mov ax, 0003H
    asm int 33H
    
    asm mov t, bl
    
    mouse.y = _DX;
    asm shr cx, 1
    mouse.x = _CX;
    
    mouse.left_button = t & 1;
    mouse.middle_button = t & 4;
    mouse.right_button = t & 2;
}

#endif

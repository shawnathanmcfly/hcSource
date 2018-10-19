#ifndef _PONG_H
#define _PONG_H

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <alloc.h>
#include <graphics.h>
#include <string.h>
#include <ctype.h>
#include "C:\squeak\squeak.h"

#define BLOCK_WIDTH 20
#define BLOCK_HEIGHT 10
#define MAX_BLOCKS 48

/* block edge signals */
#define BLOCK_TOP 1
#define BLOCK_BOTTOM 2
#define BLOCK_LEFT 3
#define BLOCK_RIGHT 4

#define BALL_WIDTH 7
#define BALL_HEIGHT 10

/* ball directions */
#define UP 0
#define UP_LEFT 1
#define DOWN_LEFT 2
#define DOWN_RIGHT 3
#define UP_RIGHT 4

#define PADDLE_WIDTH 40
#define PADDLE_HEIGHT 6

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define SND 12;

/*keyboard buffer global for easy access in functions*/
unsigned char keybuff[128];
unsigned short timer = 0;
/*offset and segment for original dos keyboard driver*/
short old_seg, old_off;

unsigned char *gfx_data;

char load_pal( char pn[] ){
  FILE *fh;
  unsigned char *cd;
  short c;

  cd = (unsigned char *)malloc( sizeof(char) );
  
  /* load pal data */
  fh = fopen( pn, "rb" );
  if( !fh )
    return 0;

  /* wait for vertical retrace */
  while ( inportb( 0x3da ) & 8 )
    ;
  while ( !(inportb( 0x3da ) & 8) )
    ;
  
  /* change palette colors according to palette file data */
  for( c = 0; c <= 255; c++ ){
    /* set palette number*/
    outportb(0x3c8, c );
    /* read palette color from file */   
    fread( cd, 1, 1, fh );
    outportb(0x3c9, *cd );
    
    fread( cd, 1, 1, fh );
    outportb(0x3c9, *cd ); 
    
    fread( cd, 1, 1, fh );
    outportb(0x3c9, *cd );
  }

  fclose( fh );
  free( cd );

  return 1;
}

char load_gfx_near( char gn[], unsigned short *w, unsigned short *h ){

  FILE *fh;
  unsigned short da;

  fh = fopen( gn, "rb" );

  if( !fh )
    return 0;

  /* get width and height of gfx data */
  fread( w, sizeof(short), 1, fh );
  fread( h, sizeof(short), 1, fh );

  da = *w * *h;

  gfx_data = (unsigned char *)malloc( da * sizeof(char) );
  if( !gfx_data )
    return -1;

  fread( gfx_data, 1, da, fh );

  /* close gfx file */
  fclose( fh );

  return 1;
}

void destroy_gfx_near( void ){
  free( gfx_data );
}

void draw_ball( unsigned short x, unsigned short y,
		unsigned short lx, unsigned short ly ){

  unsigned short oy;
  asm push di si;

  _SI = FP_OFF( gfx_data + ( 40*10 ) );
  _DI = lx + ( ly * SCREEN_WIDTH );

  for( oy = 0; oy < BALL_HEIGHT; oy++,
	 _DI += (SCREEN_WIDTH - BALL_WIDTH) ){

    _CX = BALL_WIDTH;
    _AL = 0;
    asm rep stosb;
  }

  _DI = x + ( y *SCREEN_WIDTH );

  for( oy = 0; oy < BALL_HEIGHT; oy++,
	 _DI += (SCREEN_WIDTH - BALL_WIDTH ),
         _SI += (40 - BALL_WIDTH) ){
    
    _CX = BALL_WIDTH;
    asm rep movsb;
  }

  asm pop si di;
}

void ball_move( char dir, unsigned short *x, unsigned short *y ){
  switch( dir ){
  case UP: *y -= 1; break;
  case UP_LEFT: *y -= 1; *x -= 1; break;
  case DOWN_LEFT: *y += 1; *x -= 1; break;
  case DOWN_RIGHT: *y += 1; *x += 1; break;
  case UP_RIGHT: *y -= 1; *x += 1; break;
  }
}

void draw_paddle( unsigned short x, unsigned short y,
		  unsigned short lx, unsigned short ly ){
  unsigned short oy;
  asm push di si;

  _SI = FP_OFF( gfx_data + ( 40*20 ) );
  _DI = lx + ( ly * SCREEN_WIDTH );

  for( oy = 0; oy < PADDLE_HEIGHT; oy++,
	 _DI += (SCREEN_WIDTH - PADDLE_WIDTH) ){

    _CX = PADDLE_WIDTH;
    _AL = 0;
    asm rep stosb;
  }

  _DI = x + ( y *SCREEN_WIDTH );

  for( oy = 0; oy < PADDLE_HEIGHT; oy++,
	 _DI += (SCREEN_WIDTH - PADDLE_WIDTH ) ){
    
    _CX = PADDLE_WIDTH;
    asm rep movsb;
  }

  asm pop si di;
}

#endif

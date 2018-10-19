
#include "C:\pong\PONG.H"
#include "C:\pong\MAPS.H"

typedef struct {
  unsigned short x, y, x1, y1;
  unsigned char cc, dead;  
}blocks;

char snd_len = 0;

char load_gfx_near( char gn[], unsigned short *w, unsigned short *h );

void destroy_gfx_near( void );
char load_pal( char pn[] );
void draw_ball( unsigned short x, unsigned short y,
		unsigned short lx, unsigned short ly );
void ball_move( char dir, unsigned short *x, unsigned short *y );
void draw_paddle( unsigned short x, unsigned short y,
		  unsigned short lx, unsigned short ly );
void block_hit_check( blocks far *block, unsigned char nb,
		      unsigned short bx, unsigned short by,
		      unsigned short *score, char *dir ){
  unsigned short i; nb += 6;
  
  do{
    
    if( !block[nb].dead ){

      /*write_text( byte_to_string((unsigned short)nb), 2, 0, 0 );*/
      if( bx < block[nb].x1 &&
	  bx+6 > block[nb].x &&
 	  by < block[nb].y1 &&
	  by+9 > block[nb].y ){

	*score += 100;
	gotoxy(1, 1);
	printf("SCORE: %hu", *score);

	if( by+9 < block[nb].y1 && bx+6 > block[nb].x+1 && bx < block[nb].x1-1 ){
	  if( *dir == DOWN_LEFT ) *dir = UP_LEFT;
	  if( *dir == DOWN_RIGHT ) *dir = UP_RIGHT;

	  sound( 200 );
	  snd_len = SND;
	  
	}else
	if( by > block[nb].y && bx+6 > block[nb].x+1 && bx < block[nb].x1-1 ){
	  if( *dir == UP_LEFT ) *dir = DOWN_LEFT;
	  if( *dir == UP_RIGHT ) *dir = DOWN_RIGHT;

	  sound( 200 );
	  snd_len = SND;
	  
	}else
	  if( bx+6 < block[nb].x1 && by+9 > block[nb].y-1 && by < block[nb].y1+1 ){
	   if( *dir == DOWN_RIGHT ) *dir = DOWN_LEFT;
	   if( *dir == UP_RIGHT ) *dir = UP_LEFT;

	   sound( 200 );
	   snd_len = SND;
	   
	}else
	if( bx > block[nb].x && by+9 > block[nb].y-1 &&
        by < block[nb].y1+1 ){

	  if( *dir == DOWN_LEFT ) *dir = DOWN_RIGHT;
	  if( *dir == UP_LEFT ) *dir = UP_RIGHT;

	  sound( 200 );
	  snd_len = SND;
	}

	block[nb].dead = 1;
	asm push si di;

	_DI = block[nb].x + (block[nb].y * SCREEN_WIDTH);
	
	for( i = 0; i < BLOCK_HEIGHT; i++, _DI += (SCREEN_WIDTH-BLOCK_WIDTH ) ){
	  _CX = BLOCK_WIDTH;
	  _AL = 0;
	  _ES = 0xA000;

	  asm rep stosb;
	}
	asm pop di si;
      } 
    }
    
  }while( nb-- != 0 );
  _ES = 0xA000;
}

void draw_blocks( blocks far *block, unsigned char nb ){
  unsigned short y, y_adj = 0, x_adj = 0;
  unsigned char i;
  
  for( i = 0; i < nb; i++ ){
 
    if( isdigit( map1[i] ) ){
      block[i].dead = 0;
      block[i].cc = map1[i] - 48;
      block[i].y = 16+y_adj;

      block[i].x = 3 + (x_adj * (BLOCK_WIDTH));
      block[i].x1 = block[i].x + BLOCK_WIDTH;
      block[i].y1 = block[i].y + BLOCK_HEIGHT;

      _DI = (block[i].y*SCREEN_WIDTH) + block[i].x;
	/* far data above needed ES seg, so replaced it with
	   video memory seg again. */
      _ES = 0xA000;
	
      _SI = FP_OFF( gfx_data );
      asm push si di
      for( y = 0; y < BLOCK_HEIGHT; y++, _SI += 20,
	     _DI += (SCREEN_WIDTH - BLOCK_WIDTH) ){
	char x;

	for( x = BLOCK_WIDTH; x > 0; x--, _SI++ ){
	  _AL = block[i].cc * 16;
	  asm push ax;
	  _ES = 0xA000;
	  asm pop ax;
	  asm add al, ds:[si];
	  asm stosb;
	}
      }
      asm pop di si
      x_adj++;
    }else if( map1[i] == '\n' ){
      y_adj += BLOCK_HEIGHT+1;
      x_adj = 0;
      nb++;
    }
  } 
}

int main()
{
  unsigned short gw = 0, gh, i, bx = SCREEN_WIDTH / 2 - 9,
    by = SCREEN_HEIGHT-17, blx = bx, bly = by,
    px = SCREEN_WIDTH / 2 - (PADDLE_WIDTH / 2),
    py = 200 - PADDLE_HEIGHT, plx = px, ply = py, score = 0,
    expl[ 7 ] = { 200, 200, 400, 200, 400, 400, 400 };

  unsigned char nb, *p = 0;
  
  char gn[] = {"pong.gfx"}, pn[] = {"paldata.pal"},
    err, quit = 1, cur_lvl, dir = UP_LEFT,
    ball_spd_set = 5, ball_spd = 0,
    paddle_spd_set = 3, paddle_spd = 0,
    died = 24;

  blocks far *block;

  err = load_gfx_near( gn, &gw, &gh );
  if( !err ){
   printf("Error opening graphics file\n");
    return 0;
  }else if( err == -1 ){
    printf("Error allocating data for graphic file\n");

    return 0;
  }
  
  set_keyboard();
  set_vg();

  err = load_pal( pn );
  if( !err ){
    reset_keyboard();
    set_txt();
    printf("Error opening palette file\n");
    return 0;
  }  
  
  set_freq( 5 );
  set_timerISR();

  /****************************/
  /* M A I N  G A M E L O O P */
  /****************************/
  do{
    /* setup new level */
    /* count number of blocks in map */
    
    for( i = 0, nb = 0; map1[i] != '\0'; i++  )
      if( isdigit(map1[i]) ) nb++;
    
    block = (blocks far*)farmalloc( sizeof(blocks) * nb );
    
    draw_blocks( block, nb );
    draw_paddle( px, py, plx, ply );
            
    while( !keybuff[ 0x01 ] && by+9 != 199 ){
      if( !timer){

	if( !ball_spd ){
	  blx = bx; bly = by;

	  block_hit_check( block, nb, bx, by, &score, &dir );
	  ball_move( dir, &bx, &by );
	  draw_ball( bx, by, blx, bly );

	  ball_spd = ball_spd_set;
	}

	if( paddle_spd < 0 )
	  paddle_spd = 0;

	if( keybuff[0x4d] && px + PADDLE_WIDTH < SCREEN_WIDTH && !paddle_spd ){
	  plx = px; ply = py;
	  draw_paddle( ++px, py, plx, ply );
	  paddle_spd = paddle_spd_set;
	}

	if( keybuff[0x4b] && px > 0 && !paddle_spd){
	  plx = px; ply = py;
	  draw_paddle( --px, py, plx, ply );
	  paddle_spd = paddle_spd_set;
	}
      
	if( by+9 == py-1  && bx+9 > px && bx < px+PADDLE_WIDTH ){
	  switch( dir ){
	  case DOWN_LEFT: dir = UP_LEFT; break;
	  case DOWN_RIGHT: dir = UP_RIGHT; break;
	  }
	  sound( 200 );
	  snd_len = SND;
	  ball_spd--;
	}else if( by == 11 ){
	  switch( dir ){
	  case UP_LEFT: dir = DOWN_LEFT; break;
	  case UP_RIGHT: dir = DOWN_RIGHT; break;
	  }
	  sound( 200 );
	  snd_len = SND;
	  ball_spd--;
	}else if( bx == 0 ){
	  switch( dir ){
	  case DOWN_LEFT: dir = DOWN_RIGHT; break;
	  case UP_LEFT: dir = UP_RIGHT; break;
	  }
	  sound( 200 );
	  snd_len = SND;
	  ball_spd--;
	}else if( bx+6 == 319 ){
	  switch( dir ){
	  case UP_RIGHT: dir = UP_LEFT; break;
	  case DOWN_RIGHT: dir = DOWN_LEFT; break;
	  }
	  sound( 200 );
	  snd_len = SND;
	  ball_spd--;
	}
	
	if( by+9 == 199 )
	  died = 1;

	if( !ball_spd )
	  ball_spd = 1;

	if( snd_len )
	  snd_len--;
	else
	  nosound();
	
	timer = 1;
	ball_spd--;
	paddle_spd--;
		      
      }
    }

    farfree( block );

  }while( !quit );
  
  reset_timerISR();
  set_freq( 0 );
  set_txt();
  reset_keyboard();
  destroy_gfx_near();
  
  if( died == 1 )
    printf("QUIT CRYING, BABY! WHAAAAAA!!!!\n");
  
  /*
  printf( "DEBUG INFO\n");
  printf( "------------------------\n");
  printf( "Block 1 y position: %hu\n\n", block[0].y );
  */
  
  return 0;
}

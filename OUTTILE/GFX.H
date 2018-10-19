#ifndef _GFX_H
#define _GFX_H

struct OTILES{
    
    unsigned char *data;
    unsigned short width;
    unsigned short height;
    unsigned short tile_w;
    unsigned short tile_h;
    unsigned short x, maxx, y, maxy;
    unsigned short view_x, view_y;
    unsigned short sel_x, sel_y;
    
}otiles;
    
struct TTILES{
    unsigned char *data;
    unsigned short width;
    unsigned short height;
    unsigned short tile_w;
    unsigned short tile_h;
    unsigned short x, maxx, y, maxy;
    unsigned short view_x, view_y;
    unsigned short sel_x, sel_y;
}ttiles;

unsigned short const STAT_SCREEN_X = 81, STAT_SCREEN_Y = 1;

unsigned short const UP = 0x01;
unsigned short const DOWN = 0x02;
unsigned short const RIGHT = 0x04;

/* ASM functions */
void draw_string( unsigned short x, unsigned short y, char *string, unsigned char color );
void draw_rect_fill( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned char color );
void draw_arrow( unsigned short x, unsigned short y, unsigned short arrow, unsigned char color ); 
void blackout_bg( void );

/*
 * 
 * draw_rect FUNCTION
 * 
 * experimental inline asm shit. Read good.
 * 
 * 
 */
int draw_rect( unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned char c ){

    asm push si di
    if( w < 2 || h < 2 )
        return -1;
    
    asm mov ax, 0A000H
    asm mov es, ax
    asm mov cx, [w]
    asm mov al, [c]
    asm mov bx, [y]
    asm mov di, bx
    asm shl di, 8
    asm shl bx, 6
    asm add di, bx
    asm add di, [x]
    asm mov dx, 320
    asm sub dx, cx
    asm mov bx, cx
    asm rep stosb
    asm sub di, bx
    asm mov cx, [h]
    asm dec bx
sidelines:
    asm mov es:[di], al
    asm mov es:[di+bx], al
    asm add di, 320
    asm loop sidelines
    asm inc bx
    asm mov cx, bx
    asm sub di, 320
    asm rep stosb
    
    asm pop di si
    return 0;
}

int load_pal( signed char fn[] ){
    
    FILE *fp;
    unsigned short i;
    unsigned char color;

    if( !(fp = fopen( fn, "rb" )) )
        return 0;
    
    outp( 0x03C8, 0 );
    
    for( i = 0; i < 256 * 3; i++ ){
        fread( &color, 1, 1, fp );
        outp(0x03C9, color );
    }

    fclose( fp );
    
    return 1;
}

void draw_toolbar( void ){
    draw_rect_fill( 0, 0, 80, 200, 15 );
    draw_rect_fill( 80, 180, 320 - 80, 200 - otiles.tile_w, 15 );
    draw_string( 4, 4, "Press O\nto load\nOpaque\nTiles", 1 );
    
    draw_string( 4, 104, "Press T\nto load\nTrans\nTiles", 1 );
}

int load_ttiles( char *filename ){

    FILE *fp = fopen( filename, "rb");
    unsigned short i;
    
    if( !fp )
        return -2;
    
    fread( &ttiles.width, 2, 1, fp );
    fread( &ttiles.height, 2, 1, fp );
    
    if( ttiles.data )
        free( ttiles.data );
    
    ttiles.data = ( unsigned char *)malloc( (ttiles.width * ttiles.height) * sizeof(char) );
    
    if( !ttiles.data )
        return -1;
        
    fread( ttiles.data, ttiles.width * ttiles.height, 1, fp );
    
    fclose( fp );
    
    ttiles.x = 0;
    ttiles.y = 0;
    ttiles.tile_w = 20;     /* default tile dimentions */
    ttiles.tile_h = 20;
    ttiles.view_x = 0;
    ttiles.view_y = 0;
    ttiles.sel_x = 0;
    ttiles.sel_y = 0;
    
    for( i = 0; i < ttiles.height; i += ttiles.tile_h )
        ttiles.maxy++;
    
    for( i = 0; i < ttiles.width; i += ttiles.tile_w )
        ttiles.maxx++;
    
    return 0;
}

void draw_otiles( void ){

    unsigned short y;
    _ES = 0xA000;
    _DI = 0;
    _SI = FP_OFF( otiles.data );
    _SI += otiles.view_y * (otiles.tile_h * otiles.width);
    _SI += otiles.view_x * otiles.tile_w;
    
    if( otiles.height < 100 )
        y = otiles.height;
    else
        y = 100;
    
    for( ; y; _DI += 320 - 80, y-- ){
        
        if( otiles.width < 80 )
            _CX = otiles.width;
        else
            _CX = 80;
        
        asm rep movsb;
        
        if( otiles.width < 80 )
            _CX = 80 - otiles.width;
        else if( otiles.width > 80 )
            _SI += otiles.width - 80;
        
        _AL = 15;
        asm rep stosb;
        
    }
    
    if( otiles.height < 100 )
        y = otiles.height;
    else
        y = 100;
    
    for( ; y < 100; y++, _DI += 320 - 80 ){
        _AL = 15;
        _CX = 80;
        asm rep stosb;
    }
    
    draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
}

/*
 *  load_otiles
 *  open GFX file to load in this static dynamically allocated shit
 *  
 *  returns -1 if out of memory
 *  returns -2 if file does not exist
 * 
 *  20 x 20 is the default tile dimentions. This must be changed
 *  in the program if needed.
 *
 *  TODO: make opaque tile specifically named ex: OTILEXXX.TLE
 *  
 */
int load_otiles( char *filename ){
    
    FILE *fp = fopen( filename, "rb" );
    unsigned short i;
    
    if( !fp )
        return -2;
    
    fread( &otiles.width, 2, 1, fp );
    fread( &otiles.height, 2, 1, fp );
    
    if( otiles.data )
        free( otiles.data );
    
    otiles.data = ( unsigned char *)malloc( (otiles.width * otiles.height) * sizeof(char) );
    
    if( !otiles.data )
        return -1;
        
    fread( otiles.data, otiles.width * otiles.height, 1, fp );
    
    fclose( fp );
    
    otiles.x = 0;
    otiles.y = 0;
    otiles.tile_w = 20;     /* default tile dimentions */
    otiles.tile_h = 20;
    otiles.view_x = 0;
    otiles.view_y = 0;
    otiles.sel_x = 0;
    otiles.sel_y = 0;
    
    for( i = 0; i < otiles.height; i += otiles.tile_h )
        otiles.maxy++;
    
    for( i = 0; i < otiles.width; i += otiles.tile_w )
        otiles.maxx++;
    
    return 0;
    
}

void draw_ttiles( void ){

    unsigned short y;
    _ES = 0xA000;
    _DI = 320 * 100;
    _SI = FP_OFF( ttiles.data );
    _SI += ttiles.view_y * (ttiles.tile_h * ttiles.width);
    _SI += ttiles.view_x * ttiles.tile_w;
    
    if( ttiles.height < 100 )
        y = ttiles.height;
    else
        y = 100;
    
    for( ; y; _DI += 320 - 80, y-- ){
        
        if( ttiles.width < 80 )
            _CX = ttiles.width;
        else
            _CX = 80;
        
        asm rep movsb;
        
        if( ttiles.width < 80 )
            _CX = 80 - ttiles.width;
        else if( ttiles.width > 80 )
            _SI += ttiles.width - 80;
        
        _AL = 15;
        asm rep stosb;
    }
    
    if( ttiles.height < 100 )
        y = ttiles.height;
    else
        y = 100;
    
    for( ; y < 100; y++, _DI += 320 - 80 ){
        _AL = 15;
        _CX = 80;
        asm rep stosb;
    }
    
    draw_rect( ttiles.sel_x * ttiles.tile_w, 100 + (ttiles.sel_y * ttiles.tile_h), ttiles.tile_w, ttiles.tile_h, 44 );
}

void set_ttile_mouse( void ){
    
    signed short x, y;
    
    x = mouse.x;
    y = mouse.y - 100;
    
    ttiles.sel_x = 0;
    ttiles.sel_y = 0;
    
    while( x >= ttiles.tile_w ){
        x -= ttiles.tile_w;
        ttiles.sel_x++;
    }
    
    ttiles.x = ttiles.view_x + ttiles.sel_x;
    
    while( y >= ttiles.tile_h ){
        y -= ttiles.tile_h;
        ttiles.sel_y++;
    }
    
    ttiles.y = ttiles.view_y + ttiles.sel_y;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    draw_ttiles();
    draw_rect( ttiles.sel_x * ttiles.tile_w, 100 + (ttiles.sel_y * ttiles.tile_h), ttiles.tile_w, ttiles.tile_h, 44 );
    
    asm mov ax, MOUSE_ON
    asm int 33H

}

void set_otile_mouse( void ){
    
    signed short x, y;
    
    x = mouse.x;
    y = mouse.y;
    
    otiles.sel_x = 0;
    otiles.sel_y = 0;
    
    while( x >= otiles.tile_w ){
        x -= otiles.tile_w;
        otiles.sel_x++;
    }
    
    otiles.x = otiles.view_x + otiles.sel_x;
    
    while( y >= otiles.tile_h ){
        y -= otiles.tile_h;
        otiles.sel_y++;
    }
    
    otiles.y = otiles.view_y + otiles.sel_y;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    draw_otiles();
    draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
    
    asm mov ax, MOUSE_ON
    asm int 33H

}

#endif

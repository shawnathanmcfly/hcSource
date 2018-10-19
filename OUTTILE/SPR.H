#ifndef _SPR_H
#define _SPR_H

typedef struct{
    
    unsigned char **data;
    unsigned char num_frames, animation_speed;
    
}Spr;

/*********************************************************************************/
/**                                                                             **/
/**                                 ASM FUNCTIONS                               **/
/**                                                                             **/
/*********************************************************************************/



/*********************************************************************************/
/**                                                                             **/
/**                                  C FUNCTIONS                                **/
/**                                                                             **/
/*********************************************************************************/

unsigned short const PLAYER_WIDTH = 20, PLAYER_HEIGHT = 20;

Spr *spr_badguy = 0;
Spr *spr_money = 0;

Spr * load_spr( char *filename, unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned char num_frames ){
    
    Spr *sprite;
    FILE *fp;
    long last_pos;
    unsigned short log_w, log_h, i, cur_frame;
    unsigned char *scanned_data;
    
    fp = fopen( filename, "rb" );
    if( !fp )
        return 0;
    
    sprite = ( Spr *)malloc( sizeof( Spr ) );
    if( !sprite ){
        fclose( fp );
        return 0;
    }
    
    sprite->data = ( unsigned char **)malloc( num_frames );
    if( sprite->data == 0 ){
        fclose( fp );
        free( sprite );
        return 0;
    }    
    
    scanned_data = ( unsigned char *)malloc( w );
    if( !scanned_data ){
        fclose( fp );
        free( sprite->data );
        free( sprite );
        return 0;
    }
    
    sprite->num_frames = num_frames;
    sprite->animation_speed = 2;
    
    /* get width and height of GFX file */
    fread( &log_w, 2, 1, fp );
    fread( &log_h, 2, 1, fp );
    
    /* go to spot in file to extract sprite data */
    fseek( fp, y * log_w + x, SEEK_CUR );
    
    for( cur_frame = 0; cur_frame < num_frames; cur_frame++ ){
    
        last_pos = ftell( fp );
        
        sprite->data[ cur_frame ] = ( unsigned char * )malloc( w * h );
        
        _DI = FP_OFF( sprite->data[ cur_frame ] );
        _ES = FP_SEG( sprite->data[ cur_frame ] );
        for( i = 0; i < h; i++ ){
     
            fread( scanned_data, 1, w, fp );
            _SI = FP_OFF( scanned_data );
            _CX = w;
            asm rep movsb
            fseek( fp, log_w - w, SEEK_CUR );
            
        }
        
        fseek( fp, last_pos + w, SEEK_SET );

    }
    
    free( scanned_data );
    fclose( fp );
    
    return sprite;
}

void delete_sprite( Spr *sprite ){

    unsigned char cur_frame;
    
    if( sprite ){
        for( cur_frame = 0; cur_frame < sprite->num_frames; cur_frame++ )
            free( sprite->data[ cur_frame ] );
    
        
        free( sprite->data );
        free( sprite );
    }
}

void sprite_to_buffer( unsigned char *sprite, signed short x, signed short y, unsigned short w, unsigned short h ){

    unsigned short screen_offs, offs_adjust = 0, offs_adjust2 = 0;
    
    if( y < 0 ){
       
        offs_adjust = abs(y) * w;
        h += y;
        y = 0;
        
    }else if( y + h > SCREEN_BUFFER_HEIGHT )
        h -= (y + h) - SCREEN_BUFFER_HEIGHT;
    
    if( x < 0 ){
     
        offs_adjust2 = abs(x);
        offs_adjust += offs_adjust2;
        w += x;
        x = 0;
        
    }
    
    screen_offs = y * SCREEN_BUFFER_WIDTH + x;
    
    asm mov bx, w
    asm mov dx, SCREEN_BUFFER_WIDTH
    asm sub dx, bx
    asm mov cx, h
    asm les di, screen_buffer
    asm add di, screen_offs
    asm mov si, OFFSET sprite
    asm add si, offs_adjust
loopy:
    asm push cx
    asm mov cx, bx
loopx:
    asm mov al, ds:[si]
    asm cmp al, 255
    asm je skip
    asm mov es:[di], al
skip:
    asm inc si
    asm inc di
    asm loop loopx
    asm pop cx
    asm add si, offs_adjust2
    asm add di, dx
    asm loop loopy
    
}

#endif


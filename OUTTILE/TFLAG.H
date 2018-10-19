#ifndef _TFLAG_H
#define _TFLAG_H

/*
 * 
 * Shawns crazy cool Tile FLAG system!
 * 
 * Precedence is what tiles are drawn in a certain order
 * 
 * 0x00 is drawn last 0x0F is drawn first
 * 
 * 
 */

struct Coord {
    
    unsigned short x, y, tle_file, dest_x, dest_y;
    struct Coord *next;
    
};

struct Coord *coord_head = 0;
struct Coord **cur_coord = &coord_head;

/* binary ops for setting specific flags */
const unsigned char SOLID_OFF = 0x00, SOLID_ON = 0xF0;

const unsigned char PREC_FORE = 0x0F, PREC_BACK = 0x00;

/* tflag buffer */
unsigned char far *tflag;

unsigned char solid_color = 40, prec_color = 25, coord_color = 90;

/* tflag mode set. decides which mode you're in. Whether in toggle solid mode or
   precedence mode */
const unsigned short SOLID_MODE = 0x00, PREC_MODE = 0x01;

unsigned short tflag_mode = 0; /* set to solid toggle mode default */

/* number of room transition pointed in room */
unsigned short num_coord = 0;

void print_binary( unsigned char binary_num ){

    unsigned char output, count = 8;
    
    while( count ){
            
        output = binary_num & 0x80;
        
        if( output == 0x80 )
            printf( "%d", 1 );
        else
            printf( "%d", 0 );
        
        binary_num <<= 1;
        
        count--;
    }
    
}

unsigned short tflag_init( void ){

    unsigned short counter;
    
    tflag = (unsigned char far *)farmalloc( (DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT) * sizeof(unsigned char far) );
    
    if( !tflag )
        return -1;
    
    for( counter = 0; counter < ( DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT ); counter++ )
        *(tflag+counter) = 0;
        
    return 0;
}

char tflag_uninit( void ){
    
    cur_coord = &coord_head;
    while( *cur_coord ){
        
        free( *cur_coord );
        
        *cur_coord = (*cur_coord)->next;
        
    }
    
    if( tflag ){
        farfree( tflag );
        return 0;
    }else
        return -1;
    
}

void save_tflag_buff( char *file_name ){

    FILE *fh = fopen( file_name, "a+b" );
    unsigned short counter = 0;
    unsigned char temp;
    
    if( tflag ){
    
        fwrite( "FFF", 1, 3, fh ); 
    
        for( counter = 0; counter < tle.w * tle.h; counter++ ){
            temp = *( tflag + counter );
            fwrite( &temp, 1, 1, fh );
        }
    }
    
    cur_coord = &coord_head;
    if( *cur_coord ){
        
        fwrite( "CCC", 1, 3, fh );
        fwrite( &num_coord, 2, 1, fh );
        
        while( *cur_coord ){
            
            fwrite( &(*cur_coord)->tle_file, 2, 1, fh );
            fwrite( &(*cur_coord)->x, 2, 1, fh );
            fwrite( &(*cur_coord)->y, 2, 1, fh );
            fwrite( &(*cur_coord)->dest_x, 2, 1, fh );
            fwrite( &(*cur_coord)->dest_y, 2, 1, fh );
            cur_coord = &(*cur_coord)->next;
        }
        
    }
    
    fclose( fh );
    
}

void set_solid( unsigned char far *sel_tflag ){
    
    unsigned short offs, x, y, fnum = 0;
    unsigned char tnum = 0;
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.x = 0;
    tle.y = 0;
    
    while( x >= otiles.tile_w ){
        x -= otiles.tile_w;
        tle.x++;
    }
    
    tle.x += tle.view_x;
    
    while( y >= otiles.tile_h ){
        y -= otiles.tile_h;
        tle.y++;
    }
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;
    
    *(sel_tflag + offs) ^= 0xF0;
        
    mouse_init( MOUSE_OFF );
    if( *(sel_tflag + offs) == 0xFF || *(sel_tflag + offs) == 0xF0 ){
            
        draw_rect( mouse.x - mouse.x % otiles.tile_w, mouse.y - mouse.y % otiles.tile_h , 
                    otiles.tile_w, otiles.tile_h, solid_color );
            
    }else if( *(sel_tflag + offs) == 0x0F || *(sel_tflag + offs) == 0x00 ){
            
        if( *(tle.tdata + offs ) ){
            
            tnum = *(tle.tdata + offs );
            fnum = *(tle.fnum + offs );
                
            /* find matching tile in used tile section in memory */
            cur_tle = &head;
            while( *cur_tle ){
                if( (*cur_tle)->tnum == tnum && (*cur_tle)->fnum == fnum )
                    break;
            
                cur_tle = &(*cur_tle)->next;
            }
                
            otile_draw_dir( FP_SEG( (*cur_tle)->tdata ), FP_OFF( (*cur_tle)->tdata )  );
            
        }else
            draw_rect_fill( mouse.x - mouse.x % otiles.tile_w, mouse.y - mouse.y % otiles.tile_h, 
                    otiles.tile_w, otiles.tile_h, 2 );
            
        if( *(tle.trans_tdata + offs ) ){
            tnum = *(tle.trans_tdata + offs );
            fnum = *(tle.trans_fnum + offs );
                
            /* find matching tile in used tile section in memory */
            cur_trans_tle = &trans_head;
            while( *cur_trans_tle ){
                if( (*cur_trans_tle)->tnum == tnum && (*cur_trans_tle)->fnum == fnum )
                    break;
                
                cur_trans_tle = &(*cur_trans_tle)->next;
            }
            ttile_draw_dir( FP_SEG( (*cur_trans_tle)->tdata ), FP_OFF( (*cur_trans_tle)->tdata ) );
        }
    }
    
    if( *(sel_tflag + offs) == 0x0F || *(sel_tflag + offs) == 0xFF )
                draw_rect( (mouse.x - mouse.x % otiles.tile_w) + 1, (mouse.y - mouse.y % otiles.tile_h) + 1, 
                       otiles.tile_w - 2, otiles.tile_h - 2, prec_color );
        
    mouse_init( MOUSE_ON );
}

void set_prec( unsigned char far *sel_tflag ){

    unsigned short offs, x, y, fnum = 0;
    unsigned char tnum = 0;
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.x = 0;
    tle.y = 0;
    
    while( x >= otiles.tile_w ){
        x -= otiles.tile_w;
        tle.x++;
    }
    
    tle.x += tle.view_x;
    
    while( y >= otiles.tile_h ){
        y -= otiles.tile_h;
        tle.y++;
    }
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;

    if( *(tle.trans_tdata + offs) ){
        *(sel_tflag + offs) ^= 0x0F;
        
        mouse_init( MOUSE_OFF );
        if( *(sel_tflag + offs) == 0x0F || *(sel_tflag + offs) == 0xFF ){
            
            draw_rect( (mouse.x - mouse.x % otiles.tile_w) + 1, (mouse.y - mouse.y % otiles.tile_h) + 1, 
                       otiles.tile_w - 2, otiles.tile_h - 2, prec_color );
            
        }else if( *(sel_tflag + offs) == 0xF0 || *(sel_tflag + offs) == 0x00 ){
            
            if( *(tle.tdata + offs ) ){
                
                tnum = *(tle.tdata + offs );
                fnum = *(tle.fnum + offs );
                
                /* find matching tile in used tile section in memory */
                cur_tle = &head;
                while( *cur_tle ){
                    if( (*cur_tle)->tnum == tnum && (*cur_tle)->fnum == fnum )
                        break;
                
                    cur_tle = &(*cur_tle)->next;
                }
                
                otile_draw_dir( FP_SEG( (*cur_tle)->tdata ), FP_OFF( (*cur_tle)->tdata )  );
                
            }else
                draw_rect_fill( mouse.x - mouse.x % otiles.tile_w, mouse.y - mouse.y % otiles.tile_h, 
                                otiles.tile_w, otiles.tile_h, 2 );
            
            tnum = *(tle.trans_tdata + offs );
            fnum = *(tle.trans_fnum + offs );
                
            /* find matching tile in used tile section in memory */
            cur_trans_tle = &trans_head;
            while( *cur_trans_tle ){
                if( (*cur_trans_tle)->tnum == tnum && (*cur_trans_tle)->fnum == fnum )
                    break;
                
                cur_trans_tle = &(*cur_trans_tle)->next;
            }
            
            ttile_draw_dir( FP_SEG( (*cur_trans_tle)->tdata ), FP_OFF( (*cur_trans_tle)->tdata ) );
        }
        
        if( *(sel_tflag + offs) == 0xF0 || *(sel_tflag + offs) == 0xFF )
                draw_rect( (mouse.x - mouse.x % otiles.tile_w), (mouse.y - mouse.y % otiles.tile_h), 
                       otiles.tile_w, otiles.tile_h, solid_color );
        
    }    
    
    mouse_init( MOUSE_ON );
}

void draw_tflag_map( void ){

    unsigned short x, y, offs;
    
    mouse_init( MOUSE_OFF );
    
    for( y = 0; y < tle.maxy; y++ ){
        for( x = 0; x < tle.maxx; x++){
            
            offs = tle.view_x+x + ( (tle.view_y+y) * tle.w );
            
            if( *(tflag + offs ) == 0xF0 || *(tflag + offs ) == 0xFF )
                draw_rect( x*otiles.tile_w+80, y*otiles.tile_h, otiles.tile_w, otiles.tile_h, solid_color );
                
            if( *(tflag + offs ) == 0x0F || *(tflag + offs ) == 0xFF )
                draw_rect( x*otiles.tile_w+81, y*otiles.tile_h+1, otiles.tile_w-2, otiles.tile_h-2, prec_color );
        }
    }
    
    cur_coord = &coord_head;
    while( *cur_coord ){
    
        if( (*cur_coord)->x >= tle.view_x && (*cur_coord)->x < tle.view_x + tle.maxx &&
            (*cur_coord)->y >= tle.view_y && (*cur_coord)->y < tle.view_y + tle.maxy 
        ){
            draw_rect_fill( ((*cur_coord)->x - tle.view_x) * otiles.tile_w + 83, ((*cur_coord)->y - tle.view_y) * otiles.tile_h + 3, 
                otiles.tile_h - 6, otiles.tile_h - 6, 15 );
        
            draw_string( ((*cur_coord)->x - tle.view_x) * otiles.tile_w + 80 + 6, 
                         ((*cur_coord)->y - tle.view_y) * otiles.tile_h + 6, "C", 0 );
        
        }
        cur_coord = &(*cur_coord)->next;
    }
    
    mouse_init( MOUSE_ON );
    
}

void resize_tflag_map( unsigned short new_w, unsigned short new_h ){

    struct Coord * t_coord;
    unsigned short buff_counter, dest_adj_x, dest_adj_y, src_adj_x, xbuff;
    unsigned char far *new_tflag_buff;
    unsigned char temp;
    
    if( new_w > tle.old_w ){
        dest_adj_x = new_w - tle.old_w;
        src_adj_x = 0;
        xbuff = tle.old_w;
    }else if( new_w < tle.old_w ){
        src_adj_x = tle.old_w - new_w;
        dest_adj_x = 0;
        xbuff = new_w;
    }else if( new_w == tle.old_w){
        src_adj_x = 0;
        dest_adj_x = 0;
        xbuff = new_w;
    }
    
    if( new_h > tle.old_h )
        dest_adj_y = tle.old_h;
    else if( new_h < tle.old_h )
        dest_adj_y = new_h;
    else if( new_h == tle.old_h )
        dest_adj_y = new_h;
    
    /* allocate memory for otile buffer */
    new_tflag_buff = (unsigned char far *)farmalloc( (new_w * new_h) * sizeof(unsigned char) );
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ )
        *(new_tflag_buff+buff_counter) = 0;
    
    _SI = FP_OFF( tflag );
    _ES = FP_SEG( new_tflag_buff );
    _DI = FP_OFF( new_tflag_buff );
    for( buff_counter = 0; buff_counter < dest_adj_y; buff_counter++, _SI += src_adj_x, _DI += dest_adj_x ){
        _CX = xbuff;
        asm push ds
        _DS = FP_SEG( tflag );
        asm rep movsb
        asm pop ds
    }
    
    farfree( tflag );
    tflag = ( unsigned char far *)farmalloc( (new_w * new_h) * sizeof( unsigned char ) );

    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ ){
        temp = *(new_tflag_buff+buff_counter);
        *(tflag+buff_counter) = temp;
    }
    
    /* check if any coord change tiles are out of bounds and delete them */
    cur_coord = &coord_head;
    while( *cur_coord ){
    
        if( (*cur_coord)->x > new_w || (*cur_coord)->y > new_h ){
            t_coord = *cur_coord;
            *cur_coord = (*cur_coord)->next;
            free( t_coord );
            num_coord--;
        }else
            cur_coord = &(*cur_coord)->next;
    }
}

void load_tflag_buff( char *file_name ){
    
    struct Coord * t_coord;
    FILE *fh = fopen( file_name, "rb" );
    unsigned short counter = 0, found = 0;
    char *marker;
    unsigned char temp;
    
    marker = ( char *)malloc( 4 * sizeof( char ) );
    
    do{
        fread( &marker[0], 1, 1, fh );
        
        if( marker[0] == 'F' ){
        
            fread( &marker[1], 1, 1, fh );
            fread( &marker[2], 1, 1, fh );
            
            marker[3] = '\0';
            
            if( marker[0] == 'F' && marker[1] == 'F' && marker[2] == 'F' ){
               found = 1;
               break;
            }

        }
        
    }while( !feof( fh ) );
    
    if( tflag )
        farfree( tflag );
    
    tflag = ( unsigned char far *)farmalloc( (tle.w * tle.h) * sizeof( unsigned char ) );
    
    if( found == 1 ){
        for( counter = 0; counter < tle.w * tle.h; counter++ ){
    
            fread( &temp, 1, 1, fh );
            *( tflag + counter ) = temp;
        }
    }
    
    found = 0;
    
    /* check for coord change effects */ 
    do{
        fread( &marker[0], 1, 1, fh );
        
        if( marker[0] == 'C' ){
        
            fread( &marker[1], 1, 1, fh );
            fread( &marker[2], 1, 1, fh );
            
            marker[3] = '\0';
            
            if( marker[0] == 'C' && marker[1] == 'C' && marker[2] == 'C' ){
               found = 1;
               break;
            }

        }
        
    }while( !feof( fh ) );
    
    cur_coord = &coord_head;
    while( *cur_coord ){
        
        t_coord = *cur_coord;
        
        *cur_coord = (*cur_coord)->next;
        
        free( t_coord );
        
    }
    
    
    if( found == 1 ){
    
        fread( &num_coord, 2, 1, fh );
        
        cur_coord = &coord_head;
        found = num_coord;
        while( found ){
            
            *cur_coord = ( struct Coord * )malloc( sizeof( struct Coord ) );
            (*cur_coord)->next = 0;
            found--;
            
            fread( &(*cur_coord)->tle_file, 2, 1, fh );
            fread( &(*cur_coord)->x, 2, 1, fh );
            fread( &(*cur_coord)->y, 2, 1, fh );
            fread( &(*cur_coord)->dest_x, 2, 1, fh );
            fread( &(*cur_coord)->dest_y, 2, 1, fh );
            cur_coord = &(*cur_coord)->next;
        }
        
        
    }
    
    free( marker );
    fclose( fh );
    
}

void del_room_change( void ){
 
    struct Coord * temp;
    unsigned short x, y;
        
    x = ((mouse.x - 80) / otiles.tile_w) + tle.view_x;
    y = (mouse.y / otiles.tile_h) + tle.view_y;
    
    cur_coord = &coord_head;
    while( *cur_coord ){
        
        if( x == (*cur_coord)->x && (*cur_coord)->y == y ){
        
            num_coord--;
            temp = *cur_coord;
            
            *cur_coord = (*cur_coord)->next;
            
            free( temp );
            
        }
        else
            cur_coord = &(*cur_coord)->next;
    }
}

void set_room_change( void ){
    
    struct ffblk sel_file;
    unsigned short x, y, tx, ty;
    char temp_fn[13], *dest_file = ( char *)malloc( 13 );
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.x = 0;
    tle.y = 0;
    
    while( x >= otiles.tile_w ){
        x -= otiles.tile_w;
        tle.x++;
    }
    
    tle.x += tle.view_x;
    
    while( y >= otiles.tile_h ){
        y -= otiles.tile_h;
        tle.y++;
    }
    
    tle.y += tle.view_y;
    
    strcpy( temp_fn, outfile.lastfile );
    
    reset_keyboard();
    strcpy( dest_file, list_files( "*.tle", "Pick a .TLE file..." ) );
    
    if( dest_file[0] == '\0' ){
        free( dest_file );
        getch();
        set_keyboard();
        return;
    }
    
    mouse_init( MOUSE_OFF );
    
    blackout_bg();
    
    cur_coord = &coord_head;
    while( *cur_coord )
        cur_coord = &(*cur_coord)->next;
    
    tx = tle.view_x;
    ty = tle.view_y;
    tle.view_x = 0;
    tle.view_y = 0;
    
    *cur_coord = ( struct Coord *)malloc( sizeof( struct Coord ) );
    (*cur_coord)->next = 0;
    
    (*cur_coord)->x = tle.x;
    (*cur_coord)->y = tle.y;
    
    (*cur_coord)->tle_file = 1;
    findfirst( "*.TLE", &sel_file, 0 );
    while( strcmp( dest_file, sel_file.ff_name ) ){
        findnext( &sel_file );
        (*cur_coord)->tle_file++;
    }
    
    load_tle_file( dest_file );
    
    set_keyboard();
    draw_tle_map();
    
    mouse_init( MOUSE_ON );
    
    do{
        mouse_list();
        
        if( keybuff[0x48] && tle.view_y ){
            tle.view_y--;
            draw_tle_map();
        }
                
        if( keybuff[0x50] && tle.view_y + 9 < tle.h ){
            tle.view_y++;
            draw_tle_map();
        }
                    
        if( keybuff[0x4B] && tle.view_x ){
            tle.view_x--;
            draw_tle_map();
        }
        
        if( keybuff[0x4D] && tle.view_x + 12 < tle.w ){
            tle.view_x++;
            draw_tle_map();
        }
        
        timer = 20;
        while( timer )
            ;
        
    }while( !mouse.left_button && mouse.x >= 80 && mouse.y < 180 );

    (*cur_coord)->dest_x = ((mouse.x - 80) / otiles.tile_w) + tle.view_x;
    (*cur_coord)->dest_y = (mouse.y / otiles.tile_h) + tle.view_y;
    num_coord++;
    
    tle.view_x = tx;
    tle.view_y = ty;
    
    load_tle_file( temp_fn );
    free( dest_file );
    strcpy( outfile.lastfile, temp_fn );
    
    mouse_init( MOUSE_ON );
}
#endif

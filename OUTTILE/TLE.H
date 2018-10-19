#ifndef _TLE_H
#define _TLE_H

/***************************************************************************
 * 
 *              TLE file format ( so far )
 * 
 *      T L E w  h  tile_num file_num
 *      _ _ _ __ __ _        __
 * 
 * 
 * *************************************************************************/

unsigned short const DEFAULT_TLE_WIDTH = 12,
                     DEFAULT_TLE_HEIGHT = 9,
                     
                     OPAQUE = 0x0001, TRANS = 0x0002;
                    
struct USED_TLE {
    
    unsigned char far *tdata, tnum;
    unsigned short fnum;
    
    struct USED_TLE *next;
    
};

struct USED_TRANS_TLE {
    
    unsigned char far *tdata, tnum;
    unsigned short fnum;
    
    struct USED_TRANS_TLE *next;
    
};

struct USED_TILE_WIN{
    unsigned short x;
    unsigned short view_x;
    unsigned short used_tile_amount, used_trans_tile_amount;
    unsigned short used_tle_mode;
}used_tile_win;

struct TLE{
    
    unsigned char far *tdata;
    unsigned char far *trans_tdata;
    unsigned short far *fnum;
    unsigned short far *trans_fnum;
    unsigned short x, y, trans_x, trans_y;
    unsigned short view_x, view_y;
    unsigned short maxx, maxy;
    unsigned short w, h;
    unsigned short old_w, old_h;
    
}tle;

/* used tle list initially starts with zero */
struct USED_TLE *head;
struct USED_TLE **cur_tle;
struct USED_TLE **sel_tile;

/* used tle list initially starts with zero */
struct USED_TRANS_TLE *trans_head;
struct USED_TRANS_TLE **cur_trans_tle;
struct USED_TRANS_TLE **trans_sel_tile;

void init_used_tle( void ){

    used_tile_win.used_tle_mode = OPAQUE;
    
    head = 0;
    cur_tle = &head;
    sel_tile = 0;
    used_tile_win.used_tile_amount = 0;
    
    trans_head = 0;
    cur_trans_tle = &trans_head;
    trans_sel_tile = 0;
    used_tile_win.used_trans_tile_amount = 0;
}

int insert_used_tle_data( unsigned char tn, unsigned short fn ){
    
    unsigned short y;
    
    cur_tle = &head;
    
    /*transverse list until end of it */
    while( *cur_tle ){
        
        if( (*cur_tle)->tnum == tn && (*cur_tle)->fnum == fn )
            return -3;
        
        cur_tle = &(*cur_tle)->next;
    }
    
    /* allocate listing at the end of the list */
    *cur_tle = ( struct USED_TLE *)malloc( sizeof( struct USED_TLE ) );
    if( *cur_tle == 0 )
        return -1;
    /* make room for used tile entry */
    (*cur_tle)->tdata = ( unsigned char far *)farmalloc( (otiles.tile_w * otiles.tile_h) * sizeof(char) );
    if( (*cur_tle)->tdata == 0 )
        return -2;
    
    _ES = FP_SEG( (*cur_tle)->tdata );
    _DI = FP_OFF( (*cur_tle)->tdata );
    
    _SI = FP_OFF( otiles.data );
    _SI += otiles.y * otiles.tile_h * otiles.width;
    _SI += otiles.x * otiles.tile_w;
    
    for( y = 0; y < otiles.tile_h; y++, _SI += otiles.width - otiles.tile_w ){
        _CX = otiles.tile_w;
        asm rep movsb;
    }
    
    (*cur_tle)->tnum = tn;
    (*cur_tle)->fnum = fn;
    
    used_tile_win.used_tile_amount++;
    
    /* mark end of list */
    (*cur_tle)->next = 0;
    
    return 0;
}

int insert_used_trans_tle_data( unsigned char tn, unsigned short fn ){
    
    unsigned short y;
    
    cur_trans_tle = &trans_head;
    
    /*transverse list until end of it */
    while( *cur_trans_tle ){
        
        if( (*cur_trans_tle)->tnum == tn && (*cur_trans_tle)->fnum == fn )
            return -3;
        
        cur_trans_tle = &(*cur_trans_tle)->next;
    }
    
    /* allocate listing at the end of the list */
    *cur_trans_tle = ( struct USED_TRANS_TLE *)malloc( sizeof( struct USED_TLE ) );
    if( *cur_trans_tle == 0 )
        return -1;
    /* make room for used tile entry */
    (*cur_trans_tle)->tdata = ( unsigned char far *)farmalloc( (ttiles.tile_w * ttiles.tile_h) * sizeof(char) );
    if( (*cur_trans_tle)->tdata == 0 )
        return -2;
    
    _ES = FP_SEG( (*cur_trans_tle)->tdata );
    _DI = FP_OFF( (*cur_trans_tle)->tdata );
    
    _SI = FP_OFF( ttiles.data );
    _SI += ttiles.y * ttiles.tile_h * ttiles.width;
    _SI += ttiles.x * ttiles.tile_w;
    
    for( y = 0; y < ttiles.tile_h; y++, _SI += ttiles.width - ttiles.tile_w ){
        _CX = ttiles.tile_w;
        asm rep movsb;
    }
    
    (*cur_trans_tle)->tnum = tn;
    (*cur_trans_tle)->fnum = fn;
    
    used_tile_win.used_trans_tile_amount++;
    
    /* mark end of list */
    (*cur_trans_tle)->next = 0;
    
    return 0;
}

void draw_used_tle_data( void ){

    unsigned short i;
    cur_tle = &head;
    
    _ES = 0xA000;
    _DI = 320 * 180;
    _DI += 80;
    
    for( i = used_tile_win.view_x; i ; i-- )
        cur_tle = &(*cur_tle)->next;
    
    i = 80 + (used_tile_win.view_x);
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    while( *cur_tle && i < 320 ){

        _AX = FP_SEG( (*cur_tle)->tdata );
        _SI = FP_OFF( (*cur_tle)->tdata );
        asm mov dx, otiles.tile_w;
        asm mov bx, otiles.tile_h;
        asm mov cx, bx
        asm push ds
        asm mov ds, ax
loopy:
        asm push cx
        asm mov cx, bx
        asm rep movsb
        asm pop cx
        asm add di, 320
        asm sub di, dx
        asm loop loopy

        asm pop ds
        
        _DI -= (320 * otiles.tile_h) - otiles.tile_w; 
        cur_tle = &(*cur_tle)->next;
        
        i += otiles.tile_w;
    }
    
    cur_trans_tle = &trans_head;
    
    while( *cur_trans_tle && i < 320 ){

        _AX = FP_SEG( (*cur_trans_tle)->tdata );
        _SI = FP_OFF( (*cur_trans_tle)->tdata );
        asm mov dx, otiles.tile_w;
        asm mov bx, otiles.tile_h;
        asm mov cx, bx
        asm push ds
        asm mov ds, ax
loopy3:
        asm push cx
        asm mov cx, bx
        asm rep movsb
        asm pop cx
        asm add di, 320
        asm sub di, dx
        asm loop loopy3

        asm pop ds
        
        _DI -= (320 * otiles.tile_h) - otiles.tile_w; 
        cur_trans_tle = &(*cur_trans_tle)->next;
        
        i += otiles.tile_w;
    }
    
    if( (used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount - used_tile_win.view_x) *
        otiles.tile_w < 240 &&  used_tile_win.view_x )
            used_tile_win.view_x--;
    
    if( (used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount ) * otiles.tile_w < 240 ){
    
        _CX = otiles.tile_h;
loopy2:
        asm push cx
        _CX = 240 - ((used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount ) * otiles.tile_w);
        asm mov al, 15
        asm rep stosb
        asm add di, 80
        _DI += (used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount) * otiles.tile_w;
        asm pop cx
        asm loop loopy2
    }
    
    asm mov ax, MOUSE_ON
    asm int 33H
    
}

int set_used_tile_mouse( void ){
    
    unsigned short i = 0;
    int sel_x, x;
    
    x = mouse.x - 80;
    
    sel_x = mouse.x - mouse.x % otiles.tile_w;
    
    do{
        x -= otiles.tile_w;
        i++;
    }while( x >= 0 );
    
    i += used_tile_win.view_x;
    
    sel_tile = &head;
    while( i && *sel_tile ){
        
        used_tile_win.used_tle_mode = OPAQUE;
        
        if( --i == 0 )
            break;
        
        sel_tile = &(*sel_tile)->next;
    }
    
    trans_sel_tile = &trans_head;
    while( i && *trans_sel_tile ){
        
        used_tile_win.used_tle_mode = TRANS;
        
        if( --i == 0 )
            break;
        
        trans_sel_tile = &(*trans_sel_tile)->next;
    }
    
    mouse_init( MOUSE_OFF );
    
    draw_used_tle_data();
    draw_rect( sel_x, 180, otiles.tile_w, otiles.tile_h, 45 );
    
    mouse_init( MOUSE_ON );
    
    return 0;
}

void draw_used_tle_mouse( void ){
    
    unsigned short tseg, toff, mx, my;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    _ES = 0xA000;
    
    mx = mouse.x - mouse.x % otiles.tile_w;
    my = mouse.y - mouse.y % otiles.tile_h;
    
    if( mx >= (tle.w - tle.view_x + 4) * otiles.tile_w || my >= (tle.h - tle.view_y) * otiles.tile_h ){
        
        asm mov ax, MOUSE_ON
        asm int 33H
        
        return;
    }
    
    _DI = my * 320;
    _DI += mx;
    
    if( used_tile_win.used_tle_mode == OPAQUE ){
        tseg = FP_SEG( (*sel_tile)->tdata );
        toff = FP_OFF( (*sel_tile)->tdata );
    }else if( used_tile_win.used_tle_mode == 0x0002 ){
        tseg = FP_SEG( (*trans_sel_tile)->tdata );
        toff = FP_OFF( (*trans_sel_tile)->tdata );
    }
    
    _AX = tseg;
    _SI = toff;

    asm mov dx, otiles.tile_w
    asm mov bx, otiles.tile_h
    asm mov cx, bx
    asm push ds
    asm mov ds, ax
loopy:
    asm push cx
    asm mov cx, bx
    asm push ax
loopx1:
    asm mov al, [si]
    asm cmp al, 255
    asm je skip1
    asm mov es:[di], al
skip1:
    asm inc di
    asm inc si
    asm loop loopx1
    asm pop ax
    asm pop cx
    asm add di, 320
    asm sub di, dx
    asm loop loopy
    
    asm pop ds
    
    asm mov ax, MOUSE_ON
    asm int 33H
    
}

void insert_trans_used_to_tle( void ){
    
    signed short x, y;
    unsigned short offs, tf, i;
    unsigned char tt, fnd;
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.x = 0;
    tle.y = 0;
    
    while( x >= ttiles.tile_w ){
        x -= ttiles.tile_w;
        tle.x++;
    }
    
    tle.x += tle.view_x;
    
    while( y >= ttiles.tile_h ){
        y -= ttiles.tile_h;
        tle.y++;
    }
    
    if( tle.x >= tle.w || tle.y >= tle.h )
        return;
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;
    
    outfile.cur_tile_num = (*trans_sel_tile)->tnum;
    
    tt = *(tle.trans_tdata+offs);
    tf = *(tle.trans_fnum+offs);
    
    /*** REMINDER: increased by one so 0 can produce a black tile in loading tle rooms <-\ */
    *(tle.trans_tdata+offs) = (*trans_sel_tile)->tnum; /* <------------------/ */
    *(tle.trans_fnum+offs) = (*trans_sel_tile)->fnum;
    
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.trans_tdata+i) == tt && *(tle.trans_fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        
        cur_trans_tle = &trans_head;
        while( *cur_trans_tle ){
            if( (*cur_trans_tle)->fnum == tf && (*cur_trans_tle)->tnum == tt ){
                farfree( (*cur_trans_tle)->tdata );
                free( *cur_trans_tle );
                *cur_trans_tle = (*cur_trans_tle)->next;
                used_tile_win.used_trans_tile_amount--;
            }else
                cur_trans_tle = &(*cur_trans_tle)->next;
        }
    }
}

void insert_used_to_tle( void ){
    
    signed short x, y;
    unsigned short offs, tf, i;
    unsigned char tt, fnd;
    
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
    
    if( tle.x >= tle.w || tle.y >= tle.h  )
        return;
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;
    
    outfile.cur_ttile_num = (*sel_tile)->tnum;
    
    tt = *(tle.tdata+offs);
    tf = *(tle.fnum+offs);
    
    /*** REMINDER: increased by one so 0 can produce a black tile in loading tle rooms <-\ */
    *(tle.tdata+offs) = (*sel_tile)->tnum; /* <------------------/ */
    *(tle.fnum+offs) = (*sel_tile)->fnum;
     
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.tdata+i) == tt && *(tle.fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        
        cur_tle = &head;
        while( *cur_tle ){
            if( (*cur_tle)->fnum == tf && (*cur_tle)->tnum == tt ){
                farfree( (*cur_tle)->tdata );
                free( *cur_tle );
                *cur_tle = (*cur_tle)->next;
                used_tile_win.used_tile_amount--;
            }else
                cur_tle = &(*cur_tle)->next;
        }
    }
}

void unalloc_used_tle( void ){

    cur_tle = &head;
    
    while( *cur_tle ){
        
        farfree( (*cur_tle)->tdata );
        free( *cur_tle );
        *cur_tle = (*cur_tle)->next;
        
        used_tile_win.used_tile_amount = 0;
    }
}

void unalloc_used_trans_tle( void ){

    cur_tle = &head;
    
    while( *cur_tle ){
        
        farfree( (*cur_trans_tle)->tdata );
        free( *cur_trans_tle );
        *cur_trans_tle = (*cur_trans_tle)->next;
        
        used_tile_win.used_trans_tile_amount = 0;
    }
    
}

void debug_used_tle( void ){
        
    cur_trans_tle = &trans_head;
    
    if( *cur_trans_tle == 0 )
        printf("No fuckin used tle listings\n");
    
    while( *cur_trans_tle ){
            
        printf( "TILE NUMBER: %02x FILE NUMBER: %hu \n", (*cur_trans_tle)->tnum, (*cur_trans_tle)->fnum );
        cur_trans_tle = &(*cur_trans_tle)->next;
    }
    
    printf("\n");
}

/*
 * 
 *  loads the tle struct with default sizes and shit
 * 
 *  -1 if any of the data couldnt be allocated
 */
int load_def_tle( void ){
    
    unsigned short i;
    
    tle.tdata = ( unsigned char far *)farmalloc( (DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT) * sizeof( unsigned char ) );
    tle.fnum = ( unsigned short far *)farmalloc( (DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT) * sizeof( unsigned short ) );
    
    tle.trans_tdata = ( unsigned char far *)farmalloc( (DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT) * sizeof( unsigned char ) );
    tle.trans_fnum = ( unsigned short far *)farmalloc( (DEFAULT_TLE_WIDTH * DEFAULT_TLE_HEIGHT) * sizeof( unsigned short ) );
    if( !tle.tdata || !tle.tdata || !tle.trans_tdata || !tle.trans_fnum )
        return -1;
    
    tle.view_x = 0;
    tle.view_y = 0;
    tle.maxx = 12;
    tle.maxy = 9;
    tle.w = DEFAULT_TLE_WIDTH;
    tle.h = DEFAULT_TLE_HEIGHT;
    otiles.tile_w = 20;
    otiles.tile_h = 20;
    ttiles.tile_w = 20;
    ttiles.tile_h = 20;
    
    outfile.cur_tle_file[0] = '\0';
    strcpy( outfile.lastfile, "temp" );
    
    for( i = 0; i < DEFAULT_TLE_HEIGHT * DEFAULT_TLE_WIDTH; i++ )
        *(tle.tdata+i) = 0;
    
    for( i = 0; i < DEFAULT_TLE_HEIGHT * DEFAULT_TLE_WIDTH; i++ )
        *(tle.fnum+i) = 0;
    
    for( i = 0; i < DEFAULT_TLE_HEIGHT * DEFAULT_TLE_WIDTH; i++ )
        *(tle.trans_tdata+i) = 0;
    
    for( i = 0; i < DEFAULT_TLE_HEIGHT * DEFAULT_TLE_WIDTH; i++ )
        *(tle.trans_fnum+i) = 0;
   
    return 0;
}

void unalloc_tle_data( void ){
    
    farfree( tle.tdata );
    farfree( tle.fnum );
    farfree( tle.trans_tdata );
    farfree( tle.trans_fnum );
}

unsigned short load_tle_file( char *filename ){
    
    struct ffblk sel_file;
    FILE *fp = fopen( filename, "rb" ), *fp2 = 0;
    unsigned short i = 0, t2 = 0, log_w, log_h, tf = 0, fnd = 0, tacross;
    unsigned char t = 0, tt = 0, name[3], *read_bytes;
    
    if( !fp )
        return -1;
    
    fread( &name[0], 3, 1, fp );
    
    if( name[0] != 'T' ){
        fclose( fp ); 
        return -1;
    }
    
    fread( &tle.w, 2, 1, fp );
    fread( &tle.h, 2, 1, fp );
    fread( &otiles.tile_w, 2, 1, fp );
    fread( &otiles.tile_h, 2, 1, fp );
    ttiles.tile_w = otiles.tile_w;
    ttiles.tile_h = otiles.tile_h;
 
    unalloc_used_tle();
    unalloc_used_trans_tle();
 
    fread( name, 3, 1, fp );
    
    if( name[0] != 'T' ){
        fclose( fp );
        return -1;
    }
    
    if( tle.tdata )
        farfree( tle.tdata );
    
    if( tle.fnum )
        farfree( tle.fnum );
    
    tle.tdata = (unsigned char far *)farmalloc( (tle.w * tle.h) * sizeof(char) );
    tle.fnum = (unsigned short far *)farmalloc( (tle.w * tle.h) * sizeof(unsigned short) );
    
    read_bytes = (unsigned char *)malloc( otiles.tile_w * sizeof( unsigned char ) );
    
    /* load solid tile data (opaque) */
    for( i = 0; i < tle.w * tle.h; i++, fnd = 0 ){
        fread( &t, 1, 1, fp );
        fread( &tf, 2, 1, fp );
        
        *(tle.fnum+i) = tf;
        *(tle.tdata+i) = t;
        
        /* find out if its in used tile selection */
        cur_tle = &head;
        while( *cur_tle && t > 0){
            if( (*cur_tle)->fnum == tf && (*cur_tle)->tnum == t )
                fnd = 1;
            
            cur_tle = &(*cur_tle)->next;
        }

        /* insert it into used tile section */
        if( !fnd && t > 0 ){
            
            findfirst( "*.GFX", &sel_file, 0 );
            
            for( tt = 1; tt < tf; tt++ )
                findnext( &sel_file );

            fp2 = fopen( sel_file.ff_name, "rb" );
            
            if( !fp2 ){
                fclose( fp );
                return -3;
            }
            
            *cur_tle = ( struct USED_TLE *)malloc( sizeof( struct USED_TLE ) );
            (*cur_tle)->fnum = tf;
            (*cur_tle)->tnum = t;
            (*cur_tle)->tdata = ( unsigned char far *)farmalloc( (otiles.tile_w * otiles.tile_h) * sizeof( unsigned char ) );
            
            (*cur_tle)->next = 0;

            fread( &log_w, 2, 1, fp2 );
            fread( &log_h, 2, 1, fp2 );

            t--;
            
            tacross = log_w / otiles.tile_w;
            
            for( ;t >= tacross; t -= tacross )
                fseek( fp2, otiles.tile_h * log_w, SEEK_CUR );
            
            fseek( fp2, t * otiles.tile_w, SEEK_CUR );
            
            _ES = FP_SEG( (*cur_tle)->tdata );
            _DI = FP_OFF( (*cur_tle)->tdata );
            
            for( t2 = otiles.tile_h; t2; t2-- ){
                fread( read_bytes, 1, otiles.tile_w, fp2 );
                fseek( fp2, log_w - otiles.tile_w, SEEK_CUR );
                _SI = FP_OFF( read_bytes );
                _CX = otiles.tile_w;
                asm rep movsb;
            }
            
            used_tile_win.used_tile_amount++;
            
            fclose( fp2 );
        }
    }

    i = 0;
    
    fread( &name[0], 3, 1, fp );

    if( name[0] != 'O' ){
        fclose( fp );
        free( read_bytes );
        return -2;
    }
    
    if( tle.trans_tdata )
        farfree( tle.trans_tdata );
    
    if( tle.trans_fnum )
        farfree( tle.trans_fnum );
    
    tle.trans_tdata = (unsigned char far *)farmalloc( (tle.w * tle.h) * sizeof(char) );
    tle.trans_fnum = (unsigned short far *)farmalloc( (tle.w * tle.h) * sizeof(unsigned short) );
    
    /* load solid tile data (opaque) */
    for( i = 0; i < tle.w * tle.h; i++, fnd = 0 ){
        fread( &t, 1, 1, fp );
        fread( &tf, 2, 1, fp );
        
        *(tle.trans_fnum+i) = tf;
        *(tle.trans_tdata+i) = t;
        
        /* find out if its in used tile selection */
        cur_trans_tle = &trans_head;
        while( *cur_trans_tle && t > 0){
            if( (*cur_trans_tle)->fnum == tf && (*cur_trans_tle)->tnum == t )
                fnd = 1;
            
            cur_trans_tle = &(*cur_trans_tle)->next;
        }

        /* insert it into used tile section */
        if( !fnd && t > 0 ){
            
            findfirst( "*.GFX", &sel_file, 0 );
            
            for( tt = 1; tt < tf; tt++ )
                findnext( &sel_file );

            fp2 = fopen( sel_file.ff_name, "rb" );
            
            if( !fp2 ){
                fclose( fp );
                return -3;
            }
            
            *cur_trans_tle = ( struct USED_TRANS_TLE *)malloc( sizeof( struct USED_TRANS_TLE ) );
            (*cur_trans_tle)->fnum = tf;
            (*cur_trans_tle)->tnum = t;
            (*cur_trans_tle)->tdata = ( unsigned char far *)farmalloc( (ttiles.tile_w * ttiles.tile_h) * sizeof( unsigned char ) );
            
            (*cur_trans_tle)->next = 0;

            fread( &log_w, 2, 1, fp2 );
            fread( &log_h, 2, 1, fp2 );

            t--;
            
            tacross = log_w / ttiles.tile_w;
            
            for( ;t >= tacross; t -= tacross )
                fseek( fp2, ttiles.tile_h * log_w, SEEK_CUR );
            
            fseek( fp2, t * ttiles.tile_w, SEEK_CUR );
            
            _ES = FP_SEG( (*cur_trans_tle)->tdata );
            _DI = FP_OFF( (*cur_trans_tle)->tdata );
            
            for( t2 = ttiles.tile_h; t2; t2-- ){
                fread( read_bytes, 1, ttiles.tile_w, fp2 );
                fseek( fp2, log_w - ttiles.tile_w, SEEK_CUR );
                _SI = FP_OFF( read_bytes );
                _CX = ttiles.tile_w;
                asm rep movsb;
            }
            
            used_tile_win.used_trans_tile_amount++;
            
            fclose( fp2 );
            
        }
    }
    
    free( read_bytes );
    fclose( fp );
        
    return 0;
}

void insert_tle_data( void ){
    
    signed short x, y;
    unsigned short offs;
    unsigned short tf, i;
    unsigned char tt, fnd;
    
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
    
    if( tle.x >= tle.w || tle.y >= tle.h ){
        return;
    }
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;
    
    outfile.cur_tile_num = ( otiles.y * otiles.maxx ) + otiles.x + 1;
    
     tt = *(tle.tdata+offs);
     tf = *(tle.fnum+offs);
     
     /*** REMINDER: increased by one so 0 can produce a black tile in loading tle rooms <-\ */
    *(tle.tdata+offs) = outfile.cur_tile_num; /* <----------------------------------------/ */
    *(tle.fnum+offs) = outfile.cur_otile_fid + 1;
     
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.tdata+i) == tt && *(tle.fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        
        cur_tle = &head;
        while( *cur_tle ){
            if( (*cur_tle)->fnum == tf && (*cur_tle)->tnum == tt ){
                farfree( (*cur_tle)->tdata );
                free( *cur_tle );
                *cur_tle = (*cur_tle)->next;
                used_tile_win.used_tile_amount--;
            }else
                cur_tle = &(*cur_tle)->next;
        }
    }
}

void del_tle_data( void ){
    
    signed short x, y;
    unsigned short offs;
    unsigned short tf, i;
    unsigned char tt, fnd;
    
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
    
    if( tle.x >= tle.w - tle.view_x || tle.y >= tle.h - tle.view_y )
        return;
    
    tle.y += tle.view_y;
    
    offs = (tle.y * tle.w) + tle.x;
    
    outfile.cur_tile_num = 0;
    
     tt = *(tle.tdata+offs);
     tf = *(tle.fnum+offs);
     
     /*** REMINDER: increased by one so 0 can produce a black tile in loading tle rooms <-\ */
    *(tle.tdata+offs) = 0; /* <----------------------------------------/ */
    *(tle.fnum+offs) = 0;
     
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.tdata+i) == tt && *(tle.fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        cur_tle = &head;
        while( *cur_tle ){
            if( (*cur_tle)->fnum == tf && (*cur_tle)->tnum == tt ){
                farfree( (*cur_tle)->tdata );
                free( *cur_tle );
                *cur_tle = (*cur_tle)->next;
                used_tile_win.used_tile_amount--;
            }else
                cur_tle = &(*cur_tle)->next;
        }
    }
}

void del_trans_tle_data( void ){
    signed short x, y;
    unsigned short offs;
    unsigned short tf, i;
    unsigned char tt, fnd;
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.trans_x = 0;
    tle.trans_y = 0;
    
    while( x >= ttiles.tile_w ){
        x -= ttiles.tile_w;
        tle.trans_x++;
    }
    
    tle.trans_x += tle.view_x;
    
    while( y >= ttiles.tile_h ){
        y -= ttiles.tile_h;
        tle.trans_y++;
    }
    
    if( tle.trans_x >= tle.w - tle.view_x || tle.trans_y >= tle.h - tle.view_y )
        return;
    
    tle.trans_y += tle.view_y;
    
    offs = (tle.trans_y * tle.w) + tle.trans_x;
    
    outfile.cur_tile_num = 0;
    
    tt = *(tle.trans_tdata+offs);
    tf = *(tle.trans_fnum+offs);
     
     /*** REMINDER: increased by one so 0 can produce a black tile in loading trans_tle rooms <-\ */
    *(tle.trans_tdata+offs) = 0; /* <----------------------------------------/ */
    *(tle.trans_fnum+offs) = 0;
     
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.trans_tdata+i) == tt && *(tle.trans_fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        cur_trans_tle = &trans_head;
        while( *cur_trans_tle ){
            if( (*cur_trans_tle)->fnum == tf && (*cur_trans_tle)->tnum == tt ){
                farfree( (*cur_trans_tle)->tdata );
                free( *cur_trans_tle );
                *cur_trans_tle = (*cur_trans_tle)->next;
                used_tile_win.used_trans_tile_amount--;
            }else
                cur_trans_tle = &(*cur_trans_tle)->next;
        }
    }
}

void insert_trans_tle_data( void ){
    
    signed short x, y;
    unsigned short offs;
    unsigned short tf, i;
    unsigned char tt, fnd;
    
    x = mouse.x - 80;
    y = mouse.y;
    
    tle.trans_x = 0;
    tle.trans_y = 0;
    
    while( x >= ttiles.tile_w ){
        x -= ttiles.tile_w;
        tle.trans_x++;
    }
    
    tle.trans_x += tle.view_x;
    
    while( y >= ttiles.tile_h ){
        y -= ttiles.tile_h;
        tle.trans_y++;
    }
    
    if( tle.trans_x >= tle.w || tle.trans_y >= tle.h ){
        return;
    }
    
    tle.trans_y += tle.view_y;
    
    offs = (tle.trans_y * tle.w) + tle.trans_x;
    
    outfile.cur_tile_num = ( ttiles.y * ttiles.maxx ) + ttiles.x + 1;
    
    tt = *(tle.trans_tdata+offs);
    tf = *(tle.trans_fnum+offs);
     
     /*** REMINDER: increased by one so 0 can produce a black tile in loading trans_tle rooms <-\ */
    *(tle.trans_tdata+offs) = outfile.cur_tile_num; /* <----------------------------------------/ */
    *(tle.trans_fnum+offs) = outfile.cur_ttile_fid + 1;
     
    /* scan to see if old tile is needed in the used tile section */
    for( i = 0, fnd = 0; i < tle.w * tle.h; i++ )
        if( *(tle.trans_tdata+i) == tt && *(tle.trans_fnum+i) == tf ){
            fnd = 1;
            break;
        }
    
    /* if *cur_tle is not zero, then tile still exists. do not remove from used tiles */
    if( !fnd ){
        cur_trans_tle = &trans_head;
        while( *cur_trans_tle ){
            if( (*cur_trans_tle)->fnum == tf && (*cur_trans_tle)->tnum == tt ){
                farfree( (*cur_trans_tle)->tdata );
                free( *cur_trans_tle );
                *cur_trans_tle = (*cur_trans_tle)->next;
                used_tile_win.used_trans_tile_amount--;
            }else
                cur_trans_tle = &(*cur_trans_tle)->next;
        }
    }
}

int save_tle_file( void ){
 
    FILE *fp = fopen( outfile.lastfile, "wb" );
    unsigned short i = 0, t2 = 0;
    unsigned char t = 0;
    if( !fp )
        return -1;
    
    fwrite( "TLE", 1, 3, fp );
    fwrite( &tle.w, 2, 1, fp );
    fwrite( &tle.h, 2, 1, fp );
    fwrite( &otiles.tile_w, 2, 1, fp );
    fwrite( &otiles.tile_h, 2, 1, fp );
    
    if( head ){
    
        fwrite( "TTT", 1, 3, fp );
        
        /* save solid tile data (opaque) */
        while( i < tle.w * tle.h && head ){
            t = *(tle.tdata+i);
            fwrite( &t, 1, 1 , fp );
            t2 = *(tle.fnum+i);
            fwrite( &t2, 2, 1, fp );
            i++;
        }
    }

    i = 0;
    
    if( trans_head ){
        
        fwrite( "OOO", 1, 3, fp ); 
    
        /* save transparent tile data */
        while( i < tle.w * tle.h && trans_head ){
            t = *(tle.trans_tdata+i);
            fwrite( &t, 1, 1 , fp );
            t2 = *(tle.trans_fnum+i);
            fwrite( &t2, 2, 1, fp );
            i++;
        }
    }
    
    fclose( fp );
    
    return 0;
}

int draw_tle_map( void ){

    unsigned short x, y, fnum, offs, tseg;
    unsigned char tnum;
    
    if( !head )
        return -1;
    
    blackout_bg();
    
    asm push ds
    asm pop ax
    tseg = _AX;
    
    for( y = 0; y < tle.maxy; y++ ){
        
        for( x = 0; x < tle.maxx; x++ ){
            
            offs = ((tle.view_y + y) * tle.w)  + (tle.view_x + x);
            tnum = *( tle.tdata + offs );
            fnum = *( tle.fnum + offs );
            
            if( !tnum )
                continue;
            
            /* find matching tile in used tile section in memory */
            cur_tle = &head;
            while( *cur_tle ){
                if( (*cur_tle)->tnum == tnum && (*cur_tle)->fnum == fnum )
                    break;
                
                cur_tle = &(*cur_tle)->next;
            }
            
            if( *cur_tle == 0 )
                return -1;
            
            _SI = FP_OFF( (*cur_tle)->tdata );
            _DI = y * otiles.tile_h;
            _DI *= 320;
            _DI += x * otiles.tile_w;
            _DI += 80;
            asm mov ax, otiles.tile_w
            asm push ax
            asm mov ax, otiles.tile_h
            asm push ax
            _DS = FP_SEG( (*cur_tle)->tdata );
            asm pop dx                          /* dx is height */
            asm pop bx                          /* bx is tile width */
            asm mov ax, 0A000H
            asm mov es, ax
            asm mov ax, 320
            asm sub ax, bx
            asm mov cx, dx
loopy:
            asm push cx
            asm mov cx, bx
            asm rep movsb
            asm add di, ax
            asm pop cx
            asm loop loopy
            
            _DS = tseg;
        }
    }
    
    return 0;
    
}

int draw_trans_tle_map( void ){

    unsigned short x, y, fnum, offs, tseg;
    unsigned char tnum;
    
    if( !trans_head )
        return -1;
    
    asm push ds
    asm pop ax
    tseg = _AX;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    for( y = 0; y < tle.maxy; y++ ){
        
        for( x = 0; x < tle.maxx; x++ ){
            
            offs = ((tle.view_y + y) * tle.w)  + (tle.view_x + x);
            tnum = *( tle.trans_tdata + offs );
            fnum = *( tle.trans_fnum + offs );
            
            if( !tnum )
                continue;
            
            /* find matching tile in used tile section in memory */
            cur_trans_tle = &trans_head;
            while( *cur_trans_tle ){
                if( (*cur_trans_tle)->tnum == tnum && (*cur_trans_tle)->fnum == fnum )
                    break;
                
                cur_trans_tle = &(*cur_trans_tle)->next;
            }
            
            if( *cur_trans_tle == 0 )
                return -1;
            
            _SI = FP_OFF( (*cur_trans_tle)->tdata );
            _DI = y * ttiles.tile_h;
            _DI *= 320;
            _DI += x * ttiles.tile_w;
            _DI += 80;
            asm mov ax, ttiles.tile_w
            asm push ax
            asm mov ax, ttiles.tile_h
            asm push ax
            _DS = FP_SEG( (*cur_trans_tle)->tdata );
            asm pop dx                          /* dx is height */
            asm pop bx                          /* bx is tile width */
            asm mov ax, 0A000H
            asm mov es, ax
            asm mov ax, 320
            asm sub ax, bx
            asm mov cx, dx
loopy:
            asm push cx
            asm mov cx, bx
loopx:
            asm cmp byte ptr [si], 255
            asm je  skip
            asm mov dl, ds:[si]
            asm mov es:[di], dl
skip:
            asm inc di
            asm inc si
            asm loop loopx
            asm add di, ax
            asm pop cx
            asm loop loopy
            
            _DS = tseg;
        }
    }
    
    asm mov ax, MOUSE_ON
    asm int 33H

    return 0;
    
}

void draw_otile( void ){
     
    unsigned short y, mx, my;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    my = mouse.y - mouse.y % otiles.tile_h;
    mx = mouse.x - mouse.x % otiles.tile_w;
    
    if( mx >= (tle.w - tle.view_x + 4) * otiles.tile_w || my >= (tle.h - tle.view_y) * otiles.tile_h ){
        
        asm mov ax, MOUSE_ON
        asm int 33H
        
        return;
    }
    
    _ES = 0xA000;
    _DI = my * 320;
    _DI += mx;

    _SI = FP_OFF( otiles.data );
    _SI += otiles.y * otiles.tile_h * otiles.width;
    _SI += otiles.x * otiles.tile_w;

    for( y = 0; y < otiles.tile_h; y++, _SI += otiles.width - otiles.tile_w,
        _DI += 320 - otiles.tile_w ){
        _CX = otiles.tile_w;
        asm rep movsb;
    }

    asm mov ax, MOUSE_ON
    asm int 33H
}

int draw_ttile( void ){
    unsigned short x, y, y1, tle_x = 0, tle_y = 0, mx, my;
    
    x = mouse.x - 80;
    y1 = mouse.y;
    
    while( x >= ttiles.tile_w ){
        x -= ttiles.tile_w;
        tle_x++;
    }
    
    tle_x += ttiles.view_x;
    
    while( y1 >= ttiles.tile_h ){
        y1 -= ttiles.tile_h;
        tle_y++;
    }
    
    tle_y += tle.view_y;
    
    mx = mouse.x - mouse.x % otiles.tile_w;
    my = mouse.y - mouse.y % otiles.tile_h;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    if( mx >= (tle.w - tle.view_x + 4) * otiles.tile_w || my >= (tle.h - tle.view_y) * otiles.tile_h ){
        
        asm mov ax, MOUSE_ON
        asm int 33H
        
        return 0;
    }
    
    _ES = 0xA000;
    _DI = my * 320;
    _DI += mx;
    
    _SI = FP_OFF( ttiles.data );
    _SI += ttiles.y * ttiles.tile_h * ttiles.width;
    _SI += ttiles.x * ttiles.tile_w;
        
    for( y = 0; y < ttiles.tile_h; y++, _SI += ttiles.width - ttiles.tile_w,
        _DI += 320 - ttiles.tile_w ){
        _CX = ttiles.tile_w;

loopx:
        asm cmp byte ptr [si], 255
        asm je  skip
        
        asm mov al, [si]
        asm mov es:[di], al 
skip:
        asm inc di
        asm inc si
        asm loop loopx
        
    }
    
    asm mov ax, MOUSE_ON
    asm int 33H
    
    return 0;
}

void resize_tle_map( void ){

    char w[5], h[5], *swidth, *sheight, entry = 0;
    unsigned char far *new_tile_buff, temp;
    unsigned short new_w, new_h, dest_adj_x, src_adj_x, dest_adj_y, xbuff, ftemp;
    unsigned short far *new_fnum_buff, buff_counter = 0;
    
    asm mov ax, MOUSE_OFF
    asm int 33H
    
    blackout_bg();
    
    swidth = itoa(tle.w, w, 10);
    sheight = itoa(tle.h, h, 10);
    
    tle.old_w = tle.w;
    tle.old_h = tle.h;
    
    new_w = tle.w;
    new_h = tle.h;
    
    draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Adjust map dimentions.", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+16, "RIGHT - increase width", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+32, "LEFT - decrease width", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+48, "UP - increase height", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+64, "DOWN - decrease height", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+80, "ENTER - accept changes", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y+96, "ESC - cancel", 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y + 112, "Width: ", 13 );
    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 112, swidth, 13 );
    draw_string( STAT_SCREEN_X+8, STAT_SCREEN_Y + 128, "Height: ", 13 );
    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 128, sheight, 13 );
    
    reset_keyboard();
    
    /*
    * ESC = 0x1B
    * UP = 0x0048
    * DOWN = 0x0050
    * LEFT = 0x004B
    * RIGHT = 0x004D
    * ENTER = 0x0D
    */
    
    while( entry != 0x1B && entry != 0x0D ){
        
        entry = getch();
        
        switch( entry ){
            case 0x0048:
                
                if( new_h < 200 ){
                    
                    new_h++;
                    sheight = itoa(new_h, h, 10);
                    draw_rect_fill( STAT_SCREEN_X+72, STAT_SCREEN_Y + 128, 100, 8, 2 );
                    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 128, sheight, 13 );
                }
                
                break;
            case 0x0050:
                
                if( new_h > DEFAULT_TLE_HEIGHT ){
                    
                    new_h--;
                    sheight = itoa(new_h, h, 10);
                    draw_rect_fill( STAT_SCREEN_X+72, STAT_SCREEN_Y + 128, 100, 8, 2 );
                    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 128, sheight, 13 );
                }
                
                break;
            case 0x004B:
                
                if( new_w > DEFAULT_TLE_WIDTH ){
                    
                    new_w--;
                    swidth = itoa(new_w, w, 10);
                    draw_rect_fill( STAT_SCREEN_X+72, STAT_SCREEN_Y + 112, 100, 8, 2 );
                    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 112, swidth, 13 );
                }
                
                break;
            case 0x004D:
                
                if( new_w < 200 ){
                    
                    new_w++;
                    swidth = itoa(new_w, w, 10);
                    draw_rect_fill( STAT_SCREEN_X+72, STAT_SCREEN_Y + 112, 100, 8, 2 );
                    draw_string( STAT_SCREEN_X+72, STAT_SCREEN_Y + 112, swidth, 13 );
                }
                
                break;
                
            default:
                
                break;
        }
        
    }
    
    blackout_bg();
    
    if( entry == 0x1B ){
        set_keyboard();
        asm mov ax, MOUSE_ON
        asm int 33H
        return;
        
    }
    
    if( new_w > tle.w ){
        dest_adj_x = new_w - tle.w;
        src_adj_x = 0;
        xbuff = tle.w;
    }else if( new_w < tle.w ){
        src_adj_x = tle.w - new_w;
        dest_adj_x = 0;
        xbuff = new_w;
    }else if( new_w == tle.w){
        src_adj_x = 0;
        dest_adj_x = 0;
        xbuff = new_w;
    }
    
    if( new_h > tle.h )
        dest_adj_y = tle.h;
    else if( new_h < tle.h )
        dest_adj_y = new_h;
    else if( new_h == tle.h )
        dest_adj_y = new_h;
        
    
    /**************************************/
    /*
     *      OTILE FNUM AND TDATA RESIZE SHIT
     * 
     * 
     * 
     * ************************************/
    
    /* allocate memory for otile buffer */
    new_tile_buff = (unsigned char far *)farmalloc( (new_w * new_h) * sizeof(unsigned char) );
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ )
        *(new_tile_buff+buff_counter) = 0;
    
    _SI = FP_OFF( tle.tdata );
    _ES = FP_SEG( new_tile_buff );
    _DI = FP_OFF( new_tile_buff );
    for( buff_counter = 0; buff_counter < dest_adj_y; buff_counter++, _SI += src_adj_x, _DI += dest_adj_x ){
        _CX = xbuff;
        asm push ds
        _DS = FP_SEG( tle.tdata );
        asm rep movsb
        asm pop ds
        
    }
    
    farfree( tle.tdata );
    tle.tdata = ( unsigned char far *)farmalloc( (new_w * new_h) * sizeof( unsigned char ) );

    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ ){
        temp = *(new_tile_buff+buff_counter);
        *(tle.tdata+buff_counter) = temp;
    }
    
    /* allocate memory for fnum buffer */
    new_fnum_buff = (unsigned short far *)farmalloc( (new_w * new_h) * sizeof(unsigned short) );
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ )
        *(new_fnum_buff+buff_counter) = 0;
    
    _SI = FP_OFF( tle.fnum );
    _ES = FP_SEG( new_fnum_buff );
    _DI = FP_OFF( new_fnum_buff );
    for( buff_counter = 0; buff_counter < dest_adj_y; buff_counter++, _SI += src_adj_x*2, _DI += dest_adj_x*2 ){
        _CX = xbuff;
        asm push ds
        _DS = FP_SEG( tle.fnum );
        asm rep movsw
        asm pop ds
        
    }
    
    farfree( tle.fnum );
    tle.fnum = ( unsigned short far *)farmalloc( (new_w * new_h) * sizeof( unsigned short ) );

    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ ){
        ftemp = *(new_fnum_buff+buff_counter);
        *(tle.fnum+buff_counter) = ftemp;
    }
    
    /*********************************
     * 
     *      TTILE RESIZE SHIT
     * 
     * 
     * 
     * 
     * *****************************/
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ )
        *(new_tile_buff+buff_counter) = 0;
    
    _SI = FP_OFF( tle.trans_tdata );
    _ES = FP_SEG( new_tile_buff );
    _DI = FP_OFF( new_tile_buff );
    
    for( buff_counter = 0; buff_counter < dest_adj_y; buff_counter++, _SI += src_adj_x, _DI += dest_adj_x ){
        _CX = xbuff;
        asm push ds
        _DS = FP_SEG( tle.trans_tdata );
        asm rep movsb
        asm pop ds
        
    }
    
    farfree( tle.trans_tdata );
    tle.trans_tdata = (unsigned char far *)farmalloc( (new_w * new_h) * sizeof(unsigned char) );
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ ){
        temp = *(new_tile_buff+buff_counter);
        *(tle.trans_tdata+buff_counter) = temp;
    }
    
    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ )
        *(new_fnum_buff+buff_counter) = 0;
    
    _SI = FP_OFF( tle.trans_fnum );
    _ES = FP_SEG( new_fnum_buff );
    _DI = FP_OFF( new_fnum_buff );
    for( buff_counter = 0; buff_counter < dest_adj_y; buff_counter++, _SI += src_adj_x*2, _DI += dest_adj_x*2 ){
        _CX = xbuff;
        asm push ds
        _DS = FP_SEG( tle.trans_fnum );
        asm rep movsw
        asm pop ds
        
    }

    farfree( tle.trans_fnum );
    tle.trans_fnum = ( unsigned short far *)farmalloc( (new_w * new_h) * sizeof( unsigned short ) );

    for( buff_counter = 0; buff_counter < new_w * new_h; buff_counter++ ){
        ftemp = *(new_fnum_buff+buff_counter);
        *(tle.trans_fnum+buff_counter) = ftemp;
    }
    
    farfree( new_tile_buff );
    farfree( new_fnum_buff );
    
    tle.w = new_w;
    tle.h = new_h;
    
    tle.view_x = 0;
    tle.view_y = 0;
    
    set_keyboard();

    asm mov ax, MOUSE_ON
    asm int 33H
    
    
}

void otile_draw_dir( unsigned short seg, unsigned short offs ){

    unsigned short mx, my, y;
    
    mx = mouse.x;
    my = mouse.y;
    
    mx -= mx % otiles.tile_w;
    my -= my % otiles.tile_h;
    my *= 320;
    
    _ES = 0xA000;
    _SI = offs;
    _DI = mx + my;
    
    for( y = 0; y < otiles.tile_h; y++, _DI += 320 - otiles.tile_h ){
        _CX = otiles.tile_h;
        asm push ds
        _DS = seg;
        asm rep movsb
        asm pop ds
            
    }
    
}

void ttile_draw_dir( unsigned short seg, unsigned short offs ){

    unsigned short mx, my, x, y;
    
    mx = mouse.x;
    my = mouse.y;
    
    mx -= mx % otiles.tile_w;
    my -= my % otiles.tile_h;
    my *= 320;
    
    _ES = 0xA000;
    _SI = offs;
    _DI = mx + my;
    
    for( y = 0; y < otiles.tile_h; y++, _DI += 320 - otiles.tile_w ){
        for( x = 0; x < otiles.tile_w; x++ ){
            asm push ds
            _DS = seg;
            asm lodsb
            asm cmp al, 255
            asm je skip
            asm mov es:[di], al
            skip:
            asm inc di
            asm pop ds
        }
            
    }
    
}

#endif

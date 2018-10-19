#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <alloc.h>
#include <dir.h>
#include "C:\tools\tmr.h"
#include "C:\tools\keys.h"
#include "C:\tools\mouse.h"
#include "gfx.h"
#include "file.h"
#include "tle.h"
#include "tflag.h"

#define TRUE 1
#define FALSE 0

#define DEBUGGING FALSE

/* windows to select from for level deisng */
unsigned short const WIN_OPAQUE = 0x0001, WIN_TRANS = 0x0002,
                     WIN_USED_TILE = 0x0003, WIN_LEVEL = 0x0004;

unsigned short debugx = 0, debugy = 0, debugi = 0;
int main(){
    
    int catch = 0;
    unsigned short sel_win = 0, map_win = 0, ovis = TRUE, tvis = TRUE, fvis = TRUE ,del_mode = WIN_OPAQUE, color, i;
    
    /* Mode 13H 320x200 on */
    asm mov ax, 0013H
    asm int 10H
    
    if( !load_pal( "MCFLY.PAL" ) )
       printf("Default palette pissing in an alley somewhere.");
    
    /* set first 16 colors to greyscale ( for GUI and stuff ) */
    outp( 0x03C8, 0 );
    for( i = 0, color = 0; i < 16; i++ ){
        
        outp( 0x03C9, color );
        outp( 0x03C9, color );
        outp( 0x03C9, color += 3 );
    }
    
    /* initial setup */
    otiles.data = 0;
    draw_toolbar();
    blackout_bg();
    
    /* set custom drivers */
    set_keyboard();
    mouse_init( MOUSE_ON );
    set_timerISR();
    set_freq( 5 );
    
    /* set default tle shit */
    load_def_tle();
    init_used_tle();
    tflag_init();
    
    /* OUTTILE default stuff */
    outfile.cur_tle_file[0] = '\0';
    
    /* WELCOME TO THE BEST FUCKIN TILE EDITOR DOS HAS SEEN */
    draw_string( STAT_SCREEN_X+35, STAT_SCREEN_Y+40, "Welcome to OUTTILE\n\nBy\n\nShawn Achimasi", 15 );
    
    /* Main loop */
    do{
        
        mouse_list();
        
        /***********************/
        /* mouse button checks */
        /***********************/

        /* left mouse button pressed? */
        if( mouse.left_button ){
            
            /* sense if in right toolbar area */
            if( mouse.x < 80 ){
                
                if( mouse.y <= 100 && otiles.data ){
                    
                    if( mouse.y <= otiles.height ){
                        sel_win = WIN_OPAQUE;
                        set_otile_mouse();
                    }
                }else{
                    
                    if( mouse.y - 100 < ttiles.height ){
                        sel_win = WIN_TRANS;
                        set_ttile_mouse();
                    }
                }
                
                map_win = 0;
            }
            
            /* sense if in right side of screen */
            if( mouse.x >= 80 ){
                
                /* are we in the level editor part? */
                if( mouse.y < 200 - otiles.tile_h ){
                    
                    if( map_win ){
                        if( sel_win == WIN_USED_TILE ){
                            if( used_tile_win.used_tle_mode == OPAQUE )
                                insert_used_to_tle();
                            else if(used_tile_win.used_tle_mode == TRANS )
                                insert_trans_used_to_tle();
                            
                            draw_used_tle_mouse();
                            draw_trans_tle_map();
                            
                            if( fvis )
                                draw_tflag_map();
                            
                            if( used_tile_win.used_tle_mode == OPAQUE )
                                insert_used_to_tle();
                            else if(used_tile_win.used_tle_mode == TRANS )
                                insert_trans_used_to_tle();
                            draw_used_tle_data();
                            
                        }else if( sel_win == WIN_OPAQUE && ovis == TRUE ){
                            draw_otile();
                            draw_trans_tle_map();
                            
                            if( fvis )
                                draw_tflag_map();
                            
                            insert_tle_data();
                            insert_used_tle_data( outfile.cur_tile_num, outfile.cur_otile_fid + 1 );
                            draw_used_tle_data();
                        }else if( sel_win == WIN_TRANS && tvis == TRUE ){
                            draw_ttile();
                            insert_trans_tle_data();
                            insert_used_trans_tle_data( outfile.cur_tile_num, outfile.cur_ttile_fid + 1 );
                            draw_used_tle_data();
                            
                            if( fvis == TRUE )
                                draw_tflag_map();
                        }
                    }
                    else
                        map_win = TRUE;
                        
                }else if( mouse.y >= 200 - otiles.tile_h && mouse.x < 
                    (used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount)* otiles.tile_w + 80 ){
                    sel_win = WIN_USED_TILE;
                    map_win = FALSE;
                    set_used_tile_mouse();
                }
                
            }
            
        }
        
        /* right mouse button pressed */
        if( mouse.right_button ){
            
            if( mouse.x > 80 ){
            
                if( mouse.y < 200 - otiles.tile_h && fvis == TRUE ){
                    
                    map_win = TRUE;
                    
                    if( tflag_mode == SOLID_MODE )
                        set_solid( tflag );
                    else if( tflag_mode == PREC_MODE )
                        set_prec( tflag );
                
                }
                
                do{
                    mouse_list();
                }while( mouse.right_button );
            
            }
            
        }
        
        /***********************/
        /* key checks          */
        /***********************/
        if( keybuff[0x3d] ){ /* F3 bring up list of GFX files for trans tiles to load */
            reset_keyboard();
            blackout_bg();
            if( (catch = load_ttiles( list_files( "*.gfx", "Load a trans tile." ) ) == -2 ) ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "No GFX files found.", 13 );
                getch();
                blackout_bg();
            }else if( catch == -1 ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Out of memory", 13 );
                getch();
                blackout_bg();
            }else{
                draw_ttiles();
                outfile.cur_ttile_fid = outfile.last_fid;
                sel_win = WIN_TRANS;
            }
            
            set_keyboard();
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
        }
        
        if( keybuff[0x3c] ){  /* F2 bring up list of GFX files for opaque tiles to load */
            reset_keyboard();
            blackout_bg();
            if( (catch = load_otiles( list_files( "*.gfx", "Pick a .GFX file..." ) ) == -2 ) ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "No GFX files found.", 13 );
                getch();
                blackout_bg();
            }else if( catch == -1 ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Out of memory", 13 );
                getch();
                blackout_bg();
            }else{
                draw_otiles();
                draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
                outfile.cur_otile_fid = outfile.last_fid;
                sel_win = WIN_OPAQUE;
            }
            
            set_keyboard();
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
        }
            
        if( keybuff[0x26] ){  /* bring up list of TLE files to open */
            reset_keyboard();
            blackout_bg();
            if( (catch = load_tle_file( list_files( "*.tle", "Pick a .TLE file..." ) ) ) == -1 ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "No .TLE files found", 13 );
                getch();
                blackout_bg();
            }else if( catch == -3 ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Can't open secondary file", 13 );
                getch();
                blackout_bg();
            }else{
                tle.view_x = 0;
                tle.view_y = 0;
                strcpy( outfile.cur_tle_file, outfile.lastfile );
                load_tflag_buff( outfile.cur_tle_file );
                map_win = TRUE;
            }
            set_keyboard();
            mouse_init( MOUSE_OFF );
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( tvis )
                draw_tflag_map();
            
            draw_used_tle_data();
            
            mouse_init( MOUSE_ON );
            
        }
        
        if( keybuff[0x1E] ){ /* Save file as */
            reset_keyboard();
            mouse_init( MOUSE_OFF );
            blackout_bg();
            if( !(catch = create_new_file()) ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "File Saved", 13 );
                strcpy( outfile.cur_tle_file, outfile.lastfile );
                save_tle_file();
                save_tflag_buff( outfile.lastfile );
            }else if( catch == -1 ){
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "File Exists, overwrite (y/n)?", 13 );
                
                if( getch() == 'y' ){
                    blackout_bg();
                    strcpy( outfile.cur_tle_file, outfile.lastfile );
                    save_tle_file();
                    save_tflag_buff( outfile.cur_tle_file );
                    draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "File Saved", 13 );
                }else{
                    draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Saving Aborted", 13 );
                    outfile.cur_tle_file[0] = '\0';
                }

            }else
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Sequence Aborted", 13 );
            
            getch();
            set_keyboard();
            blackout_bg();
            
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
            
            mouse_init( MOUSE_ON );
        }
        
        if( keybuff[0x1f] && outfile.cur_tle_file[0] != '\0' ){
            
            blackout_bg();
            reset_keyboard();
            
            save_tle_file();
            save_tflag_buff( outfile.lastfile );
            draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "File Saved", 13 );
            
            getch();
            set_keyboard();
            
            if( ovis && head)
                draw_tle_map();
            
            if( tvis && trans_head )
                draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
        }
        
        if( keybuff[0x42] ){ /* f9 key - toggles tflag visibitly */
            
            mouse_init( MOUSE_OFF );
            
            if( head && ovis )
                draw_tle_map();
            else
                blackout_bg();
            
            if( tvis && trans_head )
                draw_trans_tle_map();
            
            if( fvis == TRUE )
                fvis = FALSE;
            else{
                    
                fvis = TRUE;
                draw_tflag_map();
            }
        }
        
        if( keybuff[0x44] ){ /* f10 key, toggle ttile visiblilty */
        
            mouse_init( MOUSE_OFF );
            
            if( head && ovis )
                draw_tle_map();
            else
                blackout_bg();
            
            if( tvis ){
                tvis = FALSE;
            }else{
                tvis = TRUE;
                draw_trans_tle_map();
            }
            
            if( fvis )
                draw_tflag_map();
            
            mouse_init( MOUSE_ON );
        }
        
        if( keybuff[0x43] ){ /* F9 key, toggle otile visiblilty */
            
            mouse_init( MOUSE_OFF );
            
            if( ovis ){
                ovis = FALSE;
                blackout_bg();
                
            }else{
                ovis = TRUE;
                draw_tle_map();
                
            }
            
            if( trans_head && tvis )
                    draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
            
            mouse_init( MOUSE_ON );
        }
        
        if( keybuff[0x20] ){ /* D key. Deletes otile or trans tile. depending which window is toggled */
            
            
            
            if( del_mode == WIN_TRANS && tvis )
                del_trans_tle_data();
            
            else if( del_mode == WIN_OPAQUE && ovis )
                del_tle_data();
            
            mouse_init( MOUSE_OFF );
            
            blackout_bg();
            
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( fvis )
                draw_tflag_map();
            
            draw_used_tle_data();
            mouse_init( MOUSE_ON );
        }
        
        if( keybuff[0x14] ){/* T key. switched to trans tile del mode */
            
            if( ttiles.data )
                sel_win = WIN_TRANS;
            
            del_mode = WIN_TRANS;
        }
        
        if( keybuff[0x18] ){/* O key. switched to opaque tile delete mode */
            
            if( otiles.data )
                sel_win = WIN_OPAQUE;
            
            del_mode = WIN_OPAQUE;
        }
        
        if( keybuff[0x13] ){ /* R - resize map menu */
            
            resize_tle_map();
            resize_tflag_map( tle.w, tle.h );
            
            mouse_init( MOUSE_OFF );    
                
            if( ovis )
                draw_tle_map();
                
            if( tvis )
                draw_trans_tle_map();
                
            if( fvis )
                draw_tflag_map();
        
            mouse_init( MOUSE_ON );
            
        }
        
        if( keybuff[0x2d] ){ /* X - deletes room change cooridnent */
            
            del_room_change();
            
            mouse_init( MOUSE_OFF );
                
            if( ovis )
                draw_tle_map();
                
            if( tvis )
                draw_trans_tle_map();
                
            if( fvis )
                draw_tflag_map();
                
            mouse_init( MOUSE_ON );
            
        }
        
        if( keybuff[0x2e] ){  /* C - set room change tile space */
                
            if( mouse.x > 80 && mouse.y < 200 - otiles.tile_h ){
                
                blackout_bg();
                del_room_change();
                set_room_change();
                
                mouse_init( MOUSE_OFF );
                
                if( ovis )
                    draw_tle_map();
                
                if( tvis )
                    draw_trans_tle_map();
                
                if( fvis ){
                    draw_tflag_map();
                }
                
                mouse_init( MOUSE_ON );
                
            }
            
        }
        
        if( keybuff[0x02] ){ /* 1 key - go into solid mode */
         
            tflag_mode = SOLID_MODE;

        }
        
        if( keybuff[0x03] ){ /* 2 key - go into prec mode NOTE: Only works if there is a ttile in selected space */
            
            tflag_mode = PREC_MODE;
            
        }
        
        if( keybuff[0x19] ){ /* P key - go to load a palette */
            
            reset_keyboard();
            mouse_init( MOUSE_OFF );
            blackout_bg();
            if( (load_pal( list_files( "*.pal", "Pick a .PAL file..." )) == -1 ) ){
                blackout_bg();
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "PAL file not found", 13 );
                
            }else{
                blackout_bg();
                draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "PAL file loaded", 13 );
            }
            getch();
            set_keyboard();
            
            blackout_bg();
            mouse_init( MOUSE_ON );
            if( ovis )
                draw_tle_map();
            
            if( tvis )
                draw_trans_tle_map();
            
            if( tvis )
                draw_tflag_map();
            
            draw_used_tle_data();
        }
        
            /********************
             * 
             *      UP DOWN LEFT RIGHT keys move the cursor in the opaque
             *      tile section
             * 
             * 
             * ******************/
        if( keybuff[0x48] ){ /* up key */
            
            if( map_win && tle.view_y ){
                tle.view_y--;
                mouse_init( MOUSE_OFF );
                blackout_bg();
                
                if( ovis )
                    draw_tle_map();
                
                if( tvis )
                    draw_trans_tle_map();
                
                if( fvis )
                    draw_tflag_map();
                
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_OPAQUE ){
                if( otiles.y ){
                    otiles.y--;
                    
                    if( otiles.sel_y )
                        otiles.sel_y--;
                    else
                        otiles.view_y--;
                }
                
                mouse_init( MOUSE_OFF );
                
                draw_otiles();
                draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
                
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_TRANS ){
                if( ttiles.y ){
                    ttiles.y--;
                    
                    if( ttiles.sel_y )
                        ttiles.sel_y--;
                    else
                        ttiles.view_y--;
                }
                
                mouse_init( MOUSE_OFF );
                
                draw_ttiles();
                draw_rect( ttiles.sel_x * ttiles.tile_w, 100 + (ttiles.sel_y * otiles.tile_h), ttiles.tile_w, ttiles.tile_h, 44 );
                
                mouse_init( MOUSE_ON );
            }
        }
        
        if( keybuff[0x50] ){ /* down key */
            
            if( map_win && tle.maxy + tle.view_y < tle.h ){
                tle.view_y++;
                mouse_init( MOUSE_OFF );
                blackout_bg();
                if( ovis )
                    draw_tle_map();
                
                if( tvis )
                    draw_trans_tle_map();
                
                if( fvis )
                    draw_tflag_map();
                
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_OPAQUE ){
                if( ((otiles.y + 1) * otiles.tile_h) < otiles.height ){
                    otiles.y++;
                    
                    if( (otiles.sel_y + 1) * otiles.tile_h >= 100 )
                        otiles.view_y++;
                    else
                        otiles.sel_y++;
                }
                
                mouse_init( MOUSE_OFF );
                draw_otiles();
                draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
                mouse_init( MOUSE_ON);
            }else if( sel_win == WIN_TRANS ){
                    
                
            }
        }
        
        if( keybuff[0x4B] ){ /* left key */
            
            if( map_win && tle.view_x ){
                tle.view_x--;
                mouse_init( MOUSE_OFF );
                blackout_bg();
                if( ovis )
                    draw_tle_map();
                
                if( tvis )
                    draw_trans_tle_map();
                
                if( fvis )
                    draw_tflag_map();
                
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_OPAQUE ){
                if( otiles.x ){
                    otiles.x--;
                
                    if( otiles.sel_x )
                        otiles.sel_x--;
                    else
                        otiles.view_x--;
                }
                
                mouse_init( MOUSE_OFF );
                draw_otiles();
                draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_USED_TILE && used_tile_win.view_x ){
                used_tile_win.view_x--;
                draw_used_tle_data();
            }
        }
            
        if( keybuff[0x4D] ){ /* right key */
            
            if( map_win && tle.view_x + tle.maxx < tle.w ){
                tle.view_x++;
                mouse_init( MOUSE_OFF );
                blackout_bg();
                if( ovis )
                    draw_tle_map();
                
                if( tvis )
                    draw_trans_tle_map();
                
                if( fvis )
                    draw_tflag_map();
                
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_OPAQUE ){
                if( (otiles.x + 1) * otiles.tile_w < otiles.width ){
                    otiles.x++;
                
                    if( (otiles.sel_x + 1) * otiles.tile_w < 80 )
                        otiles.sel_x++;
                    else
                        otiles.view_x++;
                }
                
                mouse_init( MOUSE_OFF );
                draw_otiles();
                draw_rect( otiles.sel_x * otiles.tile_w, otiles.sel_y * otiles.tile_h, otiles.tile_w, otiles.tile_h, 44 );
                mouse_init( MOUSE_ON );
            }else if( sel_win == WIN_TRANS ){
                
            }else if( sel_win == WIN_USED_TILE && 
                (used_tile_win.used_tile_amount + used_tile_win.used_trans_tile_amount - used_tile_win.view_x) * otiles.tile_w > 240 ){
                used_tile_win.view_x++;
                draw_used_tle_data();
            }
            
        }
        
        /* lazy way to time the program, buttfuckit */
        timer = 35;
        while( timer )
            ;
        
    }while( !keybuff[0x01] );

    /* Mode 13H 320x200 off */
    asm mov ax, 0003H
    asm int 10H
    
    cur_coord = &coord_head;
    while( *cur_coord ){
    
        printf( "X: %hu - Y: %hu | DX: %hu - DY: %hu\n", (*cur_coord)->x, (*cur_coord)->y, (*cur_coord)->dest_x, (*cur_coord)->dest_y );
        printf( "FILE NUMBER: %hu \n", (*cur_coord)->tle_file );
        
        cur_coord = &(*cur_coord)->next;
    }
    
    /* print tile datta buffer
    printf("Tile number data in tdata:\n");
    debugy = tle.h;
    debugi = 0;
    while( debugy-- ){
        
        debugx = tle.w;
        while( debugx-- ){
            printf("%02x ", *(tle.trans_tdata+debugi++) );
        }
        
        printf("\n");
    }*/

    /* print file number for each tile
    printf("File number data in fnum:\n");
    debugy = tle.h;
    debugi = 0;
    while( debugy-- ){
        
        debugx = tle.w;
        while( debugx-- ){
            printf("%02x ", *(tle.fnum+debugi++) );
        }
        
        printf("\n");
    }*/
    
    /* print file number for each tile
    printf("Tflag data:\n");
    debugy = tle.h;
    debugi = 0;
    while( debugy-- ){
        
        debugx = tle.w;
        while( debugx-- ){
            printf("%02x ", *(tflag+debugi++) );
        }
        
        printf("\n");
    }*/
    
    /* used tile listings
    debug_used_tle();*/
    
    /* free up tile and tle data */
    if( otiles.data )
        free( otiles.data );
    
    if( ttiles.data )
        free( ttiles.data );
    
    unalloc_tle_data();
    unalloc_used_tle();
    tflag_uninit();
    
    /* get drivers and devices back to normal */
    mouse_init( MOUSE_OFF );
    reset_keyboard();
    set_freq( 0 );
    reset_timerISR();
    
    printf( "Thanks for using OutTile (c) 2017\n\nBy Shawn Achimasi\n\n");
    
    return 0;

}

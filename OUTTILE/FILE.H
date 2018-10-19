#ifndef _FILE_H
#define _FILE_H

struct OUTFILE{
    char lastfile[13], cur_tile_num, cur_ttile_num, cur_tle_file[13];
    unsigned short last_fid, cur_tle_fid, cur_otile_fid, cur_ttile_fid;
    
}outfile;

/*      draw_user_input
 * 
 *      returns 0 if sequence is aborted
 * 
 */
char * draw_user_input( unsigned short x, unsigned short y, unsigned char color ){
    
    unsigned short i;
    static char input[9];
    char letter;
    
    for( input[0] = '\0', i = 0; (letter = getch()) != 0x0D; ){
        
        if( letter == 0x1B )
            return 0;
        else if( letter == 0x08 && i ){
            input[--i] = '\0';
            draw_rect_fill( x + (i * 8), y, 8, 8, 2 );
        }else if( i < 8 && letter != 0x08 ){
            input[i++] = letter;
            input[i] = '\0';
            draw_string( x, y, input, color );
        }
    }
    
    blackout_bg();
    
    if( input[0] == '\0' )
        return 0;
    else
        return input;
}

/*  create_new_file FUNCTION
 * 
 *  returns 0 if creating file was succesful
 *  returns -1 file name exists
 *  returns -2 if file naming sequence was aborted or draw_user
 *  pressed enter with no chars inputed
 * 
 */
int create_new_file( void ){
 
    FILE *fh;
    struct ffblk finfo;
    char final_fname[13], *filename;
    
    draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, "Save a Bitch Ass File", 13 );
    
    filename = draw_user_input( STAT_SCREEN_X+8, STAT_SCREEN_Y+16, 13 );
    
    if( !filename ){
        blackout_bg();
        return -2;
    }
    
    strcpy( final_fname, filename );
    strcat( final_fname, ".TLE" );
    
    if( !findfirst( final_fname, &finfo, 0 ) ){
        /* file exists */
        strcpy( outfile.lastfile, final_fname );
        blackout_bg();
        return -1;
    }
    
    fh = fopen( final_fname, "wb" );
    
    fclose( fh );

    strcpy( outfile.lastfile, final_fname );
    
    blackout_bg();
    
    return 0;
}

/*
 *  list_files
 *  
 *  finds files with user defined file type
 * 
 *  returns 0 if the selected file type could not be found or sequence aborted
 */
char * list_files( char *ft, char *msg ){
    
    struct ffblk finfo;
    unsigned short ty = 0, ay = 0, num_files = 0, view = 0;
    char **list, sel_file = 0, kp = 1;
    
    if( findfirst( ft, &finfo, 0 ) )
        return 0;
    else{
        
        /* count how many TLE files there are*/
        do{
            num_files++;
        }while( !findnext( &finfo ) );
        
        list = ( char **)malloc( sizeof(char **) * num_files );
        
        findfirst( ft, &finfo, 0 );
        
        draw_string( STAT_SCREEN_X, STAT_SCREEN_Y, msg, 13 );
        
        /* draw initial arrow position */
        draw_arrow( STAT_SCREEN_X, STAT_SCREEN_Y + 18 + ay, RIGHT, 12 );
        
        /* fill list while dynamically allocating everything */
        do{
            *(list+sel_file) = ( char *)malloc( sizeof(char *) * 13 );
            strcpy( *(list+sel_file), finfo.ff_name );
            sel_file++;
        }while( !findnext( &finfo ) );
        
        for( sel_file = 0; sel_file < num_files && sel_file < 22; sel_file++, ty += 8 )
                draw_string( STAT_SCREEN_X + 10, STAT_SCREEN_Y + 16 + ty, *(list+sel_file), 12 );
        
        sel_file = 0;
        
        /*
         * ESC = 0x1B
         * UP = 0x0048
         * DOWN = 0x0050
         * LEFT = 0x004B
         * RIGHT = 0x004D
         * ENTER = 0x0D
         */
        do{
           
            kp = getch();
            
            strcpy( outfile.lastfile, *(list+sel_file) );
            
            switch( kp ){
                case 0x0050:
                    if( sel_file < num_files - 1 ){
                        sel_file++;
                        
                        if( ay < 168 ){
                            draw_rect_fill( STAT_SCREEN_X, STAT_SCREEN_Y + 18 + ay, 8, 4, 2 );
                            ay += 8;
                            draw_arrow( STAT_SCREEN_X, STAT_SCREEN_Y + 18 + ay, RIGHT, 12 );
                        }else{
                            
                            unsigned short t_sel;
                            draw_rect_fill( STAT_SCREEN_X + 10, STAT_SCREEN_Y + 16, 100, 180, 2 );
                            view++;
                            
                            for( t_sel = 0, ty = 0; t_sel < 22; t_sel++, ty += 8 )
                                draw_string( STAT_SCREEN_X + 10, STAT_SCREEN_Y + 16 + ty, *(list+view+t_sel), 12 );
                        }
                    }
                break;
                case 0x0048:
                    if( sel_file ){
                        sel_file--;
                        
                        if( ay ){
                            draw_rect_fill( STAT_SCREEN_X, STAT_SCREEN_Y + 18 + ay, 8, 4, 2 );
                            ay -= 8;
                            draw_arrow( STAT_SCREEN_X, STAT_SCREEN_Y + 18 + ay, RIGHT, 12 );
                        }else{
                            
                            unsigned short t_sel;
                            draw_rect_fill( STAT_SCREEN_X + 10, STAT_SCREEN_Y + 16, 100, 180, 2 );
                            view--;
                            
                            for( t_sel = 0, ty = 0; t_sel < 22; t_sel++, ty += 8 )
                                draw_string( STAT_SCREEN_X + 10, STAT_SCREEN_Y + 16 + ty, *(list+view+t_sel), 12 );
                        }
                        
                    }
                break;
                case 0x1B:
                    /* Free up memory used by file name data */
                    for( sel_file = 0; sel_file < num_files; sel_file++ )
                        free( *(list+sel_file) );
      
                    free( list );
                    
                    outfile.lastfile[0] = '\0';
                    
                    return outfile.lastfile;
                case 0x0D:
                    
                    blackout_bg();
                    break;
            }
            
        }while( kp != 0x1B && kp != 0x0D );
        
        outfile.last_fid = sel_file;
        
        /* Free up memory used by file name data */
        for( sel_file = 0; sel_file < num_files; sel_file++ )
            free( *(list+sel_file) );
        
        free( list );

        return outfile.lastfile;
    }
}
#endif

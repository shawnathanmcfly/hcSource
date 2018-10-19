#include		<dos.h>
#include		<conio.h>
#include		<stdio.h>
#include		<alloc.h>

int main( int argv, char *argc[] ){

    FILE *fh;
    unsigned long w, h, max;
    unsigned short adj_w, adj_h, i = 0, x, y;
    unsigned char byte;
    unsigned char far *new_buff;
    
    fh = fopen( argc[1], "rb" );
    
    if( !fh )
        printf( "%s not found.", argc[1] );
    else{
        
        fseek( fh, 18, SEEK_SET );
        fread( &w, 4, 1, fh );
        fread( &h, 4, 1, fh );
        
        adj_w = (unsigned short)w;
        adj_h = (unsigned short)h;
        
        printf( "width = %hu - height = %hu\n\n", adj_w, adj_h );
        
        fseek( fh, 1078, SEEK_SET );
        
        max = (unsigned long)adj_w * (unsigned long)adj_h;
        
        if( max > 65535L ){
                
            fclose( fh );
            printf("File is too large for conversion\n");
            printf("BMP size: %lu\n", max );
            printf("Limit is %lu\n", 65535L );
            
            return 0;
        }else
            printf( "size = %hu\n", adj_w * adj_h );
        
        new_buff = ( unsigned char far * )farmalloc( ( adj_w * adj_h ) * sizeof( unsigned char ) );
        
        while( (unsigned long)i < max ){
            fread( &byte, 1, 1, fh );
            *( new_buff + i ) = byte;
            i++;
        }
        
        i = 0;
        
        if( fh )
            fclose( fh );
        
        if( argv > 2 ){
            fh = fopen( argc[2], "wb" );
        
            fwrite( &adj_w, 2, 1, fh );
            fwrite( &adj_h, 2, 1, fh );
            
            /* flip the image, cuz bmp is store upside down. stupid. */
            for( y = adj_h; y ; y-- ){
             
                for( x = 0; x < adj_w; x++ ){
                
                    byte = *( new_buff + ( (y-1) * adj_w + x) );
                    fwrite( &byte, 1, 1, fh );
                    
                }
            }
            
        }else
            printf("ERROR: No output file name given.");
        
        farfree( new_buff );
    }
    
    return 0;
}

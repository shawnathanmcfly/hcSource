#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <alloc.h>

int main( int argv, char * argc[] ){

    FILE *fh;
    char paltype[9], snum[4] ,text;
    unsigned char *pal_buff, num;
    int color;
    unsigned short i = 0, ii = 0;
    
    if( argv > 1 ){
        
        fh = fopen( argc[1], "r" );
        
        if( !fh ){
            
            printf("File not found.\n");
            
            return 0;
            
        }else{
            
            for( i = 0; i < 8; i++ ){
                fread( &text, 1, 1, fh );
                paltype[i] = text;
                paltype[i+1] = '\0';
            }
            
            printf("Palette Type: %s\n", paltype );
            
            fseek( fh, 18, SEEK_SET );
            
            pal_buff = ( unsigned char *)malloc( (256 * 3 ) * sizeof( unsigned char ) );
            
            if( !pal_buff ){
                fclose( fh );
                printf("No memory for new palette buffer...exiting");
                return 0;
            }
            
            for( ii = 0; ii < 256 * 3; ii++, i = 0 ){
                
                do{
                    
                    fread( &text, 1, 1, fh );
                    snum[i] = text;
                    i++;
                    
                }while( snum[i-1] != 0x20 && snum[i-1] != 0x0A );
                
                snum[i-1] = '\0';
                
                color = atoi( snum );
            
                num = ( unsigned char )color;
                
                num >>= 2;
                
                *( pal_buff + ii ) = num;
                printf("%d ", *( pal_buff + ii ));
                
            }

            fclose( fh );
            fh = fopen( argc[2], "wb" );
            
            for( ii = 0; ii < 256 * 3; ii++ ){
                
                num = *( pal_buff + ii );
                fwrite( &num, 1, 1, fh );
                
            }
            
            fclose( fh );
            
            printf("\nCOMPLETED!");
            free( pal_buff );
        }
        
    }else{
        
        printf("Please input a source file, then a destination file. Dumbfuck");
        
    }
    
    return 0;
}

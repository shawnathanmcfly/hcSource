#ifndef _SOUND_H
#define _SOUND_H

#include <stdio.h>

struct SOUND_8BIT{
    unsigned char *data, lo_byte, hi_byte;
    unsigned short lin_addr, buff_size, channels;
    unsigned long sample_rate, byte_rate;
};

struct SOUND_16BIT{
    signed short *data;
    unsigned short buff_size, channels, offs, sample_rate;
    unsigned char lo_data_byte, hi_data_byte, lo_cnt_byte, hi_cnt_byte, lo_srate, hi_srate ,page;
};

struct SOUND_SETTINGS{

    unsigned char cbyte, mbyte, dsp_port, irq, dma, count_port, data_port, page;
}sb_settings;

/*********************************************/
/* DMA MODE REGISTER OPTIONS                 */
/*********************************************/

/* mode selections */
unsigned char const DMA_DEMAND = 0x00, DMA_SINGLE = 0x01, DMA_BLOCKMODE = 0x02, DMA_CASCADE = 0x03;

/* Address direction flag */
unsigned char const DMA_INC = 0x00, DMA_DEC = 0x01;

/* Auto initialize mode */
unsigned char const DMA_SINGLE_CYCLE = 0x00, DMA_AUTO = 0x01;

/* Transfer Mode */
unsigned char const DMA_VERIFY = 0x00, DMA_WRITE = 0x01, DMA_READ = 0x02;

/* 16 bit DMA channels */
unsigned char const DMA_CH5 = 0x01, DMA_CH6 = 0x02, DMA_CH7 = 0x03;

/***********************************************/
/* SOUND BLASTER 16 DSP OPTIONS                */
/***********************************************/

unsigned short const SB16_DSP_RESET = 0x0206, SB16_DSP_READ = 0x020A, SB16_DSP_WRITE = 0x020C,
                     SB16_READ_BUFFER_STATUS = 0x020E, SB16_INT_ACK = 0x020F;

void SB_set_irq( unsigned char irq );
void SB_reset_irq( unsigned char irq );

/*void pbinary( unsigned char binary_num ){

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
    
}*/

void SB_set_control_byte( unsigned char mode, unsigned char dir, unsigned char auto_init, unsigned char type ){
    
    /* SET MODE - must be between 0 and 3 - 
       0 = Demand Mode, 1 = Single Mode, 2 Block Mode Select, 3 Cascade Mode */
    
    mode <<= 6;
        
    /*  DIRECTION - will the sound go backwards or forwards in memory? 0 = inc 1 = dec */
    
    mode ^= dir << 5;
    
    /* AUTO INITIALIZE MODE - point back to beginning of memory location for sound -
       0 = Single cycle, 1 = Auto */
    
    mode ^= auto_init << 4;
    
    /* TRANSFER TYPE 0 thru 3 - 
       Verify Transfer = 0, Write Transfer = 1, Read Transfer = 2 */
    
    mode ^= type << 2;
 
    /* CHANNEL SELECT 
       0 thru 3 selects channel respectivly */
    
    mode ^= sb_settings.dma;
    
    sb_settings.cbyte = mode;
}

void SB8_write_cbyte( unsigned char cbyte ){
    
    outportb( 0x0B, cbyte );
}

void SB16_write_cbyte( unsigned char cbyte ){
    
    outportb( 0xD6, cbyte );
}

void SB8_write_mask( unsigned char mask_byte ){
    
    outportb( 0x0A, mask_byte );
    
}

void SB16_write_mask( unsigned char enable, unsigned char channel ){
    
    unsigned char mask_byte;
    
    mask_byte = enable << 2;
    mask_byte ^= channel;
    
    outportb( 0xD4, mask_byte );
}

void SB8_clear_ff( void ){

    outportb( 0x0C, 0x01 );
}

void SB16_clear_ff( void ){

    outportb( 0xD8, 0x01 );
}

unsigned char far * SB8_malloc_wav( unsigned short size ){
 
    
}

int SB8_free_wav( unsigned char *data ){


}

struct SOUND_16BIT * SB16_malloc_wav( char *filename ){

    FILE *fh;
    struct SOUND_16BIT * sound;
    unsigned long t_long, lin_addr;
    unsigned short i;
    signed short rdata;
    
    fh = fopen( filename, "rb" );
    
    if( !fh )
        return 0;
    
    sound = ( struct SOUND_16BIT * )malloc( sizeof( struct SOUND_16BIT ) );
    
    if( !sound )
        return 0;
    
    fseek( fh, 22, SEEK_SET );
    
    fread( &(*sound).channels, 2, 1, fh );
    printf("Number of channels: %d\n", sound->channels);
    
    fread( &t_long, 4, 1, fh );
    sound->sample_rate = ( unsigned short )t_long;
    sound->lo_srate = (unsigned char)sound->sample_rate;
    sound->hi_srate = (unsigned char)(sound->sample_rate >> 8);
    printf("Sample rate: %d\n", sound->sample_rate );
    
    fseek( fh, 44, SEEK_SET );
    
    for( i = 0; !feof( fh ); i++, fread( &rdata, 2, 1, fh ) )
        ;
    
    sound->buff_size = i;
    
    sound->data = ( signed short *)malloc( i * sizeof( signed short ) );
    
    if( !sound->data ){
        
        fclose( fh );
        return 0;
    }
    
    fseek( fh, 44, SEEK_SET );
    
    printf("Loading WAV file, please wait...\n");
    
    for( i = 0; i < sound->buff_size; i++ ){
    
        fread( &rdata, 2, 1, fh );
        *( sound->data + i ) = rdata;
    }
    
    printf("Buffer size: %02x\n", sound->buff_size);
    
    lin_addr = (FP_SEG( sound->data ) * 16) + FP_OFF( sound->data );
    
    sound->offs = (lin_addr / 2) % 65536L;
    printf( "Buffer Offset: %04x\n", sound->offs );
    sound->lo_data_byte = (unsigned char)sound->offs;
    sound->hi_data_byte = (unsigned char)(sound->offs >> 8);
    
    _CX = sound->buff_size;
    _CL--;
    _CH--;
    sound->lo_cnt_byte = _CL;
    sound->hi_cnt_byte = _CH;
    
    sound->page = (long)(lin_addr / 65536L);
    printf( "Page: %d\n", sound->page );
    
    fclose( fh );
    return sound;
}

void static SB16_free_wav( struct SOUND_16BIT * sound ){
        
    free( sound->data );
    free( sound );
    
}

/*NOTE: Also resets DSP  */
int static SB_set_dsp_port( void ){
    
    unsigned short wait;
    unsigned char dsp, result = 0;
    
    for( dsp = 0x10; result != 0xAA && dsp < 0x70; dsp += 0x10 ){
     
        outportb( SB16_DSP_RESET ^ dsp, 1 );
        
        for( wait = 1; wait; wait++ )
            ;
        
        outportb( SB16_DSP_WRITE ^ dsp, 0 );
        
        for( wait = 1; (result & 0x80) != 0x80; wait++ )
            result = inportb( SB16_READ_BUFFER_STATUS ^ dsp );
        
        for( wait = 1; wait && result != 0xAA; wait++ )
            result = inportb( SB16_DSP_READ ^ dsp );
    
    }
    
    dsp -= 0x10;
    
    if( dsp < 0x70 && result == 0xAA ){
        
        printf("SOUND BLASTER FOUND: port %02x\n", dsp);
        sb_settings.dsp_port = dsp;
        return 0;
    }else{
        
        printf("ERROR: No sound blaster detected...\n");
        sb_settings.dsp_port = 0;
        return -1;
        
    }

}

void static write_dsp( unsigned char command ){

    while( (inportb( SB16_DSP_WRITE ^ sb_settings.dsp_port ) & 0x80 ) != 0x80 )
        ;
    
    outportb( SB16_DSP_WRITE ^ sb_settings.dsp_port, command );
}

unsigned char static read_dsp( void ){
 
    while( (inportb( SB16_READ_BUFFER_STATUS ^ sb_settings.dsp_port ) & 0x80 ) != 0x80 )
        ;
    
    return inportb( SB16_DSP_READ ^ sb_settings.dsp_port );
}

void static SB16_set_sampling_rate( unsigned char hi, unsigned char lo ){

    write_dsp( 0x41 );
    write_dsp( hi );
    write_dsp( lo );
    
}

void SB16_play_sound( struct SOUND_16BIT * sound ){    
    
    SB16_write_mask( 1, sb_settings.dma );
    SB16_clear_ff();
    SB16_write_cbyte( sb_settings.cbyte );
    
    /* DMA settings */
    outportb( sb_settings.data_port, sound->lo_data_byte );
    outportb( sb_settings.data_port, sound->hi_data_byte );
    outportb( sb_settings.count_port, sound->lo_cnt_byte );
    outportb( sb_settings.count_port, sound->hi_cnt_byte );
    outportb( sb_settings.page, sound->page );
    
    /* sound blaster 16 bit commands */
    SB16_write_mask( 0, sb_settings.dma );
    SB16_set_sampling_rate( sound->hi_srate, sound->lo_srate );
    write_dsp( 0xB8 );
    write_dsp( 0x10 );
    write_dsp( sound->lo_cnt_byte );
    write_dsp( sound->hi_cnt_byte );
}

void SB_set_sound( unsigned char dma, unsigned char irq ){

    SB_set_irq( irq );
    SB_set_dsp_port();
    
    sb_settings.irq = irq;
    sb_settings.dma = dma;
    
    if( dma == DMA_CH5 ){
        sb_settings.data_port = 0xC4;
        sb_settings.count_port = 0xC6;
        sb_settings.page = 0x8B;
    }else if( dma == DMA_CH6 ){
        sb_settings.data_port = 0xC8;
        sb_settings.count_port = 0xCA;
        sb_settings.page = 0x89;
    }else if( dma == DMA_CH7 ){
        sb_settings.data_port = 0xCC;
        sb_settings.count_port = 0xCE;
        sb_settings.page = 0x8A;
    }
    
}

void SB_reset_sound( void ){

    SB_reset_irq( sb_settings.irq );
}

#endif


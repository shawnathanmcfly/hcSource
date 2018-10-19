#ifndef _OBJECTS_H
#define _OBJECTS_H

struct SOUND_16BIT *snd_money;

struct Object{
    
    Spr **sprite_list;
    struct SOUND_16BIT *contact;
    struct Object *next;
    unsigned short x, y, prev_x, prev_y, w, h;
    signed short health;
    unsigned char animation_speed, speed, speed_max, num_sprites, cur_sprite, cur_frame, far *buffer_section;
    signed char x_dir, y_dir;
    char *object_name;
    
    
};

struct Object *object_head = 0;
struct Object **cur_object = &object_head;

int insert_object( char *object_name, unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned char spr_amount  ){
        
    cur_object = &object_head;
    while( *cur_object )
        cur_object = &(*cur_object)->next;
    
    *cur_object = ( struct Object *)malloc( sizeof( struct Object ) );
    if( *cur_object == 0 )
        return -1;
    
    /* mark the end of the list */
    (*cur_object)->next = 0;
    
    /* TODO: may have to fix it */
    (*cur_object)->object_name = ( char *)malloc( 8 );
    if( (*cur_object)->object_name == 0 ){
        free( *cur_object );
        return -1;
    }

    strcpy( (*cur_object)->object_name, object_name );
    
    /* initial sprite list is empty and to be inserted later */
    (*cur_object)->sprite_list = 0;
    
    (*cur_object)->x = x;
    (*cur_object)->y = y;
    (*cur_object)->prev_x = x;
    (*cur_object)->prev_y = y;
    
    (*cur_object)->w = w;
    (*cur_object)->h = h;
    (*cur_object)->x_dir = 0;
    (*cur_object)->y_dir = 0;
    
    (*cur_object)->cur_sprite = 0;
    (*cur_object)->cur_frame = 0;
    (*cur_object)->num_sprites = spr_amount;
    
    (*cur_object)->sprite_list = ( Spr **)malloc( sizeof( Spr * ) * spr_amount );
    if( (*cur_object)->sprite_list == 0 ){
     
        free( (*cur_object)->object_name );
        free( *cur_object );
        return -1;
    }
    
    (*cur_object)->buffer_section = ( unsigned char far *)farmalloc( w * h );
    if( (*cur_object)->buffer_section == 0 ){
     
        free( (*cur_object)->sprite_list );
        free( (*cur_object)->object_name );
        free( *cur_object );
        return -1;
    }
    
    if( x + (*cur_object)->w > phys_view_x &&
        x < phys_view_x + SCREEN_BUFFER_WIDTH - TILE_WIDTH &&
        y + (*cur_object)->h > phys_view_y &&
        y < phys_view_y + SCREEN_BUFFER_HEIGHT ){
        
        copy_buffer_section( (*cur_object)->buffer_section, w, h, x - phys_view_x, y - phys_view_y );
        
    }
    
    if( !strcmp( object_name, "badguy" ) ){
        
        if( spr_badguy == 0 )
            spr_badguy = load_spr( "SPR0.GFX", 0, 0, w, h, 2 );
        
        (*cur_object)->sprite_list[0] = spr_badguy;
        (*cur_object)->health = 40;
        (*cur_object)->speed_max = 5;
        (*cur_object)->speed = 0;
        (*cur_object)->animation_speed = 6;
        
        
    }else if( !strcmp( object_name, "money" ) ){
     
        if( spr_money == 0 )
            spr_money = load_spr( "OBJ0.GFX", 0, 0, w, h, 1 );
        
        (*cur_object)->sprite_list[0] = spr_money;
        (*cur_object)->health = 1;
        (*cur_object)->speed_max = 0;
        (*cur_object)->speed = 0;
        (*cur_object)->animation_speed = 0;
        (*cur_object)->contact = snd_money;
        
    }
    
    return 0;
}

void delete_all_objects( void ){

    unsigned char cur_sprite;
    
    cur_object = &object_head;
    
    while( *cur_object ){
        
        /*for( cur_sprite = 0; cur_sprite < (*cur_object)->num_sprites; cur_sprite++ )
            delete_sprite( (*cur_object)->sprite_list[ cur_sprite ] );*/
        
        free( (*cur_object)->sprite_list );
        farfree( (*cur_object)->buffer_section );
        free( (*cur_object)->object_name );
        free( *cur_object );
        
        *cur_object = (*cur_object)->next;
    }
    
}
void print_object_list( void ){

    cur_object = &object_head;
    while( *cur_object ){
        printf("%s X: %d\n", (*cur_object)->object_name, (*cur_object)->x );
        cur_object = &(*cur_object)->next;
    }
    
}

void object_controller( unsigned short player_x, unsigned short player_y ){
    
    struct Object *temp = 0;
    unsigned short obj_x, obj_y, obj_w, obj_h, obj_health;
    unsigned char cur_sprite, cur_frame;
    
    cur_object = &object_head;
    while( *cur_object ){
     
        obj_x = (*cur_object)->x;
        obj_y = (*cur_object)->y;
        obj_w = (*cur_object)->w;
        obj_h = (*cur_object)->h;
        
        obj_health = (*cur_object)->health;
        
        (*cur_object)->x += (*cur_object)->x_dir;
        (*cur_object)->y += (*cur_object)->y_dir;
        
        if( player_x < obj_x + obj_w &&
            player_x + PLAYER_WIDTH > obj_x &&
            player_y < obj_y + obj_h &&
            player_y + PLAYER_HEIGHT > obj_y ){
            
                if( obj_x + obj_w > phys_view_x &&
                    obj_x < phys_view_x + SCREEN_BUFFER_WIDTH - TILE_WIDTH &&
                    obj_y + obj_h > phys_view_y &&
                    obj_y < phys_view_y + SCREEN_BUFFER_HEIGHT ){
            
                        draw_buffer_section( (*cur_object)->buffer_section, obj_w, obj_h,
                                            (*cur_object)->prev_x - phys_view_x, (*cur_object)->prev_y - phys_view_y);
                }
                
                SB16_play_sound( snd_money );
            
                /*for( cur_sprite = 0; cur_sprite < (*cur_object)->num_sprites; cur_sprite++ )
                    delete_sprite( (*cur_object)->sprite_list[ cur_sprite ] );*/
        
                free( (*cur_object)->sprite_list );
                farfree( (*cur_object)->buffer_section );
                free( (*cur_object)->object_name );
        
                temp = *cur_object;
        
                *cur_object = (*cur_object)->next;
        
                free( temp );
        }else
            cur_object = &(*cur_object)->next;
    }

    cur_object = &object_head;
    while( *cur_object ){
    
        obj_x = (*cur_object)->x;
        obj_y = (*cur_object)->y;
        obj_w = (*cur_object)->w;
        obj_h = (*cur_object)->h;
        
        if( obj_x + obj_w > phys_view_x &&
            obj_x < phys_view_x + SCREEN_BUFFER_WIDTH - TILE_WIDTH &&
            obj_y + obj_w > phys_view_y &&
            obj_y < phys_view_y + SCREEN_BUFFER_HEIGHT ){
        
            copy_buffer_section( (*cur_object)->buffer_section, obj_w, obj_h, 
                                    (*cur_object)->x - phys_view_x, (*cur_object)->y - phys_view_y );
        }
        
        cur_object = &(*cur_object)->next;
    }
    
    cur_object = &object_head;
    while( *cur_object ){
    
        obj_x = (*cur_object)->x;
        obj_y = (*cur_object)->y;
        obj_w = (*cur_object)->w;
        obj_h = (*cur_object)->h;
        cur_frame = (*cur_object)->cur_frame;
        cur_sprite = (*cur_object)->cur_sprite;
        
        if( obj_x + obj_w > phys_view_x &&
            obj_x < phys_view_x + SCREEN_BUFFER_WIDTH - TILE_WIDTH &&
            obj_y + obj_h > phys_view_y &&
            obj_y < phys_view_y + SCREEN_BUFFER_HEIGHT ){
            
            sprite_to_buffer( (*cur_object)->sprite_list[ cur_sprite ]->data[ cur_frame ], 
                              (*cur_object)->x - phys_view_x, (*cur_object)->y - phys_view_y, obj_w, obj_h );
        }
        
        cur_object = &(*cur_object)->next;
    }
}

#endif


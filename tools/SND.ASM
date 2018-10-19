                    public                              _SB_set_irq
                    public                              _SB_reset_irq
                    
                    IDEAL

                    P286

                    MODEL                               MEDIUM, C

                    DATASEG
                    
                    old_offs            DW              ?
                    old_seg             DW              ?
                    int_num             DB              ?
                    chain               DD              ?
                    
                    ENDS

                    SEGMENT CODE PARA PUBLIC 'CODE' USE16
                    ASSUME CS:CODE, DS:@data
                    START:                  

                    PROC SB_int_handler
                    push                ax dx
                    mov                 dx,             22FH
                    in                  al,             dx
                    mov                 al,             20H
                    out                 20H,            al
                    pop                 dx ax
                    
                    iret
                    ENDP SB_int_handler
                    
                    PROC _SB_set_irq
                    ARG irq:BYTE
                    push                ax bx cx dx ds es
                    mov                 al,             [irq]
                    add                 al,             8
                    mov                 [int_num],      al
                    mov                 ah,             35H
                    int                 21H
                    mov                 [word ptr chain],        es
                    mov                 [word ptr chain + 2],    bx
                    mov                 [old_offs],     bx
                    mov                 [old_seg],      es
                    cli                 
                    push                ds
                    mov                 al,             [int_num]
                    mov                 ah,             25H
                    mov                 bx,             seg SB_int_handler
                    mov                 ds,             bx
                    mov                 dx,             offset SB_int_handler
                    int                 21H
                    pop                 ds
                    mov                 bx,             1
                    mov                 cl,             [irq]
                    shl                 bx,             cl
                    not                 bx
                    in                  al,             21H
                    and                 al,             bl
                    out                 21H,            al
                    pop                 es ds dx cx bx ax
                    sti
                    ret
                    ENDP _SB_set_irq
                    
                    PROC _SB_reset_irq
                    ARG irq:BYTE
                    push                ax bx cx dx ds es
                    mov                 bx,             1
                    mov                 cl,             [irq]
                    shl                 bx,             cl
                    cli
                    in                  al,             21H
                    or                  al,             bl
                    out                 21H,            al
                    mov                 al,             [int_num]
                    mov                 dx,             [old_offs]
                    mov                 ds,             [old_seg]
                    mov                 ah,             25H
                    int                 21H
                    pop                 es ds dx cx bx ax
                    sti
                    ret
                    ENDP _SB_reset_irq
                    
                    ENDS

                    END START


                        public  _reset_timerISR
                        public  _set_timerISR
                        public  _set_freq
    
                        EXTRN   _timer:WORD
                        
                        IDEAL

                        P286
                        MODEL                                   MEDIUM, C
                        
                        DATASEG

                        ENDS
                        
                        SEGMENT CODE PARA PUBLIC 'CODE' USE16
                        ASSUME cs:CODE, ds:@data

                START:
                
                oldint_1C	dd	?
	dos_clk_adj	db	0
	dos_clk_cnt	db	0
	freq_div	dd	1193180
    
                        proc	_reset_timerISR

	xor	ax,	ax
	mov	es,	ax
	cli
	mov	ax,	[word ptr oldint_1C]
	mov	[es:1ch*4], ax
	mov	ax,	[word ptr oldint_1C + 2]
	mov	[es:1ch*4 + 2], ax
	sti
	
	ret
	endp	_reset_timerISR
	
proc	_set_timerISR
	xor	ax,	ax
	mov	es,	ax
	mov	ax,	[es:1ch*4]
	mov	[word ptr oldint_1C], ax
	mov	ax,	[es:1ch*4 + 2]
	mov	[word ptr oldint_1C + 2], ax

	cli
	mov	[word ptr es:1ch*4], offset timerISR
	mov	[es:1ch*4 + 2], cs
	sti

	ret
	endp	_set_timerISR
	
proc    timerISR
	push	ds
	pushf

	mov	ax,	SEG _timer
	mov	ds,	ax
	
	cli
	
	mov	ax,	[ds:_timer]
	test	ax,	ax
	jz	set_msec 	;if timer is zero, leave it alone
	dec	ax
set_msec:
	mov	[ds:_timer],	ax

	popf
	pop	ds
	
	cmp	[dos_clk_cnt],	0
	jne	skip_chain

	mov	al,		[dos_clk_adj]
	mov	[dos_clk_cnt],	al
	
	jmp	[oldint_1C]
skip_chain:

	dec	[dos_clk_cnt]

	mov	dx,	020h
	mov	al,	020h
	out	dx,	al

	sti
	iret
	endp	timerISR
	
proc _set_freq
	push 	bp
	mov 	bp,	sp

	mov	dx,	043h
	mov	al,	03ch
	out	dx,	al

	mov	dx,	040h

	mov	cl,	[bp+6]
	mov	al,	cl
	sal	al,	1
	
	mov	[dos_clk_adj],	al
	
	mov	ax,	65535
	shr	ax,	cl

	out	dx,	al
	xchg	al,	ah
	out	dx,	al
	
	pop	bp
	ret
	endp _set_freq

                        NOLOCALS
                        END START

public _set_keyboard
public _reset_keyboard

EXTRN          _keybuff:BYTE
EXTRN          _old_off:WORD
EXTRN          _old_seg:WORD

IDEAL

P286

MODEL               MEDIUM, C

DATASEG



ENDS

SEGMENT CODE PARA PUBLIC 'CODE' USE16
ASSUME cs:CODE, ds:@data
LOCALS
START:



proc _set_keyboard
		
	xor	ax,		ax
	mov	es,		ax

	cli		
	mov	ax,		[word ptr es:9*4+2]
	mov	[_old_seg],		ax
	
	mov	ax,		[word ptr es:9*4]
	mov	[_old_off],		ax
	
	mov	[word ptr es:9*4],	offset squeakey_driver
	mov	[word ptr es:9*4+2],	cs
	sti
	
	mov ax,                seg _keybuff
	mov es,                ax
	mov di,                offset _keybuff
	mov al,                0
	mov cx,                128
	rep stosb
	
	ret
	endp _set_keyboard

proc _reset_keyboard
	USES	es, ax, bx, cx, dx
	xor	ax,		ax
	mov	es,		ax

	cli
	mov	ax,		[cs:_old_off]
	mov	[word ptr es:9*4], ax
	mov	ax,		[cs:_old_seg]
	mov	[word ptr es:9*4+2], ax
	sti

	ret
	endp _reset_keyboard
	
proc squeakey_driver
	push	ax bx cx dx ds

	mov	ax,	SEG _keybuff
	mov	ds,	ax
	
	; read keyboard scan code
	in      al, 60h

	; update keyboard state
	xor     bh, bh
	mov     bl, al
	and     bl, 7Fh             ; bx = scan code
	shr     al, 7               ; al = 0 if pressed, 1 if released
	xor     al, 1               ; al = 1 if pressed, 0 if released
	mov     [ds:_keybuff+bx], al

	; send EOI to XT keyboard
	in      al, 61h
	mov     ah, al
	or      al, 80h
	out     61h, al
	mov     al, ah
	out     61h, al

        ; send EOI to master PIC
	mov     al, 20h
	out     20h, al

	pop	ds dx cx bx ax
	
	iret
	endp squeakey_driver
NOLOCALS
ENDS
END START

PUBLIC              _blackout_bg
PUBLIC              _draw_string
PUBLIC              _draw_rect_fill
PUBLIC              _draw_arrow

IDEAL

P286

MODEL               MEDIUM, C

DATASEG

arrow_up            db              00011000B
                    db              00100100B
                    db              01000010B
                    db              01111110B
                    
arrow_down          db              01111110B
                    db              01000010B
                    db              00100100B
                    db              00011000B
                    
arrow_right         db              01111110B
                    db              01000001B
                    db              01000001B
                    db              01111110B

ENDS

SEGMENT CODE PARA PUBLIC 'CODE' USE16
ASSUME cs:CODE, ds:@data
LOCALS
START:

;
;
;       blackout_bg
;       blanks out tile map area with black
;       used mostly for menu screens
;
;       Assumes ES is pointing to video memory
PROC                _blackout_bg
USES                DI
mov                 ax,             0A000H
mov                 es,             ax
mov                 al,             2
mov                 di,             80
mov                 cx,             180
@@loopy:
push                cx
mov                 cx,             240
rep                 stosb
add                 di,             80
pop                 cx
loop                @@loopy
ret
ENDP

;
;
;       draw_string PROC
;       SI DI is destroyed
;       
PROC                _draw_string
ARG                 x:WORD, y:WORD, chr:WORD, color:BYTE
LOCAL               nl:WORD
USES                SI, DI
mov                 ax,             0A000H
mov                 es,             ax
mov                 di,             [y]                                 ;get position in screen
mov                 bx,             di
shl                 di,             8
shl                 bx,             6
add                 di,             bx
add                 di,             [x]
mov                 [nl],           di
mov                 bx,             [chr]
string_loop:
xor                 ax,             ax
mov                 al,             [bx]
inc                 bx
cmp                 ax,             000AH
jne                 nonextline
add                 [nl],           320 * 9
mov                 di,             [nl]
jmp @@done
nonextline:
mov                 si,             8
mul                 si
mov                 si,             ax
add                 si,             0FA6EH
push                ds
mov                 ax,             0F000H
mov                 ds,             ax
mov                 ah,             [color]
mov                 cx,             8
@@loopy:
lodsb
push                cx
mov                 cx,             8
@@loopx:
shl                 al,             1
jnc                 @@skip
mov                 [es:di],        ah
@@skip:
inc                 di
loop                @@loopx
add                 di,             312
pop                 cx
loop                @@loopy
pop                 ds
sub                 di,             2552
@@done:
cmp                 [byte ptr bx],  0
jne                 string_loop
ret
ENDP                _draw_string

PROC                _draw_rect_fill
ARG                 x:WORD, y:WORD, w:WORD, h:WORD, c:BYTE
USES                SI, DI
mov                 ax,             0A000H
mov                 es,             ax
mov                 di,             [y]
mov                 bx,             di
shl                 bx,             6
shl                 di,             8
add                 di,             bx
add                 di,             [x]
mov                 dx,             320
sub                 dx,             [w]
mov                 al,             [c]
mov                 cx,             [h]
@@loopy:
push                cx
mov                 cx,             [w]
rep                 stosb
add                 di,             dx
pop                 cx
loop                @@loopy
ret
ENDP                _draw_rect_fill

;
;
;                   draw_button PROC
;                   ES is pointing to video memory dick
;                   every other register is fucked. cept' SI and DI
;
;                   NOTE: only works on backgrounds that aren't BLACK
;                   racist fucking procedure, im gonna tase it.
;
;
PROC                _draw_arrow
ARG                 x:WORD, y:WORD, arrow:WORD, color:BYTE
USES                SI, DI
mov                 ax,             0A000H
mov                 es,             ax
mov                 di,             [y]
mov                 bx,             di
shl                 di,             8
shl                 bx,             6
add                 di,             bx
add                 di,             [x]
cmp                 [arrow],        1
je                  up_arrow
cmp                 [arrow],        4
je                  right_arrow
down_arrow:
mov                 bx,             offset arrow_down
jmp                 @@done
up_arrow:
mov                 bx,             offset arrow_up
jmp                 @@done
right_arrow:
mov                 bx,             offset arrow_right
@@done:
mov                 cx,             4
mov                 al,             [color]
@@loopy:
mov                 dh,             [bx]
push                cx
mov                 cx,             8
@@loopx:
shl                 dx,             1
jnc                 @@skip
mov                 [es:di],        al
@@skip:
inc                 di
loop                @@loopx
add                 di,             320 - 8
inc                 bx
pop                 cx
loop                @@loopy
ret
ENDP                _draw_arrow

NOLOCALS
ENDS
END START

;формування і виведення на екран графіка функції y=sin(x)*cos(x)
.386
;для визначення масштабу
scale   macro   p1
    fld max_&p1
    fsub    min_&p1
    fild    max_crt_&p1
    fdivp   st(1), st(0)
    fstp    scale_&p1
endm
 

Data  segment use16
    min_x   dq  -8.0	; мінімальне значення по осі х
    max_x   dq  8.0	; максимальне значення по осі х	
    max_crt_x   dw 320	; максимальна кількість точок                                     
                        ; на екрані по осі х

    crt_x   dw  ?	; екранна координата по осі х
    scale_x dq  ?       ; масштаб по осі х
    x       dq  ?
 
    min_y   dq  -5.0	; мінімальне значення по осі х
    max_y   dq  5.0	; максимальне значення по осі х
    max_crt_y   dw  200	; максимальна кількість точок                                     
                        ; на екрані по осі х
    crt_y   dw  ?	; екранна координата по осі х
    scale_y dq  ?	; масштаб по осі х
    y       dq  ?

    step    dq  0.001
    tmp     dw  ? 
	constant dq 8.0

Data  ends
 
Code   segment use16
    assume  cs:Code, ds:Data
@Main:
    mov ax, Data
    mov ds, ax
 
    mov ax, 13h		; завдання графічного режиму 320х200
    int 10h
 
    finit		; ініціалізація співпроцесора		
 
    ;обчислення масштабного коефіцієнта по осях
    scale   x
    scale   y


    ;виводимо осі координат
    call    draw_x_and_y
 
    ; виводимо-будуємо графіки
    push 1                      ;(1 - синій)
    push offset cos_graph      
    call draw_graph

    push 2                  ; (2 - зелений)
    push offset abs_cos_graph
    call draw_graph

    push 4                  ; (4 - червоний)
    push offset sqrt_abs_cos_graph
    call draw_graph
    
    add sp, 4
 
    mov ah, 8			;затримка
    int 21h
    mov ax, 3			; завдання текстового режиму
    int 10h
    mov ax, 4C00h		; вихід
    int 21h
 

; обчислення y = cos(x)
cos_graph proc
    fld x         ; завантажити x
    fcos          ; обчислити cos(x)
    ret
cos_graph endp

; обчислення y = |cos(x)|
abs_cos_graph proc
    fld x         ; завантажити x
    fcos          ; обчислити cos(x)
    fabs          ; взяти модуль (абсолютне значення)
    ret
abs_cos_graph endp

; обчислення y = sqrt(|cos(x)|)
sqrt_abs_cos_graph proc
    fld x         ; завантажити x
    fcos          ; обчислити cos(x)
    fabs          ; взяти модуль
    fsqrt         ; взяти корінь квадратний
    ret
sqrt_abs_cos_graph endp

;обчислення y=x
x_graph   proc        	
    fld 	x          	
	ret
x_graph   endp


;побудова графіка, параметри в стеку (див. виклик) 
draw_graph  proc
    push    bp
    mov     bp, sp
    fld     min_x
 
@1:
    fst     x
    fld     st(0)
    call    word ptr [bp+4]
    fstp    y
    fld     x
    call    get_x
    fld     y
    call    get_y
    push    word ptr [bp+6]
    call    draw_pixel
    add     sp, 2
 
    fld     step
    faddp   st(1), st(0)
    fcom    max_x
    fstsw   ax
    sahf
    jna @1
    ffree   st(0)
    pop bp
    ret
draw_graph    endp
 
; переводимо значення х в екранні координати
get_x   proc
    fsub    min_x
    fdiv    scale_x
    frndint		
    fistp   crt_x
    ret
get_x   endp

; переводимо значення у в екранні координати 
get_y   proc

    fcom    min_y
    fstsw   ax
    sahf
    jc      @minus

    fcom    max_y
    fstsw   ax
    sahf
    ja      @plus

    fsub    min_y
    fdiv    scale_y
    frndint		; округлюємо st(0)
    fistp   crt_y
    mov ax, max_crt_y
    sub ax, crt_y
    mov crt_y, ax
    jmp @end_y

    @minus:
    fstp    st(0)
    mov     ax , max_crt_y
    mov     crt_y, ax
    jmp @end_y

    @plus:
    fstp    st(0)
    mov     crt_y, 0
    jmp @end_y

    @end_y:
    ret
get_y   endp
 
; виводимо точку на екран записом у відеопам'ять, в dl - код кольору точки
draw_pixel  proc
    push    bp
    mov bp, sp
    mov ax, 0A000h
    mov es, ax
    mov si, crt_y
    mov di, crt_x
    cmp si, max_crt_y
    je @end_draw
    cmp di, max_crt_x
    je @end_draw
    mov ax, max_crt_x
    mul si
    add ax, di
    mov bx, ax
    mov dx, [bp+4]
    mov byte ptr es:[bx], dl
@end_draw:
    pop bp
    ret
draw_pixel  endp
 
; малюємо кординатні осі x та y  
 
draw_x_and_y   proc
    fldz		; до st(0) записуємо 0
    call    get_y
    mov crt_x, 0
    mov cx, max_crt_x
@x_axes:
    push    15
    call    draw_pixel
    add sp, 2
    inc crt_x
    loop    @x_axes
 
    fld max_x
    fsub    min_x
    frndint
    fistp   tmp
    mov cx, tmp
 
    fld min_x
    frndint
    dec crt_y
 
    fldz
    call    get_x
    mov crt_y, 0
    mov cx, max_crt_y
@y_axes:
    push    15
    call    draw_pixel
    add sp, 2
    inc crt_y
    loop    @y_axes
 
    fld max_y
    fsub    min_y
    frndint
    fistp   tmp
    mov cx, tmp
 
    fld min_y
    frndint
    dec crt_x

    ret
draw_x_and_y   endp
 
Code   ends
    end @Main
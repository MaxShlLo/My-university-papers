.386
;======================================================
;		МОДЕЛЬ МУЛЬТИПРОГРАМНОЇ СИСТЕМИ
;======================================================
  max_prg		equ		10	;максимальна кількість "одночасно"
                                              		; виконуваних  задач
  time_slice		equ		655; кількість мікросекунд, виділених
; на один квант часу
 						; (максимальне значення 65535)

  _ST			SEGMENT		WORD STACK 'stack' use16
                  		dw      	32000 dup (?)
  top 			label		word
   			dw		400 dup (?) ; резерв для DosBox та помилок
   ; антипереповнення стека фонової задачі
  _ST			ENDS

  _DATA         SEGMENT    WORD PUBLIC 'DATA' use16

@ms_dos_busy	dd	(?)	; логічна адреса ознаки зайнятості MS-DOS

  int8set	db	0	; ознака перехоплення переривання від таймера
  int9set	db	0	; ознака перехоплення переривання від клавіатури

fon		equ	max_prg	; ознака фонової задачі;
fonsp		label 	word		; адреса збереження SP фонової задачі
sssp		dd	top		; логічна адреса стека фонової задачі

; масив значень SP для задач (для стеку кожної задачі – 1000 слів)
; задані початкові значення SP для 16 задач (від задачі 0 до задачі 15)

stp		dw	 1000,2000,3000,4000
dw	 5000,6000,7000,8000
		dw	 9000,10000,11000,12000
dw	 13000,14000,15000,16000

nprg 		dw	0	; номер активної задачі (від 0 до max_prg-1)
	; або ознака фонової задачі (fon = max_prg)
; масив стану задач
Init		db	16 dup (0)
; можливі стани:
; Ready (0) - задача готова до початкового запуску
;Execute (1)- задача виконується (виділені їй кванти часу не вичерпались)
;Hesitation (2) - задача чекає на чергові кванти часу
;Close (4)- задача закінчила свою роботу
;Stop (8)- задача зупинена (їй не виділяються кванти часу)
;Absent (16)- задача відсутня

forceStopDone db 0

; масив дозволеного числа квантів задач
clock  	db   	16 dup (1)
; задачі з більшим пріоритетом мають більшу кількість 
; дозволених квантів

; масив лічильників квантів задач 
 clockt     	db    	16 dup (0)
; скільки квантів із дозволеної кількості задача вже використала
; якщо ще не всі кванти використані, то задача продовжує роботу

screen_addr	dw	16 dup (0) 		; адреса (зміщення від початку 
; відеосторінки) області введення на екран даних задачі
; область виведення задачі – рядок на екрані обмеженої довжини, яка
; задається при запуску задачі

; масив імен задач
names	label		word
db 		'0T1T2T3T4T5T6T7T8T9TATBTCTDTETFT'

clk		dw		0	;лічильник переривань від таймера 
;(для визначення, що системний таймер працює)

_DATA	ENDS

_TEXT	 SEGMENT	 BYTE PUBLIC 'CODE' use16
 ASSUME		 CS:_TEXT,DS:_DATA

;------------------------------------------------------------
; процедура "перехоплення" переривання від таймера (int8)
;------------------------------------------------------------
setint8	PROC

;------------------------------------------------------------
mov	al,int8set
or		al,al    	; контроль "перехоплення" перехоплень
jnz	zero_8		;
MOV	AH,35H		; отримати вектор переривання
MOV	AL,8			; переривання від таймера (8)
INT	21H 			; значення що повертається:
					; es:bx – логічна адреса
					; системної процедури
					; обробки переривання від таймера

mov	cs:int8ptr,bx	; зберегти логічну адресу системної
mov	cs:int8ptr+2,es	; процедури в сегменті кодів

mov	dx,offset userint8	;формування в ds:dx логічної
push	ds			; адреси процедури користувача
push	cs			; для обробки переривань від таймера
pop	ds

MOV	AH,25H		; встановити новий вектор
MOV  	AL,8		; переривання від таймера
INT	21H 			; ds:dx – покажчик на користувацьку
; процедуру обробки переривання від таймера

mov	ax,time_slice	; встановити задану величину кванту часу
out 	40h,al		; 40h – адреса 8-розрядного порту таймера, 
 					; через який задають період таймера 
; спочатку молодший байт,
; а потім старший (в захищеному режимі це не ;можливо)

jmp 	$+2			; стандартний метод узгодження швидкісного
					; процесора з більш повільним зовнішнім
					; пристроєм. Припускаємо, що
					; "безглузда" команда jmp очищує буфер
					; попередньої вибірки команд і, тим самим,
					; уповільнює роботу процесора. Тим часом
					; зовнішній пристрій буде готовий 
					; прийняти наступний байт
	nop

mov	al,ah			; (старший байт)
	out	40h,al

pop	ds

mov	int8set,0ffh 		; заборона повторних входжень
zero_8:
ret
int8ptr	dw 	2 dup (?)
setint8	ENDP

;--------------------------------------------------------------------------
; Процедура відновлення вектору переривання від таймера
;--------------------------------------------------------------------------
retint8	PROC
;--------------------------------------------------------------------------
push	ds
push	dx

mov 	al,0ffh		; відновити нормальну роботу
out	40h,al 		; системного таймера
jmp	$+2
nop
out	40h,al
mov	dx,cs:int8ptr
mov	ds,cs:int8ptr+2

MOV	AH,25H		; відновити початковий вектор
MOV	AL,8			; переривання від таймера
INT	21H			; ds:dx – вказівник (логічна адреса) 
; на початкову (системну) процедуру
; обробки переривання від таймера
pop	dx
pop	ds
mov	int8set, 0h		; дозвіл наступних "перехоплень"
ret
retint8	ENDP

;------------------------------------------------------------
setint9	PROC
;-----------------------------------------------------------
;  процедура "перехоплення" переривання від клавіатури (int9)
;------------------------------------------------------------
mov	al,int9set
or	al,al
jnz	zero_9
MOV AH,35H	 	; отримати вектор переривання
MOV	AL,9			; переривання від клавіатури (9)
INT	21H			; значення що повертається:
					; es:bx – вказівник на 
; системну процедуру
					; обробки переривання від клавіатури

mov	cs:int9ptr,bx    	; зберегти в сегменті кодів вказівник 
mov	cs:int9ptr+2,es 	; на системну процедуру

mov	dx,offset userint9
push	ds
push	cs			; ds:dx – вказівник на 
					; процедуру користувача
pop	ds			; обробки переривання від клавіатури

MOV	AH,25H		; встановити вектор "перехоплення"
MOV	AL,9			; переривання від клавіатури (9)
INT	21H			; 
pop	ds

mov	int9set,0ffh		; заборона повторних входжень

zero_9:
ret
int9ptr	dw	2 dup (?)
setint9	ENDP

;--------------------------------------------------------------------------
;  Процедура відновлення попереднього (системного)
;  вектору переривання від клавіатури
;--------------------------------------------------------------------------
retint9	PROC
push 	ds
	push	dx
mov	dx,cs:int9ptr		; ds:dx – вказівник на
; початкову (системну)
mov	ds,cs:int9ptr+2		; процедуру обробки переривання від
						; клавіатури

MOV	AH,25H			; встановити вектор 
						; системної процедури
MOV	AL,9			 	; обробки переривання від клавіатури
INT	21H				; 
; 
pop	dx
pop		ds
mov		int9set,0h		; дозвіл наступних "перехоплень"
ret
retint9	ENDP


;------------------------------------------------------------
; Перероблена процедура CheckNumberKey 
; (При натисканні клавіш 1–0 – scan-коди 02h–0Bh)
; Обробляється подія key release.
; Логіка:
;   1. Отримати індекс задачі: '1' (02h) → 0, ..., '0' (0Bh) → 9.
;   2. Для всіх задач, що не співпадають з вибраним, встановити Init = Stop і обнулити clockt.
;   3. Для задачі, індекс якої співпадає з вибраним, встановити Init = Ready і
;      присвоїти їй повний квант (clockt = clock).
;   4. Якщо поточна активна задача (nprg) відрізняється від вибраної,
;      просто обнуляємо її clockt і встановлюємо nprg = вибраному.
;------------------------------------------------------------
CheckNumberKey proc near
    push    ax
    push    bx
    push    cx
    push    si
	push 	dx
    push    ds

    ; Перевірка, чи scan‑код належить клавішам '1'–'0'
    cmp     al, 02h         ; мінімальний scan‑код (1)
    jb      doneCheck
    cmp     al, 0Bh         ; максимальний scan‑код (0)
    ja      doneCheck

    sub     al, 2           ; '1' (02h) → 0, '2' (03h) → 1, …, '0' (0Bh) → 9
    xor     bx, bx          ; BX ← 0
    mov     bl, al          ; BX = вибраний індекс задачі

    ; Встановлюємо DS на _DATA для коректного доступу до змінних
    mov     ax, _DATA
    mov     ds, ax

    mov     cx, max_prg     ; CX = загальна кількість задач (наприклад, 10)
    xor     si, si         ; SI = 0
	xor     dx, dx         ; SI = 0

LoopTasks:
    cmp     si, bx         ; якщо поточний індекс дорівнює вибраному...
    je      HandleSelected
    ; Для всіх інших задач встановлюємо стан Stop і обнуляємо квант:
    ;mov     byte ptr [Init+si], stop
   ; mov     byte ptr [clockt+si], 0
    jmp     NextTask

Resume:
    mov     byte ptr [Init + si], ready  ; відновлюємо її роботу (можна встановити execute, за потребою)
    mov     al, byte ptr [clock + si]
    mov     byte ptr [clockt + si], al
	jmp     NextTask

Clos:
    mov     byte ptr [Init + si], close
	jmp     NextTask

HandleSelected:
    mov     al, byte ptr [Init + si]   ; читаємо стан вибраної задачі
    cmp     al, 0FFh
    je      Clos
	
	cmp     al, close
    je 		Resume

	cmp     al, stop
    je 		Resume
	

    ; Для вибраної задачі – встановлюємо її в стан Ready 
    ; та присвоюємо їй повний квант (запис із масиву clock)
    mov     byte ptr [Init+si], ready
    mov     al, clock[si]   ; завантажуємо максимальний квант для задачі
    mov     byte ptr [clockt+si], al  

	mov     cx, max_prg     ; CX = загальна кількість задач (наприклад, 10)
    xor     si, si         ; SI = 0
LoopStop:
	
	cmp     si, bx         ; якщо поточний індекс дорівнює вибраному...
    je      Skipbx

	mov     byte ptr [Init+si], stop
    mov     byte ptr [clockt+si], 0
    jmp     Skip

SkipBX:
cmp dx,1
je NextTask
mov dx,1
 jmp     Skip

Skip:
inc     si

    loop    LoopStop


NextTask:
    inc     si
    loop    LoopTasks

doneCheck:
    pop     ds
	pop 	dx
    pop     si
    pop     cx
    pop     bx
    pop     ax
    ret
CheckNumberKey endp





;-----------------------------------------------------------------------------------------------
; Процедура обробки переривань від клавіатури,
; викликається при будь-якому натисканні або відтисканні клавіш 
; клавіатури ПЕОМ,
; здійснює повернення в MS-DOS після відтискання клавіші Esc
;------------------------------------------------------------------------------------------------
userint9	proc	far
;----------------------------------------------------------------------------
esc_key	equ	01h		; скан-код клавіші Esc
pusha
push	es

; Зчитуємо "сирий" скан-код із клавіатури
    in      al,60h            ; отримуємо scan-код (старший біт встановлено, якщо це key release)
    mov     bl, al            ; зберігаємо сирий scan-код у BL
    ; Перевіряємо, чи це подія відпускання: 
    ; якщо старший біт НЕ встановлено, це key press – ігноруємо особливу обробку
	test    al, 80h
    jz      SkipSpecialKey

;in	al,60h		; ввести скан-код клавіші – розряди 0-6
mov	ah,bl			; 7-ий розряд дорівнює 0 при натисканні
and	al,7fh			; клавіші, 1– при відтисканні

push	ax			; al – "чистий" скан-код (без ознаки 
					; натискання - відтискання) 
push	2600
call	show			; відображення скан-коду на екрані

cmp	al,esc_key
je	ui9010

 ;mov     al, bl          ; відновлення scan-коду для CheckNumberKey
; Якщо це не Esc, перевіряємо, чи це клавіші 1–0 (scan-коди від 02h до 0Bh)
    cmp     al, 02h
    jb      SkipSpecialKey
    cmp     al, 0Bh
    ja      SkipSpecialKey

    ; Викликаємо спеціальну процедуру для обробки клавіш 1–0
	;call    ForceStopCurrentThread
    call    CheckNumberKey

SkipSpecialKey:
; (варіант 2)
    pop     es
    popa
    jmp     dword ptr cs:int9ptr   ; передаємо керування первинній системній процедурі

;pop	es
;popa
;jmp	dword ptr cs:int9ptr	; перехід на системну процедуру
; обробки переривань від клавіатури,
; яка виконає всі необхідні дії,
; включаючи повернення в перервану 
; програму

; (варіант 1)
; САМОСТІЙНЕ ВИРІШЕННЯ ПРОБЛЕМ З КЛАВІАТУРОЮ

ui9010:
mov	bx,ax
in	al,61h		; біт 7 порту 61h, призначений для введення
					; підтверджуючого імпульсу в клавіатуру; 
					; клавіатура блокується, поки не надійде
		; підтверджуючий імпульс
					;
	mov	ah,al
	or	al,80h		;					 |
	out	61h,al		; виведення на клавіатуру	└───┐
	jmp	$+2			;						  |
	mov	al,ah			;						  |
	out	61h,al		; підтверджуючого імпульсу	┌───┘
; при відповідному налаштуванні ОС посилання імпульсу 
;може бути не обов’язковим

	mov	al,20h		; розблокувати в контролері переривання
					; проходження запитів на переривання 
					; поточного та нижчого рівнів пріоритету,
	out	20h,al		; що забезпечить можливість наступного 
;переривання від клавіатури

	mov	ax,bx
	cmp	ah,al			; перевірка події переривання - від натискання
					; чи від відтискання клавіші клавіатури
	je	ui9040

ui9020: 			;відтискання клавіші esc_key
	push	es
	les		bx, @ms_dos_busy	; es:bx - адреса ознаки 
;зайнятості MS-DOS
	mov	al,es:[bx]		; ax - ознака зайнятості MS-DOS
	pop	es
	or	al,al			; перевірка:
; якщо була перервана робота MS-DOS
; в "невдалий" момент,
	jnz	ui9030		; тоді не можна від неї вимагати
					; виконання ряду функцій
					; (у загальному випадку, MS-DOS 
					; не забезпечує повторне входження)
	call	retint8
	call	retint9
	mov	ax,4c00h
	int	21h			; ЗАКІНЧИТИ РОБОТУ
; БАГАТОПРОГРАМНОЇ МОДЕЛІ
ui9030:
; найбільш ймовірна реакція користувача – ще раз натиснути клавішу,
;а поки користувач збереться повторно натиснути клавішу, ОС
;встигає виконати свої справи, а ми встигаємо відновити роботу 
;перерваної задачі

ui9040: 			; натискання клавіші esc_key
	pop	es		; відновити стек перерваної програми
	popa
	iret			; закінчити обробку переривання
; від ОС ми «приховали» факт натискання клавіші esc_key
userint9	endp




;------------------------------------------------------------
; процедура обробки переривання від таймера (менеджер квантів)
; коди стану задач (використовуються в масиві init)

ready		equ	0	; задача завантажена в пам’ять і
				; готова до початкового запуску
				; статус встановлюється поза менеджером квантів
execute	equ	1	; задача виконується
hesitation 	equ	2	; задача призупинена і чекає своєї черги
close		equ	4	; виконання задачі завершено
stop		equ	8	; задача зупинена 
				; статус встановлюється і змінюється
				; поза менеджера квантів
absent	equ	16	; задача відсутня 


;------------------------------------------------------------
userint8	PROC	far
;------------------------------------------------------------
	pushad			;збереження РОН в стеку перерваної задачі
	push	ds

; (варіант 3)
	pushf				;програмна імітація апаратного переривання
;ВІДМІТИМО – ознака дозволу на переривання (if) апаратурою скинута в 0.

call	cs:dword ptr int8ptr
	; виклик системної процедури обробки переривання int8,
	; яка, між іншим, розблокує 8-е переривання в контролері переривань, 
	; але апаратні переривання неможливі, оскільки if=0


	mov	ax, _DATA 		; в перерваній програмі вміст
	mov	ds, ax			; сегментного регістра ds,
					; у загальному випадку, може бути будь-яким

	inc	clk			; програмний лічильник
					; переривань від таймера
	push	clk			; може бути корисним при вивченні моделі
	push	2440
	call	show			; виведення на екран значення лічильника

	xor	esi,esi
	mov	si,nprg

 ; ---- Нова перевірка: якщо активна задача має статус Stop,
    ; то негайно переходимо до перемикання задач.
    mov     al, Init[si] 
    cmp     al, stop
    je      gotoSwitch

	cmp	si,fon			; перервана задача фонова ?
	je	disp005

				; перервана задача не фонова
	cmp	clockt[si],1 		; є ще не використані кванти ?
	jc	disp010

	dec	clockt[si]  		; зменшити лічильник квантів
	pop 	 ds
	popad			; продовжити виконання перерваної задачі
	iret
	
gotoSwitch:
mov     sp, fonsp       ; повертаємо стек фонового процесу
    mov     nprg, fon       ; встановлюємо фон як активну задачу
    pop     ds
    popad
    iret

disp005:			; перервана задача фонова
	mov	fonsp,sp
	mov	nprg,max_prg-1 	; забезпечити перегляд задач з 0-ї
	mov	cx,max_prg		; max_prg – max кількість задач
	jmp	disp015

disp010:			; перервана задача не фонова
	mov	stp[esi*2],sp 
	mov	init[si],hesitation	; призупинити поточну задачу
	mov	 cx,max_prg


disp015:
	; визначення задачі, якій необхідно передати управління
	mov	 di,max_prg+1
	sub	di,cx
	add 	di,nprg
	cmp	di,max_prg
	jc	disp018
	sub	di,max_prg
disp018:
	xor      	ebx,ebx
	mov    	bx,di
		;push	bx
		;push	3220
		;call	show

				; сх пробігає значення max_prg,max_prg-1,...,2,1
				; bx пробігає значення  nprg+1,nprg+2,..., max_prg- 1,
;0, ..., nprg
;
	cmp	init[bx],ready
	je	disp100		; перехід на початковий запуск задачі

	cmp	init[bx],hesitation
	je	disp020 			; перехід на відновлення роботи
						; наступної задачі
	loop	disp015

				; відсутні  задачі, які можна запустити
				; (перезапустити), тому 
						 
	mov	sp,fonsp			; установлюємо стек фонової задачі
	mov	nprg,fon
	pop	ds				; із стеку фонової задачі відновлюємо
	popad				; вміст регістрів
	iret					; повернення в фонову задачу
 

disp020:
				; відновлення роботи наступної задачі
		;push		bx
		;push		2480
		;call 	show
	mov	nprg,bx
	mov	sp,stp[ebx*2]
	mov	al,clock[bx]
	mov	clockt[bx],al		; встановити дозволену
						; кількість квантів
	mov	init[bx],execute		; стан задачі – задача виконується

	pop	ds
	popad
	iret

disp100:
				; першопочатковий запуск задачі
	mov	nprg,bx
	mov	sp,stp[ebx*2]
	mov	al,clock[bx]
	mov	clockt[bx],al		; встановити дозволену
						; кількість квантів
	mov	init[bx],execute

	push	names[ebx*2]		; ім'я задачі
	push	screen_addr[ebx*2]	; адреса "вікна" для задачі на екрані 
	push	22                               	; розрядність лічильника
	call	Vcount                       	; запуск


	xor	esi,esi
	mov	si,nprg                        	; на ax – номер задачі, яка
						; завершила свою роботу в межах
						; чергового кванту часу
	mov	init[si],close
	mov	sp,fonsp
	mov	nprg,fon
	pop	ds
	popad
	iret					; повернення в фонову задачу

userint8	ENDP
;-----------------------------------------------------------------------
; Vcount – процедура для моделювання незалежних задач 
; вхідні параметри:
;	1-й – ім'я задачі (два символи) [bp+8]
;	2-й – зміщення у відеосторінці "вікна" задачі [bp+6]
;	3-й – кількість двійкових розрядів лічильника [bp+4]
; Виконувані дії:
;	при запуску:
; - дозволяє переривання
; - створює в стеку 10-байтну область для локальних даних
; - розміщує в цій області за адресою [bp-2] залишок від ділення
;	3-го параметра на 32 (фактична розрядність лічильника –
;	перестраховка від помилок в завданні розрядності)
; - записує в цю область за адресою [bp-6] маску з числом
;	одиниць в молодших розрядів, що дорівнює фактичній 
;	розрядності лічильника
; - записує нуль у 4-х байтний лічильник за адресою [bp-10]

;   	у подальшому, в циклі:
; - виводить показники лічильника на екран
; - збільшує значення лічильника на 1
; - завершення задачі після переходу лічильника 
; 	зі стану "всі одиниці" в стан "всі 0"

Vcount	proc	near
;=====================================================
; процедура створює в локальній області лічильник на задану
; кількість двійкових розрядів, виконує операцію +1 в лічильник
; та виводить всі двійкові розряди лічильника в задане місце екрану
; початкове значення лічильника – 0
;При переповненні лічильника – перехід від "всіх одиниць" в стан
; "всі нулі"– процедура закінчує свою роботу.
;Процедура реєтрантна, дозволяє переривання та повторні входження
;Використовує всі РОН
;=====================================================
; параметри:
Rozr		equ 	[bp+4]		; задана розрядність лічильника
Screna	equ	[bp+6]
NameTask	equ 	[bp+8]
Screna	equ	[bp+6]
	push	bp
	mov	bp,sp
	sub	sp,10				; формування в стеку області для
						; збереження локальних даних
; локальні дані
Rozd	equ	[bp-2]				; дійсна розрядність лічильника
Maskr	equ	[bp-6]			; всі розряди в 1
Cnt	equ	[bp-10]			; лічильник

	sti

	push	es
	mov	ax,0b800h
	mov	es,ax

	mov	ax,rozr			; ax = задана кількість розрядів
; лічильника
	and	ax,31				; ax=ax mod 32 (для перестраховки)
	mov	rozd,ax		; дійсна кількість розрядів лічильника  <32
	mov	cx,ax
	mov	eax,001b
	shl	eax,cl
	dec	eax				; eax – маска з кількістю 1, що дорівнює
					  	; кількості розрядів лічильника
	mov  	Maskr,eax

	mov   	dword ptr Cnt,0   ; скидання лічильника

	mov	di, Screna			; виведення імені задачі
	mov 	dx, NameTask
	mov 	ah,1011b
	mov 	al,dh
	cld
	stosw
	mov 	al,dl
	stosw

	std					; підготовка до виведення лічильника
	add	di,cx				; починаючи з молодших розрядів
	add	di,cx
	mov	bx,di
	xor	edx,edx

l20:						; виведення значення лічильника
						; у двійковому форматі
	mov	di,bx
	mov	cx,rozd
	mov	ah,1010b			; 1010b – атрибут символу, 
; атрибут фону – 0  (чорний)
l40:
	mov	al,'0'
	shr	edx,1
	jnc	l60
	mov	al,'1'
l60:
	stosw
	loop 	l40

	inc	dword ptr Cnt		; +1 в лічильник
	mov	edx,dword ptr Cnt
	and	edx,Maskr			; перевірка на 0
	jnz	l20

	pop	es
	add	sp,10
	mov	ax,[bp+8]
	and	ax,0fh
	cli
	pop	bp
	ret	6
Vcount	endp

;=====
show	proc	near
	push 	bp
	mov	bp,sp
	pusha
	push	es
	mov	ax,0b800h
	mov	es,ax

	pushf
	std
ls20:
	mov	di,[bp+4]
	mov	bx,[bp+6]
	mov	cx,4
	mov	ah,0ah
ls40:
	mov	al,bl
	and	al,00001111b
	cmp	al,10
	jl	ls100
	add	al,7
ls100:
	add	al,30h
	stosw
	shr	bx,4
	loop	ls40

	popf
	pop	es
	popa
	pop	bp
	ret	4
show	endp
;------------------------------------------------------------
;------------Початок роботи моделі----------------
;------------------------------------------------------------
begin:
	mov 	ax,_data
	mov 	ds,ax

	mov	ax,3				; задати текстовий режим 80 х 25
	int	10h

	mov	ah,10h			; відключити режим миготіння
	mov	al,3
	mov	bl,0
	int	10h

	mov	cx,max_prg
	xor	esi,esi
	mov	bx,4
b10:
	mov	screen_addr[esi*2],bx	; заповнення таблиці адрес
						; виведення на екран даних задач
	mov	init[si],ready		; першопочаткове заповнення
						; таблиці стану задач
	add	bx,80
	inc	si
	loop	b10
;SETINT
	cli					; заборона переривань
						; переривання дозволяються
; в процедурі  Vcount
	mov	ah,34h
	int	21h			; es:bx – адреса ознаки зайнятості MS-DOS
	mov	word ptr @ms_dos_busy,bx
	mov	word ptr @ms_dos_busy+2,es

	call	setint8			; "перехоплення" int8
	call	setint9			; "перехоплення" int9

	lss	sp,sssp		; стек фонової задачі
	mov	nprg,fon
	push	'FN'
	push	1800
	push	30
	call	Vcount		; запуск фонової задачі
					; в процедурі Vcount установлюється дозвіл
					; на переривання і при чергових перериваннях
					; від таймера менеджер квантів (userint8) 
					; буде запускати інші задачі
					;
; управління в цю точку буде передано по команді RET при завершенні 
; фонової  задачі, а це можливо лише після завершення інших задач

	call	retint8		; відновлення системних векторів 
	call	retint9
	sti
	mov	ax,4c00h
	int	21h
_TEXT	ENDS

	end	begin

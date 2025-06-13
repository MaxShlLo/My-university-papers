.286

data segment
    dvalue  db 30 dup ('0', 0fh)  ; Резервування 30 десяткових розрядів
    len     dw 9                  ; Реальна кількість десяткових розрядів
    row     dw 18                 ; Рядок екрану для відображення
    col     dw 30                 ; Початкова позиція виводу
    color_flag db 0fh             ; Колір тексту (білий за замовчуванням)
data ends

code segment
assume cs:code, ds:data

start:
    push    data
    pop     ds
    push    0b800h
    pop     es

    

    mov ax, 0
    int 33h

    mov ax, 1
    int 33h

    ; Реєстрація обробника подій миші
    mov ax, 0ch
    mov cx, 1010b               ; Вибір подій: ЛКМ (натискання) і ПКМ (натискання)
    push es
    push cs
    pop es
    lea dx, prmaus
    int 33h
    pop es

    ; Основний цикл
main_loop:
    call write
    jmp main_loop

    ; Завершення програми
    mov ax, 4C00h
    int 21h

write proc
    cli                          ; Заборона переривань
    mov si, offset dvalue
    mov al, byte ptr row
    xor ah, ah
    imul ax, 160
    add ax, col 
    add ax, col
    mov di, ax

    mov cx, len
    cld
    rep movsw
    sti                          ; Дозвіл переривань
    ret
write endp

prmaus proc far
    push ds
    push es
    pusha                       ; Збереження всіх регістрів

    ; Завантаження сегментів
    push 0b800h                 ; Сегмент відеопам’яті
    pop es
    push data
    pop ds

    ; Перевірка натискання ЛКМ
    mov ax, bx                  ; Стан кнопок у регістрі BX
    and ax, 1                   ; Перевірка ЛКМ
    jz check_right_button       ; Якщо ЛКМ не натиснута, перевіряємо ПКМ

    ; Інверсія кольору
    xor color_flag, 0Fh         ; Інверсія кольору між чорним і білим

    ; Розрахунок початкової адреси числа у відеопам’яті
    mov ax, row                 ; Встановлюємо рядок, де знаходиться число
    xor ah, ah                  ; Очищуємо старший байт
    imul ax, 160                ; Рядок * 160 байтів (80 символів × 2 байти на символ)
    add ax, col                 ; Додаємо початкову колонку числа
    shl ax, 1                   ; Символ + атрибут = 2 байти
    mov di, ax                  ; DI тепер вказує на адресу у відеопам’яті

    ; Проходимо через усі символи числа
    mov cx, len                 ; Кількість символів у числі
change_color_loop:
    mov al, color_flag          ; Завантажуємо новий колір
    mov byte ptr es:[di+1], al  ; Змінюємо атрибут кольору (другий байт кожного символа)
    add di, 2                   ; Переходимо до наступного символа
    loop change_color_loop      ; Повторюємо для всіх символів числа

    jmp end_prmaus              ; Завершуємо обробку ЛКМ

check_right_button:
    ; Перевірка натискання ПКМ
    and bx, 2                   ; Якщо ПКМ натиснута
    jz end_prmaus               ; Якщо ні, вихід
    mov ax, 4C00h               ; Завершення програми
    int 21h

end_prmaus:
    popa                        ; Відновлення всіх регістрів
    pop es
    pop ds
    sti
    retf                        ; Завершення процедури
prmaus endp




code ends
end start

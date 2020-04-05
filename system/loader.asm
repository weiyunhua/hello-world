%include "inc.asm"

org 0x9000

jmp ENTRY_SEGMENT

[section .gdt]           ; 定义 .gdt 代码段
; GDT definition
;                                  段基址           段界限       段属性
GDT_ENTRY       :   Descriptor        0,               0,          0
CODE32_DESC     :   Descriptor        0,      Code32SegLen - 1,    DA_C + DA_32
;                                                                  32位可执行代码段
VIDEO_DESC      :   Descriptor     0xB8000,         0x07FFF,       DA_DRWA + DA_32
; 显存段描述符                  显存的起始地址   0XBFFFF - 0XB8000 可读可写并且之前已经访问过了的, 32 位代码段
DATA32_DESC     :   Descriptor        0,      Data32SegLen - 1,    DA_DR + DA_32
; 定义数据段, 用于定义只读数据
STACK32_DESC    :   Descriptor        0,        TopOfStack32,      DA_DRW + DA_32
; 定义栈空间, 用于保护模式下的函数调用
CODE16_DESC     :   Descriptor        0,            0xFFFF,        DA_C
; 16位保护模式
UPDATE_DESC     :   Descriptor        0,            0xFFFF,        DA_DRW
; 高速刷新缓存器
TASK_A_LDT_DESC :   Descriptor        0,        TaskALdtLen - 1,   DA_LDT
; 局部描述符 TASK A
; GDT end

GdtLen    equ   $ - GDT_ENTRY  ; 计算全局描述符段的长度

GdtPtr:                        ; 相当于 C 中的 struct
          dw    GdtLen - 1     ; 两个字节的界面
          dd    0              ; 四个字节的起始地址

; GDT Selector
; CODE32_DESC 的选择子, 全局段描述符中的选择子地址 (下标 << 3), 属性 SA_TIG, 特权级 SA_RPL0
Code32Selector   equ (0x0001 << 3) + SA_TIG + SA_RPL0
VideoSelector    equ (0x0002 << 3) + SA_TIG + SA_RPL0
Data32Selector   equ (0x0003 << 3) + SA_TIG + SA_RPL0
Stack32Selector  equ (0x0004 << 3) + SA_TIG + SA_RPL0
Code16Selector   equ (0x0005 << 3) + SA_TIG + SA_RPL0
UpdateSelector   equ (0x0006 << 3) + SA_TIG + SA_RPL0
TaskALdtSelector equ (0x0007 << 3) + SA_TIG + SA_RPL0
; end of [section .gdt]

TopOfStack16   equ  0x7c00

; 定义数据段, 定义只读数据
[section .dat]
[bits 32]
DATA32_SEGMENT:
    DTOS               db  "D.T.OS!", 0     ; 打印字符串是 D.T.OS!, 以 0 结束
    DTOS_OFFSET        equ DTOS - $$        ; 得到字符串相对于数据段起始地址的偏移量
    HELLO_WORLD        db  "Hello World!", 0
    HELLO_WORLD_OFFSET equ HELLO_WORLD - $$

Data32SegLen equ $ - DATA32_SEGMENT

[section .s16]      ; 实模式的代码段
[bits 16]
ENTRY_SEGMENT:
    mov ax, cs      ; 初始化
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, TopOfStack16

    mov [BACK_TO_REAL_MODE + 3], ax ; 填入 cs 寄存器的值

    ; initialize GDT for 32 bits code segment
    mov esi, CODE32_SEGMENT
    mov edi, CODE32_DESC

    call InitDescItem               ; 初始化 CODE32_DESC 段基址

    ; initialize GDT for 32 bits data segment
    mov esi, DATA32_SEGMENT
    mov edi, DATA32_DESC

    call InitDescItem               ; 初始化 DATA32_DESC 段基址

    mov esi, STACK32_SEGMENT
    mov edi, STACK32_DESC

    call InitDescItem               ; 初始化 STACK32_DESC 段基址

    mov esi, CODE16_SEGMENT
    mov edi, CODE16_DESC

    call InitDescItem               ; 初始化 CODE16_DESC 段基址

    mov esi, TASK_A_LDT_ENTRY
    mov edi, TASK_A_LDT_DESC

    call InitDescItem               ; 初始化 TASK_A_LDT_DESC 段基址

    mov esi, TASK_A_CODE32_SEGMENT
    mov edi, TASK_A_CODE32_DESC

    call InitDescItem               ; 初始化 TASK_A_CODE32_DESC 段基址

    mov esi, TASK_A_DATA32_SEGMENT
    mov edi, TASK_A_DATA32_DESC

    call InitDescItem               ; 初始化 TASK_A_DATA32_DESC 段基址

    mov esi, TASK_A_STACK32_SEGMENT
    mov edi, TASK_A_STACK32_DESC

    call InitDescItem               ; 初始化 TASK_A_STACK32_DESC 段基址

    ; initialize GDT pointer struct
    mov eax, 0
    mov ax, ds
    shl eax, 4
    add eax, GDT_ENTRY
    mov dword [GdtPtr + 2], eax  ; 计算偏移地址放到 GdtPtr dd 处

    ; 1. load GDT
    lgdt [GdtPtr]

    ; 2. close interrupt
    cli

    ; 3. open A20
    in al, 0x92
    or al, 00000010b              ; 将对应位置 1 表示打开 A20 地址线
    out 0x92, al

    ; 4. enter protect mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; 5. jump to 32 bits code
    jmp dword Code32Selector : 0  ; 因为要刷新流水线, 因此需要用 jmp 跳转

BACK_ENTRY_SEGMENT:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, TopOfStack16

    in al, 0x92
    and al, 11111101b
    out 0x92, al                    ; 关闭 A20 地址线

    sti                             ; 打开中断

    mov bp, HELLO_WORLD
    mov cx, 12
    mov dx, 0
    mov ax, 0x1301
    mov bx, 0x0007
    int 0x10

    jmp $

; esi    --> code segment label
; edi    --> descriptor label
; 根据代码段的标签来初始化对应段
InitDescItem:
    push eax

    mov eax, 0
    mov ax, cs
    shl eax, 4                  ; 将当前地址 << 4 位
    add eax, esi                ; 得到 32 位段基地址
    mov word [edi + 2], ax      ; 将 ax 所保存的值放到偏移 2 字节处
    shr eax, 16
    mov byte [edi + 4], al      ; 将第3个字节放到偏移为 4 的地方处
    mov byte [edi + 7], ah      ; 将 32 位代码段基地址放到最高位处

    pop eax

    ret

[section .s16]
[bits 16]
CODE16_SEGMENT:
    mov ax, UpdateSelector
    mov ds, ax                  ; 刷新寄存器
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, cr0
    and al, 11111110b
    mov cr0, eax                ; 退出保护模式, 进入实模式

BACK_TO_REAL_MODE:
    jmp 0 : BACK_ENTRY_SEGMENT  ; 跳转到16位实模式

Code16SegLen     equ   $ - CODE16_SEGMENT

[section .s32]
[bits 32]
CODE32_SEGMENT:
    mov ax, VideoSelector
    mov gs, ax                  ; 将显存段的基址放在 gs 寄存器中

    mov ax, Stack32Selector     ; 32 位代码段必须定义全局栈空间
    mov ss, ax

    mov eax, TopOfStack32
    mov esp, eax

    mov ax, Data32Selector
    mov ds, ax

    mov ebp, DTOS_OFFSET         ; 指向目标字符串(保护模式是段内偏移地址)
    mov bx, 0x0C                 ; 打印属性, 0C 代表以黑底红字来显示
    mov dh, 12                   ; 打印位置 行
    mov dl, 33                   ; 打印位置 列

    call PrintString

    mov ebp, HELLO_WORLD_OFFSET
    mov bx, 0x0C
    mov dh, 13
    mov dl, 31

    call PrintString

    mov ax, TaskALdtSelector

    lldt ax                      ; 加载局部描述符 TASK A

    jmp TaskACode32Selector : 0  ; 跳转到 TASK A 代码段
    ;jmp Code16Selector : 0

; ds:ebp   --> string address
; bx       --> attribute
; dx       --> dh : row, dl : col
PrintString:
    push ebp
    push eax
    push edi
    push cx
    push dx

print:
    mov cl, [ds:ebp]   ; 在 ebp 中取字符
    cmp cl, 0
    je end             ; 如果取到最后一个字符(0), 表示结束
    mov eax, 80
    mul dh             ; 80 * 对应行数 ==> (80 * 12)
    add al, dl         ; + 列 ==> (80 * 12 +37)
    shl eax, 1         ; << 1 ==> * 2 ==> (80 * 12 +37) * 2
    mov edi, eax       ; 放到 edi 寄存器中去
    mov ah, bl         ; 将 0C 放入 ah 中
    mov al, cl         ; 将字符放入 al 中
    mov [gs:edi], ax   ; 放入 gs:edi 寄存器中去
    inc ebp            ; 下一个打印的字符
    inc dl             ; 下一个字符打印的位置
    jmp print

end:
    pop dx
    pop cx
    pop edi
    pop eax
    pop ebp

    ret

Code32SegLen   equ   $ - CODE32_SEGMENT

[section .gs]
[bits 32]
STACK32_SEGMENT:
    times 1024 * 4 db 0                 ; 预留 4k 空间

Stack32SegLen equ $ - STACK32_SEGMENT
TopOfStack32  equ Stack32SegLen - 1     ; 初始栈段位置: 栈界限

; =============================================================
;
;                       Task A code segment
;
; =============================================================

[section .task-a-ldt]
; Task A LDT definition
;                                      段基址,         段界限,               段属性
TASK_A_LDT_ENTRY:
TASK_A_CODE32_DESC  :    Descriptor        0,     TaskACode32SegLen - 1,     DA_C + DA_32
TASK_A_DATA32_DESC  :    Descriptor        0,     TaskAData32SegLen - 1,     DA_DR + DA_32
TASK_A_STACK32_DESC :    Descriptor        0,     TaskAStack32SegLen - 1,    DA_DRW + DA_32

TaskALdtLen   equ  $ - TASK_A_LDT_ENTRY

; Task A LDT Selector
TaskACode32Selector  equ   (0x0000 << 3) + SA_TIL + SA_RPL0
TaskAData32Selector  equ   (0x0001 << 3) + SA_TIL + SA_RPL0
TaskAStack32Selector equ   (0x0002 << 3) + SA_TIL + SA_RPL0

[section .task-a-dat]
[bits 32]
TASK_A_DATA32_SEGMENT:
    TASK_A_STRING        db  "This is Task A!", 0
    TASK_A_STRING_OFFSET equ TASK_A_STRING - $$

TaskAData32SegLen  equ   $ - TASK_A_DATA32_SEGMENT

[section .task-a-gs]
[bits 32]
TASK_A_STACK32_SEGMENT:
    times 1024 db 0

TaskAStack32SegLen equ   $ - TASK_A_STACK32_SEGMENT
TaskATopOfStack32  equ   TaskAStack32SegLen - 1

[section .task-a-s32]
[bits 32]
TASK_A_CODE32_SEGMENT:
    mov ax, VideoSelector
    mov gs, ax                   ; 初始化显存段

    mov ax, TaskAStack32Selector
    mov ss, ax                   ; 初始化栈空间

    mov eax, TaskATopOfStack32
    mov esp, eax                 ; 初始化栈顶指针

    mov ax, TaskAData32Selector
    mov ds, ax                   ; 初始化数据段

    mov ebp, TASK_A_STRING_OFFSET
    mov bx, 0x0C                 ; 设置打印属性
    mov dh, 14                   ; 设置打印行数
    mov dl, 29                   ; 设置打印列数

    call TaskAPrintString

    jmp Code16Selector : 0

; ds:ebp   --> string address
; bx       --> attribute
; dx       --> dh : row, dl : col
TaskAPrintString:
    push ebp
    push eax
    push edi
    push cx
    push dx

task_print:
    mov cl, [ds:ebp]   ; 在 ebp 中取字符
    cmp cl, 0
    je task_end        ; 如果取到最后一个字符(0), 表示结束
    mov eax, 80
    mul dh             ; 80 * 对应行数 ==> (80 * 12)
    add al, dl         ; + 列 ==> (80 * 12 +37)
    shl eax, 1         ; << 1 ==> * 2 ==> (80 * 12 +37) * 2
    mov edi, eax       ; 放到 edi 寄存器中去
    mov ah, bl         ; 将 0C 放入 ah 中
    mov al, cl         ; 将字符放入 al 中
    mov [gs:edi], ax   ; 放入 gs:edi 寄存器中去
    inc ebp            ; 下一个打印的字符
    inc dl             ; 下一个字符打印的位置
    jmp task_print

task_end:
    pop dx
    pop cx
    pop edi
    pop eax
    pop ebp

    ret

TaskACode32SegLen  equ   $ - TASK_A_CODE32_SEGMENT
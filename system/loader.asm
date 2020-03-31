%include "inc.asm"

org 0x9000

jmp CODE16_SEGMENT

[section .gdt]           ; 定义 .gdt 代码段
; GDT definition
;                                  段基址           段界限       段属性
GDT_ENTRY       :   Descriptor        0,               0,          0
CODE32_DESC     :   Descriptor        0,      Code32SegLen - 1,    DA_C + DA_32
;                                                                  32位可执行代码段
; GDT end

GdtLen    equ   $ - GDT_ENTRY  ; 计算全局描述符段的长度

GdtPtr:                        ; 相当于 C 中的 struct
          dw    GdtLen - 1     ; 两个字节的界面
          dd    0              ; 四个字节的起始地址

; GDT Selector
; CODE32_DESC 的选择子, 选择子的地址 (0x0001 << 3), 属性 SA_TIG, 特权级 SA_RPL0
Code32Selector  equ (0x0001 << 3) + SA_TIG + SA_RPL0

; end of [section .gdt]

[section .s16]      ; 实模式的代码段
[bits 16]
CODE16_SEGMENT:
    mov ax, cs      ; 初始化
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; initialize GDT for 32 bits code segment
    mov eax, 0
    mov ax, cs
    shl eax, 4                      ; 将当前地址 << 4 位
    add eax, CODE32_SEGMENT         ; 得到 32 位段基地址
    mov word [CODE32_DESC + 2], ax  ; 将 ax 所保存的值放到偏移 2 字节处
    shr eax, 16
    mov byte [CODE32_DESC + 4], al  ; 将第3个字节放到偏移为 4 的地方处
    mov byte [CODE32_DESC + 7], ah  ; 将 32 位代码段基地址放到最高位处

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

[section .s32]
[bits 32]
CODE32_SEGMENT:
    mov eax, 0
    jmp CODE32_SEGMENT

Code32SegLen   equ   $ - CODE32_SEGMENT
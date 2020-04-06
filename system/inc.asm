
; Segment Attribute
DA_32    equ    0x4000
DA_DR    equ    0x90
DA_DRW   equ    0x92
DA_DRWA  equ    0x93
DA_C     equ    0x98
DA_CR    equ    0x9A
DA_CCO   equ    0x9C
DA_CCOR  equ    0x9E

; Segment Privilege
DA_DPL0		equ	  0x00    ; DPL = 0
DA_DPL1		equ	  0x20    ; DPL = 1
DA_DPL2		equ	  0x40    ; DPL = 2
DA_DPL3		equ	  0x60    ; DPL = 3

; Special Attribute
DA_LDT       equ    0x82
DA_TaskGate  equ    0x85	; 任务门类型值
DA_386TSS    equ	0x89	; 可用 386 任务状态段类型值
DA_386CGate  equ	0x8C	; 386 调用门类型值
DA_386IGate  equ	0x8E	; 386 中断门类型值
DA_386TGate  equ	0x8F	; 386 陷阱门类型值

; Selector Attribute
SA_RPL0    equ    0
SA_RPL1    equ    1
SA_RPL2    equ    2
SA_RPL3    equ    3

SA_TIG    equ    0
SA_TIL    equ    4

; 描述符
; usage: Descriptor Base, Limit, Attr
;        Base:  dd
;        Limit: dd (low 20 bits available)
;        Attr:  dw (lower 4 bits of higher byte are always 0)
%macro Descriptor 3	                          ; 段基址， 段界限， 段属性
    dw    %2 & 0xFFFF                         ; 段界限1
    dw    %1 & 0xFFFF                         ; 段基址1
    db    (%1 >> 16) & 0xFF                   ; 段基址2
    dw    ((%2 >> 8) & 0xF00) | (%3 & 0xF0FF) ; 属性1 + 段界限2 + 属性2
    db    (%1 >> 24) & 0xFF                   ; 段基址3
%endmacro                                     ; 共 8 字节


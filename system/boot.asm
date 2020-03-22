org 0x7c00

jmp short start    ; 离 start 字节数不多，所以用 short 跳转就行
                   ; jmp 占一个字节, start 占一个字节
nop                ; nop 占用一个字节

define:            ; 定义栈空间的起始地址
    BaseOfStack equ 0x7c00
                   ; 定义常量：dx 定义占用内存空间, equ 定义不会占用内存空间

header:            ; offset 3
    BS_OEMName     db "D.T.Soft"
    BPB_BytsPerSec dw 512
    BPB_SecPerClus db 1
    BPB_RsvdSecCnt dw 1
    BPB_NumFATs    db 2
    BPB_RootEntCnt dw 224
    BPB_TotSec16   dw 2880
    BPB_Media      db 0xF0
    BPB_FATSz16    dw 9
    BPB_SecPerTrk  dw 18
    BPB_NumHeads   dw 2
    BPB_HiddSec    dd 0
    BPB_TotSec32   dd 0
    BS_DrvNum      db 0
    BS_Reserved1   db 0
    BS_BootSig     db 0x29
    BS_VolID       dd 0
    BS_VolLab      db "D.T.OS-0.01"
    BS_FileSysType db "FAT12   "

start:
    mov ax, cs
    mov ss, ax
    mov ds, ax
    mov es, ax           ; 初始化寄存器
    mov sp, BaseOfStack  ; 栈顶指针地址为 0x7c00, 通过 sp 寄存器保存

    mov ax, 34           ; 指定需要读取的逻辑扇区号为 34
    mov cx, 1            ; 连续读取 1 个扇区
    mov bx, Buf          ; 存放的地址

    call ReadSector

    mov bp, Buf          ; 将目标字符串的地址赋值给 bp
    mov cx, 35           ; 将目标字符串的长度赋值给 cx

    call Print

last:
    hlt
    jmp last

; es:bp --> string address
; cx    --> string length
Print:
    mov ax, 0x1301        ; 指定打印参数
    mov bx, 0x0007        ; 指定打印参数
    int 0x10              ; 执行 0x10 中断
    ret                   ; 汇编中函数的最后一条指令必须是 ret

; no parameter              软驱复位函数
ResetFloppy:
    push ax
    push dx

    mov ah, 0x00          ; 软驱复位
    mov dl, [BS_DrvNum]   ; 驱动器号, 0 代表 a 盘
    int 0x13              ; BIOS 中的软盘数据读取中断号

    pop dx
    pop ax

    ret

; ax    --> 逻辑扇区号
; cx    --> 需要连续读取多少个扇区
; es:bx --> 读取的内存地址
ReadSector:
    push bx                ; 寄存器进栈
    push cx
    push dx
    push ax

    call ResetFloppy       ; 软驱复位

    push bx                ; 下面改变了 bx 目标地址的值, 因此先 push
    push cx                ; 需要将读取的扇区号放入 cx 中，因此 push

    mov bl, [BPB_SecPerTrk]  ; 每个柱面有 18个 扇区
    div bl                   ; 16 位除法操作
    mov cl, ah               ; 将余数赋值给 cl, 余数放在 ah 寄存器中
    add cl, 1                ; cl 中保存起始扇区号, 扇区号 = 余数 + 1
    mov ch, al               ; 将商赋值给 ch, 商放在 al 寄存器中
    shr ch, 1                ; ch 中保存柱面号, 柱面号 = 商 >> 1, 右移 1 位
    mov dh, al               ; 将商赋值给 dh
    and dh, 1                ; dh 中保存磁头号, 磁头号 = 商 & 1
    mov dl, [BS_DrvNum]      ; dl 中保存驱动器号

    pop ax                   ; 逻辑扇区号放在 ax 中, 因此 pop 出来
    pop bx                   ; 上面 push, 下面 pop

    mov ah, 0x02             ; 读的时候, 规定 ah 必须为 0x02

read:
    int 0x13                 ; 有可能读取失败
    jc read                  ; 失败重读

    pop ax                   ; 寄存器出栈
    pop dx
    pop cx
    pop bx

    ret

MsgStr db  "Hello, DTOS!"
MsgLen equ ($-MsgStr)        ; 当前地址 - MsgStr = 字符串长度
Buf:
    times 510-($-$$) db 0x00
    db 0x55, 0xaa

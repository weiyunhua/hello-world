org 0x7c00

jmp short start    ; 离 start 字节数不多，所以用 short 跳转就行
                   ; jmp 占一个字节, start 占一个字节
nop                ; nop 占用一个字节

define:            ; 定义栈空间的起始地址
    BaseOfStack equ 0x7c00
                   ; 定义常量：dx 定义占用内存空间, equ 定义不会占用内存空间
    RootEntryOffset equ 19
                   ; 目标扇区的偏移量
    RootEntryLength equ 14
                   ; 目标扇区的长度


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
    mov sp, BaseOfStack  ; 栈顶指针地址为 0x7c00, 通过 sp 寄存器保存get

    ; FindEntry func test
    mov ax, RootEntryOffset
    mov cx, RootEntryLength
    mov bx, Buf

    call ReadSector

    mov si, Target
    mov cx, TarLen
    mov dx, 0

    call FindEntry

    cmp dx, 0
    jz output
    jmp last

output:
    mov bp, MsgStr_f     ; 参数设置, 将 bp 寄存器设置为 MsgStr_f 字符串
    mov cx, MsgLen_f     ; 将 cx 寄存器设置为 MsgLen_f 长度
    call Print

    ; MemCmp func test
    mov si, MsgStr       ; 将 MsgStr 赋值给 si
    mov di, DEST         ; 将 DEST 赋值给 di
    mov cx, MsgLen       ; 将 MsgLen 赋值给 cx

    call MemCmp          ; 调用 MemCmp 函数

    cmp cx, 0
    jz label             ; 如果比较成功, 则跳转到 label 标签处
    jmp last             ; 否则直接将 cpu 挂起

label:
    mov bp, MsgStr       ; 参数设置, 将 bp 寄存器设置为 MsgStr 字符串
    mov cx, MsgLen       ; 将 cx 寄存器设置为 MsgLen 长度
    call Print

last:
    hlt
    jmp last

; es:bx --> root entry offset address
; ds:si --> target string
; cx    --> target length
;
; return:
;       (dx != 0) ? exist : noexist
;          exist --> bx is the target entry
FindEntry:
    push di                  ; 函数中有用到 di, 因此入栈
    push bp                  ; 函数中有用到 bp, 因此入栈
    push cx                  ; 需要反复用到 cx, 因此入栈

    mov dx, [BPB_RootEntCnt] ; 查找的最大值
    mov bp, sp               ; 因为 sp 不能直接访问栈顶数据, 因此通过 bp 寄存器间接访问

find:
    cmp dx, 0
    jz noexist
    mov di, bx               ; bx 中保存的是根目录区的起始地址(第0项), 每一项前11个字节
    mov cx, [bp]             ; 取目标字符串的长度
    call MemCmp
    cmp cx, 0
    jz exist
    add bx, 32                ; 表示对比失败, 那么指向下一项. 当前每项占32B, + 32
    dec dx                    ; dx--
    jmp find

exist:
noexist:
    pop cx
    pop bp
    pop di

    ret

; ds:si --> source
; es:di --> destination
; cx    --> length
;
; return:
;        (cx ==0) ? equal : noequal
MemCmp:
    push si
    push di
    push ax               ; 因为 cx 作为返回值来使用, 因此不用保存其状态

compare:
    cmp cx, 0
    jz equal              ; 如果 cx 寄存器的值为 0, 则表示相等
    mov al, [si]          ; 取 si 中的一个字节放入 al 寄存器中, [] 表示取一个字节
    cmp al, byte [di]     ; 将 al 寄存器中的值与 di 的一个字节进行比较
    jz goon               ; 比较成功则继续进行下一个字节的比较
    jmp noequal           ; 否则跳转到 noequal
goon:
    inc si                ; si++
    inc di                ; di++
    dec cx                ; cx--
    jmp compare           ; 继续执行比较操作

equal:
noequal:
    pop ax
    pop di
    pop si

    ret

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

MsgStr   db  "Hello, DTOS!"
MsgLen   equ ($-MsgStr)        ; 当前地址 - MsgStr = 字符串长度
DEST     db "Hello, DTOS?"
Target   db "LOADER     "
TarLen   equ ($-Target)
MsgStr_f db  "No LOADER..."
MsgLen_f equ ($-MsgStr_f)      ; 当前地址 - MsgStr_f = 字符串长度
Buf:
    times 510-($-$$) db 0x00
    db 0x55, 0xaa

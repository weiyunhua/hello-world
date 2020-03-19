org 0x7c00

start:
	mov ax, cs
	mov ss, ax
	mov ds, ax
	mov es, ax	; 初始化寄存器
	
	mov si, msg	; 将 msg 标签地址赋值到 si 上
	
print:
	mov al, [si]	; 取 si 里面的数据, al 代表 si 中的第一个数据. ==> C *(p)
	add si, 1		; si 地址+1
	cmp al, 0x00	; 判断 al 是否到达数据的尾部
	je last			; 上面判断为真的话, 跳转到 last
	mov ah, 0x0e	; 设置参数
	mov bx, 0x0f
	int 0x10		; 触发中断，打印一个字符
	jmp print		; 循环打印字符
	
last:
	hlt
	jmp last
	
msg:
	db 0x0a, 0x0a		; 定义两个连续数据，0x0a 代表换行
	db "Hello, DTOS!"	; 定义 ASCII 码字符
	db 0x0a, 0x0a
	times 510-($-$$) db 0x00 ;0x00 代表数据的结尾
		; $-$$代表当前行 - 汇编起始地址=前面这段代码所占的字节大小 510+2=512
	db 0x55, 0xaa		; 定义结束标识符 0x55aa
	
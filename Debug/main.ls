   1                     ; C Compiler for STM8 (COSMIC Software)
   2                     ; Parser V4.13.3 - 22 May 2025
   3                     ; Generator (Limited) V4.6.6 - 07 Jan 2026
  53                     ; 4 void delay(void)
  53                     ; 5 {
  55                     	switch	.text
  56  0000               _delay:
  58  0000 5204          	subw	sp,#4
  59       00000004      OFST:	set	4
  62                     ; 7     for(i = 30000; i > 0; i--);
  64  0002 ae7530        	ldw	x,#30000
  65  0005 1f03          	ldw	(OFST-1,sp),x
  66  0007 ae0000        	ldw	x,#0
  67  000a 1f01          	ldw	(OFST-3,sp),x
  69  000c               L72:
  73  000c 96            	ldw	x,sp
  74  000d 1c0001        	addw	x,#OFST-3
  75  0010 a601          	ld	a,#1
  76  0012 cd0000        	call	c_lgsbc
  81  0015 96            	ldw	x,sp
  82  0016 1c0001        	addw	x,#OFST-3
  83  0019 cd0000        	call	c_lzmp
  85  001c 26ee          	jrne	L72
  86                     ; 8 }
  89  001e 5b04          	addw	sp,#4
  90  0020 81            	ret
 116                     ; 10 void main(void)
 116                     ; 11 {
 117                     	switch	.text
 118  0021               _main:
 122                     ; 12     GPIO_Init(GPIOB,
 122                     ; 13               GPIO_PIN_4,
 122                     ; 14               GPIO_MODE_OUT_PP_LOW_FAST);
 124  0021 4be0          	push	#224
 125  0023 4b10          	push	#16
 126  0025 ae5005        	ldw	x,#20485
 127  0028 cd0000        	call	_GPIO_Init
 129  002b 85            	popw	x
 130  002c               L54:
 131                     ; 18         GPIO_WriteReverse(GPIOB, GPIO_PIN_4);
 133  002c 4b10          	push	#16
 134  002e ae5005        	ldw	x,#20485
 135  0031 cd0000        	call	_GPIO_WriteReverse
 137  0034 84            	pop	a
 138                     ; 19         delay();
 140  0035 adc9          	call	_delay
 143  0037 20f3          	jra	L54
 156                     	xdef	_main
 157                     	xdef	_delay
 158                     	xref	_GPIO_WriteReverse
 159                     	xref	_GPIO_Init
 178                     	xref	c_lzmp
 179                     	xref	c_lgsbc
 180                     	end

; SD BIOS ��� ���������� ����������
; (c) 22-05-2013 vinxru

     .org 08F00h-753 ; ��������� ���� ���� ������ ���� 08EFFh
                       
;----------------------------------------------------------------------------

CLC_PORT        = 0F001h
DATA_PORT       = 0F002h
CTL_PORT        = 0F003h
CLC_BIT         = 80h
SEND_MODE       = 10010000b ; ���������: 1 0 0 A �H 0 B CL   1=���� 0=�����
RECV_MODE       = 10011001b

ERR_START   	= 040h
ERR_WAIT    	= 041h
ERR_OK_NEXT 	= 042h
ERR_OK        = 043h
ERR_OK_READ   = 044h
ERR_OK_ENTRY  = 045h
ERR_OK_WRITE	= 046h
ERR_OK_RKS  	= 047h

VER_BUF = 0

;----------------------------------------------------------------------------
; ��������� RKS �����

     .dw $+4
     .dw End-1
     
;----------------------------------------------------------------------------
	      
Entry:
     ; ������ ����
     MVI	A, 80h
     STA	0FFFEh

     ; ������ �����������
     LXI	H, Img
     LXI	D, 0BE1Ch
DrawImg2:
     MOV	C, M
     DCR	C
     JZ 	DrawImg4
     INX	H
     MOV	A, M
     INX	H
DrawImg3:
     STAX	D
     DCX	D
     DCR	C
     JNZ	DrawImg3
     JMP	DrawImg2
DrawImg4:
    
     ; ����� ����
     XRA	A
     STA	0FFFEh

     ; ����� �������� ����������� �� �����
     LXI	H, aHello
     CALL	0C818h

     ; ����� ������ �����������
     CALL	PrintVer

     ; ������� ������
     lxi	h, aCrLf
     CALL	0C818h

     ; ��������� ���� SHELL.RKS ��� ��� ������
     LXI	H, aShellRks
     LXI	D, aEmpty
     CALL	CmdExec
     PUSH	PSW

     ; ������ - ���� �� ������
     CPI	04h
     JNZ 	Error2

     ; ����� ��������� "���� �� ������ BOOT/SHELL.RKS"
     LXI	H, aErrorShellRks
     CALL	0C818h
     JMP	$

;----------------------------------------------------------------------------

PrintVer:
     ; ������� ��������� ������
     MVI	A, 1
     CALL	StartCommand	; ������ ���� � ������� ��������� ������
     CALL	SwitchRecv
     
     ; �������� ������ ������ ������ � �����
     LXI	B, VER_BUF
     LXI	D, 18          ; 1 �� ���� ������, ��������� ���� - ��������� ����
     CALL	RecvBlock
          
     ; ����� ������ ������
     XRA	A
     STA	VER_BUF+17
     LXI	H, VER_BUF+1
     JMP 	0C818h

;----------------------------------------------------------------------------

Img:            .db 8, 0FFh, 2, 01Fh, 4, 06Fh, 2, 01Fh, 7, 0FFh, 5, 055h, 2, 0FFh, 234, 0
                .db 8, 0FFh, 2, 0E3h, 2, 0FDh, 2, 0F3h, 2, 0EFh, 2, 0F1h, 5, 0FFh, 3, 07Fh, 3, 055h, 3, 0D5h, 2, 07Fh, 234, 0
                .db 22, 1
                .db 1

aHello:         .db 13,"SD BIOS V1.0",13,10
aSdController:  .db "SD CONTROLLER ",0
aCrLf:          .db 13,10,0
aErrorShellRks: .db "fajl ne najden "
aShellRks:      .db "BOOT/SHELL.RKS",0
                .db "(c) 22-05-2013 vinxru"

; ��� ���� ����� ������ ��� ������� � ���������� ������

SELF_NAME    = $-512 ; ���� (����� 256 ����)
CMD_LINE     = $-256 ; ���������� ������ 256 ����

;----------------------------------------------------------------------------
; ����������� ����� SD BIOS
;----------------------------------------------------------------------------

aError:    .db "o{ibka SD "
aEmpty:    .db 0

;----------------------------------------------------------------------------
; ��� ����������������� ��, ��� ����� ���� ��������� ��� ����

Error:     
     ; ����� ���������� �������
     EI
     LXI	H, 0FF03h
     MVI	M, 82h
     
     ; ������������� �����
     LXI	SP, 07FFFh

     ; ��������� ��� ������
     PUSH	PSW

     ; ������� ������
     MVI	C, 1Fh
     CALL	0C809h
     
Error2:
     ; ����� ������ "������ SD "
     LXI	H, aError
     CALL	0C818h

     ; ����� ���� ������
     POP	PSW
     CALL	0C815h

     ; ������
     JMP	$

;----------------------------------------------------------------------------

BiosEntry:
     PUSH H             ; 1
     LXI	H, JmpTbl     ; 2
     ADD	L             ; 1
     MOV	L, A          ; 1
     MOV	L, M          ; 1
     XTHL               ; 1
     RET                ; 1

;----------------------------------------------------------------------------
; �������� 8D00. ��� �������� JmpTbl � �������� ����� ��������

JmpTbl:
     .db CmdExec           ; 0 HL-��� �����, DE-��������� ������  / A-��� ������
     .db CmdFind           ; 1 HL-��� �����, DE-�������� ������ ��� ��������, BC-����� / HL-������� ���������, A-��� ������
     .db CmdOpenDelete     ; 2 D-�����, HL-��� ����� / A-��� ������
     .db CmdSeekGetSize    ; 3 B-�����, DE:HL-������� / A-��� ������, DE:HL-�������
     .db CmdRead           ; 4 HL-������, DE-����� / HL-������� ���������, A-��� ������
     .db CmdWrite          ; 5 HL-������, DE-����� / A-��� ������
     .db CmdMove           ; 6 HL-��, DE-� / A-��� ������

;----------------------------------------------------------------------------
; HL-����, DE-�������� ������ ��� ��������, BC-����� / HL-������� ���������, A-��� ������

CmdFind:
     ; ��� �������
     MVI	A, 3
     CALL	StartCommand

     ; ����
     CALL	SendString

     ; �������� ������
     XCHG
     CALL	SendWord

     ; ������������� � ����� ������
     CALL	SwitchRecv

     ; �������
     LXI	H, 0

CmdFindLoop:
     ; ���� ���� �� ���������
     CALL	WaitForReady
     CPI	ERR_OK
     JZ		Ret0
     CPI	ERR_OK_ENTRY
     JNZ	EndCommand

     ; ����� ����� ������
     LXI	D, 20	; ����� �����
     CALL	RecvBlock

     ; ����������� ������� ������
     INX	H

     ; ����
     JMP	CmdFindLoop

;----------------------------------------------------------------------------
; D-�����, HL-��� ����� / A-��� ������

CmdOpenDelete: 
     ; ��� �������
     MVI	A, 4
     CALL	StartCommand

     ; �����
     MOV	A, D
     CALL	Send

     ; ��� �����
     CALL	SendString

     ; ���� ���� �� ���������
     CALL	SwitchRecvAndWait
     CPI	ERR_OK
     JZ		Ret0
     JMP	EndCommand
     
;----------------------------------------------------------------------------
; B-�����, DE:HL-������� / A-��� ������, DE:HL-�������

CmdSeekGetSize:
     ; ��� �������
     MVI 	A, 5
     CALL	StartCommand

     ; �����     
     MOV	A, B
     CALL	Send

     ; �������     
     CALL	SendWord
     XCHG
     CALL	SendWord

     ; ���� ���� �� ���������. �� ������ �������� ����� ERR_OK
     CALL	SwitchRecvAndWait
     CPI	ERR_OK
     JNZ	EndCommand

     ; ����� �����
     CALL	RecvWord
     XCHG
     CALL	RecvWord

     ; ���������
     JMP	Ret0
     
;----------------------------------------------------------------------------
; HL-������, DE-����� / HL-������� ���������, A-��� ������

CmdRead:
     ; ��� �������
     MVI	A, 6
     CALL	StartCommand

     ; ����� � BC
     MOV	B, D
     MOV	C, E

     ; ������ �����
     CALL	SendWord        ; HL-������

     ; ������������� � ����� ������
     CALL	SwitchRecv

     ; ����� �����. �� ����� ����� BC, �������� ����� � HL
     JMP	RecvBuf

;----------------------------------------------------------------------------
; HL-������, DE-����� / A-��� ������

CmdWrite:
     ; ��� �������
     MVI	A, 7
     CALL	StartCommand
     
     ; ������ �����
     CALL	SendWord        ; HL-������

     ; ������ ����� � HL
     XCHG

CmdWriteFile2:
     ; ��������� ���������� �������
     CALL	SwitchRecvAndWait
     CPI  	ERR_OK
     JZ  	Ret0
     CPI  	ERR_OK_WRITE
     JNZ	EndCommand

     ; ������ �����, ������� ����� ������� �� � DE
     CALL	RecvWord

     ; ������������� � ����� ��������    
     CALL	SwitchSend

     ; �������� �����. ����� BC ����� DE. (����� �������������� ����)
CmdWriteFile1:
     MOV	A, M
     INX	H
     CALL	Send
     DCX	D
     MOV	A, D
     ORA	E
     JNZ 	CmdWriteFile1

     JMP	CmdWriteFile2

;----------------------------------------------------------------------------
; HL-��, DE-� / A-��� ������

CmdMove:     
     ; ��� �������
     MVI	A, 8
     CALL	StartCommand

     ; ��� �����
     CALL	SendString

     ; ���� ���� �� ���������
     CALL	SwitchRecvAndWait
     CPI	ERR_OK_WRITE
     JNZ	EndCommand

     ; ������������� � ����� ��������
     CALL	SwitchSend

     ; ��� �����
     XCHG
     CALL	SendString

WaitEnd:
     ; ���� ���� �� ���������
     CALL	SwitchRecvAndWait
     CPI	ERR_OK
     JZ		Ret0
     JMP	EndCommand

;----------------------------------------------------------------------------
; HL-��� �����, DE-��������� ������ / A-��� ������

CmdExec:
     ; ��� �������
     MVI	A, 2
     CALL	StartCommand

     ; ��� �����
     PUSH	H
     CALL	SendString
     POP	H

     ; ���� ���� �� ��������� ����
     ; �� ������ �������� ����� ERR_OK_RKS
     CALL	SwitchRecvAndWait
     CPI	ERR_OK_RKS
     JNZ	EndCommand

     ; ��������� ��� ����� (HL-������)
     PUSH	D
     XCHG
     LXI	H, SELF_NAME
     CALL	strcpy255
     POP	D

     ; ��������� ��������� ������ (DE-������)
     LXI	H, CMD_LINE
     CALL	strcpy255

     ; *** ��� ����� ����������. ����� ������ �������� � ������������. ***

     ; ������������� ����� (���������� ������������ ��������
     LXI	SP, 07FFFh

     ; ��������� ����� �������� � BC � ��������� ��� � ����
     CALL	RecvWord
     PUSH	D
     MOV 	B, D
     MOV 	C, E

     ; ��������� ����
     CALL	RecvBuf
     JNZ 	Error

     ; ������� ������
     MVI	C, 1Fh
     CALL	0C809h

     ; ��������� ��� ���������
     MVI  A, 1		; ������ �����������
     LXI  B, BiosEntry  ; ����� ����� SD BIOS
     LXI  D, SELF_NAME  ; ����������� ���
     LXI  H, CMD_LINE   ; ��������� ������

     ; ������ ����������� ���������
     RET

;----------------------------------------------------------------------------
; ��� ���� ��������� �������. ������ �������� 8E00.
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; ������ ����� �������. 
; B - ��� �������

StartCommand:
     ; ������ ������ ���������� ������������� � ������������
     ; ����������� 256 �������, � ������ �� ������� ������������ 256+ ����
     ; �� ���� ��� ������������ ���-�� ������, ������� ����� �������� ����������
     PUSH	B
     MOV	B, A
     MVI	C, 0

StartCommand1:
     ; ����� ��������
     CALL	SwitchSend0

     ; �������� ������ �������
     MVI	A, 013h
     CALL	Send
     MVI	A, 0B4h
     CALL	Send
     MVI	A, 057h
     CALL	Send
     MOV	A, B
     CALL	Send

     ; ����� ������  
     CALL	SwitchRecv

     ; ���� ���� �������������, �� ���������� ������� ERR_START
     CALL	Recv
     CPI	ERR_START
     JZ		StartCommand2

     ; �����. � �� ���� ���������� 256 ���� (� ����� ����� 
     ; ��������� 64 �� ������, ������������ ������ ������)
     PUSH	B
     MVI	C, 0
StartCommand3:
     CALL	Recv
     DCR	C
     JNZ	StartCommand3
     POP	B
        
     ; �������
     DCR	C
     JNZ	StartCommand1    

     ; ��� ������
     MVI	A, ERR_START
     POP	B ; ������� �������� B

     ; ������� ����� �������.
StartCommandErr:
     POP	B ; ����� �������
     RET

;----------------------------------------------------------------------------
; ������������� � ������������ ����. ���������� ������ �������� ERR_OK_NEXT

StartCommand2:
     ; ��������������� ������� �������� BC
     POP	B

     ; �����         	
     CALL	WaitForReady
     CPI	ERR_OK_NEXT
     JNZ	StartCommandErr

     ; �������� �������� � �������
     ;JMP 	SwitchSend

;----------------------------------------------------------------------------
; ������������� � ����� ��������

SwitchSend:
     CALL	Recv
SwitchSend0:
     MVI	A, SEND_MODE
     STA	CTL_PORT
     RET

;----------------------------------------------------------------------------
; �������� ��������� ������� 
; � �������������� ����, ��� �� �� �������� ����

Ret0:
     XRA	A

;----------------------------------------------------------------------------
; ��������� ������� � ������� � A 
; � �������������� ����, ��� �� �� �������� ����

EndCommand:
     PUSH	PSW
     CALL	Recv
     POP	PSW
     RET

;----------------------------------------------------------------------------
; ������� ����� � DE 
; ������ A.

RecvWord:
    CALL Recv
    MOV  E, A
    CALL Recv
    MOV  D, A
    RET
    
;----------------------------------------------------------------------------
; ��������� ����� �� HL 
; ������ A.

SendWord:
    MOV		A, L
    CALL	Send
    MOV		A, H
    JMP		Send
    
;----------------------------------------------------------------------------
; �������� ������
; HL - ������
; ������ A.

SendString:
     XRA	A
     ORA	M
     JZ		Send
     CALL	Send
     INX	H
     JMP	SendString
     
;----------------------------------------------------------------------------
; ������������� � ����� ������

SwitchRecv:
     MVI	A, RECV_MODE
     STA	CTL_PORT
     RET

;----------------------------------------------------------------------------
; ������������� � ����� �������� � �������� ���������� ��.

SwitchRecvAndWait:
     CALL SwitchRecv

;----------------------------------------------------------------------------
; �������� ���������� ��.

WaitForReady:
     CALL	Recv
     CPI	ERR_WAIT
     JZ		WaitForReady
     RET

;----------------------------------------------------------------------------
; ������� DE ���� �� ������ BC
; ������ A

RecvBlock:
     PUSH	H
     LXI 	H, CLC_PORT
     INR 	D
     XRA 	A
     ORA 	E
     JZ 	RecvBlock2
RecvBlock1:
     MVI  M, CLC_BIT  ; 7
     MVI  M, 0        ; 7
     LDA	DATA_PORT   ; 13
     STAX	B		        ; 7
     INX	B		        ; 5
     DCR	E		        ; 5
     JNZ	RecvBlock1	; 10 = 54
RecvBlock2:
     DCR	D
     JNZ	RecvBlock1
     POP	H
     RET

;----------------------------------------------------------------------------
; �������� ������ �� ������ BC. 
; �� ������ HL ������� ���������
; ������ A
; ���� ��������� ��� ������, �� ������ Z=1

RecvBuf:
     LXI	H, 0
RecvBuf0:   
     ; ���������
     CALL	WaitForReady
     CPI	ERR_OK_READ
     JZ		Ret0		; �� ������ Z (��� ������)
     ORA	A
     JNZ	EndCommand	; �� ������ NZ (������)

     ; ������ ����������� ������ � DE
     CALL	RecvWord

     ; � HL ����� ������
     DAD D

     ; ������� DE ���� �� ������ BC
     CALL	RecvBlock

     JMP	RecvBuf0

;----------------------------------------------------------------------------
; ����������� ������ � ������������ 256 �������� (������� ����������)

strcpy255:
     MVI  B, 255
strcpy255_1:
     LDAX D
     INX  D
     MOV  M, A
     INX  H
     ORA  A
     RZ
     DCR  B
     JNZ  strcpy255_1
     MVI  M, 0 ; ����������
     RET

;----------------------------------------------------------------------------
; ��������� ���� �� A.

Send:
     STA	DATA_PORT     

;----------------------------------------------------------------------------
; ������� ���� � �

Recv:
     MVI	A, CLC_BIT
     STA	CLC_PORT
     XRA	A
     STA	CLC_PORT
     LDA	DATA_PORT
     RET

;----------------------------------------------------------------------------

End:
     .dw 0FFFFh

;----------------------------------------------------------------------------

.End
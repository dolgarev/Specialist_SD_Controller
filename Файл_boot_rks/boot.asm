     .org 0h

CLC_PORT        = 0F001h
DATA_PORT       = 0F002h
CTL_PORT        = 0F003h
CLC_BIT         = 80h;
SEND_MODE       = 10010000b ; ���������: 1 0 0 A �H 0 B CL   1=���� 0=�����
RECV_MODE       = 10011001b

ERR_START  	    = 040h
ERR_WAIT   	    = 041h
ERR_OK_DISK     = 042h
ERR_OK          = 043h
ERR_OK_READ	    = 044h
ERR_OK_ENTRY	  = 045h
ERR_OK_WRITE	  = 046h
ERR_OK_RKS	    = 047h

Entry:
     ; ������ ������ ���������� ������������� � ������������
     ; 256 �������. ��� ����� � ������� C ��������� 0

     ; � � ���� ��������� ����� ������������ 0C000h

     LXI	B, 0C000h
     PUSH	B     

     JMP	Boot

     NOP

;----------------------------------------------------------------------------
; �������� � ����� �����

Rst1:
     DCX	H		; HL = CLC_PORT
     MVI	M, CLC_BIT
     MVI	M, 0
     INX	H		; HL = DATA_PORT
     MOV	A, M
     RET

;----------------------------------------------------------------------------
; �������� ���������� ��

Rst2:
WaitForReady:
     Rst	1
     CPI	ERR_WAIT
     JZ		WaitForReady
     RET

;----------------------------------------------------------------------------

RetrySync:
     ; �������
     DCR C
     RZ				; ������ �������������, ������������

Boot:
     ; ����� ��������
     LXI	H, CTL_PORT
     MVI	M, SEND_MODE
     DCX	H		; HL = DATA_PORT

     ; ��� ������� BOOT
     MVI	M, 013h
     Rst	1
     MVI	M, 0B4h
     Rst	1
     MVI	M, 057h
     Rst	1
     MVI	M, 0
     Rst	1

     ; ����� ������  
     INX	H		; HL = CTL_PORT
     MVI	M, RECV_MODE
     DCX	H		; HL = DATA_PORT

     ; ���� ���� �������������, �� ���������� ������� ERR_START
     Rst	1
     CPI	ERR_START
     JNZ	RetrySync

     ; ������ ����� ERR_OK_WAIT, ERR_OK_RKS
     Rst	2
     CPI	ERR_OK_RKS
     JNZ	Rst1		; ������, ����������� ���� � ������������
     
     ; ������� �� ����� 0C000h
     POP	B

     ; ����� �������� � BC
     Rst	1
     MOV	C, A
     Rst	1
     MOV	B, A

     ; ��������� � ���� ����� �������
     PUSH	B

     ; ���������
RecvLoop:
     Rst	2
     CPI	ERR_OK_READ
     JZ		Rst1		; �� ���������, ����������� ���� � ������
     ORA	A
     JNZ	0C000h	; ������, ������������ (�� ��������� ����������)

     ; ������� ��������� ����
     Rst	1
     MOV	E, A
     Rst	1
     MOV	D, A
RecvBlock:
     Rst	1
     STAX	B
     INX	B
     DCX	D
     MOV	A, E
     ORA	D
     JNZ	RecvBlock
     JMP	RecvLoop

.End
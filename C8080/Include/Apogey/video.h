// ����� ������-�������� �������� �������. ������������� �������� ApogeyInitVideo 

extern uchar* apogeyVideoMem @ "apogey/apogeyscreenvars.c";

// ���������� ���� � ������. �� ��������� 78. ��� �������� ����� ���� �� 2 �� 94. 

// ������ ����������� ������ ������� �� 78 ��������. �� ��� ������ 64 ������.
// 8 ������ � 6 ��������� ���� ��������� �� ����� ������. 

// ������ 6 ��������� ����� ����� ������� ���� �������� EOL (0xF1), ������� 
// ���������� ����������������� ��� ����� ������. ��� ������������� ���������
// ��������� ����� ������ �� 7 ����.

// ������� ��������� (����� ������������ ���� � ��������) �� ��������� �� ����� �
// ����������� ����� ������ � ������. ��� �� ����� ������ ���� ���������, 
// ������ ������������ EOL. �� ��� ����� ������ 78 ���� ���������� ����� 
// 6 ������ �� �����.

// ��� ��������� � ������ ���������� ���-�� ���������. ��� 16 ��������� ����� ������
// ����� 94 �����. ������, 16 ���������� (������� EOL) �� ������ ��� �������� 
// �����������.

// ���� �� ���������� EOL, �� �������� ����� 15 ������. �� ����� ������ 
// ���������� 8+64+15 = 87 ����

extern uchar apogeyVideoBpl @ "apogey/apogeyscreenvars.c";

// ��������� �����������

void apogeyScreen0 () @ "apogey/apogeyscreen0.c";  // 64x25, ������� ��� ������,  BPL@78, ��� EOL, � ����������� ��������, ��� ��������, ��������� � ���������
void apogeyScreen0b() @ "apogey/apogeyscreen0b.c"; // 64x25, 0-5 ������� �������, BPL@78, EOL, � ����������� ��������, ��� ��������, ��������� � ���������	
void apogeyScreen1 () @ "apogey/apogeyscreen1.c";  // 64x25, ������� ��� ������,  BPL@78, ��� EOL, ��������� � ���������, ��� ��������
void apogeyScreen1b() @ "apogey/apogeyscreen1b.c"; // 64x25, 0-5 ������� �������, BPL@78, EOL, ��������� � ���������, ��� ��������
void apogeyScreen2a() @ "apogey/apogeyscreen2a.c"; // 64x30, ������� ��� ������,  BPL@75, EOL
void apogeyScreen2b() @ "apogey/apogeyscreen2b.c"; // 64x30, 0-5 ������� �������, BPL@78, EOL, ���������� �������� ���
void apogeyScreen2c() @ "apogey/apogeyscreen2c.c"; // 64x30, 16  ������� �������, BPL@94, ��� EOL, ���������� �������� ���
void apogeyScreen3a() @ "apogey/apogeyscreen3a.c"; // 64x51, ������� ��� ������,  BPL@75, EOL, ���������� �������� ���
void apogeyScreen3b() @ "apogey/apogeyscreen3b.c"; // 64x51, 1-5 ������� �������, BPL@78, EOL, ���������� �������� ���
void apogeyScreen3c() @ "apogey/apogeyscreen3c.c"; // 64x51, 16  ������� �������, BPL@94, ��� EOL, ���������� �������� ���

// ������� ������ ������ �� �����

void print(uchar x, uchar y, char* text)             @ "apogey/print.c";
void printn(uchar x, uchar y, char* text, uchar len) @ "apogey/printn.c";
void printcn(uchar x, uchar y, char c, uchar len)    @ "apogey/printcn.c";

// �������� ���

void waitHorzSync()                                  @ "apogey/waithorzsync.c";

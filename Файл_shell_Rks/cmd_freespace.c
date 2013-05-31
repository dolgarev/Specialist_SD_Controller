#include "shell.h"

void cmd_freespace_1(uchar y, const char* text) {
  char buf[16];
  i2s32(buf, 10, (ulong*)&fs_low, ' ');
  memcpy_back(buf+10, buf+7, 3); buf[9]  = ' ';
  memcpy_back(buf+6,  buf+4, 3); buf[5]  = ' ';
  memcpy_back(buf+2,  buf+1, 3); buf[1]  = ' ';
  strcpy(buf+13, " ��");
  print(19,    y, 64, text);  
  print(19+10, y, 64, buf);
}

void cmd_freespace() {
  uchar e;

  // ����䥩�
  drawWindow(" �������� �� ��᪥ ");
  print(27, 13, 32, "[ ANY KEY ]");
  print(19, 10, 64, "������ ᢮������� ����...");  

  if(e = fs_getfree()) { 
    drawError("�訡�� �⥭�� ��᪠", e);
  } else {                                                 

    // �뢮� ������
    graph1();
    print(19, 10, 27|0x80, "");
    cmd_freespace_1(11, "��������:");

    // ����� ��饣� ����
    if(!fs_gettotal()) cmd_freespace_1(10, "�ᥣ�:");

    // ���� ���짮��⥫�
    getch1();
  }
 
  // ����⠭����� �࠭
  repairScreen(0);
}

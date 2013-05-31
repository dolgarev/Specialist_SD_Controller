#include <n.h>
#include "shell.h"
#include "graph.h"

uchar cmd_copy(char* from, char* to) {
  uint readed;
  char buf[16];
  uchar flag = 1, e;
  ushort progress_l, progress_h;

  progress_l = progress_h = 0;

  // ���뢠�� ��室�� 䠩�
  if(e = fs_open(from)) { drawError("���������� ������ ��室�� 䠩�", e); return 1 /*��� ��१���㧪� 䠩���*/; } 

  // ����䥩�
  drawWindow(" ����஢���� ");
  print1(PRINTARGS(13, 9), 64, "��:");
  print1(PRINTARGS(17, 9), 34, from);
  print1(PRINTARGS(13,10), 64, "�:");
  print1(PRINTARGS(17,10), 34, to);
  print1(PRINTARGS(28,13), 32, "[ ESC ]");

  // �뢮� ��饩 �����
  if(e = fs_getsize()) { drawError("�訡�� �⥭�� 䠩��", e); return 1 /*��� ��१���㧪� 䠩���*/; } 
  i2s32(buf, 10, &fs_result, ' ');
  print1(PRINTARGS(13,11), 64, "�����஢���           /           ����");
  print1(PRINTARGS(36,11), 10, buf); 

  // ������� ���� 䠩�
  if(e = fs_swap()) { drawError("�訡�� fs_swap 1", e); return 1 /*��� ��१���㧪� 䠩���*/; } 
  if(e = fs_create(to)) { drawError("���������� ᮧ���� 䠩�", e); return 1 /*��� ��१���㧪� 䠩���*/; } 

  // ����஢����
  while(1) {
    // kbhit

    // �뢮� �ண���
    i2s32(buf, 10, (ulong*)&progress_l, ' ');
    print1(PRINTARGS(25,11), 10, buf); 

    // ����஢���� �����
    if(e = fs_swap()) { drawError("�訡�� fs_swap 3", e); break; }
    if(e = fs_read(panelA.files1, (MAX_FILES*sizeof(FileInfo)) & ~511) ) { drawError("�訡�� �⥭�� 䠩��", e); break; }
    if(fs_low == 0) return 0; /* � ��१���㧪�� 䠩��� */;
    if(e = fs_swap()) { drawError("�訡�� fs_swap 2", e); break; }
    if(e = fs_write(panelA.files1, fs_low)) { drawError("�訡�� ����� 䠩��", e); break; }

    // 32-� ��⭠� ��䬥⨪� pl:ph += fs_low;
    asm {
      lhld fs_low
      xchg
      lhld cmd_copy_progress_l
      dad d
      shld cmd_copy_progress_l
      jnc fs_copy_l2
      lhld cmd_copy_progress_h
      inx h
      shld cmd_copy_progress_h
fs_copy_l2:
    }
  }

  // ������� 䠩� � ��砥 �訡��. �訡�� 㤠����� �� ��ࠡ��뢠��.
  fs_delete(to);

  return 0 /* � ��१���㧪�� 䠩��� */;
}

//---------------------------------------------------------------------------

const char* getName(const char* name) {
  const char* p;
  for(p = name; *p; p++)
    if(*p == '/')
      name = p+1;
  return name;
}

//---------------------------------------------------------------------------

void cmd_copymove(uchar copymode) {
  char *name, e, *title;
  uint l;
  uint old;
  char buf[256];

  title = copymode ? " ����஢���� " : " ��२���������/��७�� ";

  // ����頥� � ���� ��� ��室���� 䠩��
  addPath1(A_CMDLINE);
  if(cmdline[0] == 0) return;
//  absolutePath();
  strcpy(buf, cmdline);

  if(shiftPressed()) {
    // �����㥬 ��� c ��ࢮ� ������
    getSelectedName(cmdLine);
    cmdline_pos = strlen(cmdline);
  } else {
    // �����㥬 ���� � ��ன ������
    strcpy(cmdline, panelB.path);
    cmdline_pos = strlen(cmdline);
  
    // ������塞 � ����� ��� /, �� �� ��⮬
    // ��⮬�⮬ ���������� ��� 䠩��
    if(cmdline_pos != 1) {
      if(cmdline_pos == 255) { // ���� ��९����� 
        drawError(title, ERR_RECV_STRING);
        goto end;
      }
      strcpy(cmdline+cmdline_pos, "/");
      ++cmdline_pos;
    }
  }

  // �⪫�砥� ��१���㧪� 䠩���
  e=1; 
  
  // ������塞 ���짮��⥫� ������� ���� ��� ���
  if(inputBox(title)) {

    // �᫨ �� ���� ��� �����祭�� /, ����� ���� �������� ���    
    if(cmdline[cmdline_pos - 1] == '/') {      
      name = getName(buf);
      if(strlen(name) + cmdline_pos >= 256) { 
        drawError(title, ERR_RECV_STRING);
        goto end;      
      }
      strcpy(cmdline + cmdline_pos, name);
    }

    // �८�ࠧ������ �⭮�⥫쭮�� ��� � ��᮫���
    absolutePath();
    
    if(copymode) {
      e = cmd_copy(buf, cmdline);
    } else {
      // ��७�� 䠩�� ��� �����
      e = fs_move(buf, cmdline);

      // �뢮� �訡��
      drawError("�訡�� ��७��/��२���������", e);
    }
  }

  // �� ��७�� 䠩�� ���� �������� ��� ������
  // � �� ����஢���� ᯨ᮪ 䠩�� �ᯮ������ ��� ����
  // ���쪮 �᫨ �� �뫮 �訡��
  if(!e) {
    getFiles();
    swapPanels(); getFiles(); swapPanels();
  }

  // ����⠭������� �࠭ � ��� ��ப�
end:
  repairScreen(0);
}

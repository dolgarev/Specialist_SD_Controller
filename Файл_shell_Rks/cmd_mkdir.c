#include "shell.h"
#include "dlg.h"

void cmd_new(uchar dir) {
  uchar e;
  char* title;

  // ��頥� ��� ��ப�
  clearCmdLine();

  // ���� ����� 䠩��
  title = dir ? " �������� ����� " : " �������� 䠩�� ";
  if(inputBox(title)) {

    // �᫨ ��� �������
    if(cmdline_pos) {    

      // �८�ࠧ������ �⭮�⥫쭮�� ��� � ��᮫���
      absolutePath();

      if(dir) {
        // �������� �����
        e = fs_mkdir(cmdline);

        // ���������� ᯨ᪠ 䠩���
        if(!e) getFiles();

        // �뢮� �訡��
        drawError("�訡�� ᮧ����� �����", e);
      } else {
        // ������塞 � ��� ��ப� *
        if(cmdline_pos==255) { 
          drawError("�訡�� ᮧ����� 䠩��", ERR_RECV_STRING);
        } else {
//!          memcpy_back(cmdline+1, cmdline, strlen(cmdline)+1);
//!          cmdline[0] = '*';

          // ����� ।����
          drawError("�訡�� ᮧ����� 䠩��", fs_create(cmdline));

//!          cmd_run("edit.rks", 0);
          return; // ��� ��뢠���� repairScreen

          // �㭪�� fs_exec ��ࠥ� ��ப�, � �� ������
//          if(!dir) drawHelp(); 
        }
      }
    }
  }

  // ����⠭������� �࠭ � ��� ��ப�
  repairScreen(0);
}

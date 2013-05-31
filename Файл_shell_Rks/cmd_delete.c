#include "shell.h"

void cmd_delete() {
  uchar e;

  // �������� � ���� ��� ��� ���
  getSelectedName(cmdLine);
  if(cmdline[0]==0) { 
    clearCmdLine();
    return;
  }
  cmdLine_pos = strlen(cmdLine);

  // ������ � ���짮��⥫�
  if(inputBox(" �������� ")) {

    // �८�ࠧ������ �⭮�⥫쭮�� ��� � ��᮫���
    absolutePath();

    // �������� 䠩��
    e = fs_delete(cmdline);

    // ���������� ᯨ᪠ 䠩���
    if(!e) getFiles();

    // �뢮� �訡��
    drawError("�訡�� 㤠����� 䠩��", e);
  }

  // ����⠭������� �࠭ � ��� ��ப�
  repairScreen(0);
}

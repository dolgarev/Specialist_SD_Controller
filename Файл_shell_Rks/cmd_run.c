#include "shell.h"

void cmd_run2(const char* prog, const char* cmdLine) {
  char e;

  // �����
  e = fs_exec(prog, cmdLine);

  // �뢮��� �訡��
  drawError(prog, e); 

  // ����⠭������� �࠭ � ��� ��ப�
  repairScreen(0);
}

void cmd_run(const char* prog, uchar selectedFile) {  
  char *p, *c;

  if(selectedFile) getSelectedName(cmdline);

  // �᫨ 㪠���� ��� �ணࠬ�, � ��� ��ப� ��।����� ��������� 
  c = cmdLine;

  // �᫨ �� 㪠���� ��� �ணࠬ�
  if(prog == 0)  {

    // �८�ࠧ������ �⭮�⥫쭮�� ��� � ��᮫���
    absolutePath();

    // ���������� ��� ��ப� �� ��ࢮ�� �஡���
    prog = cmdLine;
    c = strchr(prog, 32);
    if(c) *c=0, ++c; else c="";
  }

  cmd_run2(prog, c);
}

// �� �����ন������ � ��� . � ..
#include <spec/bios.h>
#include <spec/color.h>
#include <n.h>
#include <stdlib.h>
#include "graph.h"
#include <mem.h>
#include <string.h>
#include "shell.h"
#include "keyb.h"
#include "dlg.h"

struct Panel panelA, panelB;
uint panelGraphOffset;

const char parentDir[20] = { '.','.',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x10,0,0,0,0,0,0,0,0 };

char test[256];

void  moveCursor_();
void drawCmdLine();

ushort cursorX1;
uchar cursorY1;

char cmdline[256];
uint cmdline_pos = 0;

#define COLUMNS_CNT 2
#define ROWS_CNT 18

void printName(uchar* a, char* name) {
  print1(a, 0, 8, name); 
  print1(a + 0x600, 1, 3, name + 8); 
}

void absolutePath() {
  uint l;
  if(cmdline[0] == '/') {
    strcpy(cmdline, cmdline+1);
  } else {
    l = strlen(panelA.path+1);
    if(l != 0) l++;
    memcpy_back(cmdline+l, cmdline, strlen(cmdline)+1);
    memcpy(cmdline, panelA.path+1, l);
    if(l != 0) cmdline[l-1] = '/';
  }
}

void drawColumn_(uchar i) {
  uchar* a;
  uchar x = 3+i*15, y;
  ushort xx;
  register ushort n = panelA.offset+i*ROWS_CNT;
  FileInfo* f = panelA.files1 + n;  

  graph0();

  a = TEXTCOORDS(x,2);
  for(y=0; y!=ROWS_CNT; ++y, a+=10) {
    if(n>=panelA.cnt) {
      fillRect1(a,8,255,255,(ROWS_CNT-y)*10);
      break;
    }
    printName(a, f->fname);
    ++f; ++n;
  }
}

void drawFiles_() {
  drawColumn_(0);
  drawColumn_(1);
}

void drawFile1(uchar* a, uchar x, uchar y) {
  register ushort n = panelA.offset+y+x*ROWS_CNT;
  graph0();
  if(n<panelA.cnt) printName(a, panelA.files1[n].fname); //print1(a, 0, 11, panelA.files1[n].fname);
              else print1(a, 0, 11|0x80, "");
}

void drawFile(uchar x, uchar y) {
  drawFile1(TEXTCOORDS(3+x*15, 2+y), x, y);
}

void hideCursor_() {
  if(cursorX1) {
    setColor(COLOR_CYAN);
    graphXor();
    fillRect(cursorX1-3, cursorY1, cursorX1-1, cursorY1+9);
    fillRect(cursorX1+(12*6)+1, cursorY1, cursorX1+(12*6)+3, cursorY1+9);
    setColor(COLOR_WHITE);
    fillRect(cursorX1, cursorY1, cursorX1+(12*6), cursorY1+9);
    cursorX1 = 0;
  }
}

void normalizeFileName(char* d, const char* s) {
  uchar i;
  for(i=0; i!=11; ++i, ++s) {
    if(i==8) *d = '.', ++d;
    if(*s!=' ') *d = *s, ++d;
  }
  if(d[-1]=='.') --d;
  *d = 0;
}

void getSelectedName(char* name) {
  uint n;
  n = panelA.offset+panelA.cursorY+panelA.cursorX*ROWS_CNT;
  if(n>=panelA.cnt) { name[0]=0; return; }
  normalizeFileName(name, panelA.files1[n].fname);
}

void drawFileInfo_() {
  ushort n;
  FileInfo* f;
  char buf[16];

  setColor(COLOR_WHITE);
  graph0();

  n = panelA.offset+panelA.cursorY+panelA.cursorX*ROWS_CNT;
  if(n>=panelA.cnt) f = (FileInfo*)parentDir;
               else f = panelA.files1 + n;

  if(f->fattrib & 0x10) {
    print1(PRINTARGS(3,21), 10, "     \x10DIR\x11");
  } else {
    i2s32(buf, 10, &f->fsize, ' ');           
    print1(PRINTARGS(3,21), 10, buf);
  }

//    ���� 0-4 - ���� �����, ����᪠���� ���祭�� 1-31;
//    ���� 5-8 - ����� ����, ����᪠���� ���祭�� 1-12;
//    ���� 9-15 - ���, ���� �� 1980 ���� (<�� MS-DOS>), �������� ���祭�� �� 0 �� 127 �����⥫쭮, � ���� 1980-2107 ����.

  if(f->fdate==0 && f->ftime==0) {
    print1(PRINTARGS(14,21), 16|0x80, "");
  } else {
    i2s(buf, f->fdate & 31, 2, ' ');  
    buf[2] = '-';
    i2s(buf+3, (f->fdate>>5) & 15, 2, '0');
    buf[5] = '-';
    i2s(buf+6, (f->fdate>>9)+1980, 4, '0');
    buf[10] = ' ';
    i2s(buf+11, f->ftime>>11, 2, '0');
    buf[13] = ':';
    i2s(buf+14, (f->ftime>>5)&63, 2, '0');
    print1(PRINTARGS(14,21), 32, buf);
  }

//    ���� 0-4 - ���稪 ᥪ㭤 (�� ���), �����⨬� ���祭�� 0-29, � ���� 0-58 ᥪ㭤;
//    ���� 5-10 - ������, �����⨬� ���祭�� 0-59;
//    ���� 11-15 - ���, �����⨬� ���祭�� 0-23.

}

void moveCursor_() {
  hideCursor_();
  cursorX1 = panelA.cursorX*88+16;
  cursorY1 = (panelA.cursorY+2)*10;
  
  setColor(COLOR_CYAN);
  graphXor();
  fillRect(cursorX1-3, cursorY1, cursorX1+(12*6)+3, cursorY1+9);

  drawFileInfo_();
}

void swapPanels() {
  panelGraphOffset = 0x1700 - panelGraphOffset;
  memswap(&panelA, &panelB, sizeof(struct Panel));  
}

void clearCmdLine() {
  cmdline_pos = 0;
  drawCmdLine();
}

void drawPathInCmdLine() {
  ushort o, l, old;
  graph0();
  setColor(COLOR_WHITE);  
  fillrect1(FILLRECTARGS(0,230,383,239));
  l = strlen(panelA.path);
  if(l>=30) o=l-30, l=30; else o=0;
  print1(PRINTARGS(1, 23), l, panelA.path+o);
  panelA.cmdLineOff = 2+l;
  print1(PRINTARGS(panelA.cmdLineOff-1, 23), 1, ">");
  drawCmdLine();
}

void drawPath_(uchar active) {
  ushort l;
  ushort x,x1,x2,x3;
  char* p;

  p  = panelA.path;

  setColor(COLOR_CYAN);

  x1 = 18;
  x3 = x1+6*27;
  graph0();
  fillRect(x1,0,x3,9);
  graph1();
  fillRect(x1,3,x3,3);
  fillRect(x1,5,x3,5);

  if(active) graph1(); else graph0();

  l = strlen(p);
  if(l>=25) p=panelA.path+(l-25), l=25;
  x = (32*3-l*3)/4;
  fillRect(x*4-3, 0, x*4+l*6+6, 9);
  print1(PIXELCOORDS((x/2), 0), (((uchar)x)&1) ? 1 : 3, l, p);

  setColor(COLOR_WHITE);  
}

// 


void drawCmdLine() {
  cmdline[cmdline_pos] = 0;
  graph0();
  drawInput(PRINTARGS(panelA.cmdLineOff, 23), 62-panelA.cmdLineOff);
}

#define SORT_STACK_MAX 32

uchar cmpFileInfo(FileInfo* a, FileInfo* b) {
  uchar i, j;
  i = (a->fattrib&0x10);
  j = (b->fattrib&0x10);
  if(i<j) return 1;
  if(j<i) return 0;
  if(1==memcmp(a->fname, b->fname, sizeof(a->fname))) return 1;
  return 0;
}

void sort(FileInfo* low, FileInfo* high) {
  FileInfo *i, *j, *x;
  FileInfo *st_[SORT_STACK_MAX*2], **st = st_;
  uchar stc = 0;
  while(1) {
    i = low;
    j = high;
    x = low + (high-low)/2;
    while(1) {
      while(0!=cmpFileInfo(x, i)) i++;
      while(0!=cmpFileInfo(j, x)) j--;
      if(i <= j) {
        memswap(i, j, sizeof(FileInfo));
        if(x==i) x=j; else if(x==j) x=i;
        i++; j--;   
      }
      if(j<=i) break;
    }
    if(i < high) {
      if(low < j) if(stc != SORT_STACK_MAX) *st = low, ++st, *st = j, ++st, ++stc;
      low = i; 
      continue;
    }
    if(low < j) { 
      high = j;
      continue; 
    }
    if(stc==0) break;
    --stc, --st, high = *st, --st, low = *st; 
  }
}


void prepareFileName(char* buf, char *path) {
  uchar c, ni, i;

  memset(buf, ' ', 11);    
  i = 0; ni = 8;
  while(1) {
    c = *path; ++path;
    if(c == 0) return;
    if(c == '.') { i = 8; ni = 11; continue; }                 
    if(i == ni) continue;
    *buf = c; ++buf;
  }                     
}

void getFiles() {
  FileInfo *f, *st;
  char *n;
  uchar i;
  uint j;
  FileInfo dir;
    
  panelA.cnt = 0;
  panelA.offset = 0;
  panelA.cursorX = 0;
  panelA.cursorY = 0;

  f = panelA.files1;
  if((panelA.path[0]!='/' || panelA.path[1]!=0)) {
    memcpy(f, parentDir, sizeof(FileInfo));
    ++f;
    ++panelA.cnt;    
  }
  st = f;
  i = fs_findfirst(panelA.path, f, MAX_FILES-panelA.cnt);  
  if(i==ERR_MAX_FILES) i=0;
  if(i==0) {
    f += fs_low;
    panelA.cnt += fs_low;
  } else {
    drawError("����祭�� ᯨ᪠ 䠩���", i);
  }
 
  for(j=panelA.cnt, f=panelA.files1; j; --j, ++f) {
    if((f->fattrib & 0x10)==0)
      for(i=12, n=f->fname; i; --i, ++n)
        if((uchar)*n>='A' && (uchar)*n<='Z')
          *n = *n-('A'-'a');
  }

  if(panelA.cnt > 1)
    sort(st, ((FileInfo*)panelA.files1) + (panelA.cnt-1));
}

void reloadFiles(char* back) {  
  ushort l;
  FileInfo* f;

  graphOffset = panelGraphOffset;
  drawPath_(1);   
  graphOffset = 0;

  drawPathInCmdLine();

  getFiles();

  // ���� � ����� ������� �����
  if(back)
  for(l=0, f=panelA.files1; l<panelA.cnt; ++l, ++f) {
    if(0==memcmp(f->fname, back, 11)) {
      // �������. ���� ������ ����� �� ����� ������, �������� ������.
      if(l>=2*ROWS_CNT) {
        panelA.offset = l-ROWS_CNT-(l%ROWS_CNT);
        l-=panelA.offset;
      }
      // ������������ ������
      panelA.cursorX = l/ROWS_CNT;
      panelA.cursorY = op_div16_mod;
      break;
    }
  }

  // �������������� ����� � ������
  graphOffset = panelGraphOffset;
  hideCursor_();
  drawFiles_();
  moveCursor_();
  graphOffset = 0;
}

void dropPath() {
  ushort l;
  char buf[11];
  char* p;
  FileInfo* f;

  // �������� �� ���� ��������� �����
  for(l=strlen(panelA.path), p=panelA.path+l;; --l, --p) {
    if(*p=='/') { 
      prepareFileName(buf, p+1);
      if(l==0) ++p;
      *p=0;
      break;
    }
    if(l==0) return;
  }

  reloadFiles(buf);
/*
  // ��������� �����
  graphOffset = panelGraphOffset;
  drawPath_(1);   
  graphOffset = 0;
  drawPathInCmdLine();
  getFiles();

  // ���� � ����� ������� �����
  for(l=0, f=panelA.files1; l<panelA.cnt; ++l, ++f) {
    if(0==memcmp(f->fname, buf, 11)) {
      // �������. ���� ������ ����� �� ����� ������, �������� ������.
      if(l>=2*ROWS_CNT) {
        panelA.offset = l-ROWS_CNT-(l%ROWS_CNT);
        l-=panelA.offset;
      }
      // ������������ ������
      panelA.cursorX = l/ROWS_CNT;
      panelA.cursorY = op_div16_mod;
      break;
    }
  }

  // �������������� ����� � ������
  graphOffset = panelGraphOffset;
  hideCursor_();
  drawFiles_();
  moveCursor_();
  graphOffset = 0;
*/
}

#define A_ENTER   0
#define A_CMDLINE 1
//#define A_CMDLINE_NOPREP 2
//#define A_CMDLINE_FILE 3

uchar addPath1(uchar mode) {
  char buf[13];
  char *d, *s, i;
  FileInfo* f;
  uint l, o;

  // �������
  if(panelA.cnt == 0) return 1;

  // ��������� ����
  f = panelA.files1 + (panelA.offset + panelA.cursorY + panelA.cursorX*ROWS_CNT);
  s = f->fname;

  // ������ �������� �����
//  if(mode == A_CMDLINE_FILE)
//    if(f->fattrib & 0x10) 
//      return 1;

  // ����� �� �����
  if(s[0] == '.') { 
    if(mode != A_ENTER) return 1;
    dropPath(); 
    return 0; 
  }

  // ����������� ��� �����
  normalizeFileName(buf, s);

  // ������ ����� ����
  o = strlen(panelA.path);

  // ��������� ��������� ���� � ����
  d = panelA.path + o;
  if(o != 1) {
    if(o + strlen(buf) >= 254) return 1; // ������� ������� ����
    *d = '/'; ++d;
  }  
  strcpy(d, buf);    

  // ���� ����� Enter, �� ������ � �����
  if(mode == A_ENTER) {
    if(f->fattrib & 0x10) { 
      reloadFiles(0);
      return 0;
    }
  }

  // �������� ���� � ��������� ������
  strcpy(cmdline, panelA.path);
  cmdline_pos = strlen(cmdline);
  panelA.path[o] = 0;

  // ����������� ���� � ������ ��
  absolutePath();

  // ���� ����� Enter, �� ��������� ����
  if(mode == A_ENTER) {
    cmd_run2(cmdline, "");
    clearCmdLine();
  }

  return 0;
}

void drawAll_() { 
  graph1();
  setColor(COLOR_CYAN);
  rect1(RECTARGS(6,3,186,45+ROWS_CNT*10));
  rect1(RECTARGS(8,5,184,43+ROWS_CNT*10));
  fillRect1(FILLRECTARGS(96,6,96,24+ROWS_CNT*10));
  fillRect1(FILLRECTARGS(9,25+ROWS_CNT*10,184,25+ROWS_CNT*10));
  setColor(COLOR_YELLOW);
  graph0();
  print1(TEXTCOORDS(7,1), 0, 4, "Name");
  print1(TEXTCOORDS(22,1), 0, 4, "Name");
  setColor(COLOR_WHITE);
}

void drawHelp() {
  uchar i;
  uchar* d;
  
  graph0();
  setColor(COLOR_WHITE);
//  fillRect1(TEXTCOORDS(0, 24)-1, 47, 255, 255, 16);
  print1(TEXTCOORDS(1, 24), 3, 64, "1FREE   2NEW    3VIEW   4EDIT   5COPY   6REN    7DIR    8DEL"); 

  setColor(COLOR_CYAN);
  graphXor();
  for(i=0, d = TEXTCOORDS(2, 24)-1; i<8; i++, d+=0x600)
    fillRect1(d, 3, 255, 255, 10);
}

void dupPanel() {
  memcpy(panelA.files1, panelB.files1, sizeof(FileInfo)*panelB.cnt);
  panelA.cnt = panelB.cnt; 
}

void drawFiles2_go() {
  graphOffset = panelGraphOffset; hideCursor_(); drawFiles_(); moveCursor_();
  swapPanels();
  graphOffset = panelGraphOffset; drawFiles_(); drawFileInfo_();
  swapPanels();
  graphOffset = 0;
}

void repairScreen(uchar dontDrawFiles) {
  graph0();

  // ��頥� �࠭
  clrscr10(PIXELCOORDS(0,23), 48, 23);

  // ���㥬 ���᪠���
  //drawHelp();

  // ��頥� ��� ��ப�
  cmdline[0] = 0;
  cmdline_pos = 0;
  drawPathInCmdLine();

  // ���㥬 ࠬ��
  graphOffset = 0;      drawAll_();
  graphOffset = 0x1700; drawAll_();

  // ���㥬 ���������
  graphOffset = panelGraphOffset; drawPath_(1);
  swapPanels();
  graphOffset = panelGraphOffset; drawPath_(0);
  swapPanels();

  // ����� ���
  cursorX1 = 0;

  // �뢮��� ᯨ᮪ 䠩���
  if(!dontDrawFiles) drawFiles2_go();

  graphOffset = 0;
}

void cursor_left() {
  graphOffset = panelGraphOffset;

  if(panelA.cursorX) { 
    --panelA.cursorX; 
  } else
  if(panelA.offset) { 
    if(ROWS_CNT > panelA.offset) { 
      panelA.offset = 0; 
      hideCursor_();
      drawFiles_();
    } else {
      panelA.offset -= ROWS_CNT; 
      hideCursor_();
      scroll((char*)0x9D14, (char*)0x9214, 10, ROWS_CNT*10);
      drawColumn_(0);
    }
  } else
  if(panelA.cursorY) {
    panelA.cursorY = 0; 
  }

  moveCursor_();

  graphOffset = 0;
}

void cursor_right() {
  uint w;

  graphOffset = panelGraphOffset;

  // ������������ ������ ������
  w = panelA.offset + panelA.cursorY + panelA.cursorX*22;
  if(w + ROWS_CNT >= panelA.cnt) { //! ���������� > � >=
    // ��� ��������� ����
    if(w + 1 >= panelA.cnt) { 
      graphOffset = 0;
      return;
    }
    // ��������� ��������� �� Y
    panelA.cursorY = panelA.cnt - (panelA.offset + panelA.cursorX*ROWS_CNT + 1);
    // ������������ ������
    if(panelA.cursorY>ROWS_CNT-1) {
      panelA.cursorY -= ROWS_CNT;
      if(panelA.cursorX == 1) { 
        panelA.offset += ROWS_CNT;
        hideCursor_();
        drawFiles_();
      } else {
        panelA.cursorX++; 
      }
    }
  } else
  if(panelA.cursorX == 1) { 
    panelA.offset += ROWS_CNT;
    hideCursor_();
    scroll((char*)0x9214, (char*)0x9D14, 10, ROWS_CNT*10);
    drawColumn_(1);
  } else {
    panelA.cursorX++;
  }

  moveCursor_();

  graphOffset = 0;
}

void cursor_up() {
  graphOffset = panelGraphOffset;

  if(panelA.cursorY) { 
    --panelA.cursorY;
  } else        
  if(panelA.cursorX) { 
    --panelA.cursorX;
    panelA.cursorY = ROWS_CNT-1; 
  } else        
  if(panelA.offset) {
    --panelA.offset; 
    hideCursor_();
    scroll((char*)(0x9214+10), (char*)0x9214, 10, (ROWS_CNT-1)*10);
    scroll((char*)(0x9D14+10), (char*)0x9D14, 10, (ROWS_CNT-1)*10);
    drawFile(0, 0);
    drawFile(1, 0);
  }

  moveCursor_();

  graphOffset = 0;
}

void cursor_down() {
  if(panelA.offset + panelA.cursorX*ROWS_CNT + panelA.cursorY + 1 >= panelA.cnt) return;

  graphOffset = panelGraphOffset;

  if(panelA.cursorY < ROWS_CNT-1) {
    ++panelA.cursorY;
  } else        
  if(panelA.cursorX == 0) {
    panelA.cursorY = 0;
    ++panelA.cursorX; 
  } else { 
    ++panelA.offset; 
    hideCursor_();
    scroll((char*)0x9214, (char*)(0x9214+10), 10, (ROWS_CNT-1)*10);
    scroll((char*)0x9D14, (char*)(0x9D14+10), 10, (ROWS_CNT-1)*10);
    drawFile(0, ROWS_CNT-1);
    drawFile(1, ROWS_CNT-1);
  }

  moveCursor_();

  graphOffset = 0;
}

void cmd_tab() {
  graphOffset = panelGraphOffset; 
  hideCursor_();
  drawPath_(0);
  swapPanels();
  graphOffset = panelGraphOffset; 
  moveCursor_();
  drawPath_(1);
  graphOffset = 0; 
  drawPathInCmdLine();
}

void cmd_char(char c) {
  if(cmdline_pos == 255) return; 
  cmdline[cmdline_pos] = c;
  ++cmdline_pos;
  drawCmdLine();
}

void cmd_bkspc() {
  if(cmdline_pos == 0) return;
  --cmdline_pos;    
  drawCmdLine();
}

void main() {
  uchar c, y, w, r;
  uchar* e;
  uint l;

  fs_init();

  setColorAutoDisable();
 
  panelA.files1 = (FileInfo*)START_FILE_BUFFER;
  panelB.files1 = ((FileInfo*)START_FILE_BUFFER)+MAX_FILES;
  
  clrscr();

  //puthex(MAX_FILES/256);
  //puthex(MAX_FILES);
  //getch();

  graphOffset = 0;
  cursorX1=0;

  // ������
  strcpy(panelA.path, "/");
  strcpy(panelB.path, "/");
  drawHelp();           
  repairScreen(0);

  // �������� ������
  getFiles();
  memcpy(panelB.files1, panelA.files1, panelA.cnt*sizeof(*panelA.files1));
  panelB.cnt = panelA.cnt;

  // ������ �����
  drawFiles2_go();

  while(1) {
    c = getch1();

    switch(c) {
      case KEY_BKSPC: cmd_bkspc(); continue;
      case KEY_F1:    cmd_freespace(); continue;
      case KEY_F2:    cmd_new(0); continue;
      case KEY_F3:    cmd_run("view.rks", 1); continue;
      case KEY_F4:    if(shiftPressed()) cmd_new(0); else cmd_run("edit.rks", 1); continue;
      case KEY_F5:    cmd_copymove(1); continue;
      case KEY_F6:    cmd_copymove(0); continue;
      case KEY_F7:    cmd_new(1); continue;
      case KEY_F8:    cmd_delete(); continue;
      case KEY_ENTER: if(cmdline_pos) cmd_run(0, 0); else addPath1(A_ENTER); continue;
      case KEY_ESC:   if(cmdline_pos) clearCmdLine(); else dropPath(); continue;
      case KEY_LEFT:  cursor_left(); continue;
      case KEY_RIGHT: cursor_right(); continue; 
      case KEY_DOWN:  cursor_down(); continue;
      case KEY_UP:    cursor_up(); continue; 
      case KEY_TAB:   cmd_tab();  continue; 
    }                 

    cmd_char(c);
  }
}

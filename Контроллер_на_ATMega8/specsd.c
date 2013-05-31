// SD Controller for Computer "Specialst"
// (c) 26-05-2013 vinxru (aleksey.f.morozov@gmail.com)

//#include <stdafx.h>

#define F_CPU 8000000UL        //freq 8 MHz

#include "common.h"
#include <string.h>
#include "sd.h"
#include "fs.h"
#include "proto.h"

#ifndef X86_DEBUG
#include <delay.h>
#endif

#define O_OPEN   0
#define O_CREATE 1
#define O_MKDIR  2
#define O_DELETE 100
#define O_SWAP   101

#define ERR_START       0x40
#define ERR_WAIT        0x41
#define ERR_OK_NEXT         0x42
#define ERR_OK_CMD          0x43
#define ERR_OK_READ         0x44
#define ERR_OK_ENTRY        0x45
#define ERR_OK_WRITE        0x46
#define ERR_OK_RKS          0x47

unsigned char buf[512];

/*******************************************************************************
* ��� ��������                                                                 *
*******************************************************************************/

void recvBin(BYTE* d, WORD l) {
  for(; l; --l) {
    *d++ = wrecv();
  }  
}

void recvString() {
  BYTE c;
  BYTE* p = buf;       
  do {
    c = wrecv();
    if(p != buf + FS_MAXFILE) *p++ = c; else lastError = ERR_RECV_STRING;
  } while(c);
}

void sendBin(BYTE* p, WORD l) {
  for(; l; l--)
    send(*p++);
}

void sendBinf(flash BYTE* d, BYTE l) {
  for(; l; --l)
    send(*d++);
}

/*******************************************************************************
* �������� ���� ������ �����                                                   *
*******************************************************************************/

WORD readLength;

void readInt(char rks) { 
  WORD readedLength, lengthFromFile;
  BYTE* wptr;

  while(readLength) { 
    // ������ ����� ����� (����������� ������ �� ������)
    if(fs_tell()) return;
    readedLength = 512 - (fs_tmp % 512);
    if(readedLength > readLength) readedLength = readLength;

    // ��������� �������
    readLength -= readedLength;

    // ������ ����
    if(fs_read0(buf, readedLength)) return;

    // ��������� RKS �����
    wptr = buf;
    if(rks) { // ���� rks=1, ����� ������� ���� ���������, ��� �� readLength>4 � fs_file.ptr=0, ����� ����� ���� �������� ����
      rks = 0;

      // �������� ����� ��������
      send(ERR_OK_RKS);
      sendBin(buf, 2);    
      send(ERR_WAIT);

      // ������������ ���������
      wptr += 4;
      readedLength -= 4;

      // ����� �� �����
      lengthFromFile = *(WORD*)(buf+2) - *(WORD*)(buf) + 1;

      // ������������ �����  
      if(readedLength > lengthFromFile) {
        readedLength = lengthFromFile;
      } else {          
        lengthFromFile -= readedLength;
        if(readLength > lengthFromFile) lengthFromFile = readedLength;
      }
    }  

    // ���������� ����
    send(0);    
    sendBin((BYTE*)&readedLength, 2);
    sendBin(wptr, readedLength);
    send(ERR_WAIT);
  }

  // ���� ��� ��
  if(!lastError) lastError = ERR_OK_READ;
}

/*******************************************************************************
* ������ ������ �����������                                                    *
*******************************************************************************/

void cmd_ver() {
  sendStart(1);
    
  // ������ + �������������
  sendBinf("V1.0 23-05-2013 ", 16);
              //0123456789ABCDEF
}

/*******************************************************************************
* BOOT / EXEC                                                                  *
*******************************************************************************/

void cmd_boot_exec() {
  // ���� �� ���������
  if(buf[0]==0) strcpyf(buf, "boot/sdbios.rks");      

  // ��������� ����
  if(fs_open()) return;
  
  // ������������ ������ �����
  readLength = 0xFFFF;  
  if(fs_getfilesize()) return;
  if(readLength > fs_tmp) readLength = (WORD)fs_tmp;

  // ����� RKS ������ ���� ������ >4 ����. �� ������� � readLength = 0 � ���������
  // �������� ERR_OK. �� ��� ��� ��� ���� ERR_OK_RKS, ��� ����� ������� 
  if(readLength < 4) readLength = 0;

  readInt(/*rks*/1);  
}

void cmd_boot() { 
  buf[0] = 0;
  cmd_boot_exec();  
}

void cmd_exec() {     
  // ����� ����� �����
  recvString();

  // ����� �������� � �������������
  sendStart(ERR_WAIT);
  if(lastError) return; // ������������ ������
  
  cmd_boot_exec();    
}

/*******************************************************************************
* ������/���������� ����� ������ � �����                                       *
*******************************************************************************/

typedef struct {
    char    fname[11];    // File name
    BYTE    fattrib;    // Attribute
    DWORD   fsize;        // File size
    union {
      struct {
        WORD    ftime;        // Last modified time
        WORD    fdate;        // Last modified date 
      };
      DWORD ftimedate;
    };
} FILINFO2;

void cmd_find() {
  WORD n;
  FILINFO2 info;              
  
  // ��������� ����
  recvString();

  // ��������� ���� ���-�� ���������
  recvBin((BYTE*)&n, 2);

  // ����� �������� � �������������
  sendStart(ERR_WAIT);
  if(lastError) return;

  // ��������� �����
  if(buf[0] != ':') {
    if(fs_opendir()) return;
  }

  for(; n; --n) {
    /* ������ �������� ��������� */
    if(fs_readdir()) return;

    /* ����� */
    if(FS_DIRENTRY[0] == 0) {
      lastError = ERR_OK_CMD;
      return;
    }

    /* ������� ����� ��� ���������� */
    memcpy(info.fname, FS_DIRENTRY+DIR_Name, 12);
    memcpy(&info.fsize, FS_DIRENTRY+DIR_FileSize, 4);
    memcpy(&info.ftimedate, FS_DIRENTRY+DIR_WrtTime, 4);
    //memcpy(memcpy(memcpy(info.fname, FS_DIRENTRY+DIR_Name, 12, FS_DIRENTRY+DIR_FileSize, 4), FS_DIRENTRY+DIR_WrtTime, 4);

    /* ���������� */
    send(ERR_OK_ENTRY);
    sendBin((BYTE*)&info, sizeof(info));
    send(ERR_WAIT);
  }

  /* ����������� �� ������� */  
  lastError = ERR_MAX_FILES; /*! ���� �����������, ��� �� �� ���� ������ ������ */
}

/*******************************************************************************
* �������/������� ����/�����                                                   *
*******************************************************************************/

void cmd_open() {
  BYTE mode;
 
  /* ��������� ����� */
  mode = wrecv();    

  // ��������� ��� �����
  recvString();

  // ����� �������� � �������������
  sendStart(ERR_WAIT);

  // ���������/������� ����/�����
  if(mode == O_SWAP) {
    fs_swap();
  } else
  if(mode == O_DELETE) {
    fs_delete();
  } else
  if(mode == O_OPEN) {
    fs_open();
  } else 
  if(mode < 3) {
    fs_open0(mode);
  } else {
    lastError = ERR_INVALID_COMMAND;
  }

  // ��
  if(!lastError) lastError = ERR_OK_CMD;
}

/*******************************************************************************
* ����������� ����/�����                                                       *
*******************************************************************************/

void cmd_move() {
  recvString();
  sendStart(ERR_WAIT);
  fs_openany();
  sendStart(ERR_OK_WRITE);
  recvStart();
  recvString();
  sendStart(ERR_WAIT);
  if(!lastError) fs_move0();
  if(!lastError) lastError = ERR_OK_CMD;
}

/*******************************************************************************
* ����������/��������� ��������� ������                                        *
*******************************************************************************/

void cmd_lseek() {
  BYTE mode;
  DWORD off;

  // ��������� ����� � ��������
  mode = wrecv();    
  recvBin((BYTE*)&off, 4);    

  // ����� �������� � �������������
  sendStart(ERR_WAIT);

  // ������ �����
  if(mode==100) {
    if(fs_getfilesize()) return;
  }

  // ������ �����  
  else if(mode==101) {
    if(fs_gettotal()) return;
  }
 
  // ��������� ����� �� �����
  else if(mode==102) {
    if(fs_getfree()) return;
  }

  else {
    /* ������������ ��������. fs_tmp ����������� */
    if(fs_lseek(off, mode)) return;
  }

  // �������� ���������
  send(ERR_OK_CMD);
  sendBin((BYTE*)&fs_tmp, 4);  
  lastError = 0; // �� ������ ������, ��������� ��� �������
}

/*******************************************************************************
* ��������� �� �����                                                           *
*******************************************************************************/

void cmd_read() {
  DWORD s;

  // �����
  recvBin((BYTE*)&readLength, 2);

  // ����� �������� � �������������
  sendStart(ERR_WAIT);

  // ������������ ����� ������ �����
  if(fs_getfilesize()) return;
  s = fs_tmp; 
  if(fs_tell()) return;
  s -= fs_tmp;
                    
  if(readLength > s)
    readLength = (WORD)s;

  // ���������� ��� ����� �����
  readInt(/*rks*/0);
}

/*******************************************************************************
* �������� ������ � ����                                                       *
*******************************************************************************/

void cmd_write() {
  // ���������
  recvBin((BYTE*)&fs_wtotal, 2); 

  // �����
  sendStart(ERR_WAIT);
           
  // ����� �����
  if(fs_wtotal==0) {
    fs_write_eof();
    lastError = ERR_OK_CMD;
    return;
  }
  
  // ������ ������
  do {
    if(fs_write_start()) return;

    // ��������� �� ���������� ���� ������
    send(ERR_OK_WRITE);
    sendBin((BYTE*)&fs_file_wlen, 2);
    recvStart();    
    recvBin(fs_file_wbuf, fs_file_wlen);
    sendStart(ERR_WAIT);

    if(fs_write_end()) return;
  } while(fs_wtotal);

  lastError = ERR_OK_CMD;
}

/*******************************************************************************
*                                                                              *
*******************************************************************************/

#define TAPE_DELAY         (1000000/1400/2)
#define TAPE               PORTC.1
#define TAPE_PILOT_SIZE    32

void tape(BYTE data) {
  BYTE i = 8;
  do {
    if(data & 0x80) {
      TAPE = 1; delay_us(TAPE_DELAY); // TAPE = const - ��� ���� ������� ����������,
      TAPE = 0; delay_us(TAPE_DELAY); // ������� ����������� ��������� ���������.
    } else {
      TAPE = 0; delay_us(TAPE_DELAY);
      TAPE = 1; delay_us(TAPE_DELAY);    
    }
    data <<= 1;
  } while(--i);
}

BYTE tapeEmulator() {
  WORD i, s;
  BYTE* p;

  strcpyf(buf, "boot/boot.rks");
  if(fs_open()) goto abort;
  if(fs_getfilesize()) goto abort;  
  if(fs_tmp < 7) goto abort;
  if(fs_tmp > 512) goto abort;
  s = (WORD)fs_tmp; 
  if(fs_read0(buf, s)) goto abort;
  
  i = TAPE_PILOT_SIZE;
  do {
    tape(0);
  } while(--i);
        
  tape(0xE6);

  p = buf;
  do {
    tape(*p++);
  } while(--s);
    
  return 0;                   
abort:
  return 1;
}

/*******************************************************************************
* ������� ���������                                                            *
*******************************************************************************/

#ifdef X86_DEBUG
void test() {
  BYTE c;  
#else
void main() {
  BYTE c;

  // ��������� ������ �����-������
  DDRD  = 0;
  PORTD = 0xFF;
  DDRB  = 0b00101101; // MISO
  PORTB = 0b00010001; // ������������� �������� �� MISO  
  DDRC  = 0b00000010; // TAPEEN, TAPE, PULSE
  PORTC = 0b00000101; // TAPEEN, TAPE, PULSE
  
  // �����, ���� �� ��������������� �������
  delay_ms(100);
#endif  

  // ������ �������� �������
  fs_init();

  // ���� ���� �2 ������� �� �����, ��������� �������� �����������
  if(PINC.2==0) {
    if(tapeEmulator()) {
      // � ������ ������ ������ �����������
      while(1) { PORTB.0 = !PORTB.0; delay_ms(100); }    
    }    
  } 
          
  //strcpyf(buf, "boot.rks");
  //if(fs_open()) {
    //while(1) { PORTB.0 = !PORTB.0; delay_ms(100); }    
  //}  
        
  while(1) {
#ifndef X86_DEBUG
    // ����� ���������
    PORTB.0 = 0;
#endif
    // ��������� ���� ������� �������
    recvStart();
    // ��������� ������ �������
step2:    
    c = wrecv();
retry:
    if(c != 19) goto step2;
    c = wrecv();
    if(c != 180) goto retry;
    c = wrecv();
    if(c != 87) goto retry;
    c = wrecv();
    
#ifndef X86_DEBUG
    // �������� ���������
    PORTB.0 = 1;
#endif

    // ��������� ������� �����
    sendStart(ERR_START);
    send(ERR_WAIT);
    if(fs_check()) {
      send(ERR_DISK_ERR);
      continue;
    }

    // ���������� ������
    lastError = 0;
    
    /* ��������� ��������� */
    if(c == 0) { 
      cmd_boot();
    } else { 
      send(ERR_OK_NEXT);
      recvStart();
           
      switch(c) {
        case 1:  cmd_ver();          break;
        case 2:  cmd_exec();         break; 
        case 3:  cmd_find();         break;
        case 4:  cmd_open();         break;     
        case 5:  cmd_lseek();        break;     
        case 6:  cmd_read();         break;     
        case 7:  cmd_write();        break; 
        case 8:  cmd_move();         break;
        default: lastError = ERR_INVALID_COMMAND;      
      }
    }
    
    // ����� ������
    if(lastError) sendStart(lastError);
  }
}

/*
BYTE buf[512];
void main() {
 fs_init();     
 fs_check();    
 fs_readdir();  
 fs_delete();   
 fs_open0(0);   
 fs_move();     
 fs_lseek(0);   
 fs_read0(0,0); 
 fs_read(0,0,0);
 fs_write_eof();
 fs_write_start(); 
 fs_write_end();
}
*/

#ifndef _6502_H_
#define _6502_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdbool>
#include "dialogeditbreakpoint.h"
#include "ui_dialogeditbreakpoint.h"
typedef uint16_t WORD;
typedef uint8_t BYTE;

extern BYTE opcode;
extern int clockticks6502;
extern int timerTicks;

extern WORD savepc,savepc2;
extern BYTE value;
extern int sum,saveflags;

extern void (*adrmode[256])();
extern void (*instruction[256])();
extern int ticks[256];
extern WORD help;

extern BYTE a_reg,x_reg,y_reg,flag_reg,s_reg;
extern WORD pc_reg;
extern BYTE gameImage[65536];
extern BYTE gameHeader[8192];
extern BYTE gameRom[32768*256];
extern BYTE gameChr[8192*256];
extern BYTE gameBgt[4096*256];
extern BYTE gameBgp[1024*256];
extern BYTE gameWram[8192*256];
extern BYTE gameSram[2048*256];
extern BYTE gamePal[32*256];
extern bool isExec;
extern bool statStp,statWai;
extern int frameAdvance;
extern int bankSizeLut[10];
extern int mrMask[7];
extern int mrVal[7];
extern int mrSize[7];
extern int mrShiftMax[7];
extern int minGranularity[8];
extern int minGranularityRom[8];
extern BYTE *mrPtrs[7];

extern BYTE get6502memory(WORD addr);
extern void put6502memory(WORD addr, BYTE byte);
extern void init6502(void);
extern void reset6502(void);
extern void nmi6502(void);
extern void irq6502(void);
extern void exec6502(void);

#endif

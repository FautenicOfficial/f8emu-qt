#include "6502.h"

/* flags = NVRBDIZC */
BYTE a_reg,x_reg,y_reg,flag_reg,s_reg;
WORD pc_reg = 0;

/* Macros for convenience */
#define A a_reg
#define X x_reg
#define Y y_reg
#define P flag_reg
#define S s_reg
#define PC pc_reg

/* internal registers */
BYTE opcode;
int clockticks6502 = 0;
int timerTicks;

/* help variables */
WORD savepc,savepc2;
BYTE value;
int sum,saveflags;

/* arrays */
void (*adrmode[256])();
void (*instruction[256])();
int ticks[256];
WORD help;
BYTE gameImage[65536];
BYTE gameHeader[8192];
BYTE gameRom[32768*256];
BYTE gameChr[8192*256];
BYTE gameBgt[4096*256];
BYTE gameBgp[1024*256];
BYTE gameWram[8192*256];
BYTE gameSram[2048*256];
BYTE gamePal[32*256];
bool isExec = true;
bool statStp = false,statWai = false;
int frameAdvance = -1;
int bankSizeLut[10] = {0,1,2,4,8,16,32,64,128,256};
int mrMask[7] = {0x8000,0xE000,0xF000,0xFC00,0xE000,0xF800,0xFFE0};
int mrVal[7] = {0x8000,0x4000,0x6000,0x7000,0x2000,0x7800,7500};
#define mrOffset mrVal
int mrSize[7] = {0x8000,0x2000,0x1000,0x0400,0x2000,0x0800,0020};
int mrShiftMax[7] = {16,14,13,11,14,12,7};
int minGranularity[8] = {1,4,3,4,2,4,3,4};
int minGranularityRom[8] = {2,4,3,4,3,4,3,5};
BYTE *mrPtrs[7] = {gameRom,gameChr,gameBgt,gameBgp,gameWram,gameSram,gamePal};

BYTE get6502memory(WORD addr)
{
    for(int i=0; i<numBreakpoints; i++) {
        if((dBreakpointAddr[i]==addr)&&dBreakpointE[i]&&dBreakpointR[i]) {isExec=false; break;}
    }
    return(gameImage[addr]);
}

void put6502memory(WORD addr, BYTE byte)
{
    for(int i=0; i<numBreakpoints; i++) {
        if((dBreakpointAddr[i]==addr)&&dBreakpointE[i]&&dBreakpointW[i]) {isExec=false; break;}
    }
    //Check if we can write to RAM
    //Check if we need to write to SRAM
    if((addr&0x8000)==0 && (addr&0xFF80)!=0x7480) {
        bool canWriteToRam = true;
        for(int i=1; i<7; i++) {
            if((addr&mrMask[i])==mrVal[i]) {
                if(gameHeader[0x10|i]) {
                    if(gameHeader[0x18|i]&0x80) {
                        int a = gameHeader[0x18|i]&0x7F;
                        int offset = addr-mrOffset[i];
                        if(a==0) {
                            mrPtrs[i][offset] = byte;
                        } else {
                            for(int j=1; j<=4; j++) {
                                if(a==j) {
                                    int b = offset&(mrSize[i]-(mrSize[i]>>(a-1)));
                                    int c = offset&((mrSize[i]-1)>>(a-1));
                                    mrPtrs[i][(gameImage[0x7440|(i<<3)|(b>>(mrShiftMax[i]-4))]<<(mrShiftMax[i]-a))|c] = byte;
                                    break;
                                }
                            }
                        }
                    } else {
                        canWriteToRam = false;
                    }
                }
                break;
            }
        }
        if(canWriteToRam) {
            gameImage[addr] = byte;
        }
    }
    //Check if we need to switch banks
    if(((addr&0xFFC0)==0x7440)&&(addr!=0x7447)&&((addr&0xFFF8)!=0x7478)) {
        int i = (addr>>3)&7;
        int b = addr&7;
        int a = gameHeader[0x18|i]&0x7F;
        int realByte = byte&(bankSizeLut[gameHeader[0x10|i]+a]-1);
        for(int j=((i==0)?minGranularityRom[b]:minGranularity[b]); j<=4; j++) {
            if(a==j) {
                int bankOff = (realByte<<(mrShiftMax[i]-a));
                for(int k=0; k<(mrSize[i]>>(a-1)); k++) {
                    gameImage[mrOffset[i]|((mrSize[i]>>3)*b)|k] = mrPtrs[i][bankOff|k];
                }
                break;
            }
        }
    }
}

/* Adressing modes */
/* Implied */
void implied6502()
{
}
/* #Immediate */
void immediate6502()
{
      savepc=PC++;
}
/* ABS */
void abs6502()
{
      savepc = static_cast<WORD>(gameImage[PC] + (gameImage[PC + 1] << 8));
      PC++;
      PC++;
}
/* Branch */
void relative6502()
{
      savepc = gameImage[PC++];
      if (savepc & 0x80) savepc -= 0x100;
      if ((savepc>>8) != (PC>>8))
              clockticks6502++;
}
/* (ABS) */
void indirect6502()
{
      help = static_cast<WORD>(gameImage[PC] + (gameImage[PC + 1] << 8));
      savepc = static_cast<WORD>(gameImage[help] + (gameImage[help + 1] << 8));
      PC++;
      PC++;
}
/* ABS,X */
void absx6502()
{
      savepc = static_cast<WORD>(gameImage[PC] + (gameImage[PC + 1] << 8));
      PC++;
      PC++;
      if (ticks[opcode]==4)
              if ((savepc>>8) != ((savepc+X)>>8))
                      clockticks6502++;
      savepc += X;
}
/* ABS,Y */
void absy6502()
{
      savepc = static_cast<WORD>(gameImage[PC] + (gameImage[PC + 1] << 8));
      PC++;
      PC++;

      if (ticks[opcode]==4)
              if ((savepc>>8) != ((savepc+Y)>>8))
                      clockticks6502++;
      savepc += Y;
}
/* ZP */
void zp6502()
{
      savepc=gameImage[PC++];
}
/* ZP,X */
void zpx6502()
{
      savepc=gameImage[PC++]+X;
      savepc &= 0x00ff;
}
/* ZP,Y */
void zpy6502()
{
      savepc=gameImage[PC++]+Y;
      savepc &= 0x00ff;
}
/* (ZP,X) */
void indx6502()
{
      value = gameImage[PC++]+X;
      savepc = static_cast<WORD>(gameImage[value] + (gameImage[value+1] << 8));
}
/* (ZP),Y */
void indy6502()
{
      value = gameImage[PC++];
      savepc = static_cast<WORD>(gameImage[value] + (gameImage[value+1] << 8));
      if (ticks[opcode]==5)
              if ((savepc>>8) != ((savepc+Y)>>8))
                      clockticks6502++;
      savepc += Y;
}
/* (ABS,X) */
void indabsx6502()
{
      help = static_cast<WORD>(gameImage[PC] + (gameImage[PC + 1] << 8) + X);
      savepc = static_cast<WORD>(gameImage[help] + (gameImage[help + 1] << 8));
}
/* (ZP) */
void indzp6502()
{
      value = gameImage[PC++];
      savepc = static_cast<WORD>(gameImage[value] + (gameImage[value + 1] << 8));
}
/* ZP,Branch */
void zprel6502()
{
    savepc2=gameImage[PC++];
    savepc = gameImage[PC++];
    if (savepc & 0x80) savepc -= 0x100;
    if ((savepc>>8) != (PC>>8))
            clockticks6502++;
}

//BCD functions
WORD tobcd(BYTE x) {
    WORD ones = (x&0xF);
    WORD tens = (x>>4)&0xF;
    return (10*tens)+ones;
}
BYTE frombcd(WORD x) {
    WORD ones = x%10;
    WORD tens = x/10;
    return static_cast<BYTE>(ones|(tens<<4));
}

/* Instructions */
void adc6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      saveflags=(P & 0x01);
      WORD sum = static_cast<WORD>(A+value+saveflags);
      if (P & 0x08)
      {
          WORD realA = tobcd(A);
          WORD realVal = tobcd(value);
          WORD realRes = static_cast<WORD>(realA+realVal+saveflags);
          sum = frombcd(realRes%100);
          if(realRes>=100) P |= 0x1; else P &= 0xfe;
          clockticks6502++;
      }
      else
      {
          if(sum&0xFF00) P |= 0x1; else P &= 0xfe;
      }
      if((sum^A)&(sum^value)&0x0080) P |= 0x40; else P &= 0xbf;
      A=static_cast<BYTE>(sum);
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void and6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      A &= value;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void asl6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      P= (P & 0xfe) | ((value >>7) & 0x01);
      value <<= 1;
      put6502memory(savepc,value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void asla6502()
{
      P= (P & 0xfe) | ((A >>7) & 0x01);
      A <<= 1;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void bcc6502()
{
      if ((P & 0x01)==0)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value = gameImage[PC++];
}
void bcs6502()
{
      if (P & 0x01)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void beq6502()
{
      if (P & 0x02)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void bit6502()
{
      adrmode[opcode]();
      value=get6502memory(savepc);

      /* non-destrucive logically And between value and the accumulator
       * and set zero flag */
      if (value & A) P &= 0xfd; else P |= 0x02;

      /* set negative and overflow flags from value */
      P = (P & 0x3f) | (value & 0xc0);
}
void bmi6502()
{
      if (P & 0x80)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void bne6502()
{
      if ((P & 0x02)==0)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void bpl6502()
{
      if ((P & 0x80)==0)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void brk6502()
{
      PC++;
      put6502memory(0x0100+S--,(PC>>8));
      put6502memory(0x0100+S--,(PC & 0xff));
      put6502memory(0x0100+S--,P);
      P |= 0x14;
      PC = static_cast<WORD>(gameImage[0xfffe] + (gameImage[0xffff] << 8));
      /*if(gameImage[0x7428]==1 || gameImage[0x7428]==2) {
          dbgHaltInit = true;
          isExec = false;
      }*/
}
void bvc6502()
{
      if ((P & 0x40)==0)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void bvs6502()
{
      if (P & 0x40)
      {
              adrmode[opcode]();
              PC += savepc;
              clockticks6502++;
      }
      else
              value=gameImage[PC++];
}
void clc6502()
{
      P &= 0xfe;
}
void cld6502()
{
      P &= 0xf7;
}
void cli6502()
{
      P &= 0xfb;
}
void clv6502()
{
      P &= 0xbf;
}
void cmp6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      if (A+0x100-value>0xff) P |= 0x01; else P &= 0xfe;
      value=static_cast<BYTE>(A+0x100-value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void cpx6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      if (X+0x100-value>0xff) P |= 0x01; else P &= 0xfe;
      value=static_cast<BYTE>(X+0x100-value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void cpy6502()
{
      adrmode[opcode]();
      value = get6502memory(savepc);
      if (Y+0x100-value>0xff) P |= 0x01; else P &= 0xfe;
      value=static_cast<BYTE>(Y+0x100-value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void dec6502()
{
      adrmode[opcode]();
      put6502memory(savepc,get6502memory(savepc)-1);
      //gameImage[savepc]--;
      value = gameImage[savepc];
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void dex6502()
{
      X--;
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void dey6502()
{
      Y--;
      if (Y) P &= 0xfd; else P |= 0x02;
      if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}
void eor6502()
{
      adrmode[opcode]();
      A ^= get6502memory(savepc);
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void inc6502()
{
      adrmode[opcode]();
      put6502memory(savepc,get6502memory(savepc)+1);
      //gameImage[savepc]++;
      value = gameImage[savepc];
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void inx6502()
{
      X++;
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void iny6502()
{
      Y++;
      if (Y) P &= 0xfd; else P |= 0x02;
      if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}
void jmp6502()
{
      adrmode[opcode]();
      PC=savepc;
}
void jsr6502()
{
      PC++;
      put6502memory(0x0100+S--,(PC >> 8));
      put6502memory(0x0100+S--,(PC & 0xff));
      PC--;
      adrmode[opcode]();
      PC=savepc;
}
void lda6502()
{
      adrmode[opcode]();
      A=get6502memory(savepc);
      // set the zero flag
      if (A) P &= 0xfd; else P |= 0x02;
      // set the negative flag
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void ldx6502()
{
      adrmode[opcode]();
      X=get6502memory(savepc);
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void ldy6502()
{
      adrmode[opcode]();
      Y=get6502memory(savepc);
      if (Y) P &= 0xfd; else P |= 0x02;
      if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}
void lsr6502()
{
      adrmode[opcode]();
      value=get6502memory(savepc);

      /* set carry flag if shifting right causes a bit to be lost */
      P= (P & 0xfe) | (value & 0x01);

      value = value >>1;
      put6502memory(savepc,value);

      /* set zero flag if value is zero */
      if (value != 0) P &= 0xfd; else P |= 0x02;

      /* set negative flag if bit 8 set??? can this happen on an LSR? */
      if ((value & 0x80) == 0x80)
         P |= 0x80;
      else
         P &= 0x7f;
}
void lsra6502()
{
      P= (P & 0xfe) | (A & 0x01);
      A = A >>1;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void nop6502()
{
}
void ora6502()
{
      adrmode[opcode]();
      A |= get6502memory(savepc);
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void pha6502()
{
      gameImage[0x100+S--] = A;
}
void php6502()
{
      gameImage[0x100+S--] = P;
}
void pla6502()
{
      A=gameImage[++S+0x100];
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void plp6502()
{
      P=gameImage[++S+0x100] | 0x20;
}
void rol6502()
{
      saveflags=(P & 0x01);
      adrmode[opcode]();
      value = get6502memory(savepc);
      P= (P & 0xfe) | ((value >>7) & 0x01);
      value <<= 1;
      value |= saveflags;
      put6502memory(savepc,value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void rola6502()
{
      saveflags=(P & 0x01);
      P= (P & 0xfe) | ((A >>7) & 0x01);
      A <<= 1;
      A |= saveflags;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void ror6502()
{
      saveflags=(P & 0x01);
      adrmode[opcode]();
      value=get6502memory(savepc);
      P= (P & 0xfe) | (value & 0x01);
      value = value >>1;
      if (saveflags) value |= 0x80;
      put6502memory(savepc,value);
      if (value) P &= 0xfd; else P |= 0x02;
      if (value & 0x80) P |= 0x80; else P &= 0x7f;
}
void rora6502()
{
      saveflags=(P & 0x01);
      P= (P & 0xfe) | (A & 0x01);
      A = A >>1;
      if (saveflags) A |= 0x80;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void rti6502()
{
      P=gameImage[++S+0x100] | 0x20;
      PC=gameImage[++S+0x100];
      PC |= (gameImage[++S+0x100] << 8);
}
void rts6502()
{
      PC=gameImage[++S+0x100];
      PC |= (gameImage[++S+0x100] << 8);
      PC++;
}
void sbc6502()
{
    adrmode[opcode]();
    value = get6502memory(savepc);
    saveflags=(P & 0x01)^1;
    WORD sum = static_cast<WORD>(A-value-saveflags);
    if (P & 0x08)
    {
        WORD realA = tobcd(A);
        WORD realVal = 99-tobcd(value);
        WORD realRes = static_cast<WORD>(realA+realVal+(P&0x01));
        sum = frombcd(realRes%100);
        if(realRes>=100) P |= 0x1; else P &= 0xfe;
        clockticks6502++;
    }
    else
    {
        if(sum&0xFF00) P &= 0xfe; else P |= 0x1;
    }
    if((sum^A)&(sum^value)&0x0080) P |= 0x40; else P &= 0xbf;
    A=static_cast<BYTE>(sum);
    if (A) P &= 0xfd; else P |= 0x02;
    if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void sec6502()
{
      P |= 0x01;
}
void sed6502()
{
      P |= 0x08;
}
void sei6502()
{
      P |= 0x04;
}
void sta6502()
{
      adrmode[opcode]();
      put6502memory(savepc,A);
}
void stx6502()
{
      adrmode[opcode]();
      put6502memory(savepc,X);
}
void sty6502()
{
      adrmode[opcode]();
      put6502memory(savepc,Y);
}
void tax6502()
{
      X=A;
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void tay6502()
{
      Y=A;
      if (Y) P &= 0xfd; else P |= 0x02;
      if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}
void tsx6502()
{
      X=S;
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void txa6502()
{
      A=X;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void txs6502()
{
      S=X;
}
void tya6502()
{
      A=Y;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void bra6502()
{
      adrmode[opcode]();
      PC += savepc;
      clockticks6502++;
}
void deca6502()
{
      A--;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void inca6502()
{
      A++;
      if (A) P &= 0xfd; else P |= 0x02;
      if (A & 0x80) P |= 0x80; else P &= 0x7f;
}
void phx6502()
{
      put6502memory(0x100+S--,X);
}
void plx6502()
{
      X=gameImage[++S+0x100];
      if (X) P &= 0xfd; else P |= 0x02;
      if (X & 0x80) P |= 0x80; else P &= 0x7f;
}
void phy6502()
{
      put6502memory(0x100+S--,Y);
}
void ply6502()
{
      Y=gameImage[++S+0x100];
      if (Y) P &= 0xfd; else P |= 0x02;
      if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}
void stz6502()
{
      adrmode[opcode]();
      put6502memory(savepc,0);
}
void tsb6502()
{
      adrmode[opcode]();
      put6502memory(savepc,get6502memory(savepc)|A);
      //gameImage[savepc] |= A;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void trb6502()
{
      adrmode[opcode]();
      put6502memory(savepc,get6502memory(savepc)&(A^0xff));
      //gameImage[savepc] = gameImage[savepc] & (A ^ 0xff);
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb06502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xfe;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb16502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xfd;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb26502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xfb;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb36502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xf7;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb46502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xef;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb56502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xdf;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb66502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0xbf;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void rmb76502()
{
    adrmode[opcode]();
      gameImage[savepc] &= 0x7f;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb06502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x01;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb16502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x02;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb26502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x04;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb36502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x08;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb46502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x10;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb56502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x20;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb66502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x40;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void smb76502()
{
    adrmode[opcode]();
      gameImage[savepc] |= 0x80;
      if (gameImage[savepc]) P &= 0xfd; else P |= 0x02;
}
void bbr06502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x01)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr16502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x02)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr26502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x04)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr36502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x08)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr46502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x10)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr56502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x20)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr66502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x40)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbr76502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x80)==0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs06502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x01)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs16502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x02)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs26502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x04)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs36502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x08)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs46502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x10)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs56502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x20)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs66502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x40)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void bbs76502()
{
    adrmode[opcode]();
    if ((gameImage[savepc2] & 0x80)!=0) PC += savepc;
    else value = gameImage[PC++];
}
void stp6502()
{
    statStp = true;
}
void wai6502()
{
    statWai = true;
}

/* Init CPU */
void init6502()
{
      ticks[0x00]=7; instruction[0x00]=brk6502; adrmode[0x00]=implied6502;
      ticks[0x01]=6; instruction[0x01]=ora6502; adrmode[0x01]=indx6502;
      ticks[0x02]=2; instruction[0x02]=nop6502; adrmode[0x02]=immediate6502;
      ticks[0x03]=1; instruction[0x03]=nop6502; adrmode[0x03]=implied6502;
      ticks[0x04]=3; instruction[0x04]=tsb6502; adrmode[0x04]=zp6502;
      ticks[0x05]=3; instruction[0x05]=ora6502; adrmode[0x05]=zp6502;
      ticks[0x06]=5; instruction[0x06]=asl6502; adrmode[0x06]=zp6502;
      ticks[0x07]=5; instruction[0x07]=rmb06502; adrmode[0x07]=zp6502;
      ticks[0x08]=3; instruction[0x08]=php6502; adrmode[0x08]=implied6502;
      ticks[0x09]=3; instruction[0x09]=ora6502; adrmode[0x09]=immediate6502;
      ticks[0x0a]=2; instruction[0x0a]=asla6502; adrmode[0x0a]=implied6502;
      ticks[0x0b]=1; instruction[0x0b]=nop6502; adrmode[0x0b]=implied6502;
      ticks[0x0c]=4; instruction[0x0c]=tsb6502; adrmode[0x0c]=abs6502;
      ticks[0x0d]=4; instruction[0x0d]=ora6502; adrmode[0x0d]=abs6502;
      ticks[0x0e]=6; instruction[0x0e]=asl6502; adrmode[0x0e]=abs6502;
      ticks[0x0f]=5; instruction[0x0f]=bbr06502; adrmode[0x0f]=zprel6502;
      ticks[0x10]=2; instruction[0x10]=bpl6502; adrmode[0x10]=relative6502;
      ticks[0x11]=5; instruction[0x11]=ora6502; adrmode[0x11]=indy6502;
      ticks[0x12]=3; instruction[0x12]=ora6502; adrmode[0x12]=indzp6502;
      ticks[0x13]=1; instruction[0x13]=nop6502; adrmode[0x13]=implied6502;
      ticks[0x14]=3; instruction[0x14]=trb6502; adrmode[0x14]=zp6502;
      ticks[0x15]=4; instruction[0x15]=ora6502; adrmode[0x15]=zpx6502;
      ticks[0x16]=6; instruction[0x16]=asl6502; adrmode[0x16]=zpx6502;
      ticks[0x17]=5; instruction[0x17]=rmb16502; adrmode[0x17]=zp6502;
      ticks[0x18]=2; instruction[0x18]=clc6502; adrmode[0x18]=implied6502;
      ticks[0x19]=4; instruction[0x19]=ora6502; adrmode[0x19]=absy6502;
      ticks[0x1a]=2; instruction[0x1a]=inca6502; adrmode[0x1a]=implied6502;
      ticks[0x1b]=1; instruction[0x1b]=nop6502; adrmode[0x1b]=implied6502;
      ticks[0x1c]=4; instruction[0x1c]=trb6502; adrmode[0x1c]=abs6502;
      ticks[0x1d]=4; instruction[0x1d]=ora6502; adrmode[0x1d]=absx6502;
      ticks[0x1e]=7; instruction[0x1e]=asl6502; adrmode[0x1e]=absx6502;
      ticks[0x1f]=5; instruction[0x1f]=bbr16502; adrmode[0x1f]=zprel6502;
      ticks[0x20]=6; instruction[0x20]=jsr6502; adrmode[0x20]=abs6502;
      ticks[0x21]=6; instruction[0x21]=and6502; adrmode[0x21]=indx6502;
      ticks[0x22]=2; instruction[0x22]=nop6502; adrmode[0x22]=immediate6502;
      ticks[0x23]=1; instruction[0x23]=nop6502; adrmode[0x23]=implied6502;
      ticks[0x24]=3; instruction[0x24]=bit6502; adrmode[0x24]=zp6502;
      ticks[0x25]=3; instruction[0x25]=and6502; adrmode[0x25]=zp6502;
      ticks[0x26]=5; instruction[0x26]=rol6502; adrmode[0x26]=zp6502;
      ticks[0x27]=5; instruction[0x27]=rmb26502; adrmode[0x27]=zp6502;
      ticks[0x28]=4; instruction[0x28]=plp6502; adrmode[0x28]=implied6502;
      ticks[0x29]=3; instruction[0x29]=and6502; adrmode[0x29]=immediate6502;
      ticks[0x2a]=2; instruction[0x2a]=rola6502; adrmode[0x2a]=implied6502;
      ticks[0x2b]=1; instruction[0x2b]=nop6502; adrmode[0x2b]=implied6502;
      ticks[0x2c]=4; instruction[0x2c]=bit6502; adrmode[0x2c]=abs6502;
      ticks[0x2d]=4; instruction[0x2d]=and6502; adrmode[0x2d]=abs6502;
      ticks[0x2e]=6; instruction[0x2e]=rol6502; adrmode[0x2e]=abs6502;
      ticks[0x2f]=5; instruction[0x2f]=bbr26502; adrmode[0x2f]=zprel6502;
      ticks[0x30]=2; instruction[0x30]=bmi6502; adrmode[0x30]=relative6502;
      ticks[0x31]=5; instruction[0x31]=and6502; adrmode[0x31]=indy6502;
      ticks[0x32]=3; instruction[0x32]=and6502; adrmode[0x32]=indzp6502;
      ticks[0x33]=1; instruction[0x33]=nop6502; adrmode[0x33]=implied6502;
      ticks[0x34]=4; instruction[0x34]=bit6502; adrmode[0x34]=zpx6502;
      ticks[0x35]=4; instruction[0x35]=and6502; adrmode[0x35]=zpx6502;
      ticks[0x36]=6; instruction[0x36]=rol6502; adrmode[0x36]=zpx6502;
      ticks[0x37]=5; instruction[0x37]=rmb36502; adrmode[0x37]=zp6502;
      ticks[0x38]=2; instruction[0x38]=sec6502; adrmode[0x38]=implied6502;
      ticks[0x39]=4; instruction[0x39]=and6502; adrmode[0x39]=absy6502;
      ticks[0x3a]=2; instruction[0x3a]=deca6502; adrmode[0x3a]=implied6502;
      ticks[0x3b]=1; instruction[0x3b]=nop6502; adrmode[0x3b]=implied6502;
      ticks[0x3c]=4; instruction[0x3c]=bit6502; adrmode[0x3c]=absx6502;
      ticks[0x3d]=4; instruction[0x3d]=and6502; adrmode[0x3d]=absx6502;
      ticks[0x3e]=7; instruction[0x3e]=rol6502; adrmode[0x3e]=absx6502;
      ticks[0x3f]=5; instruction[0x3f]=bbr36502; adrmode[0x3f]=zprel6502;
      ticks[0x40]=6; instruction[0x40]=rti6502; adrmode[0x40]=implied6502;
      ticks[0x41]=6; instruction[0x41]=eor6502; adrmode[0x41]=indx6502;
      ticks[0x42]=2; instruction[0x42]=nop6502; adrmode[0x42]=immediate6502;
      ticks[0x43]=1; instruction[0x43]=nop6502; adrmode[0x43]=implied6502;
      ticks[0x44]=3; instruction[0x44]=nop6502; adrmode[0x44]=zp6502;
      ticks[0x45]=3; instruction[0x45]=eor6502; adrmode[0x45]=zp6502;
      ticks[0x46]=5; instruction[0x46]=lsr6502; adrmode[0x46]=zp6502;
      ticks[0x47]=5; instruction[0x47]=rmb46502; adrmode[0x47]=zp6502;
      ticks[0x48]=3; instruction[0x48]=pha6502; adrmode[0x48]=implied6502;
      ticks[0x49]=3; instruction[0x49]=eor6502; adrmode[0x49]=immediate6502;
      ticks[0x4a]=2; instruction[0x4a]=lsra6502; adrmode[0x4a]=implied6502;
      ticks[0x4b]=1; instruction[0x4b]=nop6502; adrmode[0x4b]=implied6502;
      ticks[0x4c]=3; instruction[0x4c]=jmp6502; adrmode[0x4c]=abs6502;
      ticks[0x4d]=4; instruction[0x4d]=eor6502; adrmode[0x4d]=abs6502;
      ticks[0x4e]=6; instruction[0x4e]=lsr6502; adrmode[0x4e]=abs6502;
      ticks[0x4f]=5; instruction[0x4f]=bbr46502; adrmode[0x4f]=zprel6502;
      ticks[0x50]=2; instruction[0x50]=bvc6502; adrmode[0x50]=relative6502;
      ticks[0x51]=5; instruction[0x51]=eor6502; adrmode[0x51]=indy6502;
      ticks[0x52]=3; instruction[0x52]=eor6502; adrmode[0x52]=indzp6502;
      ticks[0x53]=1; instruction[0x53]=nop6502; adrmode[0x53]=implied6502;
      ticks[0x54]=4; instruction[0x54]=nop6502; adrmode[0x54]=zpx6502;
      ticks[0x55]=4; instruction[0x55]=eor6502; adrmode[0x55]=zpx6502;
      ticks[0x56]=6; instruction[0x56]=lsr6502; adrmode[0x56]=zpx6502;
      ticks[0x57]=5; instruction[0x57]=rmb56502; adrmode[0x57]=zp6502;
      ticks[0x58]=2; instruction[0x58]=cli6502; adrmode[0x58]=implied6502;
      ticks[0x59]=4; instruction[0x59]=eor6502; adrmode[0x59]=absy6502;
      ticks[0x5a]=3; instruction[0x5a]=phy6502; adrmode[0x5a]=implied6502;
      ticks[0x5b]=1; instruction[0x5b]=nop6502; adrmode[0x5b]=implied6502;
      ticks[0x5c]=8; instruction[0x5c]=nop6502; adrmode[0x5c]=abs6502;
      ticks[0x5d]=4; instruction[0x5d]=eor6502; adrmode[0x5d]=absx6502;
      ticks[0x5e]=7; instruction[0x5e]=lsr6502; adrmode[0x5e]=absx6502;
      ticks[0x5f]=5; instruction[0x5f]=bbr56502; adrmode[0x5f]=zprel6502;
      ticks[0x60]=6; instruction[0x60]=rts6502; adrmode[0x60]=implied6502;
      ticks[0x61]=6; instruction[0x61]=adc6502; adrmode[0x61]=indx6502;
      ticks[0x62]=2; instruction[0x62]=nop6502; adrmode[0x62]=immediate6502;
      ticks[0x63]=1; instruction[0x63]=nop6502; adrmode[0x63]=implied6502;
      ticks[0x64]=3; instruction[0x64]=stz6502; adrmode[0x64]=zp6502;
      ticks[0x65]=3; instruction[0x65]=adc6502; adrmode[0x65]=zp6502;
      ticks[0x66]=5; instruction[0x66]=ror6502; adrmode[0x66]=zp6502;
      ticks[0x67]=5; instruction[0x67]=rmb66502; adrmode[0x67]=zp6502;
      ticks[0x68]=4; instruction[0x68]=pla6502; adrmode[0x68]=implied6502;
      ticks[0x69]=3; instruction[0x69]=adc6502; adrmode[0x69]=immediate6502;
      ticks[0x6a]=2; instruction[0x6a]=rora6502; adrmode[0x6a]=implied6502;
      ticks[0x6b]=1; instruction[0x6b]=nop6502; adrmode[0x6b]=implied6502;
      ticks[0x6c]=5; instruction[0x6c]=jmp6502; adrmode[0x6c]=indirect6502;
      ticks[0x6d]=4; instruction[0x6d]=adc6502; adrmode[0x6d]=abs6502;
      ticks[0x6e]=6; instruction[0x6e]=ror6502; adrmode[0x6e]=abs6502;
      ticks[0x6f]=5; instruction[0x6f]=bbr66502; adrmode[0x6f]=zprel6502;
      ticks[0x70]=2; instruction[0x70]=bvs6502; adrmode[0x70]=relative6502;
      ticks[0x71]=5; instruction[0x71]=adc6502; adrmode[0x71]=indy6502;
      ticks[0x72]=3; instruction[0x72]=adc6502; adrmode[0x72]=indzp6502;
      ticks[0x73]=1; instruction[0x73]=nop6502; adrmode[0x73]=implied6502;
      ticks[0x74]=4; instruction[0x74]=stz6502; adrmode[0x74]=zpx6502;
      ticks[0x75]=4; instruction[0x75]=adc6502; adrmode[0x75]=zpx6502;
      ticks[0x76]=6; instruction[0x76]=ror6502; adrmode[0x76]=zpx6502;
      ticks[0x77]=5; instruction[0x77]=rmb76502; adrmode[0x77]=zp6502;
      ticks[0x78]=2; instruction[0x78]=sei6502; adrmode[0x78]=implied6502;
      ticks[0x79]=4; instruction[0x79]=adc6502; adrmode[0x79]=absy6502;
      ticks[0x7a]=4; instruction[0x7a]=ply6502; adrmode[0x7a]=implied6502;
      ticks[0x7b]=1; instruction[0x7b]=nop6502; adrmode[0x7b]=implied6502;
      ticks[0x7c]=6; instruction[0x7c]=jmp6502; adrmode[0x7c]=indabsx6502;
      ticks[0x7d]=4; instruction[0x7d]=adc6502; adrmode[0x7d]=absx6502;
      ticks[0x7e]=7; instruction[0x7e]=ror6502; adrmode[0x7e]=absx6502;
      ticks[0x7f]=5; instruction[0x7f]=bbr76502; adrmode[0x7f]=zprel6502;
      ticks[0x80]=2; instruction[0x80]=bra6502; adrmode[0x80]=relative6502;
      ticks[0x81]=6; instruction[0x81]=sta6502; adrmode[0x81]=indx6502;
      ticks[0x82]=2; instruction[0x82]=nop6502; adrmode[0x82]=immediate6502;
      ticks[0x83]=1; instruction[0x83]=nop6502; adrmode[0x83]=implied6502;
      ticks[0x84]=2; instruction[0x84]=sty6502; adrmode[0x84]=zp6502;
      ticks[0x85]=2; instruction[0x85]=sta6502; adrmode[0x85]=zp6502;
      ticks[0x86]=2; instruction[0x86]=stx6502; adrmode[0x86]=zp6502;
      ticks[0x87]=5; instruction[0x87]=smb06502; adrmode[0x87]=zp6502;
      ticks[0x88]=2; instruction[0x88]=dey6502; adrmode[0x88]=implied6502;
      ticks[0x89]=2; instruction[0x89]=bit6502; adrmode[0x89]=immediate6502;
      ticks[0x8a]=2; instruction[0x8a]=txa6502; adrmode[0x8a]=implied6502;
      ticks[0x8b]=1; instruction[0x8b]=nop6502; adrmode[0x8b]=implied6502;
      ticks[0x8c]=4; instruction[0x8c]=sty6502; adrmode[0x8c]=abs6502;
      ticks[0x8d]=4; instruction[0x8d]=sta6502; adrmode[0x8d]=abs6502;
      ticks[0x8e]=4; instruction[0x8e]=stx6502; adrmode[0x8e]=abs6502;
      ticks[0x8f]=5; instruction[0x8f]=bbs06502; adrmode[0x8f]=zprel6502;
      ticks[0x90]=2; instruction[0x90]=bcc6502; adrmode[0x90]=relative6502;
      ticks[0x91]=6; instruction[0x91]=sta6502; adrmode[0x91]=indy6502;
      ticks[0x92]=3; instruction[0x92]=sta6502; adrmode[0x92]=indzp6502;
      ticks[0x93]=1; instruction[0x93]=nop6502; adrmode[0x93]=implied6502;
      ticks[0x94]=4; instruction[0x94]=sty6502; adrmode[0x94]=zpx6502;
      ticks[0x95]=4; instruction[0x95]=sta6502; adrmode[0x95]=zpx6502;
      ticks[0x96]=4; instruction[0x96]=stx6502; adrmode[0x96]=zpy6502;
      ticks[0x97]=5; instruction[0x97]=smb16502; adrmode[0x97]=zp6502;
      ticks[0x98]=2; instruction[0x98]=tya6502; adrmode[0x98]=implied6502;
      ticks[0x99]=5; instruction[0x99]=sta6502; adrmode[0x99]=absy6502;
      ticks[0x9a]=2; instruction[0x9a]=txs6502; adrmode[0x9a]=implied6502;
      ticks[0x9b]=1; instruction[0x9b]=nop6502; adrmode[0x9b]=implied6502;
      ticks[0x9c]=4; instruction[0x9c]=stz6502; adrmode[0x9c]=abs6502;
      ticks[0x9d]=5; instruction[0x9d]=sta6502; adrmode[0x9d]=absx6502;
      ticks[0x9e]=5; instruction[0x9e]=stz6502; adrmode[0x9e]=absx6502;
      ticks[0x9f]=5; instruction[0x9f]=bbs16502; adrmode[0x9f]=zprel6502;
      ticks[0xa0]=3; instruction[0xa0]=ldy6502; adrmode[0xa0]=immediate6502;
      ticks[0xa1]=6; instruction[0xa1]=lda6502; adrmode[0xa1]=indx6502;
      ticks[0xa2]=3; instruction[0xa2]=ldx6502; adrmode[0xa2]=immediate6502;
      ticks[0xa3]=1; instruction[0xa3]=nop6502; adrmode[0xa3]=implied6502;
      ticks[0xa4]=3; instruction[0xa4]=ldy6502; adrmode[0xa4]=zp6502;
      ticks[0xa5]=3; instruction[0xa5]=lda6502; adrmode[0xa5]=zp6502;
      ticks[0xa6]=3; instruction[0xa6]=ldx6502; adrmode[0xa6]=zp6502;
      ticks[0xa7]=5; instruction[0xa7]=smb26502; adrmode[0xa7]=zp6502;
      ticks[0xa8]=2; instruction[0xa8]=tay6502; adrmode[0xa8]=implied6502;
      ticks[0xa9]=3; instruction[0xa9]=lda6502; adrmode[0xa9]=immediate6502;
      ticks[0xaa]=2; instruction[0xaa]=tax6502; adrmode[0xaa]=implied6502;
      ticks[0xab]=1; instruction[0xab]=nop6502; adrmode[0xab]=implied6502;
      ticks[0xac]=4; instruction[0xac]=ldy6502; adrmode[0xac]=abs6502;
      ticks[0xad]=4; instruction[0xad]=lda6502; adrmode[0xad]=abs6502;
      ticks[0xae]=4; instruction[0xae]=ldx6502; adrmode[0xae]=abs6502;
      ticks[0xaf]=5; instruction[0xaf]=bbs26502; adrmode[0xaf]=zprel6502;
      ticks[0xb0]=2; instruction[0xb0]=bcs6502; adrmode[0xb0]=relative6502;
      ticks[0xb1]=5; instruction[0xb1]=lda6502; adrmode[0xb1]=indy6502;
      ticks[0xb2]=3; instruction[0xb2]=lda6502; adrmode[0xb2]=indzp6502;
      ticks[0xb3]=1; instruction[0xb3]=nop6502; adrmode[0xb3]=implied6502;
      ticks[0xb4]=4; instruction[0xb4]=ldy6502; adrmode[0xb4]=zpx6502;
      ticks[0xb5]=4; instruction[0xb5]=lda6502; adrmode[0xb5]=zpx6502;
      ticks[0xb6]=4; instruction[0xb6]=ldx6502; adrmode[0xb6]=zpy6502;
      ticks[0xb7]=5; instruction[0xb7]=smb36502; adrmode[0xb7]=zp6502;
      ticks[0xb8]=2; instruction[0xb8]=clv6502; adrmode[0xb8]=implied6502;
      ticks[0xb9]=4; instruction[0xb9]=lda6502; adrmode[0xb9]=absy6502;
      ticks[0xba]=2; instruction[0xba]=tsx6502; adrmode[0xba]=implied6502;
      ticks[0xbb]=1; instruction[0xbb]=nop6502; adrmode[0xbb]=implied6502;
      ticks[0xbc]=4; instruction[0xbc]=ldy6502; adrmode[0xbc]=absx6502;
      ticks[0xbd]=4; instruction[0xbd]=lda6502; adrmode[0xbd]=absx6502;
      ticks[0xbe]=4; instruction[0xbe]=ldx6502; adrmode[0xbe]=absy6502;
      ticks[0xbf]=5; instruction[0xbf]=bbs36502; adrmode[0xbf]=zprel6502;
      ticks[0xc0]=3; instruction[0xc0]=cpy6502; adrmode[0xc0]=immediate6502;
      ticks[0xc1]=6; instruction[0xc1]=cmp6502; adrmode[0xc1]=indx6502;
      ticks[0xc2]=2; instruction[0xc2]=nop6502; adrmode[0xc2]=immediate6502;
      ticks[0xc3]=1; instruction[0xc3]=nop6502; adrmode[0xc3]=implied6502;
      ticks[0xc4]=3; instruction[0xc4]=cpy6502; adrmode[0xc4]=zp6502;
      ticks[0xc5]=3; instruction[0xc5]=cmp6502; adrmode[0xc5]=zp6502;
      ticks[0xc6]=5; instruction[0xc6]=dec6502; adrmode[0xc6]=zp6502;
      ticks[0xc7]=5; instruction[0xc7]=smb46502; adrmode[0xc7]=zp6502;
      ticks[0xc8]=2; instruction[0xc8]=iny6502; adrmode[0xc8]=implied6502;
      ticks[0xc9]=3; instruction[0xc9]=cmp6502; adrmode[0xc9]=immediate6502;
      ticks[0xca]=2; instruction[0xca]=dex6502; adrmode[0xca]=implied6502;
      ticks[0xcb]=3; instruction[0xcb]=wai6502; adrmode[0xcb]=implied6502;
      ticks[0xcc]=4; instruction[0xcc]=cpy6502; adrmode[0xcc]=abs6502;
      ticks[0xcd]=4; instruction[0xcd]=cmp6502; adrmode[0xcd]=abs6502;
      ticks[0xce]=6; instruction[0xce]=dec6502; adrmode[0xce]=abs6502;
      ticks[0xcf]=5; instruction[0xcf]=bbs46502; adrmode[0xcf]=zprel6502;
      ticks[0xd0]=2; instruction[0xd0]=bne6502; adrmode[0xd0]=relative6502;
      ticks[0xd1]=5; instruction[0xd1]=cmp6502; adrmode[0xd1]=indy6502;
      ticks[0xd2]=3; instruction[0xd2]=cmp6502; adrmode[0xd2]=indzp6502;
      ticks[0xd3]=1; instruction[0xd3]=nop6502; adrmode[0xd3]=implied6502;
      ticks[0xd4]=4; instruction[0xd4]=nop6502; adrmode[0xd4]=zpx6502;
      ticks[0xd5]=4; instruction[0xd5]=cmp6502; adrmode[0xd5]=zpx6502;
      ticks[0xd6]=6; instruction[0xd6]=dec6502; adrmode[0xd6]=zpx6502;
      ticks[0xd7]=5; instruction[0xd7]=smb56502; adrmode[0xd7]=zp6502;
      ticks[0xd8]=2; instruction[0xd8]=cld6502; adrmode[0xd8]=implied6502;
      ticks[0xd9]=4; instruction[0xd9]=cmp6502; adrmode[0xd9]=absy6502;
      ticks[0xda]=3; instruction[0xda]=phx6502; adrmode[0xda]=implied6502;
      ticks[0xdb]=3; instruction[0xdb]=stp6502; adrmode[0xdb]=implied6502;
      ticks[0xdc]=4; instruction[0xdc]=nop6502; adrmode[0xdc]=abs6502;
      ticks[0xdd]=4; instruction[0xdd]=cmp6502; adrmode[0xdd]=absx6502;
      ticks[0xde]=7; instruction[0xde]=dec6502; adrmode[0xde]=absx6502;
      ticks[0xdf]=5; instruction[0xdf]=bbs56502; adrmode[0xdf]=zprel6502;
      ticks[0xe0]=3; instruction[0xe0]=cpx6502; adrmode[0xe0]=immediate6502;
      ticks[0xe1]=6; instruction[0xe1]=sbc6502; adrmode[0xe1]=indx6502;
      ticks[0xe2]=2; instruction[0xe2]=nop6502; adrmode[0xe2]=immediate6502;
      ticks[0xe3]=1; instruction[0xe3]=nop6502; adrmode[0xe3]=implied6502;
      ticks[0xe4]=3; instruction[0xe4]=cpx6502; adrmode[0xe4]=zp6502;
      ticks[0xe5]=3; instruction[0xe5]=sbc6502; adrmode[0xe5]=zp6502;
      ticks[0xe6]=5; instruction[0xe6]=inc6502; adrmode[0xe6]=zp6502;
      ticks[0xe7]=5; instruction[0xe7]=smb66502; adrmode[0xe7]=zp6502;
      ticks[0xe8]=2; instruction[0xe8]=inx6502; adrmode[0xe8]=implied6502;
      ticks[0xe9]=3; instruction[0xe9]=sbc6502; adrmode[0xe9]=immediate6502;
      ticks[0xea]=2; instruction[0xea]=nop6502; adrmode[0xea]=implied6502;
      ticks[0xeb]=1; instruction[0xeb]=nop6502; adrmode[0xeb]=implied6502;
      ticks[0xec]=4; instruction[0xec]=cpx6502; adrmode[0xec]=abs6502;
      ticks[0xed]=4; instruction[0xed]=sbc6502; adrmode[0xed]=abs6502;
      ticks[0xee]=6; instruction[0xee]=inc6502; adrmode[0xee]=abs6502;
      ticks[0xef]=5; instruction[0xef]=bbs66502; adrmode[0xef]=zprel6502;
      ticks[0xf0]=2; instruction[0xf0]=beq6502; adrmode[0xf0]=relative6502;
      ticks[0xf1]=5; instruction[0xf1]=sbc6502; adrmode[0xf1]=indy6502;
      ticks[0xf2]=3; instruction[0xf2]=sbc6502; adrmode[0xf2]=indzp6502;
      ticks[0xf3]=1; instruction[0xf3]=nop6502; adrmode[0xf3]=implied6502;
      ticks[0xf4]=4; instruction[0xf4]=nop6502; adrmode[0xf4]=zpx6502;
      ticks[0xf5]=4; instruction[0xf5]=sbc6502; adrmode[0xf5]=zpx6502;
      ticks[0xf6]=6; instruction[0xf6]=inc6502; adrmode[0xf6]=zpx6502;
      ticks[0xf7]=5; instruction[0xf7]=smb76502; adrmode[0xf7]=zp6502;
      ticks[0xf8]=2; instruction[0xf8]=sed6502; adrmode[0xf8]=implied6502;
      ticks[0xf9]=4; instruction[0xf9]=sbc6502; adrmode[0xf9]=absy6502;
      ticks[0xfa]=4; instruction[0xfa]=plx6502; adrmode[0xfa]=implied6502;
      ticks[0xfb]=1; instruction[0xfb]=nop6502; adrmode[0xfb]=implied6502;
      ticks[0xfc]=4; instruction[0xfc]=nop6502; adrmode[0xfc]=abs6502;
      ticks[0xfd]=4; instruction[0xfd]=sbc6502; adrmode[0xfd]=absx6502;
      ticks[0xfe]=7; instruction[0xfe]=inc6502; adrmode[0xfe]=absx6502;
      ticks[0xff]=5; instruction[0xff]=bbs76502; adrmode[0xff]=zprel6502;
}

/* Reset CPU */
void reset6502()
{
       A=X=Y=P=0;
       P |= 0x20;
       S=0xff;
       PC=get6502memory(0xfffc);
       PC |= get6502memory(0xfffd) << 8;

       statStp = statWai = false;
}

/* Non maskerable interrupt */
void nmi6502()
{
      put6502memory(0x0100+S--,(PC>>8));
      put6502memory(0x0100+S--,(PC & 0xff));
      put6502memory(0x0100+S--,P);
      P |= 0x04;
      PC=get6502memory(0xfffa);
      PC |= get6502memory(0xfffb) << 8;

      statWai = false;
}

/* Maskerable Interrupt */
void irq6502()
{
   put6502memory(0x0100+S--,(PC>>8));
   put6502memory(0x0100+S--,(PC & 0xff));
   put6502memory(0x0100+S--,P);
   P |= 0x04;
   PC=get6502memory(0xfffe);
   PC |= get6502memory(0xffff) << 8;

   statWai = false;
}

/* Execute Instruction */
void exec6502()
{
   while (timerTicks > 0) 
   {
       if(!isExec || statStp || statWai) break;
       for(int i=0; i<numBreakpoints; i++) {
           if ((dBreakpointAddr[i]==PC)&&dBreakpointE[i]&&dBreakpointX[i]) {isExec=false; break;}
       }
      // fetch instruction
      opcode = gameImage[PC++];

      // execute instruction
      instruction[opcode]();

      // calculate clock cycles
      clockticks6502 += ticks[opcode];
      timerTicks -= clockticks6502;
      clockticks6502 = 0;
   }
   if(frameAdvance>0) {frameAdvance--;}
   if(frameAdvance==0) {isExec=false;}
}



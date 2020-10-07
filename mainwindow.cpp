#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QFile>
#include <QIODevice>
#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QtGui>
using namespace std;

bool isRunning = false;
static string outFileName, prefix;
static unsigned char pixels[256*192*4];
static int timer2 = 0;
static int channelTimer[4] = {0,0,0,0};
static int channelSample[4] = {0,0,0,0};
static int channelNoiseVal[4] = {1,1,1,1};
static float waveformLut[16*8] = {1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,
                                  1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                  1,.75f,.5f,.25f,0,-.25f,-.5f,-.75f,-1,-.75f,-.5f,-.25f,0,.25f,.5f,.75f,
                                  1,.75f,.5f,.25f,0,-.25f,-.5f,-.75f,-1,-.75f,-.5f,-.25f,0,.25f,.5f,.75f,
                                  1,0.866667f,0.733333f,0.6f,0.466667f,0.533333f,0.2f,0.066667f,-0.066667f,-0.2f,-0.533333f,-0.466667f,-0.6f,-0.733333f,-0.866667f,-1,
                                  1,0.866667f,0.733333f,0.6f,0.466667f,0.533333f,0.2f,0.066667f,-0.066667f,-0.2f,-0.533333f,-0.466667f,-0.6f,-0.733333f,-0.866667f,-1,
                                  1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,
                                  1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};
static QTimer * loopTimer;
static QLabel * labelMap;
static RtAudio dac;
static RtAudio::StreamParameters oParams;
static RtAudio::StreamOptions options;
QSettings * ctrlSettings;
QMap<QString, QVariant> ctrlMap;
int curPortToCfg;
static char saveStateSlot = '0';

void clearScreen()
{
    for(int i=0; i<(256*192*4); i+=4)
    {
        pixels[i]=0;
        pixels[i+1]=0;
        pixels[i+2]=0;
    }
    QImage imageMap(pixels,256,192,QImage::Format_RGB32);
    QPixmap pixelMap = QPixmap::fromImage(imageMap);
    pixelMap = pixelMap.scaled(labelMap->width(),labelMap->height());
    labelMap->setPixmap(pixelMap);
}
void clearInputRam() {
    for(int i=0x7480; i<0x7500; i++)
    {
        gameImage[i] = 0;
    }
    gameImage[0x74F8] = static_cast<BYTE>(ctrlMap["Port1CtrlType"].toInt());
    gameImage[0x74F9] = static_cast<BYTE>(ctrlMap["Port2CtrlType"].toInt());
    gameImage[0x74FA] = static_cast<BYTE>(ctrlMap["Port3CtrlType"].toInt());
    gameImage[0x74FB] = static_cast<BYTE>(ctrlMap["Port4CtrlType"].toInt());
    for(int i=0; i<4; i++) {
        if(gameImage[0x74F8|i]!=0) {
            if(gameImage[0x74F8|i]==2) {
                gameImage[0x74F8|i]++;
            }
            gameImage[0x74FC|i]++;
        }
    }
}

void MainWindow::closePreviousGame()
{
    //clear ram (for debugging convenience)
    for(int i=0; i<0x10000; i++)
    {
        gameImage[i] = 0;
    }
    //close previous game
    if(isRunning)
    {
        loopTimer->stop();
        clearScreen();
        isRunning = false;
        //open sram
        int ored = gameHeader[0x18]|gameHeader[0x19]|gameHeader[0x1A]|gameHeader[0x1B]|gameHeader[0x1C]|gameHeader[0x1D]|gameHeader[0x1E];
        ored &= 0x80;
        QFile outFile;
        if(ored) {
            outFile.setFileName(outFileName.c_str());
            outFile.open(QIODevice::WriteOnly);
            //get rom
            if(gameHeader[0x18]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameRom),0x8000*gameHeader[0x10]);
            }
            //get chr
            if(gameHeader[0x19]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameChr),0x2000*gameHeader[0x11]);
            }
            //get bgt
            if(gameHeader[0x1A]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameBgt),0x1000*gameHeader[0x12]);
            }
            //get bgp
            if(gameHeader[0x1B]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameBgp),0x400*gameHeader[0x13]);
            }
            //get wram
            if(gameHeader[0x1C]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameWram),0x2000*gameHeader[0x14]);
            }
            //get sram
            if(gameHeader[0x1D]&0x80) {
                outFile.write(reinterpret_cast<char*>(gameSram),0x800*gameHeader[0x15]);
            }
            //get pal
            if(gameHeader[0x1E]&0x80) {
                outFile.write(reinterpret_cast<char*>(gamePal),0x20*gameHeader[0x16]);
            }
            //close sram
            outFile.close();
        }
        statusBar()->showMessage("Closed game.",3000);
    }
}
void MainWindow::openNewGame(QString path)
{
    //clear ram (for debugging convenience)
    for(int i=0; i<0x10000; i++)
    {
        gameImage[i] = 0;
    }
    clearInputRam();
    //parse rom
    QFile romFile(path);
    romFile.open(QIODevice::ReadOnly);
    romFile.read(reinterpret_cast<char*>(gameHeader),8192);
    //open sram
    int ored = gameHeader[0x18]|gameHeader[0x19]|gameHeader[0x1A]|gameHeader[0x1B]|gameHeader[0x1C]|gameHeader[0x1D]|gameHeader[0x1E];
    ored &= 0x80;
    QFile outFile;
    if(ored) {
        outFileName.assign(prefix);
        outFileName += "sram/";
        for(int i=4; i<8; i++) {
            outFileName += static_cast<char>(gameHeader[i]);
        }
        outFileName += ".bin";
        outFile.setFileName(outFileName.c_str());
        if(QFile::exists(outFileName.c_str()))
        {
            outFile.open(QIODevice::ReadOnly);
        }
    }
    //get rom
    int readSize = 0x8000*bankSizeLut[gameHeader[0x10]];
    if(gameHeader[0x18]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameRom),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameRom),readSize);
    }
    if(gameHeader[0x10]) {
        int offset = (bankSizeLut[gameHeader[0x10]]-1)*0x8000;
        for(int i=0; i<0x8000; i++) {
            gameImage[0x8000+i] = gameRom[offset+i];
        }
    }
    //get chr
    readSize = 0x2000*bankSizeLut[gameHeader[0x11]];
    if(gameHeader[0x19]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameChr),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameChr),readSize);
    }
    if(gameHeader[0x11]) {
        int offset = (bankSizeLut[gameHeader[0x11]]-1)*0x2000;
        for(int i=0; i<0x2000; i++) {
            gameImage[0x4000+i] = gameChr[offset+i];
        }
    }
    //get bgt
    readSize = 0x1000*bankSizeLut[gameHeader[0x12]];
    if(gameHeader[0x1A]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameBgt),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameBgt),readSize);
    }
    if(gameHeader[0x12]) {
        int offset = (bankSizeLut[gameHeader[0x12]]-1)*0x1000;
        for(int i=0; i<0x1000; i++) {
            gameImage[0x6000+i] = gameBgt[offset+i];
        }
    }
    //get bgp
    readSize = 0x400*bankSizeLut[gameHeader[0x13]];
    if(gameHeader[0x1B]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameBgp),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameBgp),readSize);
    }
    if(gameHeader[0x13]) {
        int offset = (bankSizeLut[gameHeader[0x13]]-1)*0x400;
        for(int i=0; i<0x400; i++) {
            gameImage[0x6000+i] = gameBgp[offset+i];
        }
    }
    //get wram
    readSize = 0x2000*bankSizeLut[gameHeader[0x14]];
    if(gameHeader[0x1C]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameWram),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameWram),readSize);
    }
    if((gameHeader[0x14]&0x7F)==0) {
        int offset = (bankSizeLut[gameHeader[0x14]]-1)*0x2000;
        for(int i=0; i<0x2000; i++) {
            gameImage[0x2000+i] = gameWram[offset+i];
        }
    }
    //get sram
    readSize = 0x800*bankSizeLut[gameHeader[0x15]];
    if(gameHeader[0x1D]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameSram),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameSram),readSize);
    }
    if(gameHeader[0x15]) {
        int offset = (bankSizeLut[gameHeader[0x15]]-1)*0x800;
        for(int i=0; i<0x800; i++) {
            gameImage[0x7800+i] = gameSram[offset+i];
        }
    }
    //get pal
    readSize = 0x20*bankSizeLut[gameHeader[0x16]];
    if(gameHeader[0x1E]&0x80) {
        outFile.read(reinterpret_cast<char*>(gameSram),readSize);
    } else {
        romFile.read(reinterpret_cast<char*>(gameSram),readSize);
    }
    if(gameHeader[0x16]) {
        int offset = (bankSizeLut[gameHeader[0x16]]-1)*0x20;
        for(int i=0; i<0x20; i++) {
            gameImage[0x7500+i] = gameSram[offset+i];
        }
    }
    if(ored) {
        //close sram
        outFile.close();
    }
    romFile.close();
    //init 6502
    reset6502();
    //setup audio and graphics callback
    timer2 = 0;
    loopTimer = new QTimer;
    connect(loopTimer,SIGNAL(timeout()),this,SLOT(emuLoop()));
    //run game loop
    isRunning = true;
    loopTimer->start(16);
    string msg = "Opened game \"";
    int i = 64;
    while(true) {
        char x = static_cast<char>(gameHeader[i++]);
        if(x==0) {break;}
        if(i==0xFC0) {break;}
        msg += x;
    }
    msg += "\".";
    statusBar()->showMessage(msg.c_str(),3000);
}

void putPixel(int color,int xd,int yd)
{
    int pix = ((xd+(yd<<8))<<2);
    pixels[pix+2] = 85*((color&0x30)>>4);
    pixels[pix+1] = 85*((color&0x0C)>>2);
    pixels[pix] = 85*(color&0x03);
}
void putPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd)
{
    int addr = 0x4000+(tile<<4)+ys;
    int entry0 = (gameImage[addr]>>(7-xs))&1;
    int entry1 = (gameImage[addr+8]>>(7-xs))&1;
    int entry = entry0+(entry1<<1);
    if(entry!=0 && xd<256 && yd<192 && xd>=0 && yd>=0)
    {
        int color = gameImage[0x7500+(pal<<2)+entry];
        putPixel(color,xd,yd);
    }
}

void MainWindow::emuLoop()
{
    if(isExec) {
        //execute code
        timerTicks = 65536;
        exec6502();
        /*if(dbgHalt) {
            closePreviousGame();
            dbgHalt = false;
        }*/
        if(gameImage[0x7420] && isExec) {nmi6502();}
        /*if(isTimerRunning) {
            timerFrameVal++;
        }*/
        //fill bg
        int color = gameImage[0x7500];
        for(int y=0; y<192; y++)
        {
            for(int x=0; x<256; x++)
            {
                putPixel(color,x,y);
            }
        }
        //run sprites 1
        int splx = gameImage[0x7400];
        int sply = (gameImage[0x7401]<192)?gameImage[0x7401]:192;
        for(int addr=0x7600; addr<0x7800; addr+=4)
        {
            int idx = (addr>>2)&0x7F;
            if(gameImage[addr+3]&8)
            {
                int yhm = ((gameImage[addr+3]&0x20)>>5)+1;
                int xhm = ((gameImage[addr+3]&0x10)>>4)+1;
                for(int yh=0; yh<yhm; yh++)
                {
                    for(int xh=0; xh<xhm; xh++)
                    {
                        for(int y=0; y<8; y++)
                        {
                            for(int x=0; x<8; x++)
                            {
                                int xd = ((gameImage[addr+3]&0x40)==0)?(gameImage[addr]+x+(xh<<3)):(xhm<<3)-(x+(xh<<3))+gameImage[addr]-1;
                                int yd = ((gameImage[addr+3]&0x80)==0)?(gameImage[addr+1]+y+(yh<<3)):(yhm<<3)-(y+(yh<<3))+gameImage[addr+1]-1;
                                if((xd<splx && yd<sply && gameImage[0x7424]&0x80 && gameImage[0x7580|idx]&0x08)||(xd>=splx && yd<sply && gameImage[0x7424]&0x40 && gameImage[0x7580|idx]&0x04)||
                            (xd<splx && yd>=sply && gameImage[0x7424]&0x20 && gameImage[0x7580|idx]&0x02)||(xd>=splx && yd>=sply && gameImage[0x7424]&0x10 && gameImage[0x7580|idx]&0x01))
                                {
                                    putPixelFromTile((256+gameImage[addr+2])^xh^(yh<<4),4+(gameImage[addr+3]&3),x,y,xd,yd);
                                }
                            }
                        }
                    }
                }
            }
        }
        //run bg
        if(gameImage[0x7424]&0x08)
        {
            int scrlx = gameImage[0x7408]+((gameImage[0x7404]&0x80)<<1);
            int scrly = gameImage[0x7409]+((gameImage[0x7404]&0x40)<<2);
            for(int y=0; y<sply; y++)
            {
                for(int x=0; x<splx; x++)
                {
                    int px = (scrlx+x)&511;
                    int py = (scrly+y)&511;
                    int etile = ((px>>4)&15)+(((py>>4)&15)<<4)+(px&0x100)+((py&0x100)<<1);
                    int estile = ((px>>3)&1)+(((py>>3)&1)<<1);
                    int mask = 0xC0>>(estile<<1);
                    int shift = 6-(estile<<1);
                    int sx = px&7;
                    int sy = py&7;
                    px = ((px&0xFF)>>3)+((px&0x100)<<2);
                    py = ((py&0xF8)<<2)+((py&0x100)<<3);
                    putPixelFromTile(gameImage[0x6000+px+py],
                       (gameImage[0x7000+etile]&mask)>>shift,
                       sx,sy,x,y);
                }
            }
        }
        if(gameImage[0x7424]&0x04)
        {
            int scrlx = gameImage[0x740A]+((gameImage[0x7404]&0x20)<<3);
            int scrly = gameImage[0x740B]+((gameImage[0x7404]&0x10)<<4);
            for(int y=0; y<sply; y++)
            {
                for(int x=splx; x<256; x++)
                {
                    int px = (scrlx+x)&511;
                    int py = (scrly+y)&511;
                    int etile = ((px>>4)&15)+(((py>>4)&15)<<4)+(px&0x100)+((py&0x100)<<1);
                    int estile = ((px>>3)&1)+(((py>>3)&1)<<1);
                    int mask = 0xC0>>(estile<<1);
                    int shift = 6-(estile<<1);
                    int sx = px&7;
                    int sy = py&7;
                    px = ((px&0xFF)>>3)+((px&0x100)<<2);
                    py = ((py&0xF8)<<2)+((py&0x100)<<3);
                    putPixelFromTile(gameImage[0x6000+px+py],
                       (gameImage[0x7000+etile]&mask)>>shift,
                       sx,sy,x,y);
                }
            }
        }
        if(gameImage[0x7424]&0x02)
        {
            int scrlx = gameImage[0x740C]+((gameImage[0x7404]&0x08)<<5);
            int scrly = gameImage[0x740D]+((gameImage[0x7404]&0x04)<<6);
            for(int y=sply; y<192; y++)
            {
                for(int x=0; x<splx; x++)
                {
                    int px = (scrlx+x)&511;
                    int py = (scrly+y)&511;
                    int etile = ((px>>4)&15)+(((py>>4)&15)<<4)+(px&0x100)+((py&0x100)<<1);
                    int estile = ((px>>3)&1)+(((py>>3)&1)<<1);
                    int mask = 0xC0>>(estile<<1);
                    int shift = 6-(estile<<1);
                    int sx = px&7;
                    int sy = py&7;
                    px = ((px&0xFF)>>3)+((px&0x100)<<2);
                    py = ((py&0xF8)<<2)+((py&0x100)<<3);
                    putPixelFromTile(gameImage[0x6000+px+py],
                       (gameImage[0x7000+etile]&mask)>>shift,
                       sx,sy,x,y);
                }
            }
        }
        if(gameImage[0x7424]&0x01)
        {
            int scrlx = gameImage[0x740E]+((gameImage[0x7404]&0x02)<<7);
            int scrly = gameImage[0x740F]+((gameImage[0x7404]&0x01)<<8);
            for(int y=sply; y<192; y++)
            {
                for(int x=splx; x<256; x++)
                {
                    int px = (scrlx+x)&511;
                    int py = (scrly+y)&511;
                    int etile = ((px>>4)&15)+(((py>>4)&15)<<4)+(px&0x100)+((py&0x100)<<1);
                    int estile = ((px>>3)&1)+(((py>>3)&1)<<1);
                    int mask = 0xC0>>(estile<<1);
                    int shift = 6-(estile<<1);
                    int sx = px&7;
                    int sy = py&7;
                    px = ((px&0xFF)>>3)+((px&0x100)<<2);
                    py = ((py&0xF8)<<2)+((py&0x100)<<3);
                    putPixelFromTile(gameImage[0x6000+px+py],
                        (gameImage[0x7000+etile]&mask)>>shift,
                        sx,sy,x,y);
                }
            }
        }
        //run sprites 0
        for(int addr=0x7600; addr<0x7800; addr+=4)
        {
            int idx = (addr>>2)&0x7F;
            if((gameImage[addr+3]&8)==0)
            {
                int yhm = ((gameImage[addr+3]&0x20)>>5)+1;
                int xhm = ((gameImage[addr+3]&0x10)>>4)+1;
                for(int yh=0; yh<yhm; yh++)
                {
                    for(int xh=0; xh<xhm; xh++)
                    {
                        for(int y=0; y<8; y++)
                        {
                            for(int x=0; x<8; x++)
                            {
                                int xd = ((gameImage[addr+3]&0x40)==0)?(gameImage[addr]+x+(xh<<3)):(xhm<<3)-(x+(xh<<3))+gameImage[addr]-1;
                                int yd = ((gameImage[addr+3]&0x80)==0)?(gameImage[addr+1]+y+(yh<<3)):(yhm<<3)-(y+(yh<<3))+gameImage[addr+1]-1;
                                if((xd<splx && yd<sply && gameImage[0x7424]&0x80 && gameImage[0x7580|idx]&0x08)||(xd>=splx && yd<sply && gameImage[0x7424]&0x40 && gameImage[0x7580|idx]&0x04)||
                            (xd<splx && yd>=sply && gameImage[0x7424]&0x20 && gameImage[0x7580|idx]&0x02)||(xd>=splx && yd>=sply && gameImage[0x7424]&0x10 && gameImage[0x7580|idx]&0x01))
                                {
                                    putPixelFromTile((256+gameImage[addr+2])^xh^(yh<<4),4+(gameImage[addr+3]&3),x,y,xd,yd);
                                }
                            }
                        }
                    }
                }
            }
        }
        //display
        QImage imageMap(pixels,256,192,QImage::Format_RGB32);
        QPixmap pixelMap = QPixmap::fromImage(imageMap);
        pixelMap = pixelMap.scaled(labelMap->width(),labelMap->height());
        labelMap->setPixmap(pixelMap);
    }
}

int audioLoop(void * outputBuffer,void *,unsigned int nBufferFrames,double streamTime,RtAudioStreamStatus status, void * data)
{
    Q_UNUSED(streamTime)
    Q_UNUSED(status)
    Q_UNUSED(data)
    float * out = reinterpret_cast<float*>(outputBuffer);
    if(isRunning && isExec)
    {
        for(int i=0; i<static_cast<int>(nBufferFrames); i++)
        {
            float samp = 0.0f;
            for(int n=0; n<4; n++)
            {
                if(gameImage[0x7425]&(8>>n))
                {
                    int freq = ((gameImage[0x7418|(n<<1)]&0x0F)<<8)|gameImage[0x7419|(n<<1)];
                    if(freq<0.5f) {freq=4096;}
                    float amp = (static_cast<float>(gameImage[0x7418|(n<<1)]>>4))/128.0f;
                    int inst = (gameImage[0x7410]&(0xC0>>(n<<1)))>>((3-n)<<1);
                    inst = (inst<<1)|((gameImage[0x7414]&(8>>n))>>(3-n));
                    channelTimer[n] += freq;
                    for(int j=0; j<2; j++) {
                        if(channelTimer[n]>=3000) {
                            channelTimer[n]-=3000;
                            channelSample[n]++;
                            channelSample[n]&=0x0F;
                            if((inst>>1)==3) {
                                if(waveformLut[96|channelSample[n]]>0) {
                                    channelNoiseVal[n] = ((rand()&1)<<1)-1;
                                }
                            }
                        }
                    }
                    if((inst>>1)==3) {
                        float val = static_cast<float>(channelNoiseVal[n]);
                        samp += amp*val;
                    } else {
                        samp += amp*waveformLut[(inst<<4)|channelSample[n]];
                    }
                }
            }
            *out++ = samp;
            *out++ = samp;
        }
        timer2 += nBufferFrames;
        timer2 &= 0xFFFFF;
    }
    else
    {
        for(int i=0; i<static_cast<int>(nBufferFrames); i++)
        {
            *out++ = 0.0f;
            *out++ = 0.0f;
        }
    }
    return 0;
}
void audioError(RtAudioError::Type type,const std::string &errorText)
{
    std::cerr << "ERROR CALLBACK:\n";
    if(type==RtAudioError::WARNING)
    {
        std::cerr << errorText << "\n";
    }
    else
    {
        throw RtAudioError(errorText,type);
    }
}

MainWindow::MainWindow(QWidget *parent,int argc, char *argv[]) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label->installEventFilter(this);
    labelMap = ui->label;
    dtv = new DialogTileViewer(this);
    dbv = new DialogBackgroundViewer(this);
    dhe = new DialogHexEditor(this);
    dd = new DialogDebugger(this);
    dsss = new DialogSetSaveSlot(this);
    dmw = new DialogMemoryWatch(this);

    prefix.assign(argv[0]);
    while(true) {
        prefix.pop_back();
        char last = prefix.back();
        if((last=='/')||(last=='\\')) {break;}
    }

    ctrlSettings = new QSettings("Fautenic","f8emu-qt");
    if(ctrlSettings->value("InputSettings").isNull()) {
        ctrlMap.insert("Port1CtrlType",1);
        ctrlMap.insert("Port1Up",Qt::Key_Up);
        ctrlMap.insert("Port1Down",Qt::Key_Down);
        ctrlMap.insert("Port1Left",Qt::Key_Left);
        ctrlMap.insert("Port1Right",Qt::Key_Right);
        ctrlMap.insert("Port1Start",Qt::Key_Return);
        ctrlMap.insert("Port1A",Qt::Key_Z);
        ctrlMap.insert("Port1B",Qt::Key_X);
        ctrlMap.insert("Port1C",Qt::Key_C);
        ctrlMap.insert("Port1L",Qt::Key_Q);
        ctrlMap.insert("Port1R",Qt::Key_W);
        ctrlMap.insert("Port1X",Qt::Key_A);
        ctrlMap.insert("Port1Y",Qt::Key_S);
        ctrlMap.insert("Port1Z",Qt::Key_D);
        ctrlSettings->setValue("InputSettings",ctrlMap);
    } else {
        ctrlMap = ctrlSettings->value("InputSettings").toMap();
    }

    init6502();
    clearScreen();

    unsigned int bufferFrames = 256;
    void * audioData = calloc(2,sizeof(float));
    oParams.deviceId = dac.getDefaultOutputDevice();
    oParams.nChannels = 2;
    oParams.firstChannel = 0;
    options.flags = RTAUDIO_SCHEDULE_REALTIME;
    dac.showWarnings(true);
    try
    {
        dac.openStream(&oParams,nullptr,RTAUDIO_FLOAT32,48000,&bufferFrames,&audioLoop,audioData,&options,&audioError);
        dac.startStream();
    }
    catch(RtAudioError& e)
    {
        e.printMessage();
        if(dac.isStreamOpen())
        {
            dac.closeStream();
        }
        exit(-1);
    }

    if(argc==2)
    {
        QString path(argv[1]);
        this->openNewGame(path);
    }
}
MainWindow::~MainWindow()
{
    closePreviousGame();
    delete ui;
    dac.stopStream();
    if(dac.isStreamOpen())
    {
        dac.closeStream();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    closePreviousGame();
    QFileDialog romFileDialog(nullptr,tr("Load ROM"),".",tr("ROM Files (*.f8)"));
    romFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    romFileDialog.setFileMode(QFileDialog::ExistingFile);
    if(QDialog::Accepted != romFileDialog.exec()) {
        return;
    }
    QString path = romFileDialog.selectedFiles()[0];
    this->openNewGame(path);
}
void MainWindow::on_actionClose_triggered()
{
    closePreviousGame();
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(ctrlMap["Port1CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port1Up"].toInt()) {
            gameImage[0x7480] |= 0x80;
        } else if(event->key()==ctrlMap["Port1Down"].toInt()) {
            gameImage[0x7480] |= 0x40;
        } else if(event->key()==ctrlMap["Port1Left"].toInt()) {
            gameImage[0x7480] |= 0x20;
        } else if(event->key()==ctrlMap["Port1Right"].toInt()) {
            gameImage[0x7480] |= 0x10;
        } else if(event->key()==ctrlMap["Port1Start"].toInt()) {
            gameImage[0x7480] |= 0x08;
        } else if(event->key()==ctrlMap["Port1A"].toInt()) {
            gameImage[0x7480] |= 0x04;
        } else if(event->key()==ctrlMap["Port1B"].toInt()) {
            gameImage[0x7480] |= 0x02;
        } else if(event->key()==ctrlMap["Port1C"].toInt()) {
            gameImage[0x7480] |= 0x01;
        }
    }
    if(ctrlMap["Port1CtrlType"]==2) {
        if(event->key()==ctrlMap["Port1L"].toInt()) {
            gameImage[0x7484] |= 0x20;
        } else if(event->key()==ctrlMap["Port1R"].toInt()) {
            gameImage[0x7484] |= 0x10;
        } else if(event->key()==ctrlMap["Port1X"].toInt()) {
            gameImage[0x7484] |= 0x04;
        } else if(event->key()==ctrlMap["Port1Y"].toInt()) {
            gameImage[0x7484] |= 0x02;
        } else if(event->key()==ctrlMap["Port1Z"].toInt()) {
            gameImage[0x7484] |= 0x01;
        }
    }

    if(ctrlMap["Port2CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port2Up"].toInt()) {
            gameImage[0x7481] |= 0x80;
        } else if(event->key()==ctrlMap["Port2Down"].toInt()) {
            gameImage[0x7481] |= 0x40;
        } else if(event->key()==ctrlMap["Port2Left"].toInt()) {
            gameImage[0x7481] |= 0x20;
        } else if(event->key()==ctrlMap["Port2Right"].toInt()) {
            gameImage[0x7481] |= 0x10;
        } else if(event->key()==ctrlMap["Port2Start"].toInt()) {
            gameImage[0x7481] |= 0x08;
        } else if(event->key()==ctrlMap["Port2A"].toInt()) {
            gameImage[0x7481] |= 0x04;
        } else if(event->key()==ctrlMap["Port2B"].toInt()) {
            gameImage[0x7481] |= 0x02;
        } else if(event->key()==ctrlMap["Port2C"].toInt()) {
            gameImage[0x7481] |= 0x01;
        }
    }
    if(ctrlMap["Port2CtrlType"]==2) {
        if(event->key()==ctrlMap["Port2L"].toInt()) {
            gameImage[0x7485] |= 0x20;
        } else if(event->key()==ctrlMap["Port2R"].toInt()) {
            gameImage[0x7485] |= 0x10;
        } else if(event->key()==ctrlMap["Port2X"].toInt()) {
            gameImage[0x7485] |= 0x04;
        } else if(event->key()==ctrlMap["Port2Y"].toInt()) {
            gameImage[0x7485] |= 0x02;
        } else if(event->key()==ctrlMap["Port2Z"].toInt()) {
            gameImage[0x7485] |= 0x01;
        }
    }

    if(ctrlMap["Port3CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port3Up"].toInt()) {
            gameImage[0x7482] |= 0x80;
        } else if(event->key()==ctrlMap["Port3Down"].toInt()) {
            gameImage[0x7482] |= 0x40;
        } else if(event->key()==ctrlMap["Port3Left"].toInt()) {
            gameImage[0x7482] |= 0x20;
        } else if(event->key()==ctrlMap["Port3Right"].toInt()) {
            gameImage[0x7482] |= 0x10;
        } else if(event->key()==ctrlMap["Port3Start"].toInt()) {
            gameImage[0x7482] |= 0x08;
        } else if(event->key()==ctrlMap["Port3A"].toInt()) {
            gameImage[0x7482] |= 0x04;
        } else if(event->key()==ctrlMap["Port3B"].toInt()) {
            gameImage[0x7482] |= 0x02;
        } else if(event->key()==ctrlMap["Port3C"].toInt()) {
            gameImage[0x7482] |= 0x01;
        }
    }
    if(ctrlMap["Port3CtrlType"]==2) {
        if(event->key()==ctrlMap["Port3L"].toInt()) {
            gameImage[0x7486] |= 0x20;
        } else if(event->key()==ctrlMap["Port3R"].toInt()) {
            gameImage[0x7486] |= 0x10;
        } else if(event->key()==ctrlMap["Port3X"].toInt()) {
            gameImage[0x7486] |= 0x04;
        } else if(event->key()==ctrlMap["Port3Y"].toInt()) {
            gameImage[0x7486] |= 0x02;
        } else if(event->key()==ctrlMap["Port3Z"].toInt()) {
            gameImage[0x7486] |= 0x01;
        }
    }

    if(ctrlMap["Port4CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port4Up"].toInt()) {
            gameImage[0x7483] |= 0x80;
        } else if(event->key()==ctrlMap["Port4Down"].toInt()) {
            gameImage[0x7483] |= 0x40;
        } else if(event->key()==ctrlMap["Port4Left"].toInt()) {
            gameImage[0x7483] |= 0x20;
        } else if(event->key()==ctrlMap["Port4Right"].toInt()) {
            gameImage[0x7483] |= 0x10;
        } else if(event->key()==ctrlMap["Port4Start"].toInt()) {
            gameImage[0x7483] |= 0x08;
        } else if(event->key()==ctrlMap["Port4A"].toInt()) {
            gameImage[0x7483] |= 0x04;
        } else if(event->key()==ctrlMap["Port4B"].toInt()) {
            gameImage[0x7483] |= 0x02;
        } else if(event->key()==ctrlMap["Port4C"].toInt()) {
            gameImage[0x7483] |= 0x01;
        }
    }
    if(ctrlMap["Port4CtrlType"]==2) {
        if(event->key()==ctrlMap["Port4L"].toInt()) {
            gameImage[0x7487] |= 0x20;
        } else if(event->key()==ctrlMap["Port4R"].toInt()) {
            gameImage[0x7487] |= 0x10;
        } else if(event->key()==ctrlMap["Port4X"].toInt()) {
            gameImage[0x7487] |= 0x04;
        } else if(event->key()==ctrlMap["Port4Y"].toInt()) {
            gameImage[0x7487] |= 0x02;
        } else if(event->key()==ctrlMap["Port4Z"].toInt()) {
            gameImage[0x7487] |= 0x01;
        }
    }
}
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(ctrlMap["Port1CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port1Up"].toInt()) {
            gameImage[0x7480] &= 0x7F;
        } else if(event->key()==ctrlMap["Port1Down"].toInt()) {
            gameImage[0x7480] &= 0xBF;
        } else if(event->key()==ctrlMap["Port1Left"].toInt()) {
            gameImage[0x7480] &= 0xDF;
        } else if(event->key()==ctrlMap["Port1Right"].toInt()) {
            gameImage[0x7480] &= 0xEF;
        } else if(event->key()==ctrlMap["Port1Start"].toInt()) {
            gameImage[0x7480] &= 0xF7;
        } else if(event->key()==ctrlMap["Port1A"].toInt()) {
            gameImage[0x7480] &= 0xFB;
        } else if(event->key()==ctrlMap["Port1B"].toInt()) {
            gameImage[0x7480] &= 0xFD;
        } else if(event->key()==ctrlMap["Port1C"].toInt()) {
            gameImage[0x7480] &= 0xFE;
        }
    }
    if(ctrlMap["Port1CtrlType"]==2) {
        if(event->key()==ctrlMap["Port1L"].toInt()) {
            gameImage[0x7484] &= 0xDF;
        } else if(event->key()==ctrlMap["Port1R"].toInt()) {
            gameImage[0x7484] &= 0xEF;
        } else if(event->key()==ctrlMap["Port1X"].toInt()) {
            gameImage[0x7484] &= 0xFB;
        } else if(event->key()==ctrlMap["Port1Y"].toInt()) {
            gameImage[0x7484] &= 0xFD;
        } else if(event->key()==ctrlMap["Port1Z"].toInt()) {
            gameImage[0x7484] &= 0xFE;
        }
    }

    if(ctrlMap["Port2CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port2Up"].toInt()) {
            gameImage[0x7481] &= 0x7F;
        } else if(event->key()==ctrlMap["Port2Down"].toInt()) {
            gameImage[0x7481] &= 0xBF;
        } else if(event->key()==ctrlMap["Port2Left"].toInt()) {
            gameImage[0x7481] &= 0xDF;
        } else if(event->key()==ctrlMap["Port2Right"].toInt()) {
            gameImage[0x7481] &= 0xEF;
        } else if(event->key()==ctrlMap["Port2Start"].toInt()) {
            gameImage[0x7481] &= 0xF7;
        } else if(event->key()==ctrlMap["Port2A"].toInt()) {
            gameImage[0x7481] &= 0xFB;
        } else if(event->key()==ctrlMap["Port2B"].toInt()) {
            gameImage[0x7481] &= 0xFD;
        } else if(event->key()==ctrlMap["Port2C"].toInt()) {
            gameImage[0x7481] &= 0xFE;
        }
    }
    if(ctrlMap["Port2CtrlType"]==2) {
        if(event->key()==ctrlMap["Port2L"].toInt()) {
            gameImage[0x7485] &= 0xDF;
        } else if(event->key()==ctrlMap["Port2R"].toInt()) {
            gameImage[0x7485] &= 0xEF;
        } else if(event->key()==ctrlMap["Port2X"].toInt()) {
            gameImage[0x7485] &= 0xFB;
        } else if(event->key()==ctrlMap["Port2Y"].toInt()) {
            gameImage[0x7485] &= 0xFD;
        } else if(event->key()==ctrlMap["Port2Z"].toInt()) {
            gameImage[0x7485] &= 0xFE;
        }
    }

    if(ctrlMap["Port3CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port3Up"].toInt()) {
            gameImage[0x7482] &= 0x7F;
        } else if(event->key()==ctrlMap["Port3Down"].toInt()) {
            gameImage[0x7482] &= 0xBF;
        } else if(event->key()==ctrlMap["Port3Left"].toInt()) {
            gameImage[0x7482] &= 0xDF;
        } else if(event->key()==ctrlMap["Port3Right"].toInt()) {
            gameImage[0x7482] &= 0xEF;
        } else if(event->key()==ctrlMap["Port3Start"].toInt()) {
            gameImage[0x7482] &= 0xF7;
        } else if(event->key()==ctrlMap["Port3A"].toInt()) {
            gameImage[0x7482] &= 0xFB;
        } else if(event->key()==ctrlMap["Port3B"].toInt()) {
            gameImage[0x7482] &= 0xFD;
        } else if(event->key()==ctrlMap["Port3C"].toInt()) {
            gameImage[0x7482] &= 0xFE;
        }
    }
    if(ctrlMap["Port3CtrlType"]==2) {
        if(event->key()==ctrlMap["Port3L"].toInt()) {
            gameImage[0x7486] &= 0xDF;
        } else if(event->key()==ctrlMap["Port3R"].toInt()) {
            gameImage[0x7486] &= 0xEF;
        } else if(event->key()==ctrlMap["Port3X"].toInt()) {
            gameImage[0x7486] &= 0xFB;
        } else if(event->key()==ctrlMap["Port3Y"].toInt()) {
            gameImage[0x7486] &= 0xFD;
        } else if(event->key()==ctrlMap["Port3Z"].toInt()) {
            gameImage[0x7486] &= 0xFE;
        }
    }

    if(ctrlMap["Port4CtrlType"]!=0) {
        if(event->key()==ctrlMap["Port4Up"].toInt()) {
            gameImage[0x7483] &= 0x7F;
        } else if(event->key()==ctrlMap["Port4Down"].toInt()) {
            gameImage[0x7483] &= 0xBF;
        } else if(event->key()==ctrlMap["Port4Left"].toInt()) {
            gameImage[0x7483] &= 0xDF;
        } else if(event->key()==ctrlMap["Port4Right"].toInt()) {
            gameImage[0x7483] &= 0xEF;
        } else if(event->key()==ctrlMap["Port4Start"].toInt()) {
            gameImage[0x7483] &= 0xF7;
        } else if(event->key()==ctrlMap["Port4A"].toInt()) {
            gameImage[0x7483] &= 0xFB;
        } else if(event->key()==ctrlMap["Port4B"].toInt()) {
            gameImage[0x7483] &= 0xFD;
        } else if(event->key()==ctrlMap["Port4C"].toInt()) {
            gameImage[0x7483] &= 0xFE;
        }
    }
    if(ctrlMap["Port4CtrlType"]==2) {
        if(event->key()==ctrlMap["Port4L"].toInt()) {
            gameImage[0x7487] &= 0xDF;
        } else if(event->key()==ctrlMap["Port4R"].toInt()) {
            gameImage[0x7487] &= 0xEF;
        } else if(event->key()==ctrlMap["Port4X"].toInt()) {
            gameImage[0x7487] &= 0xFB;
        } else if(event->key()==ctrlMap["Port4Y"].toInt()) {
            gameImage[0x7487] &= 0xFD;
        } else if(event->key()==ctrlMap["Port4Z"].toInt()) {
            gameImage[0x7487] &= 0xFE;
        }
    }
}
void MainWindow::on_actionConfigure_Input_triggered()
{
    QMap<QString, QVariant> tempMap = ctrlMap = ctrlSettings->value("InputSettings").toMap();
    DialogInput di(this);
    if(di.exec()==QDialog::Rejected)
    {
        ctrlSettings->setValue("InputSettings",tempMap);
    } else {
        ctrlSettings->setValue("InputSettings",ctrlMap);
    }
}
void MainWindow::on_actionReset_triggered()
{
    reset6502();
    statusBar()->showMessage("Reset game.",3000);
}
void MainWindow::on_actionQuit_triggered()
{
    exit(0);
}
void MainWindow::on_actionTile_Viewer_triggered()
{
    dtv->show();
}
void MainWindow::on_actionBackground_Viewer_triggered()
{
    dbv->show();
}
void MainWindow::on_actionHex_Editor_triggered()
{
    dhe->show();
}
void MainWindow::on_actionDebugger_triggered()
{
    dd->show();
}
void MainWindow::on_actionGame_Info_triggered()
{
    DialogGameInfo dgi(this);
    dgi.exec();
}

void MainWindow::on_actionPause_triggered()
{
    isExec = !isExec;
    frameAdvance = -1;
}
void MainWindow::on_actionFrame_Advance_triggered()
{
    frameAdvance = 2;
    isExec = true;
}
void MainWindow::on_actionLoad_State_triggered()
{
    string saveFileName;
    saveFileName.assign(prefix);
    saveFileName += "save/";
    for(int i=4; i<8; i++) {
        saveFileName += static_cast<char>(gameHeader[i]);
    }
    saveFileName += '_';
    saveFileName += saveStateSlot;
    saveFileName += ".bin";

    QFile outFile;
    outFile.setFileName(saveFileName.c_str());
    if(QFile::exists(saveFileName.c_str()))
    {
        outFile.open(QIODevice::ReadOnly);

        char c;
        for(int i=0; i<65536; i++)
        {
            outFile.getChar(&c);
            gameImage[i] = static_cast<BYTE>(c);
        }
        outFile.getChar(&c);
        a_reg = static_cast<BYTE>(c);
        outFile.getChar(&c);
        outFile.getChar(&c);
        x_reg = static_cast<BYTE>(c);
        outFile.getChar(&c);
        y_reg = static_cast<BYTE>(c);
        outFile.getChar(&c);
        s_reg = static_cast<BYTE>(c);
        outFile.getChar(&c);
        flag_reg = static_cast<BYTE>(c);
        outFile.getChar(&c);
        pc_reg = static_cast<unsigned char>(c);
        outFile.getChar(&c);
        pc_reg |= static_cast<unsigned char>(c)<<8;
        outFile.getChar(&c);
        timerTicks = static_cast<unsigned char>(c);
        outFile.getChar(&c);
        timerTicks |= static_cast<unsigned char>(c)<<8;
        outFile.getChar(&c);
        timerTicks |= static_cast<unsigned char>(c)<<16;
        outFile.getChar(&c);
        timerTicks |= static_cast<unsigned char>(c)<<24;

        outFile.close();
        string msg = "Loaded state ";
        msg += saveStateSlot;
        msg += ".";
        statusBar()->showMessage(msg.c_str(),3000);
    }
}
void MainWindow::on_actionSave_State_triggered()
{
    string saveFileName;
    saveFileName.assign(prefix);
    saveFileName += "save/";
    for(int i=4; i<8; i++) {
        saveFileName += static_cast<char>(gameHeader[i]);
    }
    saveFileName += '_';
    saveFileName += saveStateSlot;
    saveFileName += ".bin";

    QFile outFile;
    outFile.setFileName(saveFileName.c_str());
    if(outFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        for(int i=0; i<65536; i++)
        {
            outFile.putChar(static_cast<char>(gameImage[i]));
        }
        outFile.putChar(static_cast<char>(a_reg));
        outFile.putChar(0);
        outFile.putChar(static_cast<char>(x_reg));
        outFile.putChar(static_cast<char>(y_reg));
        outFile.putChar(static_cast<char>(s_reg));
        outFile.putChar(static_cast<char>(flag_reg));
        outFile.putChar(static_cast<char>(pc_reg&0xFF));
        outFile.putChar(static_cast<char>((pc_reg>>8)&0xFF));
        outFile.putChar(static_cast<char>(timerTicks&0xFF));
        outFile.putChar(static_cast<char>((timerTicks>>8)&0xFF));
        outFile.putChar(static_cast<char>((timerTicks>>16)&0xFF));
        outFile.putChar(static_cast<char>((timerTicks>>24)&0xFF));
        outFile.putChar(0);
        outFile.putChar(0);
        outFile.putChar(0);
        outFile.putChar(0);

        outFile.close();
        string msg = "Saved state ";
        msg += saveStateSlot;
        msg += ".";
        statusBar()->showMessage(msg.c_str(),3000);
    }
}
void MainWindow::on_actionSet_Save_Slot_triggered()
{
    char y = static_cast<char>(dsss->exec());
    if(y!=-1) {saveStateSlot = y;}
    string msg = "Save state slot ";
    msg += saveStateSlot;
    msg += " selected.";
    statusBar()->showMessage(msg.c_str(),3000);
}
void MainWindow::on_actionMemory_Watch_triggered()
{
    dmw->show();
}

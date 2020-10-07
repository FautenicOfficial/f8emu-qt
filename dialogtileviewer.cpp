#include "dialogtileviewer.h"
#include "ui_dialogtileviewer.h"

unsigned char tvPixelsB[128*128*4];
unsigned char tvPixelsS[128*128*4];
unsigned char tvPixelsP[16*2*4];
QLabel * tvLabelMapB;
QLabel * tvLabelMapS;
QLabel * tvLabelMapP;
QTimer * tvLoopTimer;
int curPalB=0, curPalS=4;

DialogTileViewer::DialogTileViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTileViewer)
{
    ui->setupUi(this);
    ui->label->installEventFilter(this);
    tvLabelMapB = ui->label;
    ui->label_2->installEventFilter(this);
    tvLabelMapS = ui->label_2;
    ui->label_3->installEventFilter(this);
    tvLabelMapP = ui->label_3;

    tvLoopTimer = new QTimer;
    connect(tvLoopTimer,SIGNAL(timeout()),this,SLOT(tvDrawLoop()));
    tvLoopTimer->start(16);
}
DialogTileViewer::~DialogTileViewer()
{
    tvLoopTimer->stop();
    delete ui;
}

void DialogTileViewer::tvPutPixel(int color,int xd,int yd,unsigned char * pixels)
{
    int pix = ((xd+(yd<<7))<<2);
    pixels[pix+2] = 85*((color&0x30)>>4);
    pixels[pix+1] = 85*((color&0x0C)>>2);
    pixels[pix] = 85*(color&0x03);
}
void DialogTileViewer::tvPutPixelFromTile(int tile,int pal,int xs,int ys,int xd,int yd,unsigned char * pixels)
{
    int addr = 0x4000+(tile<<4)+ys;
    int entry0 = (gameImage[addr]>>(7-xs))&1;
    int entry1 = (gameImage[addr+8]>>(7-xs))&1;
    int entry = entry0+(entry1<<1);
    int color = gameImage[0x7500+(pal<<2)+entry];
    tvPutPixel(color,xd,yd,pixels);
}
void DialogTileViewer::tvDrawLoop()
{
    for(int i=0; i<128; i++) {
        for(int j=0; j<128; j++) {
            int xs = i&7;
            int ys = j&7;
            int tile = (i>>3)|((j>>3)<<4);
            tvPutPixelFromTile(tile,curPalB,xs,ys,i,j,tvPixelsB);
            tvPutPixelFromTile(tile|256,curPalS,xs,ys,i,j,tvPixelsS);
        }
    }
    for(int i=0; i<16; i++) {
        for(int j=0; j<2; j++) {
            int pix = ((i+(j<<4))<<2);
            int color = gameImage[0x7500|i|(j<<4)];
            tvPixelsP[pix+2] = 85*((color&0x30)>>4);
            tvPixelsP[pix+1] = 85*((color&0x0C)>>2);
            tvPixelsP[pix] = 85*(color&0x03);
        }
    }
    QImage imageMapB(tvPixelsB,128,128,QImage::Format_RGB32);
    QPixmap pixelMapB = QPixmap::fromImage(imageMapB);
    pixelMapB = pixelMapB.scaled(tvLabelMapB->width(),tvLabelMapB->height());
    tvLabelMapB->setPixmap(pixelMapB);
    QImage imageMapS(tvPixelsS,128,128,QImage::Format_RGB32);
    QPixmap pixelMapS = QPixmap::fromImage(imageMapS);
    pixelMapS = pixelMapS.scaled(tvLabelMapS->width(),tvLabelMapS->height());
    tvLabelMapS->setPixmap(pixelMapS);
    QImage imageMapP(tvPixelsP,16,2,QImage::Format_RGB32);
    QPixmap pixelMapP = QPixmap::fromImage(imageMapP);
    pixelMapP = pixelMapP.scaled(tvLabelMapP->width(),tvLabelMapP->height());
    tvLabelMapP->setPixmap(pixelMapP);
}

void DialogTileViewer::on_pushButton_clicked()
{
    curPalB++;
    curPalB&=3;
}
void DialogTileViewer::on_pushButton_2_clicked()
{
    curPalS++;
    curPalS&=3;
    curPalS|=4;
}

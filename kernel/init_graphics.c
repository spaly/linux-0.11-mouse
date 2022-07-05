#include <linux/kernel.h>
#include <asm/io.h>
#include "linux/tty.h"
#include <unistd.h>
#define memstart 0xA0000
#define memsize 64000
//#define cursor_side 3
#define width 320 //分辨率
#define height 200
//#define barrier_width 10

int ff=0;
int sys_init_graphics(){
    
if (ff==0){
    //数据放入显存片
    outb(0x05,0x3CE);
    outb(0x40,0x3CF);// 设定256色, 且取出方式为移动拼装
    outb(0x06,0x3CE);
    outb(0x05,0x3CF);// 设定显存地址区域，禁止字符区域
    outb(0x04,0x3C4);
    outb(0x08,0x3C5);//4个显存片连在一起

    //设置屏幕分辨率
    outb(0x01,0x3D4);
    outb(0x4F,0x3D5);// end horizontal display=79
    outb(0x03,0x3D4);
    outb(0x82,0x3D5);// display enable skew=0

    outb(0x07,0x3D4);
    outb(0x1F,0x3D5);// vertical display end 8,9 bit=1,0 
    outb(0x12,0x3D4);
    outb(0x8F,0x3D5);// vertical display end low 7b =0x8F
    outb(0x17,0x3D4);
    outb(0xA3,0x3D5);// SLDIV=1 ,scanline clock/=2

    outb(0x14,0x3D4);
    outb(0x40,0x3D5);// DW=1
    outb(0x13,0x3D4);
    outb(0x28,0x3D5);// Offset=40
    
    outb(0x0C,0x3D4);
    outb(0x00,0x3D5);
    outb(0x0D,0x3D4);
    outb(0x00,0x3D5);//Start Address=0xA0000
    ff=1;
}
    int i,j,x,y;
    char *p=memstart;
    for(i=0;i<memsize;i++) *p++=3; //背景蓝绿
    return 0;
}

int sys_repaint(int x,int y,char c){
	int i,j,lx,rx,ly,ry;
	lx=x>>9; rx=x&511; if (rx>width) rx=width;
	ly=y>>9; ry=y&511; if (ry>height) ry=height;
	char *p;
	for(i=lx;i<=rx;++i)
		for(j=ly;j<=ry; ++j)
			p=(char*)memstart+j*width+i,*p=c;
	return 0;
}

#include <linux/kernel.h>
#include <asm/io.h>
#include "linux/tty.h"
#define memstart 0xA0000
#define memsize 64000
#define cursor_side 3
#define width 320 //分辨率
#define height 200
#define barrier_width 10
int volatile jumpp;
int ff=0;
int sys_init_graphics(){
    int i,j,x,y; printk("##\n");
    char *p=0xA0000;
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
    //绘制屏幕
    outb(0x0C,0x3D4);
    outb(0x00,0x3D5);
    outb(0x0D,0x3D4);
    outb(0x00,0x3D5);//Start Address=0xA0000
    ff=1;
}
    //绘制鼠标
    p=memstart;
    for(i=0;i<memsize;i++) *p++=3; //背景蓝绿
    //3-blue 4-red 12-purple
    x=20; y=10;
    for(i=x-cursor_side;i<=x+cursor_side;++i)
        for(j=y-cursor_side;j<=y+cursor_side;++j){
            p=(char *) memstart+j*width+i;
            *p=12; //鼠标红色
        }
    return 0;
}

int sys_get_message(){
	if(jumpp>0) --jumpp;
	return jumpp;
}

int sys_repaint(int x,int y,int h){
	int i,j,w;
	char *p;
	i=x; j=y; p=0xA0000; w=barrier_width;
	if (i+w>=320||i<20) return 0;
	if (i==33||j==33){
        p=0xA0000;
	    for (i=0;i<memsize;++i) *p++=3;
	    return 0;
	}
	else if(i==44||j==44){
        p=0xA0000;
	    for(i=0;i<memsize;++i) *p++=4;
	    return 0;
	}
    else{
	    for(i=x;i<=x+w;++i){	
		    for(j=y;j<=y+h;++j){
			    p=0xA0000+j*320+i;
			    *p=12;
		    }
		}
    }
    return 0;
}

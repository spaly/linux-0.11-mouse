#define __LIBRARY__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
_syscall1(int, get_message, int*, msg)
_syscall0(int, init_graphics)
_syscall3(int, repaint, int, x, int, y, char, c)
_syscall2(int, timer_create, int, ms, int, type)

#define BIRD_X      120
#define BIRD_WIDTH  10
#define BIRD_HEIGHT 8

typedef struct{
    int xpos1, xpos2, ypos1, ypos2;
}obstacle;

obstacle ob[2000];
obstacle bird, all;

#define MAX_X 320
#define MAX_Y 200

obstacle pre[2000];
obstacle a[2000];

int bird_y = 100;
int maxx(int a, int b) { return a > b ? a : b; }
int minn(int a, int b) { return a < b ? a : b; }

void paint(obstacle p, char x) {
    int xpos1, xpos2, ypos1, ypos2;
    xpos1 = p.xpos1, xpos2 = p.xpos2;
    ypos1 = p.ypos1, ypos2 = p.ypos2;
    if (xpos1 < 0) xpos1 = 0; 
    if (xpos2 < 0) xpos2 = 0;
    if (ypos1 < 0) ypos1 = 0;
    if (ypos2 < 0) ypos2 = 0;
    if (xpos1 >= MAX_X) xpos1 = MAX_X - 1;
    if (xpos2 >= MAX_X) xpos2 = MAX_X - 1;
    if (ypos1 >= MAX_Y) ypos1 = MAX_Y - 1;
    if (ypos2 >= MAX_Y) ypos2 = MAX_Y - 1;
    if (xpos1 == xpos2 && xpos1 == 0) return;
    if (ypos1 == ypos2 && ypos1 == 0) return;
    if (xpos1 == xpos2 && xpos1 == MAX_X - 1) return;
    if (ypos1 == ypos2 && ypos1 == MAX_Y - 1) return;

    repaint(((xpos1 << 9) | xpos2), ((ypos1 << 9) | ypos2), x);
}

int obstacle_count;

int get_stucked(int posx, int posy) {
    int pos, i, j;
    for (pos = 0; pos < obstacle_count; ++pos) {
        if (posx - (BIRD_WIDTH / 2) > a[pos].xpos2) break;
        if (posx + (BIRD_WIDTH / 2) < a[pos].xpos1) continue;
        if ((posx - (BIRD_WIDTH / 2) >= a[pos].xpos1 && posx - (BIRD_WIDTH / 2) <= a[pos].xpos2)
         || (posx + (BIRD_WIDTH / 2) >= a[pos].xpos1 && posx + (BIRD_WIDTH / 2) <= a[pos].xpos2)
         || (posx - (BIRD_WIDTH / 2) >= a[pos].xpos1 && posx + (BIRD_WIDTH / 2) <= a[pos].xpos2)) 
        {
            if ((posy - (BIRD_HEIGHT / 2) >= a[pos].ypos1 && posy - (BIRD_HEIGHT / 2) <= a[pos].ypos2)
             || (posy + (BIRD_HEIGHT / 2) >= a[pos].ypos1 && posy + (BIRD_HEIGHT / 2) <= a[pos].ypos2)
             || (posy - (BIRD_HEIGHT / 2) >= a[pos].ypos1 && posy + (BIRD_HEIGHT / 2) <= a[pos].ypos2)) 
            {
                return 1;
            }
        }
    }
    return 0;
}

int GAME_OVER() {
    paint(all, 44); sleep(5); exit(0);
}

int main(){
    int speed,pos,i,j,*m,tmp;
    int now_x=50,now_y=100;
    int ok=0;
    printf("Select your speed(300ms~3000ms): \n");
    scanf("%d",&speed);
    if (speed<300) speed=300; if (speed>3000) speed=3000;

    srand(time(NULL));
    obstacle_count=rand()%200+100;

    for (i=0;i<obstacle_count;++i){
	if (!i){
		a[i].xpos1=200-(BIRD_WIDTH/2);
        	a[i].xpos2=200+(BIRD_WIDTH/2);	
	}
	else{
        	a[i].xpos1=a[i-1].xpos1+64;
        	a[i].xpos2=a[i-1].xpos2+64;
	}
       	tmp=rand()%3;
        if (!tmp) a[i].ypos1=0,a[i].ypos2=rand()%50+30;
        else if (tmp==1)
            a[i].ypos1=rand()%20+120,a[i].ypos2=199;
        else{
            a[i].ypos1=0,a[i].ypos2=rand()%50+30;
            ++i;
            a[i].xpos1=a[i-1].xpos1;
            a[i].xpos2=a[i-1].xpos2;
            a[i].ypos1=rand()%20+120,a[i].ypos2=199;
            if (a[i].ypos1<a[i-1].ypos2+120) a[i].ypos1=a[i-1].ypos2+120;
        }
    }

    init_graphics();
    all.xpos1=0,all.xpos2=319;
    all.ypos1=0,all.ypos2=199;

    for(i=0;i<obstacle_count;++i) {paint(a[i],12);}

    bird.xpos1 = now_x - (BIRD_WIDTH / 2);
    bird.xpos2 = now_x + (BIRD_WIDTH / 2);
    bird.ypos1 = now_y - (BIRD_HEIGHT / 2);
    bird.ypos2 = now_y + (BIRD_HEIGHT / 2);

    paint(bird, 12);

    for (i=0;i<obstacle_count;++i) {paint(a[i],12);}
    sleep(5);
    timer_create(speed,0);
    while(1) {
        get_message(m);
        if (*m == -1) continue;

        if (*m == 1 || *m == 0) 
            now_y -= 20;
        if (*m == 2)
            now_y += 20;
            
        paint(bird, 3);
        bird.ypos1 = now_y - (BIRD_HEIGHT / 2);
        bird.ypos2 = now_y + (BIRD_HEIGHT / 2);
        paint(bird, 12);

        if (get_stucked(now_x, now_y)) { ok = 1; break; }
        if (now_y - (BIRD_HEIGHT / 2) < 0 || now_y + (BIRD_HEIGHT / 2) >= 200) { ok = 1; break; }
        for (i = 0; i < obstacle_count; ++i){
            paint(a[i], 3),
            a[i].xpos1 -= 20, 
            a[i].xpos2 -= 20, 
            paint(a[i], 12);
        }
    }
    if (ok) GAME_OVER();
    return 0;
}

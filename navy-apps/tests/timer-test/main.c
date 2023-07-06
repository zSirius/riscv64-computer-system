#include <unistd.h>
#include <stdio.h>
#include <sys/_timeval.h>

int main(){
    uint32_t oldTime, curTime;
    oldTime = NDL_GetTicks();
    while(1){
        curTime = NDL_GetTicks();
        if(curTime - oldTime >=500){
            printf("msec = %ld. Time has gone 5 seconds.\n", oldTime.tv_sec);
            oldTime = curTime;
        }
    }

    return 0;
}
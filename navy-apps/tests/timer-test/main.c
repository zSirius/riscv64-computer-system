#include <unistd.h>
#include <stdio.h>
#include <sys/_timeval.h>

uint32_t NDL_GetTicks();
int main(){
    uint32_t oldTime, curTime;
    oldTime = NDL_GetTicks();
    while(1){
        curTime = NDL_GetTicks();
        if(curTime - oldTime >=500){
            printf("msec = %d. Time has gone 5 seconds.\n", oldTime);
            oldTime = curTime;
        }
    }

    return 0;
}
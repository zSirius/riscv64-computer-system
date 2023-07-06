#include <unistd.h>
#include <stdio.h>
#include <sys/_timeval.h>

int main(){
    struct timeval oldTime, curTime;
    _gettimeofday(&oldTime, NULL);
    while(1){
        _gettimeofday(&curTime, NULL);
        if(curTime.tv_sec - oldTime.tv_sec >=5){
            printf("Time has gone 5 seconds.\n");
            oldTime.tv_sec = curTime.tv_sec;
            oldTime.tv_usec = curTime.tv_usec;
        }
    }

    return 0;
}
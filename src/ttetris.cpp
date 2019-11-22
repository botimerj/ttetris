#include <term.h>

int main(int argc, char* argv[]){
    Term rterm;
    while(rterm.is_running()){
        rterm.render();
        //usleep(16666);
        usleep(33333);
    }
    return 0;
}

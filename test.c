//#define wait(a) a*2;
int stat = 5;
//int pid = wait(stat);
int pid = 5;
int main(){
    if ((pid > 0) && (wait(&stat) >= -1)){

    }
}

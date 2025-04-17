//
// Created by Guy Friedman on 16/04/2025.
//

#include "Commands.h"
#include "signals.h"
#include <stdlib.h>
#include <stdio.h>

#define TOTAL_NUMBER_OF_TESTS (10)
#define DEBUG_VAR(x) std::cout << "[DEBUG] " << #x << " (type: " << typeid(x).name() << ") = " << x << std::endl;


void test_activator(int test);
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();


int main(int argc, char *argv[]) {
    int test = -1;

    bool continuus = !true;
    bool run_all_tests = true;

    if ((!run_all_tests)&&(test == -1)) {
        printf("select test:\n");
        scanf("%d", &test);
    }

    if ((!run_all_tests)&&(!continuus)) {
        test_activator(test);
    } else if (((!run_all_tests)&&(continuus))){
        while (test != -1) {
            test_activator(test);
            printf("select another test, or select -1 to quit: \n"); //cout << "select another test, or select -1 to quit: " << endl;
            scanf("%d", &test); //cin >> test;
        }
    }
    if (run_all_tests) {
        for (int i = 0; i < TOTAL_NUMBER_OF_TESTS + 1; i++) {
            test_activator(i);
        }
    }


}





void test1(){
    //test to check - how to run getcwd
}

void test2(){
    //test to check -
}

void test3(){
    //test to check -
}

void test4(){
    //test to check -
}

void test5(){
    //test to check -
}

void test6(){
    //test to check -
}

void test7(){
    //test to check -
}

void test8(){
    //test to check -
}

void test9(){
    //test to check -
}

void test10(){
    //test to check -
}

void test_activator(int test) {
    printf("test number: %d\n", test); //cout << "test number:" << test << endl;
    switch (test) {
        case 1:
            test1();
        break;
        case 2:
            test2();
        break;
        case 3:
            test3();
        break;
        case 4:
            test4();
        break;
        case 5:
            test5();
        break;
        case 6:
            test6();
        break;
        case 7:
            test7();
        break;
        case 8:
            test8();
        break;
        case 9:
            test9();
        break;
        case 10:
            test10();
        break;
        default:
        printf("Invalid test number!\n"); //std::cout << "Invalid test number!" << std::endl;
        break;
    }
    printf("finished test %d blyat.\n", test); //cout << "finished test " << test << " blyat." << endl;
}

/*

void test1(){
    //test to check -
}

void test2(){
    //test to check -
}

void test3(){
    //test to check -
}

void test4(){
    //test to check -
}

void test5(){
    //test to check -
}

void test6(){
    //test to check -
}

void test7(){
    //test to check -
}

void test8(){
    //test to check -
}

void test9(){
    //test to check -
}

void test10(){
    //test to check -
}

*/


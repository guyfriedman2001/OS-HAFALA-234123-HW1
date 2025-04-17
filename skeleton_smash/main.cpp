//
// Created by Guy Friedman on 16/04/2025.
//

#include "Commands.h"
#include "signals.h"
#include <stdlib.h>
#include <stdio.h>

#define TOTAL_NUMBER_OF_TESTS (10)

// Define debug_mode at compile time, you can modify this to enable/disable debugging
#define debug_mode true

// Use the preprocessor to conditionally define DEBUG based on debug_mode
#if debug_mode
    #define DEBUG  // Enable debug features
#else
    #undef DEBUG  // Disable debug features
#endif

// Debug macro for printing variable information
#ifdef DEBUG
    #include <iostream>
    #include <typeinfo>
    #define DEBUG_VAR(x) std::cout << "\033[1;33m[DEBUG]\033[0m " << #x << " (type: " << typeid(x).name() << ") = " << x << std::endl;
#else
    #define DEBUG_VAR(x) // Empty: does nothing in non-debug mode
#endif

// Debug macros for timing execution
#ifdef DEBUG
    #include <chrono>
    #define DEBUG_TIME_START(name) auto start_##name = std::chrono::high_resolution_clock::now();
    #define DEBUG_TIME_END(name) { \
        auto end_##name = std::chrono::high_resolution_clock::now(); \
        auto duration_##name = std::chrono::duration_cast<std::chrono::milliseconds>(end_##name - start_##name).count(); \
        std::cout << "[DEBUG] Time for " << #name << ": " << duration_##name << "ms" << std::endl; \
    }
#else
    #define DEBUG_TIME_START(name) 
    #define DEBUG_TIME_END(name) 
#endif

#if 0
#define DEBUG_VAR(x) std::cout << "[DEBUG] " << #x << " (type: " << typeid(x).name() << ") = " << x << std::endl;
#endif

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


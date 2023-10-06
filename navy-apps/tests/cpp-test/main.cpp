#include <stdio.h>

class Test {
public:
    Test()  { printf("%s,%d: Hello, Project-N!\n", __func__, __LINE__); }
    ~Test() { printf("%s,%d: Goodbye, Project-N!\n", __func__, __LINE__); }
};

Test test;

int main(int argc,char *argv[],char *envp[]) {
    printf("%s,%d: Hello world!\n", __func__, __LINE__);
    // while (1){
        for (int i = 0; i < argc; i++){
            printf("arg %d is %s\n", i, argv[i]);
        }
        for (int i = 0; envp[i] != NULL;i++){
            printf("env %d is %s\n", i, envp[i]);
        }
    // }
    return 0;
}



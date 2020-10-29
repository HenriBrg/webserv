#define _POSIX_SOURCE
#include <unistd.h>
#undef _POSIX_SOURCE
#include <stdio.h>

#include <string>
#include <iostream>


std::string get() {

    char cwd[256];
    std::string tmp;

    if (getcwd(cwd, sizeof(cwd)) == NULL)
      perror("getcwd() error");
    else printf("current working directory is: %s\n", cwd);

    tmp = std::string(cwd);
    return tmp;
}

int main() {

    std::cout << get() << std::endl;
   
}
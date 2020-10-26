# include <string>
# include <iostream>

// clang++ test/tmp.cpp && ./a.out

int main() {
   
    // Whatever

    int i = 1;

    switch (i)
    {
    case 2:
        std::cout << "BBB" << std::endl;
        break ;
    case 1:
        std::cout << "AAA" << std::endl;
        std::cout << std::to_string(i) << std::endl;
    case 3:
        std::cout << "CCC" << std::endl;
        break ;
    default:
        std::cout << "DEFAULT" << std::endl;
        break;
    }

}
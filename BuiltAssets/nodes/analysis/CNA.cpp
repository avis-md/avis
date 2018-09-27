/// Common Neighbor Analysis
///  Result mapping;
///   0 = Other
///   1 = FCC
///   2 = HCP
///   3 = BCC
///   4 = ICO

#include <iostream>

//in enum Conventional Adaptive
int type = 0;
//in range 0.1 0.5
float radius = 0;

//entry
void Do () {
    std::string opt[] = { "Conventional", "Adaptive" };
    std::cout << "The option selected is \"" << opt[type] << "\"" << std::endl;
}
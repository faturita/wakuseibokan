/**
  g++ src/boilerplate.cpp -o boilerplate -std=c++14




  **/


#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

class Widget
{
private:

    int number;

public:
    Widget()
    {
        number = 3;
    }
    Widget(Widget&& rhs)            // Move constructor
    {

    }
    Widget& oerator=(Widget&& rhs)
    {

    }
    Widget(std::initializer_list<int> m)
    {
        for (const auto& p:m)
        {
            number = p;
        }
    }

    int getNumber()
    {
        return number;
    }

    decltype (auto)  giveMeSomething()
    {
        int a = rand() % 2;
        if (a % 2==0)
            //return 3;  // This does not compile
            return std::string("chau");
        else
            return std::string("Hola");
    }

    bool isWhatYouThink(bool value)
    {
        return false;
    }

    bool isWhatYouThink(int value)
    {
        return true;
    }

    bool isWhatYouThink(char value) = delete;
};

int main(int argc, char *argv[])
{

    std::unordered_map<std::string, int> m;

    for (const auto& p:m)
    {
        std::cout << "p" << std::endl;
    }

    int x=0;
    int y(0);
    int xz = { 0 };

    Widget w1;
    Widget w2 = w1;
    w1=w2;
    Widget w9{3,3,2};
    //Widget w9(3,3,2)   // This does not compile

    using MyWidget = Widget;

    MyWidget mw;

    std::cout << "Number:" << w9.getNumber() << std::endl;

    auto val = w9.giveMeSomething();

    std::cout << "Value:" << val << std::endl;

    std::vector<int> v{1,2,3,4};

    auto k = v.size();

    std::cout << "Size:" << k << std::endl;

    for (const auto k:v)
    {
        std::cout << "Value:" << k << std::endl;
    }

    // Unscoped enums
    enum Color { red=1,gren,blue} ;

    enum class RealColors { Magenta=1, Cyan, Violet};

    RealColors col = RealColors::Magenta;

    if (col == RealColors::Magenta)
        std::cout << "Magenta selected." << std::endl;

    std::cout << "Color:" << static_cast<int>(col) << std::endl;


    using UserInfo = std::tuple<std::string, std::string, std::string> ;
    enum Fields { Name, Email, Address};

    UserInfo uf;

    std::get<Name>(uf) = std::string("Ronald");

    std::cout << "Field Value:" << std::get<Name>(uf) << std::endl;


    if (w9.isWhatYouThink(9))
        std::cout << "Yes" << std::endl;
    if (w9.isWhatYouThink(true))
        std::cout << "Yes" << std::endl;
    //if (w9.isWhatYouThink('d')) // Does not compile
    //    std::cout << "Yes" << std::endl;





    return 1;
}

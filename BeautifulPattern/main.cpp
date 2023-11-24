#include <iostream>
#include <windows.h>

int main(){

    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD cP;

    int n = 0;
    int xLen = 0;
    int yLen = 0;
    int counter = 0;

    int a = 2;

    int temp = 0;

    std::cout << "n = ";
    std::cin >> n;
    
    xLen = (n*2)+1;
    yLen = (n*2)+1;

    temp = 0;

    system("cls");

for(int i = 0; i < (n*2)+1; i++){

    counter++;

    temp = counter;

    yLen++;

    for(int x = xLen; x > (counter-1); x--){
        cP.X = x;
        cP.Y = temp;
        SetConsoleCursorPosition(consoleHandle, cP);
        std::cout << ((n+a) % n)%10;
        temp++;
    }

    temp = counter;
    yLen -= 2;

    for(int x = xLen; x < (xLen*2)-(counter-1); x++){
        cP.X = x;
        cP.Y = temp;
        SetConsoleCursorPosition(consoleHandle, cP);
        std::cout <<  ((n+a) % n)%10;
        temp++;
    }

    temp = counter;

    yLen++;

    for(int y = yLen; y < (xLen*2)-(counter-1); y++){
        cP.X = temp;
        cP.Y = y;
        SetConsoleCursorPosition(consoleHandle, cP);
        std::cout << ((n+a) % n)%10;
        temp++;
    }

    temp = (xLen*2)-counter;
    


    for(int y = yLen; y < (xLen*2)-(counter-1); y++){
        cP.X = temp;
        cP.Y = y;
        SetConsoleCursorPosition(consoleHandle, cP);
        std::cout << ((n+a) % n)%10;
        temp--;
    }

    a++;

}
    

    while(1);
    
    system("cls.exe");

}

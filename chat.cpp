#include <iostream>
#include <string>
#include "chat.h"
using namespace std;

void chatSystem() {
    string message;
    cout << "Type your message: ";
    cin.ignore();
    getline(cin, message);
    cout << "You: " << message << endl;
}
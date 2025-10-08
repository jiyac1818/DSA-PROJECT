#include <iostream>
#include <string>
#include "auth.h"
using namespace std;

bool authenticateUser() {
    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    if (username == "admin" && password == "1234") {
        cout << "Login successful!\n";
        return true;
    } else {
        cout << "Invalid credentials.\n";
        return false;
    }
}
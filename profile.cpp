#include <iostream>
#include <string>
#include "profile.h"
using namespace std;

void manageProfile() {
    string name, bio;
    cout << "Enter your name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter your bio: ";
    getline(cin, bio);

    cout << "\nProfile Updated!\n";
    cout << "Name: " << name << "\nBio: " << bio << endl;
}
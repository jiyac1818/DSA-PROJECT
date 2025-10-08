#include <iostream>
#include <string>
#include "posts.h"
using namespace std;

void postSystem() {
    string post;
    cout << "Write your post: ";
    cin.ignore();
    getline(cin, post);
    cout << "Post published: " << post << endl;
}
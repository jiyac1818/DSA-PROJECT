#include <iostream>
#include "auth.h"
#include "profile.h"
#include "chat.h"
#include "posts.h"
#include "notifications.h"
#include "auth.cpp"
#include "profile.cpp"
#include "chat.cpp"
#include "posts.cpp"
#include "notifications.cpp"
using namespace std;

int main() {
    if (!authenticateUser()) {
        cout << "Authentication failed. Exiting program.\n";
        return 0;
    }

    int choice;
    do {
        cout << "\n===== CONNECTIFY MAIN MENU =====\n";
        cout << "1. Profile Management\n";
        cout << "2. Chat System\n";
        cout << "3. Posts\n";
        cout << "4. Notifications\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: manageProfile(); break;
            case 2: chatSystem(); break;
            case 3: postSystem(); break;
            case 4: showNotifications(); break;
            case 5: cout << "Exiting Connectify. Goodbye!\n"; break;
            default: cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 5);

    return 0;
}
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm> // for std::transform

struct Contact {
    std::string name;
    std::string phone;
    std::string email;
};

void loadContacts(const std::string& filename, std::vector<Contact>& contacts) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "No existing contacts found.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Contact c;
        if (std::getline(ss, c.name, ',') &&
            std::getline(ss, c.phone, ',') &&
            std::getline(ss, c.email)) {
            contacts.push_back(c);
        }
    }
}

void saveContacts(const std::string& filename, const std::vector<Contact>& contacts) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error saving contacts!\n";
        return;
    }

    for (const auto& c : contacts) {
        file << c.name << "," << c.phone << "," << c.email << "\n";
    }
}

void addContact(std::vector<Contact>& contacts) {
    Contact c;
    std::cout << "Enter name: ";
    std::getline(std::cin, c.name);
    std::cout << "Enter phone: ";
    std::getline(std::cin, c.phone);
    std::cout << "Enter email: ";
    std::getline(std::cin, c.email);

    contacts.push_back(c);
    std::cout << "Contact added.\n";
}

void listContacts(const std::vector<Contact>& contacts) {
    if (contacts.empty()) {
        std::cout << "No contacts to show.\n";
        return;
    }
    std::cout << "\nContacts:\n";
    for (size_t i = 0; i < contacts.size(); ++i) {
        std::cout << i + 1 << ". " << contacts[i].name
                  << ", Phone: " << contacts[i].phone
                  << ", Email: " << contacts[i].email << "\n";
    }
}

// Helper to convert string to lowercase (for case-insensitive search)
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

void searchContacts(const std::vector<Contact>& contacts) {
    if (contacts.empty()) {
        std::cout << "No contacts to search.\n";
        return;
    }

    std::cout << "Enter search term (name): ";
    std::string term;
    std::getline(std::cin, term);
    term = toLower(term);

    bool found = false;
    for (size_t i = 0; i < contacts.size(); ++i) {
        if (toLower(contacts[i].name).find(term) != std::string::npos) {
            std::cout << i + 1 << ". " << contacts[i].name
                      << ", Phone: " << contacts[i].phone
                      << ", Email: " << contacts[i].email << "\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No contacts matched the search.\n";
    }
}

void editContact(std::vector<Contact>& contacts) {
    if (contacts.empty()) {
        std::cout << "No contacts to edit.\n";
        return;
    }
    listContacts(contacts);
    std::cout << "Enter the number of the contact to edit: ";
    size_t index;
    std::cin >> index;
    std::cin.ignore();
    if (index == 0 || index > contacts.size()) {
        std::cout << "Invalid contact number.\n";
        return;
    }

    Contact& c = contacts[index - 1];
    std::cout << "Editing contact: " << c.name << "\n";

    std::cout << "Enter new name (or leave empty to keep \"" << c.name << "\"): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) c.name = input;

    std::cout << "Enter new phone (or leave empty to keep \"" << c.phone << "\"): ";
    std::getline(std::cin, input);
    if (!input.empty()) c.phone = input;

    std::cout << "Enter new email (or leave empty to keep \"" << c.email << "\"): ";
    std::getline(std::cin, input);
    if (!input.empty()) c.email = input;

    std::cout << "Contact updated.\n";
}

void deleteContact(std::vector<Contact>& contacts) {
    if (contacts.empty()) {
        std::cout << "No contacts to delete.\n";
        return;
    }
    listContacts(contacts);
    std::cout << "Enter the number of the contact to delete: ";
    size_t index;
    std::cin >> index;
    std::cin.ignore();
    if (index == 0 || index > contacts.size()) {
        std::cout << "Invalid contact number.\n";
        return;
    }

    contacts.erase(contacts.begin() + index - 1);
    std::cout << "Contact deleted.\n";
}

int main() {
    std::vector<Contact> contacts;
    const std::string filename = "contacts.txt";

    loadContacts(filename, contacts);

    char choice;
    do {
        std::cout << "\n--- Contact Book Menu ---\n";
        std::cout << "a) Add Contact\n";
        std::cout << "l) List Contacts\n";
        std::cout << "s) Search Contacts\n";
        std::cout << "e) Edit Contact\n";
        std::cout << "d) Delete Contact\n";
        std::cout << "q) Quit\n";
        std::cout << "Choose option: ";
        std::cin >> choice;
        std::cin.ignore();  // remove newline

        switch (choice) {
            case 'a':
            case 'A':
                addContact(contacts);
                saveContacts(filename, contacts);
                break;
            case 'l':
            case 'L':
                listContacts(contacts);
                break;
            case 's':
            case 'S':
                searchContacts(contacts);
                break;
            case 'e':
            case 'E':
                editContact(contacts);
                saveContacts(filename, contacts);
                break;
            case 'd':
            case 'D':
                deleteContact(contacts);
                saveContacts(filename, contacts);
                break;
            case 'q':
            case 'Q':
                std::cout << "Goodbye!\n";
                break;
            default:
                std::cout << "Invalid option.\n";
        }
    } while (choice != 'q' && choice != 'Q');

    return 0;
}

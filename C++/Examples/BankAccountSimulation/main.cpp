#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <limits>

class BankAccount {
private:
    std::string owner;
    double balance;
    double interestRate;
    std::vector<std::string> transactions;

public:
    BankAccount(const std::string& ownerName, double initialBalance = 0.0, double rate = 0.035)
        : owner(ownerName), balance(initialBalance), interestRate(rate) {}

    void deposit(double amount) {
        if (amount <= 0) {
            std::cout << "Deposit amount must be positive.\n";
            return;
        }
        balance += amount;
        transactions.push_back("Deposited $" + std::to_string(amount));
        std::cout << "Deposited $" << amount << ". New balance: $" << balance << "\n";
    }

    void withdraw(double amount) {
        if (amount <= 0) {
            std::cout << "Withdrawal amount must be positive.\n";
            return;
        }
        if (amount > balance) {
            std::cout << "Insufficient funds. Withdrawal cancelled.\n";
            return;
        }
        balance -= amount;
        transactions.push_back("Withdrew $" + std::to_string(amount));
        std::cout << "Withdrew $" << amount << ". New balance: $" << balance << "\n";
    }

    void checkBalance() const {
        std::cout << owner << "'s current balance: $" << balance << "\n";
    }

    void applyInterest() {
        double interest = balance * interestRate;
        balance += interest;
        transactions.push_back("Applied interest: $" + std::to_string(interest));
        std::cout << "Interest of $" << interest << " applied. New balance: $" << balance << "\n";
    }

    void showTransactionHistory() const {
        std::cout << "--- Transaction History for " << owner << " ---\n";
        if (transactions.empty()) {
            std::cout << "No transactions yet.\n";
            return;
        }
        for (const auto& t : transactions) {
            std::cout << t << "\n";
        }
    }
};

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void accountMenu(BankAccount& account) {
    int choice;
    do {
        std::cout << "\n--- Account Menu for " << account.checkBalance, account.showTransactionHistory, account.applyInterest, account.withdraw, account.deposit << " ---\n";
        std::cout << "1. Deposit\n2. Withdraw\n3. Check Balance\n4. Apply Interest\n5. Show Transaction History\n6. Exit to Main Menu\n";
        std::cout << "Choose an option: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInput();
            continue;
        }

        switch (choice) {
            case 1: {
                double amount;
                std::cout << "Enter amount to deposit: $";
                std::cin >> amount;
                if (std::cin.fail()) {
                    std::cout << "Invalid amount.\n";
                    clearInput();
                    break;
                }
                account.deposit(amount);
                break;
            }
            case 2: {
                double amount;
                std::cout << "Enter amount to withdraw: $";
                std::cin >> amount;
                if (std::cin.fail()) {
                    std::cout << "Invalid amount.\n";
                    clearInput();
                    break;
                }
                account.withdraw(amount);
                break;
            }
            case 3:
                account.checkBalance();
                break;
            case 4:
                account.applyInterest();
                break;
            case 5:
                account.showTransactionHistory();
                break;
            case 6:
                std::cout << "Returning to main menu.\n";
                break;
            default:
                std::cout << "Invalid option. Try again.\n";
        }
    } while (choice != 6);
}

int main() {
    std::map<std::string, BankAccount> accounts;

    int mainChoice;
    do {
        std::cout << "\n--- Main Menu ---\n";
        std::cout << "1. Create Account\n2. Select Account\n3. Exit\nChoose an option: ";
        std::cin >> mainChoice;

        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number.\n";
            clearInput();
            continue;
        }

        if (mainChoice == 1) {
            std::string newName;
            std::cout << "Enter new account owner name: ";
            clearInput();
            std::getline(std::cin, newName);

            if (accounts.find(newName) != accounts.end()) {
                std::cout << "Account already exists.\n";
            } else {
                accounts[newName] = BankAccount(newName);
                std::cout << "Account created for " << newName << ".\n";
            }
        } else if (mainChoice == 2) {
            std::string selName;
            std::cout << "Enter account owner name: ";
            clearInput();
            std::getline(std::cin, selName);

            auto it = accounts.find(selName);
            if (it == accounts.end()) {
                std::cout << "Account not found.\n";
            } else {
                accountMenu(it->second);
            }
        } else if (mainChoice == 3) {
            std::cout << "Exiting program. Goodbye!\n";
        } else {
            std::cout << "Invalid option. Try again.\n";
        }

    } while (mainChoice != 3);

    return 0;
}

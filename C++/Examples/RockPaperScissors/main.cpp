#include <iostream>
#include <cstdlib>
#include <ctime>

enum Choice { ROCK = 1, PAPER, SCISSORS };

Choice getComputerChoice() {
    return static_cast<Choice>(rand() % 3 + 1);
}

Choice getPlayerChoice() {
    int choice;
    while (true) {
        std::cout << "Choose (1) Rock, (2) Paper, (3) Scissors: ";
        std::cin >> choice;
        if (choice >= 1 && choice <= 3) {
            return static_cast<Choice>(choice);
        }
        std::cout << "Invalid choice. Try again.\n";
    }
}

void showChoice(Choice c) {
    switch (c) {
        case ROCK: std::cout << "Rock"; break;
        case PAPER: std::cout << "Paper"; break;
        case SCISSORS: std::cout << "Scissors"; break;
    }
}

int determineWinner(Choice player, Choice computer) {
    if (player == computer) return 0; // Tie
    if ((player == ROCK && computer == SCISSORS) ||
        (player == PAPER && computer == ROCK) ||
        (player == SCISSORS && computer == PAPER))
        return 1;  // Player wins
    return -1;     // Computer wins
}

int getBestOfRounds() {
    int n;
    while (true) {
        std::cout << "Enter best-of how many rounds (odd number like 3, 5, 7): ";
        std::cin >> n;
        if (n > 0 && n % 2 == 1) return n;
        std::cout << "Invalid input. Please enter a positive odd number.\n";
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    std::cout << "Welcome to Rock, Paper, Scissors!\n";

    int bestOf = getBestOfRounds();
    int roundsToWin = bestOf / 2 + 1;

    int playerScore = 0;
    int computerScore = 0;

    char playAgain = 'y';
    while (playAgain == 'y' || playAgain == 'Y') {
        playerScore = 0;
        computerScore = 0;
        int round = 1;

        while (playerScore < roundsToWin && computerScore < roundsToWin) {
            std::cout << "\nRound " << round++ << ":\n";

            Choice playerChoice = getPlayerChoice();
            Choice computerChoice = getComputerChoice();

            std::cout << "You chose: ";
            showChoice(playerChoice);
            std::cout << "\nComputer chose: ";
            showChoice(computerChoice);
            std::cout << "\n";

            int result = determineWinner(playerChoice, computerChoice);

            if (result == 1) {
                std::cout << "You win this round!\n";
                ++playerScore;
            } else if (result == -1) {
                std::cout << "Computer wins this round.\n";
                ++computerScore;
            } else {
                std::cout << "It's a tie!\n";
            }

            std::cout << "Score - You: " << playerScore << " | Computer: " << computerScore << "\n";
        }

        if (playerScore > computerScore) {
            std::cout << "\nYou won the best-of-" << bestOf << " series! Congratulations!\n";
        } else {
            std::cout << "\nComputer won the best-of-" << bestOf << " series. Better luck next time!\n";
        }

        std::cout << "Play again? (y/n): ";
        std::cin >> playAgain;
        std::cout << "\n";

        if (playAgain == 'y' || playAgain == 'Y') {
            bestOf = getBestOfRounds();
            roundsToWin = bestOf / 2 + 1;
        }
    }

    std::cout << "Thanks for playing!\n";
    return 0;
}

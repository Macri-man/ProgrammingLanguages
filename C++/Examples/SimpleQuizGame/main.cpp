#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // for shuffle
#include <random>
#include <ctime>

struct Question {
    std::string text;
    std::vector<std::string> choices;  // can be 2 for T/F or 4 for multiple-choice
    char correctAnswer;
    bool isTrueFalse = false;
};

// Load questions from file with simple marker "TF" line before T/F questions
bool loadQuestions(const std::string& filename, std::vector<Question>& questions) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open " << filename << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        Question q;

        // Check for True/False marker
        if (line == "TF") {
            q.isTrueFalse = true;
            if (!std::getline(file, q.text)) {
                std::cerr << "Error reading T/F question text\n";
                return false;
            }
            // Expect 2 choices for True/False
            q.choices.resize(2);
            for (int i = 0; i < 2; ++i) {
                if (!std::getline(file, q.choices[i])) {
                    std::cerr << "Error reading T/F choices\n";
                    return false;
                }
            }
        } else {
            // Normal multiple-choice question
            q.text = line;
            q.choices.resize(4);
            for (int i = 0; i < 4; ++i) {
                if (!std::getline(file, q.choices[i])) {
                    std::cerr << "Error reading choices for question.\n";
                    return false;
                }
            }
        }

        if (!std::getline(file, line) || line.empty()) {
            std::cerr << "Error reading correct answer.\n";
            return false;
        }

        q.correctAnswer = toupper(line[0]);

        questions.push_back(q);

        // Skip empty line (optional)
        std::getline(file, line);
    }

    return true;
}

void runQuiz(std::vector<Question>& questions) {
    // Randomize question order
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(questions.begin(), questions.end(), g);

    int score = 0;
    for (size_t i = 0; i < questions.size(); ++i) {
        Question& q = questions[i];
        std::cout << "\nQ" << i + 1 << ": " << q.text << "\n";

        // Shuffle choices if multiple choice
        std::vector<std::string> shuffledChoices = q.choices;
        std::vector<char> labels = {'A', 'B', 'C', 'D'};
        if (q.isTrueFalse) labels = {'A', 'B'};

        // For multiple choice, shuffle and adjust correct answer
        if (!q.isTrueFalse) {
            // Save original correct answer text
            std::string correctText;
            for (const auto& choice : q.choices) {
                if (!choice.empty() && choice[0] == q.correctAnswer) {
                    correctText = choice;
                    break;
                }
            }

            std::shuffle(shuffledChoices.begin(), shuffledChoices.end(), g);

            // Find new correctAnswer label after shuffle
            for (int idx = 0; idx < (int)shuffledChoices.size(); ++idx) {
                if (shuffledChoices[idx] == correctText) {
                    q.correctAnswer = labels[idx];
                    break;
                }
            }
        }

        // Display choices
        for (size_t j = 0; j < shuffledChoices.size(); ++j) {
            std::cout << labels[j] << ") " << shuffledChoices[j].substr(3) << "\n"; // substr(3) to remove "A) " prefix
        }

        std::cout << "Your answer (" << (q.isTrueFalse ? "A/B" : "A/B/C/D") << "): ";
        char answer;
        std::cin >> answer;
        answer = toupper(answer);

        if (answer == q.correctAnswer) {
            std::cout << "Correct!\n";
            ++score;
        } else {
            std::cout << "Wrong! Correct answer was " << q.correctAnswer << ".\n";
        }
    }

    std::cout << "\n--- Quiz Over ---\n";
    std::cout << "You scored " << score << " out of " << questions.size() << "!\n";

    // High score tracking
    const std::string highscoreFile = "highscore.txt";
    int highscore = 0;

    std::ifstream infile(highscoreFile);
    if (infile >> highscore) {
        if (score > highscore) {
            std::cout << "New high score! Previous was " << highscore << "\n";
            std::ofstream outfile(highscoreFile);
            outfile << score;
        } else {
            std::cout << "High score remains " << highscore << "\n";
        }
    } else {
        // No high score yet
        std::ofstream outfile(highscoreFile);
        outfile << score;
        std::cout << "Set the first high score!\n";
    }
}

int main() {
    std::vector<Question> questions;
    if (!loadQuestions("quiz.txt", questions)) {
        return 1;
    }

    runQuiz(questions);

    return 0;
}

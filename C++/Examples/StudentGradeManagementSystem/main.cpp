#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cmath>

struct Student {
    std::string name;
    std::vector<double> grades;

    double average() const {
        if (grades.empty()) return 0.0;
        double sum = 0;
        for (double g : grades) sum += g;
        return sum / grades.size();
    }
};

double median(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    std::vector<double> sorted = v;
    std::sort(sorted.begin(), sorted.end());
    size_t n = sorted.size();
    if (n % 2 == 1) return sorted[n / 2];
    else return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
}

double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    double avg = 0;
    for (double x : v) avg += x;
    avg /= v.size();

    double variance = 0;
    for (double x : v) variance += (x - avg) * (x - avg);
    variance /= (v.size() - 1);

    return std::sqrt(variance);
}

int getInt(const std::string& prompt, int min = 1, int max = 100) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cout << "Invalid input. Please enter a number between " << min << " and " << max << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

double getGrade(const std::string& prompt, double min = 0.0, double max = 100.0) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cout << "Invalid grade. Enter a number between " << min << " and " << max << ".\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

void inputStudents(std::vector<Student>& students) {
    int n = getInt("How many students to enter? ", 1, 1000);

    for (int i = 0; i < n; ++i) {
        Student s;
        std::cout << "\nEnter name for student #" << (i + 1) << ": ";
        std::getline(std::cin, s.name);

        int numGrades = getInt("How many grades for " + s.name + "? ", 1, 100);

        s.grades.clear();
        for (int j = 0; j < numGrades; ++j) {
            double grade = getGrade("Enter grade #" + std::to_string(j + 1) + ": ");
            s.grades.push_back(grade);
        }

        students.push_back(s);
    }

    // Sort descending by average after input
    std::sort(students.begin(), students.end(), [](const Student& a, const Student& b) {
        return a.average() > b.average();
    });
}

void printReport(const std::vector<Student>& students) {
    if (students.empty()) {
        std::cout << "No student data to display.\n";
        return;
    }

    double highestAvg = -1;
    double lowestAvg = 101;
    std::string topStudent, bottomStudent;

    std::cout << "\nStudent Grade Report:\n";
    std::cout << std::fixed << std::setprecision(2);

    for (const auto& s : students) {
        double avg = s.average();
        double med = median(s.grades);
        double sd = stddev(s.grades);
        std::cout << "Name: " << s.name
                  << ", Average: " << avg
                  << ", Median: " << med
                  << ", Std Dev: " << sd << "\n";

        if (avg > highestAvg) {
            highestAvg = avg;
            topStudent = s.name;
        }
        if (avg < lowestAvg) {
            lowestAvg = avg;
            bottomStudent = s.name;
        }
    }

    std::cout << "\nHighest Average: " << highestAvg << " by " << topStudent << "\n";
    std::cout << "Lowest Average: " << lowestAvg << " by " << bottomStudent << "\n";
}

bool saveToFile(const std::string& filename, const std::vector<Student>& students) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file to write.\n";
        return false;
    }

    for (const auto& s : students) {
        file << s.name;
        for (double grade : s.grades) {
            file << "," << grade;
        }
        file << "\n";
    }

    return true;
}

bool loadFromFile(const std::string& filename, std::vector<Student>& students) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file to read.\n";
        return false;
    }

    students.clear();

    std::string line;
    while (std::getline(file, line)) {
        Student s;
        size_t pos = 0;
        size_t commaPos = line.find(',');

        if (commaPos == std::string::npos) continue;

        s.name = line.substr(0, commaPos);

        pos = commaPos + 1;

        while ((commaPos = line.find(',', pos)) != std::string::npos) {
            double grade = std::stod(line.substr(pos, commaPos - pos));
            s.grades.push_back(grade);
            pos = commaPos + 1;
        }
        // Last grade
        if (pos < line.size()) {
            double grade = std::stod(line.substr(pos));
            s.grades.push_back(grade);
        }

        students.push_back(s);
    }

    // Sort descending by average after loading
    std::sort(students.begin(), students.end(), [](const Student& a, const Student& b) {
        return a.average() > b.average();
    });

    return true;
}

void showMenu() {
    std::cout << "\nMenu:\n";
    std::cout << "1. Input students\n";
    std::cout << "2. Print report\n";
    std::cout << "3. Save to file\n";
    std::cout << "4. Load from file\n";
    std::cout << "5. Exit\n";
    std::cout << "Enter choice: ";
}

int main() {
    std::vector<Student> students;

    bool running = true;

    while (running) {
        showMenu();
        int option;
        std::cin >> option;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (option) {
            case 1:
                inputStudents(students);
                break;
            case 2:
                printReport(students);
                break;
            case 3:
                if (saveToFile("students.csv", students)) {
                    std::cout << "Data saved successfully to students.csv\n";
                }
                break;
            case 4:
                if (loadFromFile("students.csv", students)) {
                    std::cout << "Data loaded successfully from students.csv\n";
                }
                break;
            case 5:
                running = false;
                break;
            default:
                std::cout << "Invalid option. Try again.\n";
                break;
        }
    }

    std::cout << "Goodbye!\n";
    return 0;
}

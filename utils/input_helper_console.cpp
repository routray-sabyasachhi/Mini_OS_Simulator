#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace ui {

static void clearLine() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string color(const std::string& text, const std::string& code) {
    if (code.empty()) return text;
    return "\x1b[" + code + "m" + text + "\x1b[0m";
}

void clearScreen() {
    std::cout << "\x1b[2J\x1b[H";
}

void printTitle(const std::string& title) {
    clearScreen();
    std::string line(title.size() + 8, '=');
    std::cout << line << "\n";
    std::cout << "   " << title << "\n";
    std::cout << line << "\n";
}

void printSection(const std::string& title) {
    std::cout << "[ " << title << " ]\n";
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            clearLine();
            continue;
        }

        std::stringstream ss(line);
        int value = 0;
        if (!(ss >> value) || (ss >> std::ws && !ss.eof())) {
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        if (value < minValue || value > maxValue) {
            std::cout << "Value must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        return value;
    }
}

std::vector<int> readIntList(const std::string& prompt, int count, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            clearLine();
            continue;
        }

        std::stringstream ss(line);
        std::vector<int> values;
        int x = 0;
        while (ss >> x) {
            if (x < minValue || x > maxValue) {
                values.clear();
                break;
            }
            values.push_back(x);
        }
        if (static_cast<int>(values.size()) == count) {
            return values;
        }
        std::cout << "Please enter exactly " << count << " valid values.\n";
    }
}

void waitForEnter() {
    std::cout << "Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

void printSeparator(int width) {
    std::cout << std::string(width, '-') << "\n";
}

} // namespace ui

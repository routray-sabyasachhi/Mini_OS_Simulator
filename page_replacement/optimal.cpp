#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

static void printFrames(const std::vector<int>& frames) {
    std::cout << "Frames: ";
    for (int f : frames) {
        if (f == -1) std::cout << "- ";
        else std::cout << f << " ";
    }
    std::cout << "\n";
}

void runOptimalPageReplacement() {
    ui::printSection("Page Replacement - Optimal");
    int framesCount = ui::readInt("Enter number of frames (1-10): ", 1, 10);
    int n = ui::readInt("Enter reference string length (1-20): ", 1, 20);

    std::vector<int> refs(n);
    for (int i = 0; i < n; ++i) {
        refs[i] = ui::readInt("Reference " + std::to_string(i + 1) + " (0-9): ", 0, 9);
    }

    std::vector<int> frames(framesCount, -1);
    int faults = 0;

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        // HIT means page already loaded in any frame.
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        std::cout << "\nStep " << (i + 1) << ": Page " << page << (hit ? " (HIT)" : " (FAULT)") << "\n";

        if (!hit) {
            faults++;
            int empty = -1;
            for (int f = 0; f < framesCount; ++f) {
                if (frames[f] == -1) { empty = f; break; }
            }
            if (empty != -1) {
                frames[empty] = page;
            } else {
                // Optimal replacement evicts the page used farthest in the future.
                int victim = -1;
                int farthest = -1;
                for (int f = 0; f < framesCount; ++f) {
                    int nextUse = 1e9;
                    for (int j = i + 1; j < n; ++j) {
                        if (refs[j] == frames[f]) { nextUse = j; break; }
                    }
                    if (nextUse > farthest) {
                        farthest = nextUse;
                        victim = f;
                    }
                }
                frames[victim] = page;
            }
        }
        printFrames(frames);
    }

    std::cout << "\nTotal Page Faults: " << faults << "\n";
}

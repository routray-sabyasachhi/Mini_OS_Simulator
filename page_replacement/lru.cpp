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

void runLRUPageReplacement() {
    ui::printSection("Page Replacement - LRU");
    int framesCount = ui::readInt("Enter number of frames (1-10): ", 1, 10);
    int n = ui::readInt("Enter reference string length (1-20): ", 1, 20);

    std::vector<int> refs(n);
    for (int i = 0; i < n; ++i) {
        refs[i] = ui::readInt("Reference " + std::to_string(i + 1) + " (0-9): ", 0, 9);
    }

    std::vector<int> frames(framesCount, -1);
    std::vector<int> lastUsed(framesCount, -1); // Stores last access time per frame.
    int faults = 0;

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        bool hit = false;
        for (int f = 0; f < framesCount; ++f) {
            if (frames[f] == page) {
                // Update last used time on hit.
                lastUsed[f] = i;
                hit = true;
                break;
            }
        }

        std::cout << "\nStep " << (i + 1) << ": Page " << page << (hit ? " (HIT)" : " (FAULT)") << "\n";
        if (!hit) {
            faults++;
            int empty = -1;
            for (int f = 0; f < framesCount; ++f) {
                if (frames[f] == -1) { empty = f; break; }
            }
            if (empty != -1) {
                frames[empty] = page;
                lastUsed[empty] = i;
            } else {
                // Evict the least recently used frame.
                int lruIndex = 0;
                for (int f = 1; f < framesCount; ++f) {
                    if (lastUsed[f] < lastUsed[lruIndex]) lruIndex = f;
                }
                frames[lruIndex] = page;
                lastUsed[lruIndex] = i;
            }
        }
        printFrames(frames);
    }

    std::cout << "\nTotal Page Faults: " << faults << "\n";
}

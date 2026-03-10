#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <queue>
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

void runFIFOPageReplacement() {
    ui::printSection("Page Replacement - FIFO");
    int framesCount = ui::readInt("Enter number of frames (1-10): ", 1, 10);
    int n = ui::readInt("Enter reference string length (1-20): ", 1, 20);

    std::vector<int> refs(n);
    for (int i = 0; i < n; ++i) {
        refs[i] = ui::readInt("Reference " + std::to_string(i + 1) + " (0-9): ", 0, 9);
    }

    std::vector<int> frames(framesCount, -1);
    std::queue<int> q; // Stores frame indices in arrival order for FIFO.
    int faults = 0;

    for (int i = 0; i < n; ++i) {
        int page = refs[i];
        // HIT means page already loaded in any frame.
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        std::cout << "\nStep " << (i + 1) << ": Page " << page << (hit ? " (HIT)" : " (FAULT)") << "\n";

        if (!hit) {
            faults++;
            if (static_cast<int>(q.size()) < framesCount) {
                // Fill empty frame first.
                for (int f = 0; f < framesCount; ++f) {
                    if (frames[f] == -1) {
                        frames[f] = page;
                        q.push(f);
                        break;
                    }
                }
            } else {
                // Evict the oldest loaded frame.
                int idx = q.front();
                q.pop();
                frames[idx] = page;
                q.push(idx);
            }
        }
        printFrames(frames);
    }

    std::cout << "\nTotal Page Faults: " << faults << "\n";
}

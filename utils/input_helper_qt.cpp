#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QString>
#include <QWidget>

namespace ui {

static QPlainTextEdit* g_output = nullptr;
static QWidget* g_parent = nullptr;

void setOutputWidget(QPlainTextEdit* output) {
    g_output = output;
}

void setParentWidget(QWidget* parent) {
    g_parent = parent;
}

static void appendLine(const QString& text) {
    if (!g_output) return;
    g_output->appendPlainText(text);
}

std::string color(const std::string& text, const std::string&) {
    return text;
}

void clearScreen() {
    if (g_output) g_output->clear();
}

void printTitle(const std::string& title) {
    clearScreen();
    std::string line(title.size() + 8, '=');
    appendLine(QString::fromStdString(line));
    appendLine(QString::fromStdString("   " + title));
    appendLine(QString::fromStdString(line));
}

void printSection(const std::string& title) {
    appendLine(QString::fromStdString("[ " + title + " ]"));
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        bool ok = false;
        int value = QInputDialog::getInt(
            g_parent,
            "Input",
            QString::fromStdString(prompt),
            minValue,
            minValue,
            maxValue,
            1,
            &ok
        );
        if (ok) return value;
        QMessageBox::information(g_parent, "Input Required", "Please provide a value to continue.");
    }
}

std::vector<int> readIntList(const std::string& prompt, int count, int minValue, int maxValue) {
    while (true) {
        bool ok = false;
        QString text = QInputDialog::getText(
            g_parent,
            "Input",
            QString::fromStdString(prompt),
            QLineEdit::Normal,
            "",
            &ok
        );
        if (!ok) {
            QMessageBox::information(g_parent, "Input Required", "Please provide values to continue.");
            continue;
        }

        std::stringstream ss(text.toStdString());
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
        QMessageBox::warning(g_parent, "Invalid Input",
                             "Please enter exactly " + QString::number(count) + " valid values.");
    }
}

void waitForEnter() {
    QMessageBox::information(g_parent, "Continue", "Press OK to continue.");
}

void printSeparator(int width) {
    appendLine(QString(width, '-'));
}

} // namespace ui

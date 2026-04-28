#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>

#include "cpu_scheduling/cpu_utils.h"
#include "cpu_scheduling/schedulers_gui.h"
#include "disk_scheduling/disk_algorithms.h"
#include "memory_management/memory_algorithms.h"
#include "page_replacement/page_algorithms.h"

static QColor colorForPid(int pid) {
    static const std::vector<QColor> palette = {
        QColor("#4E79A7"), QColor("#F28E2B"), QColor("#E15759"),
        QColor("#76B7B2"), QColor("#59A14F"), QColor("#EDC948"),
        QColor("#B07AA1"), QColor("#FF9DA7"), QColor("#9C755F"),
        QColor("#BAB0AC")
    };
    if (pid <= 0) return QColor("#D0D0D0");
    return palette[(pid - 1) % static_cast<int>(palette.size())];
}

class GanttWidget : public QWidget {
public:
    void setData(const std::vector<GanttEntry>& entries, int currentTime = -1) {
        entries_ = entries;
        currentTime_ = currentTime;
        update();
    }

    void setDarkMode(bool enabled) {
        darkMode_ = enabled;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        const QColor chartBackground = darkMode_ ? QColor("#cfd5dd") : QColor("#eef2f6");
        const QColor chartText = QColor("#222222");
        const QColor chartBorder = darkMode_ ? QColor("#8f98a3") : QColor("#9aa4b2");

        p.fillRect(rect(), chartBackground);
        if (entries_.empty()) {
            p.setPen(QColor("#888888"));
            p.drawText(rect(), Qt::AlignCenter, "No Gantt data");
            return;
        }

        int minTime = entries_.front().start;
        int maxTime = entries_.front().end;
        for (const auto& e : entries_) {
            minTime = std::min(minTime, e.start);
            maxTime = std::max(maxTime, e.end);
        }
        int total = std::max(1, maxTime - minTime);
        int visibleTime = (currentTime_ < 0) ? maxTime : std::clamp(currentTime_, minTime, maxTime);

        const int h = height() - 58;
        const int y = 26;
        int x = 10;
        int w = width() - 20;

        p.setPen(QPen(chartBorder, 1));
        p.drawRect(x, y, w, h);

        for (const auto& e : entries_) {
            if (visibleTime <= e.start) continue;

            int sx = x + static_cast<int>(std::round((e.start - minTime) * 1.0 * w / total));
            int shownEnd = std::min(e.end, visibleTime);
            int ex = x + static_cast<int>(std::round((shownEnd - minTime) * 1.0 * w / total));
            int rw = std::max(1, ex - sx);
            QRect r(sx, y, rw, h);
            p.fillRect(r.adjusted(1, 1, -1, -1), colorForPid(e.pid));
            p.setPen(chartText);
            QString label = (e.pid == -1) ? "IDLE" : QString("P%1").arg(e.pid);
            if (rw > 36) p.drawText(r, Qt::AlignCenter, label);

            p.drawText(QRect(sx - 18, 0, 36, y - 4), Qt::AlignCenter, QString::number(e.start));
            if (shownEnd == e.end || shownEnd == visibleTime) {
                p.drawText(QRect(ex - 18, 0, 36, y - 4), Qt::AlignCenter, QString::number(shownEnd));
            }
        }

        p.setPen(chartText);
        p.drawText(x, y + h + 18, QString::number(minTime));
        p.drawText(x + w - 30, y + h + 18, QString::number(maxTime));
    }

private:
    std::vector<GanttEntry> entries_;
    int currentTime_ = -1;
    bool darkMode_ = false;
};

class BarChartWidget : public QWidget {
public:
    void setData(const std::vector<QString>& labels, const std::vector<double>& values) {
        labels_ = labels;
        values_ = values;
        update();
    }

    void setDarkMode(bool enabled) {
        darkMode_ = enabled;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        const QColor chartBackground = darkMode_ ? QColor("#cfd5dd") : QColor("#eef2f6");
        const QColor chartText = QColor("#222222");
        const QColor chartBorder = darkMode_ ? QColor("#8f98a3") : QColor("#9aa4b2");

        p.fillRect(rect(), chartBackground);
        if (labels_.empty() || values_.empty()) {
            p.setPen(QColor("#888888"));
            p.drawText(rect(), Qt::AlignCenter, "No data");
            return;
        }

        double maxVal = *std::max_element(values_.begin(), values_.end());
        if (maxVal <= 0.0) maxVal = 1.0;

        int margin = 30;
        int w = width() - margin * 2;
        int h = height() - margin * 2;
        int x0 = margin;
        int y0 = height() - margin;

        p.setPen(chartBorder);
        p.drawLine(x0, y0, x0 + w, y0);
        p.drawLine(x0, y0, x0, y0 - h);

        int count = static_cast<int>(values_.size());
        int barW = std::max(10, w / std::max(1, count) - 10);

        for (int i = 0; i < count; ++i) {
            int barH = static_cast<int>(std::round(values_[i] / maxVal * h));
            int bx = x0 + i * (barW + 10) + 5;
            int by = y0 - barH;
            QRect r(bx, by, barW, barH);
            p.fillRect(r, QColor("#4E79A7"));
            p.setPen(chartText);
            p.drawRect(r);
            p.drawText(QRect(bx, y0 + 2, barW + 20, 20), Qt::AlignLeft | Qt::AlignTop, labels_[i]);
        }
    }

private:
    std::vector<QString> labels_;
    std::vector<double> values_;
    bool darkMode_ = false;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Mini Operating System Simulator - Professional GUI");
        auto* central = new QWidget(this);
        centralWidget_ = central;
        auto* root = new QVBoxLayout(central);

        auto* header = new QGridLayout();
        header->setColumnStretch(0, 1);
        header->setColumnStretch(1, 2);
        header->setColumnStretch(2, 1);

        themeToggleButton_ = new QPushButton("Dark Mode", central);
        themeToggleButton_->setCheckable(true);
        themeToggleButton_->setCursor(Qt::PointingHandCursor);

        titleLabel_ = new QLabel("Mini Operating System Simulator");
        QFont tf = titleLabel_->font();
        tf.setPointSize(14);
        tf.setBold(true);
        titleLabel_->setFont(tf);
        titleLabel_->setAlignment(Qt::AlignCenter);

        appRunTimerLabel_ = new QLabel("OS Run Timer: 00:00:00");
        appRunTimerLabel_->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

        header->addWidget(themeToggleButton_, 0, 0, Qt::AlignLeft);
        header->addWidget(titleLabel_, 0, 1, Qt::AlignCenter);
        header->addWidget(appRunTimerLabel_, 0, 2, Qt::AlignRight);
        root->addLayout(header);

        tabs_ = new QTabWidget(central);
        tabs_->addTab(buildCpuTab(), "CPU Scheduling");
        tabs_->addTab(buildComparisonTab(), "Comparison Dashboard");
        tabs_->addTab(buildMemoryTab(), "Memory Management");
        tabs_->addTab(buildPageTab(), "Page Replacement");
        tabs_->addTab(buildDiskTab(), "Disk Scheduling");

        root->addWidget(tabs_, 1);
        setCentralWidget(central);
        resize(1200, 720);

        animationTimer_ = new QTimer(this);
        connect(animationTimer_, &QTimer::timeout, this, [this]() { advanceAnimation(); });

        appRunTimer_ = new QTimer(this);
        appRunTimer_->setInterval(1000);
        connect(appRunTimer_, &QTimer::timeout, this, [this]() {
            ++appRunSeconds_;
            updateAppRunTimerLabel();
        });
        updateAppRunTimerLabel();
        appRunTimer_->start();
        connect(themeToggleButton_, &QPushButton::toggled, this, [this](bool checked) {
            applyTheme(checked);
        });
        applyTheme(false);

        refreshProcessTable();
        applyDefaultModuleValues();
    }

private:
    QWidget* centralWidget_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QPushButton* themeToggleButton_ = nullptr;
    QTabWidget* tabs_ = nullptr;
    QTimer* animationTimer_ = nullptr;
    QTimer* appRunTimer_ = nullptr;
    std::function<void(int)> animationStepFn_;
    std::function<void()> animationDoneFn_;
    int animationIndex_ = 0;
    int animationCount_ = 0;
    int appRunSeconds_ = 0;
    QLabel* appRunTimerLabel_ = nullptr;

    QSpinBox* procCount_ = nullptr;
    QTableWidget* procTable_ = nullptr;
    QComboBox* algoCombo_ = nullptr;
    QSpinBox* quantumSpin_ = nullptr;
    QLabel* metricsLabel_ = nullptr;
    QLabel* cpuTimelineLabel_ = nullptr;
    QTableWidget* resultTable_ = nullptr;
    GanttWidget* gantt_ = nullptr;

    QCheckBox* cmpFcfs_ = nullptr;
    QCheckBox* cmpSjf_ = nullptr;
    QCheckBox* cmpSrtf_ = nullptr;
    QCheckBox* cmpPrio_ = nullptr;
    QCheckBox* cmpRr_ = nullptr;
    QSpinBox* cmpProcCount_ = nullptr;
    QTableWidget* cmpProcTable_ = nullptr;
    QSpinBox* cmpQuantum_ = nullptr;
    QLabel* cmpStatusLabel_ = nullptr;
    QTableWidget* cmpTable_ = nullptr;
    BarChartWidget* cmpChart_ = nullptr;

    QSpinBox* memBlocks_ = nullptr;
    QSpinBox* memProcs_ = nullptr;
    QComboBox* memMode_ = nullptr;
    QSpinBox* memMemSize_ = nullptr;
    QSpinBox* memPageSize_ = nullptr;
    QTableWidget* memBlocksTable_ = nullptr;
    QTableWidget* memProcsTable_ = nullptr;
    QComboBox* memAlgo_ = nullptr;
    QTableWidget* memResult_ = nullptr;
    QLabel* memTotals_ = nullptr;
    QLabel* memStatusLabel_ = nullptr;
    QPlainTextEdit* memDetails_ = nullptr;
    QLabel* memBlocksTitle_ = nullptr;
    QLabel* memBlocksRowLabel_ = nullptr;
    QLabel* memAlgoRowLabel_ = nullptr;
    QLabel* memMemSizeRowLabel_ = nullptr;
    QLabel* memPageSizeRowLabel_ = nullptr;

    QSpinBox* pageFrames_ = nullptr;
    QLineEdit* pageRefs_ = nullptr;
    QComboBox* pageAlgo_ = nullptr;
    QTableWidget* pageTable_ = nullptr;
    QLabel* pageStatusLabel_ = nullptr;
    QLabel* pageFaults_ = nullptr;

    QSpinBox* diskSize_ = nullptr;
    QSpinBox* diskHead_ = nullptr;
    QComboBox* diskAlgo_ = nullptr;
    QComboBox* diskDir_ = nullptr;
    QLineEdit* diskReqs_ = nullptr;
    QPlainTextEdit* diskOutput_ = nullptr;
    QLabel* diskStatusLabel_ = nullptr;
    QLabel* diskTotal_ = nullptr;

    QWidget* buildCpuTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QWidget(tab);
        auto* leftLayout = new QVBoxLayout(left);

        auto* inputBox = new QGroupBox("Process Input", left);
        auto* inputLayout = new QVBoxLayout(inputBox);

        auto* form = new QFormLayout();
        procCount_ = new QSpinBox(inputBox);
        procCount_->setRange(1, 10);
        procCount_->setValue(4);
        form->addRow("Number of processes", procCount_);

        algoCombo_ = new QComboBox(inputBox);
        algoCombo_->addItems({"FCFS", "SJF Non-Preemptive", "SJF Preemptive (SRTF)", "Priority", "Round Robin"});
        form->addRow("Algorithm", algoCombo_);

        quantumSpin_ = new QSpinBox(inputBox);
        quantumSpin_->setRange(1, 10);
        quantumSpin_->setValue(2);
        form->addRow("Quantum (RR)", quantumSpin_);
        inputLayout->addLayout(form);

        procTable_ = new QTableWidget(inputBox);
        procTable_->setColumnCount(4);
        procTable_->setHorizontalHeaderLabels({"PID", "Arrival", "Burst", "Priority"});
        procTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        inputLayout->addWidget(procTable_);

        auto* btnRow = new QHBoxLayout();
        auto* sampleBtn = new QPushButton("Load Sample", inputBox);
        auto* runBtn = new QPushButton("Run", inputBox);
        auto* clearBtn = new QPushButton("Clear", inputBox);
        btnRow->addWidget(sampleBtn);
        btnRow->addWidget(runBtn);
        btnRow->addWidget(clearBtn);
        inputLayout->addLayout(btnRow);

        leftLayout->addWidget(inputBox, 1);
        layout->addWidget(left, 1);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);

        metricsLabel_ = new QLabel("Averages: -", right);
        cpuTimelineLabel_ = new QLabel("CPU Timeline: waiting to start", right);
        resultTable_ = new QTableWidget(right);
        resultTable_->setColumnCount(6);
        resultTable_->setHorizontalHeaderLabels({"PID", "Completion", "Waiting", "Turnaround", "Response", "Priority"});
        resultTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        gantt_ = new GanttWidget();
        gantt_->setMinimumHeight(160);

        rightLayout->addWidget(metricsLabel_);
        rightLayout->addWidget(cpuTimelineLabel_);
        rightLayout->addWidget(resultTable_, 1);
        rightLayout->addWidget(new QLabel("Gantt Chart"));
        rightLayout->addWidget(gantt_);

        layout->addWidget(right, 1);

        connect(procCount_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
            refreshProcessTable();
        });
        connect(algoCombo_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
            quantumSpin_->setEnabled(text.contains("Round Robin"));
        });
        connect(sampleBtn, &QPushButton::clicked, this, [this]() { loadSampleProcesses(); });
        connect(runBtn, &QPushButton::clicked, this, [this]() { runCpuScheduling(); });
        connect(clearBtn, &QPushButton::clicked, this, [this]() { clearCpuResults(); });

        quantumSpin_->setEnabled(false);
        return tab;
    }

    QWidget* buildComparisonTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QGroupBox("Comparison Input", tab);
        auto* leftLayout = new QVBoxLayout(left);
        auto* form = new QFormLayout();
        cmpProcCount_ = new QSpinBox(left);
        cmpProcCount_->setRange(1, 10);
        cmpProcCount_->setValue(4);
        form->addRow("Processes", cmpProcCount_);
        leftLayout->addLayout(form);

        cmpProcTable_ = new QTableWidget(left);
        cmpProcTable_->setColumnCount(4);
        cmpProcTable_->setHorizontalHeaderLabels({"PID", "Arrival", "Burst", "Priority"});
        cmpProcTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        leftLayout->addWidget(cmpProcTable_, 1);

        auto* inputButtons = new QHBoxLayout();
        auto* cmpSampleBtn = new QPushButton("Load Sample", left);
        auto* cmpClearBtn = new QPushButton("Reset", left);
        inputButtons->addWidget(cmpSampleBtn);
        inputButtons->addWidget(cmpClearBtn);
        leftLayout->addLayout(inputButtons);

        auto* algoBox = new QGroupBox("Algorithms", left);
        auto* algoLayout = new QVBoxLayout(algoBox);
        cmpFcfs_ = new QCheckBox("FCFS", left);
        cmpSjf_ = new QCheckBox("SJF Non-Preemptive", left);
        cmpSrtf_ = new QCheckBox("SJF Preemptive (SRTF)", left);
        cmpPrio_ = new QCheckBox("Priority", left);
        cmpRr_ = new QCheckBox("Round Robin", left);
        cmpFcfs_->setChecked(true);
        cmpSjf_->setChecked(true);
        cmpSrtf_->setChecked(true);
        cmpPrio_->setChecked(true);
        cmpRr_->setChecked(true);

        algoLayout->addWidget(cmpFcfs_);
        algoLayout->addWidget(cmpSjf_);
        algoLayout->addWidget(cmpSrtf_);
        algoLayout->addWidget(cmpPrio_);
        algoLayout->addWidget(cmpRr_);

        cmpQuantum_ = new QSpinBox(left);
        cmpQuantum_->setRange(1, 10);
        cmpQuantum_->setValue(2);
        algoLayout->addWidget(new QLabel("Quantum (RR)"));
        algoLayout->addWidget(cmpQuantum_);
        auto* run = new QPushButton("Run Comparison", left);
        algoLayout->addWidget(run);
        leftLayout->addWidget(algoBox);
        leftLayout->addStretch(1);

        layout->addWidget(left, 0);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);
        cmpStatusLabel_ = new QLabel("Comparison Timeline: waiting to start", right);
        cmpTable_ = new QTableWidget(right);
        cmpTable_->setColumnCount(4);
        cmpTable_->setHorizontalHeaderLabels({"Algorithm", "Avg Waiting", "Avg Turnaround", "Avg Response"});
        cmpTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        cmpChart_ = new BarChartWidget();
        cmpChart_->setMinimumHeight(220);

        rightLayout->addWidget(cmpStatusLabel_);
        rightLayout->addWidget(cmpTable_, 1);
        rightLayout->addWidget(new QLabel("Average Waiting Time (lower is better)"));
        rightLayout->addWidget(cmpChart_, 0);
        layout->addWidget(right, 1);

        connect(cmpProcCount_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
            refreshComparisonTable();
        });
        connect(cmpSampleBtn, &QPushButton::clicked, this, [this]() { loadComparisonSampleProcesses(); });
        connect(cmpClearBtn, &QPushButton::clicked, this, [this]() { refreshComparisonTable(); });
        connect(run, &QPushButton::clicked, this, [this]() { runComparison(); });
        refreshComparisonTable();
        return tab;
    }

    QWidget* buildMemoryTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QGroupBox("Memory Input", tab);
        auto* leftLayout = new QVBoxLayout(left);
        auto* form = new QFormLayout();
        memMode_ = new QComboBox(left);
        memMode_->addItems({"Contiguous Allocation", "Non-Contiguous (Paging)"});
        memBlocks_ = new QSpinBox(left);
        memBlocks_->setRange(1, 20);
        memBlocks_->setValue(5);
        memMemSize_ = new QSpinBox(left);
        memMemSize_->setRange(1, 8192);
        memMemSize_->setValue(1024);
        memPageSize_ = new QSpinBox(left);
        memPageSize_->setRange(1, 1024);
        memPageSize_->setValue(64);
        memProcs_ = new QSpinBox(left);
        memProcs_->setRange(1, 20);
        memProcs_->setValue(5);
        memAlgo_ = new QComboBox(left);
        memAlgo_->addItems({"First Fit", "Best Fit", "Next Fit"});
        form->addRow("Mode", memMode_);
        form->addRow("Blocks", memBlocks_);
        form->addRow("Memory Size", memMemSize_);
        form->addRow("Page Size", memPageSize_);
        form->addRow("Processes", memProcs_);
        form->addRow("Algorithm", memAlgo_);
        leftLayout->addLayout(form);

        memBlocksRowLabel_ = qobject_cast<QLabel*>(form->labelForField(memBlocks_));
        memAlgoRowLabel_ = qobject_cast<QLabel*>(form->labelForField(memAlgo_));
        memMemSizeRowLabel_ = qobject_cast<QLabel*>(form->labelForField(memMemSize_));
        memPageSizeRowLabel_ = qobject_cast<QLabel*>(form->labelForField(memPageSize_));

        memBlocksTable_ = new QTableWidget(left);
        memBlocksTable_->setColumnCount(1);
        memBlocksTable_->setHorizontalHeaderLabels({"Block Size"});
        memBlocksTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        memProcsTable_ = new QTableWidget(left);
        memProcsTable_->setColumnCount(1);
        memProcsTable_->setHorizontalHeaderLabels({"Process Size"});
        memProcsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        memBlocksTitle_ = new QLabel("Blocks");
        leftLayout->addWidget(memBlocksTitle_);
        leftLayout->addWidget(memBlocksTable_, 1);
        leftLayout->addWidget(new QLabel("Processes"));
        leftLayout->addWidget(memProcsTable_, 1);

        auto* run = new QPushButton("Run", left);
        leftLayout->addWidget(run);
        layout->addWidget(left, 1);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);
        memResult_ = new QTableWidget(right);
        memResult_->setColumnCount(5);
        memResult_->setHorizontalHeaderLabels({"PID", "ProcSize", "Block", "BlockSize", "IntFrag"});
        memResult_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        memTotals_ = new QLabel("Totals: -", right);
        memStatusLabel_ = new QLabel("Memory Timeline: waiting to start", right);
        rightLayout->addWidget(memResult_, 1);
        rightLayout->addWidget(memStatusLabel_);
        rightLayout->addWidget(memTotals_);
        memDetails_ = new QPlainTextEdit(right);
        memDetails_->setReadOnly(true);
        memDetails_->setPlaceholderText("Details (paging mode)");
        rightLayout->addWidget(memDetails_, 0);
        layout->addWidget(right, 1);

        connect(memBlocks_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { refreshMemoryTables(); });
        connect(memProcs_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { refreshMemoryTables(); });
        connect(memMode_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { refreshMemoryUi(); });
        connect(memMemSize_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
            memPageSize_->setMaximum(std::max(1, size));
            refreshMemoryUi();
        });
        connect(memPageSize_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) { refreshMemoryUi(); });
        connect(run, &QPushButton::clicked, this, [this]() { runMemory(); });

        refreshMemoryUi();
        return tab;
    }

    QWidget* buildPageTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QGroupBox("Page Input", tab);
        auto* leftLayout = new QVBoxLayout(left);
        auto* form = new QFormLayout();
        pageFrames_ = new QSpinBox(left);
        pageFrames_->setRange(1, 10);
        pageFrames_->setValue(3);
        pageAlgo_ = new QComboBox(left);
        pageAlgo_->addItems({"FIFO", "LRU", "Optimal"});
        pageRefs_ = new QLineEdit(left);
        pageRefs_->setPlaceholderText("e.g. 7 0 1 2 0 3 0 4");
        form->addRow("Frames", pageFrames_);
        form->addRow("Algorithm", pageAlgo_);
        form->addRow("Reference String", pageRefs_);
        leftLayout->addLayout(form);
        auto* run = new QPushButton("Run", left);
        leftLayout->addWidget(run);
        leftLayout->addStretch(1);
        layout->addWidget(left, 0);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);
        pageTable_ = new QTableWidget(right);
        pageTable_->setColumnCount(4);
        pageTable_->setHorizontalHeaderLabels({"Step", "Page", "Result", "Frames"});
        pageTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        pageStatusLabel_ = new QLabel("Page Timeline: waiting to start", right);
        pageFaults_ = new QLabel("Total Page Faults: -", right);
        rightLayout->addWidget(pageTable_, 1);
        rightLayout->addWidget(pageStatusLabel_);
        rightLayout->addWidget(pageFaults_);
        layout->addWidget(right, 1);

        connect(run, &QPushButton::clicked, this, [this]() { runPage(); });
        return tab;
    }

    QWidget* buildDiskTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QGroupBox("Disk Input", tab);
        auto* leftLayout = new QVBoxLayout(left);
        auto* form = new QFormLayout();
        diskAlgo_ = new QComboBox(left);
        diskAlgo_->addItems({"FCFS", "SSTF", "SCAN", "C-SCAN"});
        diskSize_ = new QSpinBox(left);
        diskSize_->setRange(1, 5000);
        diskSize_->setValue(200);
        diskHead_ = new QSpinBox(left);
        diskHead_->setRange(0, 4999);
        diskHead_->setValue(50);
        diskDir_ = new QComboBox(left);
        diskDir_->addItems({"Left", "Right"});
        diskReqs_ = new QLineEdit(left);
        diskReqs_->setPlaceholderText("e.g. 82 170 43 140 24 16 190");
        form->addRow("Algorithm", diskAlgo_);
        form->addRow("Disk Size", diskSize_);
        form->addRow("Head Position", diskHead_);
        form->addRow("Direction (SCAN)", diskDir_);
        form->addRow("Requests", diskReqs_);
        leftLayout->addLayout(form);
        auto* run = new QPushButton("Run", left);
        leftLayout->addWidget(run);
        leftLayout->addStretch(1);
        layout->addWidget(left, 0);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);
        diskOutput_ = new QPlainTextEdit(right);
        diskOutput_->setReadOnly(true);
        diskStatusLabel_ = new QLabel("Disk Timeline: waiting to start", right);
        diskTotal_ = new QLabel("Total Head Movement: -", right);
        rightLayout->addWidget(diskOutput_, 1);
        rightLayout->addWidget(diskStatusLabel_);
        rightLayout->addWidget(diskTotal_);
        layout->addWidget(right, 1);

        connect(run, &QPushButton::clicked, this, [this]() { runDisk(); });
        connect(diskSize_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
            diskHead_->setMaximum(std::max(0, size - 1));
        });
        return tab;
    }

    void startAnimation(int steps,
                        int intervalMs,
                        std::function<void(int)> onStep,
                        std::function<void()> onDone = {}) {
        if (animationTimer_) animationTimer_->stop();
        animationStepFn_ = std::move(onStep);
        animationDoneFn_ = std::move(onDone);
        animationIndex_ = 0;
        animationCount_ = std::max(0, steps);

        if (animationCount_ <= 0) {
            if (animationDoneFn_) animationDoneFn_();
            return;
        }

        animationTimer_->setInterval(std::max(1, intervalMs));
        advanceAnimation();
        if (animationIndex_ < animationCount_) animationTimer_->start();
    }

    void advanceAnimation() {
        if (animationIndex_ >= animationCount_) {
            if (animationTimer_) animationTimer_->stop();
            if (animationDoneFn_) animationDoneFn_();
            animationStepFn_ = {};
            animationDoneFn_ = {};
            return;
        }

        if (animationStepFn_) animationStepFn_(animationIndex_);
        ++animationIndex_;

        if (animationIndex_ >= animationCount_) {
            if (animationTimer_) animationTimer_->stop();
            if (animationDoneFn_) animationDoneFn_();
            animationStepFn_ = {};
            animationDoneFn_ = {};
        }
    }

    void applyTheme(bool darkMode) {
        if (themeToggleButton_) themeToggleButton_->setText(darkMode ? "Light Mode" : "Dark Mode");
        if (gantt_) gantt_->setDarkMode(darkMode);
        if (cmpChart_) cmpChart_->setDarkMode(darkMode);

        const QString background = darkMode ? "#1d1f24" : "#e9edf2";
        const QString panel = darkMode ? "#2a2d33" : "#f3f5f8";
        const QString panelAlt = darkMode ? "#343840" : "#dde4ec";
        const QString text = darkMode ? "#f5f7fa" : "#101418";
        const QString muted = darkMode ? "#d7dbe2" : "#344054";
        const QString border = darkMode ? "#525861" : "#c7ced8";
        const QString accent = darkMode ? "#4ea1ff" : "#2563eb";
        const QString accentStrong = darkMode ? "#79b8ff" : "#1d4ed8";
        const QString inputBg = darkMode ? "#cfd5dd" : "#edf1f5";
        const QString inputText = "#101418";

        if (centralWidget_) {
            centralWidget_->setStyleSheet(QString(
                "QWidget { background-color: %1; color: %2; }"
                "QLabel { color: %2; background: transparent; }"
                "QGroupBox { background-color: %3; border: 1px solid %4; border-radius: 10px; margin-top: 12px; font-weight: 600; color: %2; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; color: %2; }"
                "QPushButton { background-color: %3; color: %2; border: 1px solid %4; border-radius: 8px; padding: 8px 14px; }"
                "QPushButton:hover { border-color: %5; }"
                "QPushButton:pressed { background-color: %6; }"
                "QPushButton:checked { background-color: %5; color: white; border-color: %5; }"
                "QTabWidget::pane { border: 1px solid %4; background: %3; border-radius: 10px; top: -1px; }"
                "QTabBar::tab { background: %6; color: %2; border: 1px solid %4; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; padding: 10px 18px; min-width: 140px; }"
                "QTabBar::tab:selected { background: %5; color: white; border-color: %5; }"
                "QTabBar::tab:hover:!selected { background: %3; border-color: %5; }"
                "QLineEdit, QSpinBox, QComboBox, QPlainTextEdit { background-color: %7; color: %8; border: 1px solid %4; border-radius: 6px; padding: 4px 6px; }"
                "QComboBox QAbstractItemView { background-color: %7; color: %8; selection-background-color: %5; selection-color: white; }"
                "QHeaderView::section { background-color: %6; color: %2; border: 1px solid %4; padding: 6px; }"
                "QTableWidget { background-color: %7; color: %8; gridline-color: #ffffff; border: 1px solid %4; }"
                "QTableCornerButton::section { background-color: %6; border: 1px solid %4; }"
                "QCheckBox { color: %2; spacing: 8px; }"
                "QCheckBox::indicator { width: 16px; height: 16px; }"
                "QCheckBox::indicator:unchecked { border: 1px solid %4; background: %7; }"
                "QCheckBox::indicator:checked { border: 1px solid %5; background: %5; }"
            ).arg(background, text, panel, border, accent, panelAlt, inputBg, inputText));
        }

        if (titleLabel_) {
            titleLabel_->setStyleSheet(QString("color: %1;").arg(darkMode ? accentStrong : accent));
        }
        if (appRunTimerLabel_) {
            appRunTimerLabel_->setStyleSheet(QString(
                "color: %1; background-color: %2; border: 1px solid %3; border-radius: 8px; padding: 8px 12px;")
                .arg(text, panel, border));
        }
        if (themeToggleButton_) {
            themeToggleButton_->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 8px 14px; font-weight: 600; }"
                "QPushButton:hover { border-color: %4; }"
                "QPushButton:checked { background-color: %4; color: white; border-color: %4; }")
                .arg(panel, text, border, accent));
        }

        const QList<QLabel*> labels = {metricsLabel_, cpuTimelineLabel_, cmpStatusLabel_, memTotals_, memStatusLabel_,
                                       pageStatusLabel_, pageFaults_, diskStatusLabel_, diskTotal_};
        for (auto* label : labels) {
            if (label) label->setStyleSheet(QString("color: %1;").arg(muted));
        }
    }

    void updateAppRunTimerLabel() {
        if (!appRunTimerLabel_) return;
        appRunTimerLabel_->setText("OS Run Timer: " + QTime(0, 0).addSecs(appRunSeconds_).toString("hh:mm:ss"));
    }

    void refreshProcessTable() {
        int rows = procCount_->value();
        procTable_->setRowCount(rows);
        for (int i = 0; i < rows; ++i) {
            auto* pidItem = new QTableWidgetItem(QString::number(i + 1));
            pidItem->setFlags(pidItem->flags() & ~Qt::ItemIsEditable);
            procTable_->setItem(i, 0, pidItem);
            ensureItem(procTable_, i, 1, "0");
            ensureItem(procTable_, i, 2, "1");
            ensureItem(procTable_, i, 3, "1");
        }
    }

    void loadSampleProcesses() {
        procCount_->setValue(4);
        refreshProcessTable();
        setCell(procTable_, 0, 1, "0");
        setCell(procTable_, 0, 2, "6");
        setCell(procTable_, 0, 3, "2");
        setCell(procTable_, 1, 1, "1");
        setCell(procTable_, 1, 2, "8");
        setCell(procTable_, 1, 3, "1");
        setCell(procTable_, 2, 1, "2");
        setCell(procTable_, 2, 2, "7");
        setCell(procTable_, 2, 3, "3");
        setCell(procTable_, 3, 1, "3");
        setCell(procTable_, 3, 2, "3");
        setCell(procTable_, 3, 3, "4");
    }

    void refreshComparisonTable() {
        if (!cmpProcTable_ || !cmpProcCount_) return;
        int rows = cmpProcCount_->value();
        cmpProcTable_->setRowCount(rows);
        for (int i = 0; i < rows; ++i) {
            auto* pidItem = new QTableWidgetItem(QString::number(i + 1));
            pidItem->setFlags(pidItem->flags() & ~Qt::ItemIsEditable);
            cmpProcTable_->setItem(i, 0, pidItem);
            ensureItem(cmpProcTable_, i, 1, QString::number(i));
            ensureItem(cmpProcTable_, i, 2, QString::number((i % 4) + 2));
            ensureItem(cmpProcTable_, i, 3, QString::number((i % 3) + 1));
        }
    }

    void loadComparisonSampleProcesses() {
        if (!cmpProcCount_ || !cmpProcTable_) return;
        cmpProcCount_->setValue(4);
        refreshComparisonTable();
        setCell(cmpProcTable_, 0, 1, "0");
        setCell(cmpProcTable_, 0, 2, "6");
        setCell(cmpProcTable_, 0, 3, "2");
        setCell(cmpProcTable_, 1, 1, "1");
        setCell(cmpProcTable_, 1, 2, "8");
        setCell(cmpProcTable_, 1, 3, "1");
        setCell(cmpProcTable_, 2, 1, "2");
        setCell(cmpProcTable_, 2, 2, "7");
        setCell(cmpProcTable_, 2, 3, "3");
        setCell(cmpProcTable_, 3, 1, "3");
        setCell(cmpProcTable_, 3, 2, "3");
        setCell(cmpProcTable_, 3, 3, "4");
    }

    void clearCpuResults() {
        resultTable_->setRowCount(0);
        metricsLabel_->setText("Averages: -");
        cpuTimelineLabel_->setText("CPU Timeline: waiting to start");
        gantt_->setData({});
    }

    std::vector<Process> readProcesses() const {
        return readProcessesFromTable(procTable_);
    }

    std::vector<Process> readComparisonProcesses() const {
        return readProcessesFromTable(cmpProcTable_);
    }

    std::vector<Process> readProcessesFromTable(QTableWidget* table) const {
        std::vector<Process> procs;
        if (!table) return procs;
        int rows = table->rowCount();
        procs.reserve(rows);
        for (int i = 0; i < rows; ++i) {
            Process p{};
            p.pid = i + 1;
            p.arrival = cellInt(table, i, 1, 0);
            p.burst = std::max(1, cellInt(table, i, 2, 1));
            p.priority = std::max(1, cellInt(table, i, 3, 1));
            procs.push_back(p);
        }
        return procs;
    }

    static int totalRuntime(const std::vector<GanttEntry>& entries) {
        if (entries.empty()) return 0;
        return entries.back().end;
    }

    static int runningPidAtTime(const std::vector<GanttEntry>& entries, int time) {
        for (const auto& entry : entries) {
            if (time >= entry.start && time < entry.end) return entry.pid;
        }
        return -2;
    }

    static std::vector<int> executionProgress(const std::vector<Process>& processes,
                                              const std::vector<GanttEntry>& entries,
                                              int time) {
        std::vector<int> progress(processes.size(), 0);
        for (const auto& entry : entries) {
            if (time <= entry.start) continue;
            int segmentEnd = std::min(time, entry.end);
            if (segmentEnd <= entry.start || entry.pid <= 0 || entry.pid > static_cast<int>(progress.size())) continue;
            progress[entry.pid - 1] += segmentEnd - entry.start;
        }
        return progress;
    }

    static QString progressSummary(const std::vector<Process>& processes, const std::vector<int>& progress) {
        QStringList parts;
        for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
            parts << QString("P%1 %2/%3").arg(processes[i].pid).arg(progress[i]).arg(processes[i].burst);
        }
        return parts.join("   ");
    }

    void runCpuScheduling() {
        auto procs = readProcesses();
        if (procs.empty()) return;

        ScheduleResult result{};
        QString algo = algoCombo_->currentText();
        if (algo == "FCFS") result = scheduleFCFS(procs);
        else if (algo == "SJF Non-Preemptive") result = scheduleSJFNonPreemptive(procs);
        else if (algo == "SJF Preemptive (SRTF)") result = scheduleSJFPreemptive(procs);
        else if (algo == "Priority") result = schedulePriority(procs);
        else if (algo == "Round Robin") result = scheduleRoundRobin(procs, quantumSpin_->value());

        resultTable_->setRowCount(static_cast<int>(result.processes.size()));
        for (int i = 0; i < static_cast<int>(result.processes.size()); ++i) {
            setCell(resultTable_, i, 0, QString::number(result.processes[i].pid));
            setCell(resultTable_, i, 1, "-");
            setCell(resultTable_, i, 2, "-");
            setCell(resultTable_, i, 3, "-");
            setCell(resultTable_, i, 4, "-");
            setCell(resultTable_, i, 5, QString::number(result.processes[i].priority));
        }

        metricsLabel_->setText("Averages: animating...");
        cpuTimelineLabel_->setText("CPU Timeline: t=0 | waiting to begin");
        gantt_->setData(result.gantt, 0);

        const int runtime = totalRuntime(result.gantt);
        startAnimation(runtime + 1, 350,
                       [this, result, runtime](int time) {
                           int runningPid = runningPidAtTime(result.gantt, time);
                           QString runningLabel;
                           if (runningPid == -1) runningLabel = "IDLE";
                           else if (runningPid > 0) runningLabel = QString("P%1").arg(runningPid);
                           else runningLabel = (time >= runtime) ? "Completed" : "Waiting";

                           auto progress = executionProgress(result.processes, result.gantt, time);
                           cpuTimelineLabel_->setText(
                               QString("CPU Timeline: t=%1 / %2 | Running: %3 | %4")
                                   .arg(time)
                                   .arg(runtime)
                                   .arg(runningLabel)
                                   .arg(progressSummary(result.processes, progress)));
                           gantt_->setData(result.gantt, time);

                           for (int i = 0; i < static_cast<int>(result.processes.size()); ++i) {
                               if (result.completion[i] <= time) {
                                   setCell(resultTable_, i, 1, QString::number(result.completion[i]));
                                   setCell(resultTable_, i, 2, QString::number(result.waiting[i]));
                                   setCell(resultTable_, i, 3, QString::number(result.turnaround[i]));
                                   setCell(resultTable_, i, 4, QString::number(result.response[i]));
                               }
                           }
                       },
                       [this, result]() {
                           metricsLabel_->setText(QString("Averages: Waiting=%1  Turnaround=%2  Response=%3")
                                                  .arg(result.metrics.avg_waiting, 0, 'f', 2)
                                                  .arg(result.metrics.avg_turnaround, 0, 'f', 2)
                                                  .arg(result.metrics.avg_response, 0, 'f', 2));
                       });
    }

    void runComparison() {
        auto procs = readComparisonProcesses();
        if (procs.empty()) return;

        std::vector<std::pair<QString, ScheduleResult>> results;
        if (cmpFcfs_->isChecked()) results.push_back({"FCFS", scheduleFCFS(procs)});
        if (cmpSjf_->isChecked()) results.push_back({"SJF Non-Preemptive", scheduleSJFNonPreemptive(procs)});
        if (cmpSrtf_->isChecked()) results.push_back({"SJF Preemptive", scheduleSJFPreemptive(procs)});
        if (cmpPrio_->isChecked()) results.push_back({"Priority", schedulePriority(procs)});
        if (cmpRr_->isChecked()) results.push_back({"Round Robin", scheduleRoundRobin(procs, cmpQuantum_->value())});

        cmpTable_->setRowCount(static_cast<int>(results.size()));
        for (int i = 0; i < static_cast<int>(results.size()); ++i) {
            for (int col = 0; col < 4; ++col) setCell(cmpTable_, i, col, "-");
        }
        cmpStatusLabel_->setText("Comparison Timeline: starting playback");
        cmpChart_->setData({}, {});

        startAnimation(static_cast<int>(results.size()), 450,
                       [this, results](int step) {
                           cmpStatusLabel_->setText(
                               QString("Comparison Timeline: step %1 / %2")
                                   .arg(step + 1)
                                   .arg(static_cast<int>(results.size())));
                           std::vector<QString> labels;
                           std::vector<double> values;
                           for (int i = 0; i <= step && i < static_cast<int>(results.size()); ++i) {
                               const auto& name = results[i].first;
                               const auto& m = results[i].second.metrics;
                               setCell(cmpTable_, i, 0, name);
                               setCell(cmpTable_, i, 1, QString::number(m.avg_waiting, 'f', 2));
                               setCell(cmpTable_, i, 2, QString::number(m.avg_turnaround, 'f', 2));
                               setCell(cmpTable_, i, 3, QString::number(m.avg_response, 'f', 2));
                               labels.push_back(name.left(6));
                               values.push_back(m.avg_waiting);
                           }
                           cmpChart_->setData(labels, values);
                       },
                       [this, results]() {
                           cmpStatusLabel_->setText(
                               QString("Comparison Timeline: complete (%1 algorithms)")
                                   .arg(static_cast<int>(results.size())));
                       });
    }

    void refreshMemoryTables() {
        int procs = memProcs_->value();

        if (memMode_ && memMode_->currentIndex() == 1) {
            memBlocksTable_->setRowCount(0);
        } else {
            int blocks = memBlocks_->value();
            memBlocksTable_->setRowCount(blocks);
            for (int i = 0; i < blocks; ++i) ensureItem(memBlocksTable_, i, 0, "100");
        }

        memProcsTable_->setRowCount(procs);
        for (int i = 0; i < procs; ++i) ensureItem(memProcsTable_, i, 0, "90");
    }

    void refreshMemoryUi() {
        bool paging = memMode_ && memMode_->currentIndex() == 1;

        if (memBlocks_) memBlocks_->setVisible(!paging);
        if (memBlocksRowLabel_) memBlocksRowLabel_->setVisible(!paging);
        if (memBlocksTable_) memBlocksTable_->setVisible(!paging);
        if (memBlocksTitle_) memBlocksTitle_->setVisible(!paging);

        if (memMemSize_) memMemSize_->setVisible(paging);
        if (memMemSizeRowLabel_) memMemSizeRowLabel_->setVisible(paging);
        if (memPageSize_) memPageSize_->setVisible(paging);
        if (memPageSizeRowLabel_) memPageSizeRowLabel_->setVisible(paging);

        if (memAlgo_) {
            memAlgo_->clear();
            if (paging) memAlgo_->addItems({"Paging Allocation"});
            else memAlgo_->addItems({"First Fit", "Best Fit", "Next Fit"});
        }
        if (memAlgoRowLabel_) memAlgoRowLabel_->setVisible(true);

        if (memResult_) {
            if (paging) {
                memResult_->setColumnCount(6);
                memResult_->setHorizontalHeaderLabels({"PID", "ProcSize", "Pages", "Frames", "IntFrag", "Allocated"});
            } else {
                memResult_->setColumnCount(5);
                memResult_->setHorizontalHeaderLabels({"PID", "ProcSize", "Block", "BlockSize", "IntFrag"});
            }
        }

        if (memDetails_) {
            memDetails_->setVisible(paging);
            if (!paging) memDetails_->clear();
        }

        if (memTotals_) memTotals_->setText("Totals: -");
        if (memStatusLabel_) memStatusLabel_->setText("Memory Timeline: waiting to start");
        refreshMemoryTables();
    }

    void applyDefaultModuleValues() {
        loadSampleProcesses();
        loadComparisonSampleProcesses();
        if (pageRefs_) pageRefs_->setText("7 0 1 2 0 3 0 4");
        if (diskReqs_) diskReqs_->setText("82 170 43 140 24 16 190");
    }

    void runMemory() {
        bool paging = memMode_ && memMode_->currentIndex() == 1;

        std::vector<int> blocks, procs;
        if (!paging) {
            for (int i = 0; i < memBlocksTable_->rowCount(); ++i) blocks.push_back(cellInt(memBlocksTable_, i, 0, 0));
        }
        for (int i = 0; i < memProcsTable_->rowCount(); ++i) procs.push_back(cellInt(memProcsTable_, i, 0, 0));

        if (paging) {
            int memorySize = memMemSize_->value();
            int pageSize = memPageSize_->value();
            if (pageSize <= 0 || pageSize > memorySize) {
                memTotals_->setText("Totals: invalid paging parameters (pageSize must be <= memorySize)");
                return;
            }

            PagingResult res = pagingAllocate(memorySize, pageSize, procs);

            memResult_->setRowCount(static_cast<int>(procs.size()));
            QString header = QString("Physical Memory=%1  Page/Frame=%2  Frames=%3  Free=%4  Remainder=%5\n\n")
                                 .arg(res.memorySize)
                                 .arg(res.pageSize)
                                 .arg(res.totalFrames)
                                 .arg(res.freeFrames)
                                 .arg(res.systemRemainder);
            if (memDetails_) memDetails_->setPlainText(header);
            memTotals_->setText("Totals: animating...");
            memStatusLabel_->setText("Memory Timeline: starting playback");

            for (int i = 0; i < static_cast<int>(procs.size()); ++i) {
                for (int col = 0; col < 6; ++col) setCell(memResult_, i, col, "-");
            }

            startAnimation(static_cast<int>(procs.size()), 350,
                           [this, procs, res, header](int step) {
                               memStatusLabel_->setText(
                                   QString("Memory Timeline: process %1 / %2")
                                       .arg(step + 1)
                                       .arg(static_cast<int>(procs.size())));
                               QString details = header;
                               int shownUnallocated = 0;
                               int shownInternal = res.systemRemainder;
                               for (int i = 0; i <= step && i < static_cast<int>(procs.size()); ++i) {
                                   bool allocated = true;
                                   for (int f : res.pageTable[i]) {
                                       if (f == -1) {
                                           allocated = false;
                                           break;
                                       }
                                   }

                                   setCell(memResult_, i, 0, QString::number(i + 1));
                                   setCell(memResult_, i, 1, QString::number(procs[i]));
                                   setCell(memResult_, i, 2, QString::number(res.pagesNeeded[i]));

                                   if (!allocated) {
                                       setCell(memResult_, i, 3, "-");
                                       setCell(memResult_, i, 4, "-");
                                       setCell(memResult_, i, 5, "NO");
                                       ++shownUnallocated;
                                       details += QString("P%1: NOT ALLOCATED\n").arg(i + 1);
                                   } else {
                                       setCell(memResult_, i, 3, QString::number(res.pagesNeeded[i]));
                                       setCell(memResult_, i, 4, QString::number(res.internal[i]));
                                       setCell(memResult_, i, 5, "YES");
                                       shownInternal += res.internal[i];

                                       details += QString("P%1: ").arg(i + 1);
                                       for (int p = 0; p < static_cast<int>(res.pageTable[i].size()); ++p) {
                                           if (p) details += "  ";
                                           details += QString("%1->%2").arg(p).arg(res.pageTable[i][p]);
                                       }
                                       details += "\n";
                                   }
                               }

                               if (memDetails_) memDetails_->setPlainText(details);
                               memTotals_->setText(QString("Totals: Internal=%1  Unallocated=%2  FreeFrames=%3")
                                                   .arg(shownInternal)
                                                   .arg(shownUnallocated)
                                                   .arg(res.freeFrames));
                           },
                           [this, procs]() {
                               memStatusLabel_->setText(
                                   QString("Memory Timeline: complete (%1 processes)")
                                       .arg(static_cast<int>(procs.size())));
                           });
            return;
        }

        MemoryResult res{};
        QString algo = memAlgo_->currentText();
        if (algo == "First Fit") res = firstFit(blocks, procs);
        else if (algo == "Best Fit") res = bestFit(blocks, procs);
        else res = nextFit(blocks, procs);

        memResult_->setRowCount(static_cast<int>(procs.size()));
        for (int i = 0; i < static_cast<int>(procs.size()); ++i) {
            for (int col = 0; col < 5; ++col) setCell(memResult_, i, col, "-");
        }
        memTotals_->setText("Totals: animating...");
        memStatusLabel_->setText("Memory Timeline: starting playback");

        startAnimation(static_cast<int>(procs.size()), 350,
                       [this, procs, blocks, res](int step) {
                           memStatusLabel_->setText(
                               QString("Memory Timeline: process %1 / %2")
                                   .arg(step + 1)
                                   .arg(static_cast<int>(procs.size())));
                           int shownInternal = 0;
                           int shownUnallocated = 0;
                           for (int i = 0; i <= step && i < static_cast<int>(procs.size()); ++i) {
                               setCell(memResult_, i, 0, QString::number(i + 1));
                               setCell(memResult_, i, 1, QString::number(procs[i]));
                               if (res.allocation[i] == -1) {
                                   setCell(memResult_, i, 2, "-");
                                   setCell(memResult_, i, 3, "-");
                                   setCell(memResult_, i, 4, "-");
                                   ++shownUnallocated;
                               } else {
                                   setCell(memResult_, i, 2, QString::number(res.allocation[i] + 1));
                                   setCell(memResult_, i, 3, QString::number(blocks[res.allocation[i]]));
                                   setCell(memResult_, i, 4, QString::number(res.internal[i]));
                                   shownInternal += res.internal[i];
                               }
                           }

                           memTotals_->setText(QString("Totals: Internal=%1  External=%2  Unallocated=%3")
                                               .arg(shownInternal)
                                               .arg(res.totalExternal)
                                               .arg(shownUnallocated));
                       },
                       [this, procs]() {
                           memStatusLabel_->setText(
                               QString("Memory Timeline: complete (%1 processes)")
                                   .arg(static_cast<int>(procs.size())));
                       });
    }

    void runPage() {
        auto refs = parseIntList(pageRefs_->text());
        if (refs.empty()) return;

        PageResult res{};
        QString algo = pageAlgo_->currentText();
        if (algo == "FIFO") res = pageFIFO(pageFrames_->value(), refs);
        else if (algo == "LRU") res = pageLRU(pageFrames_->value(), refs);
        else res = pageOptimal(pageFrames_->value(), refs);

        pageTable_->setRowCount(static_cast<int>(res.framesSteps.size()));
        for (int i = 0; i < static_cast<int>(res.framesSteps.size()); ++i) {
            for (int col = 0; col < 4; ++col) setCell(pageTable_, i, col, "-");
        }
        pageStatusLabel_->setText("Page Timeline: starting playback");
        pageFaults_->setText("Total Page Faults: animating...");

        startAnimation(static_cast<int>(res.framesSteps.size()), 300,
                       [this, res](int step) {
                           pageStatusLabel_->setText(
                               QString("Page Timeline: step %1 / %2")
                                   .arg(step + 1)
                                   .arg(static_cast<int>(res.framesSteps.size())));
                           int faults = 0;
                           for (int i = 0; i <= step && i < static_cast<int>(res.framesSteps.size()); ++i) {
                               setCell(pageTable_, i, 0, QString::number(i + 1));
                               setCell(pageTable_, i, 1, QString::number(res.refs[i]));
                               setCell(pageTable_, i, 2, res.hits[i] ? "HIT" : "FAULT");
                               setCell(pageTable_, i, 3, framesToString(res.framesSteps[i]));
                               if (!res.hits[i]) ++faults;
                           }
                           pageFaults_->setText(QString("Total Page Faults: %1").arg(faults));
                       },
                       [this, res]() {
                           pageStatusLabel_->setText(
                               QString("Page Timeline: complete (%1 references)")
                                   .arg(static_cast<int>(res.framesSteps.size())));
                       });
    }

    void runDisk() {
        auto reqs = parseIntList(diskReqs_->text());
        if (reqs.empty()) return;
        int diskSize = diskSize_->value();
        int head = diskHead_->value();
        DiskResult res{};
        QString algo = diskAlgo_->currentText();
        if (algo == "FCFS") res = diskFCFS(diskSize, head, reqs);
        else if (algo == "SSTF") res = diskSSTF(diskSize, head, reqs);
        else if (algo == "SCAN") res = diskSCAN(diskSize, head, reqs, diskDir_->currentIndex() == 1);
        else res = diskCSCAN(diskSize, head, reqs);

        diskOutput_->setPlainText("Sequence:\n");
        diskStatusLabel_->setText("Disk Timeline: starting playback");
        diskTotal_->setText("Total Head Movement: animating...");

        startAnimation(static_cast<int>(res.sequence.size()), 320,
                       [this, res, head](int step) {
                           diskStatusLabel_->setText(
                               QString("Disk Timeline: request %1 / %2")
                                   .arg(step + 1)
                                   .arg(static_cast<int>(res.sequence.size())));
                           QStringList parts;
                           for (int i = 0; i <= step && i < static_cast<int>(res.sequence.size()); ++i) {
                               parts << QString::number(res.sequence[i]);
                           }

                           int movement = 0;
                           int current = head;
                           for (int i = 0; i <= step && i < static_cast<int>(res.sequence.size()); ++i) {
                               movement += std::abs(res.sequence[i] - current);
                               current = res.sequence[i];
                           }

                           diskOutput_->setPlainText(
                               QString("Sequence:\n%1\n\nCurrent Head: %2")
                                   .arg(parts.join(" -> "))
                                   .arg(current));
                           diskTotal_->setText(QString("Total Head Movement: %1").arg(movement));
                       },
                       [this, res]() {
                           diskStatusLabel_->setText(
                               QString("Disk Timeline: complete (%1 head moves)")
                                   .arg(static_cast<int>(res.sequence.size())));
                       });
    }

    static void setCell(QTableWidget* table, int row, int col, const QString& text) {
        auto* item = table->item(row, col);
        if (!item) {
            item = new QTableWidgetItem();
            table->setItem(row, col, item);
        }
        item->setText(text);
    }

    static void ensureItem(QTableWidget* table, int row, int col, const QString& text) {
        if (!table->item(row, col)) {
            table->setItem(row, col, new QTableWidgetItem(text));
        }
    }

    static int cellInt(QTableWidget* table, int row, int col, int fallback) {
        auto* item = table->item(row, col);
        if (!item) return fallback;
        bool ok = false;
        int v = item->text().toInt(&ok);
        return ok ? v : fallback;
    }

    static std::vector<int> parseIntList(const QString& text) {
        std::vector<int> out;
        for (const auto& part : text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)) {
            bool ok = false;
            int v = part.toInt(&ok);
            if (ok) out.push_back(v);
        }
        return out;
    }

    static QString framesToString(const std::vector<int>& frames) {
        QStringList parts;
        for (int f : frames) {
            if (f == -1) parts << "-";
            else parts << QString::number(f);
        }
        return parts.join(" ");
    }
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

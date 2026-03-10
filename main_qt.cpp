#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
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
    void setData(const std::vector<GanttEntry>& entries) {
        entries_ = entries;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#FFFFFF"));
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

        const int h = height() - 30;
        const int y = 10;
        int x = 10;
        int w = width() - 20;

        p.setPen(QPen(QColor("#333333"), 1));
        p.drawRect(x, y, w, h);

        for (const auto& e : entries_) {
            int sx = x + static_cast<int>(std::round((e.start - minTime) * 1.0 * w / total));
            int ex = x + static_cast<int>(std::round((e.end - minTime) * 1.0 * w / total));
            int rw = std::max(1, ex - sx);
            QRect r(sx, y, rw, h);
            p.fillRect(r.adjusted(1, 1, -1, -1), colorForPid(e.pid));
            p.setPen(QColor("#222222"));
            QString label = (e.pid == -1) ? "IDLE" : QString("P%1").arg(e.pid);
            p.drawText(r, Qt::AlignCenter, label);
        }

        p.setPen(QColor("#222222"));
        p.drawText(x, y + h + 15, QString::number(minTime));
        p.drawText(x + w - 30, y + h + 15, QString::number(maxTime));
    }

private:
    std::vector<GanttEntry> entries_;
};

class BarChartWidget : public QWidget {
public:
    void setData(const std::vector<QString>& labels, const std::vector<double>& values) {
        labels_ = labels;
        values_ = values;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#FFFFFF"));
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

        p.setPen(QColor("#333333"));
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
            p.setPen(QColor("#222222"));
            p.drawRect(r);
            p.drawText(QRect(bx, y0 + 2, barW + 20, 20), Qt::AlignLeft | Qt::AlignTop, labels_[i]);
        }
    }

private:
    std::vector<QString> labels_;
    std::vector<double> values_;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Mini Operating System Simulator - Professional GUI");
        auto* central = new QWidget(this);
        central->setStyleSheet("background-color: #f0f0f0;");
        auto* root = new QVBoxLayout(central);

        auto* title = new QLabel("Mini Operating System Simulator");
        QFont tf = title->font();
        tf.setPointSize(14);
        tf.setBold(true);
        title->setFont(tf);
        title->setAlignment(Qt::AlignCenter);
        root->addWidget(title);

        auto* tabs = new QTabWidget(central);
        tabs->addTab(buildCpuTab(), "CPU Scheduling");
        tabs->addTab(buildComparisonTab(), "Comparison Dashboard");
        tabs->addTab(buildMemoryTab(), "Memory Management");
        tabs->addTab(buildPageTab(), "Page Replacement");
        tabs->addTab(buildDiskTab(), "Disk Scheduling");

        root->addWidget(tabs, 1);
        setCentralWidget(central);
        resize(1200, 720);

        refreshProcessTable();
    }

private:
    QSpinBox* procCount_ = nullptr;
    QTableWidget* procTable_ = nullptr;
    QComboBox* algoCombo_ = nullptr;
    QSpinBox* quantumSpin_ = nullptr;
    QLabel* metricsLabel_ = nullptr;
    QTableWidget* resultTable_ = nullptr;
    GanttWidget* gantt_ = nullptr;

    QCheckBox* cmpFcfs_ = nullptr;
    QCheckBox* cmpSjf_ = nullptr;
    QCheckBox* cmpSrtf_ = nullptr;
    QCheckBox* cmpPrio_ = nullptr;
    QCheckBox* cmpRr_ = nullptr;
    QSpinBox* cmpQuantum_ = nullptr;
    QTableWidget* cmpTable_ = nullptr;
    BarChartWidget* cmpChart_ = nullptr;

    QSpinBox* memBlocks_ = nullptr;
    QSpinBox* memProcs_ = nullptr;
    QTableWidget* memBlocksTable_ = nullptr;
    QTableWidget* memProcsTable_ = nullptr;
    QComboBox* memAlgo_ = nullptr;
    QTableWidget* memResult_ = nullptr;
    QLabel* memTotals_ = nullptr;

    QSpinBox* pageFrames_ = nullptr;
    QLineEdit* pageRefs_ = nullptr;
    QComboBox* pageAlgo_ = nullptr;
    QTableWidget* pageTable_ = nullptr;
    QLabel* pageFaults_ = nullptr;

    QSpinBox* diskSize_ = nullptr;
    QSpinBox* diskHead_ = nullptr;
    QComboBox* diskAlgo_ = nullptr;
    QComboBox* diskDir_ = nullptr;
    QLineEdit* diskReqs_ = nullptr;
    QPlainTextEdit* diskOutput_ = nullptr;
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
        resultTable_ = new QTableWidget(right);
        resultTable_->setColumnCount(6);
        resultTable_->setHorizontalHeaderLabels({"PID", "Completion", "Waiting", "Turnaround", "Response", "Priority"});
        resultTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        gantt_ = new GanttWidget();
        gantt_->setMinimumHeight(160);

        rightLayout->addWidget(metricsLabel_);
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

        auto* left = new QGroupBox("Algorithms", tab);
        auto* leftLayout = new QVBoxLayout(left);
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

        leftLayout->addWidget(cmpFcfs_);
        leftLayout->addWidget(cmpSjf_);
        leftLayout->addWidget(cmpSrtf_);
        leftLayout->addWidget(cmpPrio_);
        leftLayout->addWidget(cmpRr_);

        cmpQuantum_ = new QSpinBox(left);
        cmpQuantum_->setRange(1, 10);
        cmpQuantum_->setValue(2);
        leftLayout->addWidget(new QLabel("Quantum (RR)"));
        leftLayout->addWidget(cmpQuantum_);
        auto* run = new QPushButton("Run Comparison", left);
        leftLayout->addWidget(run);
        leftLayout->addStretch(1);

        layout->addWidget(left, 0);

        auto* right = new QWidget(tab);
        auto* rightLayout = new QVBoxLayout(right);
        cmpTable_ = new QTableWidget(right);
        cmpTable_->setColumnCount(4);
        cmpTable_->setHorizontalHeaderLabels({"Algorithm", "Avg Waiting", "Avg Turnaround", "Avg Response"});
        cmpTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        cmpChart_ = new BarChartWidget();
        cmpChart_->setMinimumHeight(220);

        rightLayout->addWidget(cmpTable_, 1);
        rightLayout->addWidget(new QLabel("Average Waiting Time (lower is better)"));
        rightLayout->addWidget(cmpChart_, 0);
        layout->addWidget(right, 1);

        connect(run, &QPushButton::clicked, this, [this]() { runComparison(); });
        return tab;
    }

    QWidget* buildMemoryTab() {
        auto* tab = new QWidget(this);
        auto* layout = new QHBoxLayout(tab);

        auto* left = new QGroupBox("Memory Input", tab);
        auto* leftLayout = new QVBoxLayout(left);
        auto* form = new QFormLayout();
        memBlocks_ = new QSpinBox(left);
        memBlocks_->setRange(1, 20);
        memBlocks_->setValue(5);
        memProcs_ = new QSpinBox(left);
        memProcs_->setRange(1, 20);
        memProcs_->setValue(5);
        memAlgo_ = new QComboBox(left);
        memAlgo_->addItems({"First Fit", "Best Fit", "Next Fit"});
        form->addRow("Blocks", memBlocks_);
        form->addRow("Processes", memProcs_);
        form->addRow("Algorithm", memAlgo_);
        leftLayout->addLayout(form);

        memBlocksTable_ = new QTableWidget(left);
        memBlocksTable_->setColumnCount(1);
        memBlocksTable_->setHorizontalHeaderLabels({"Block Size"});
        memBlocksTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        memProcsTable_ = new QTableWidget(left);
        memProcsTable_->setColumnCount(1);
        memProcsTable_->setHorizontalHeaderLabels({"Process Size"});
        memProcsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        leftLayout->addWidget(new QLabel("Blocks"));
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
        rightLayout->addWidget(memResult_, 1);
        rightLayout->addWidget(memTotals_);
        layout->addWidget(right, 1);

        connect(memBlocks_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { refreshMemoryTables(); });
        connect(memProcs_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { refreshMemoryTables(); });
        connect(run, &QPushButton::clicked, this, [this]() { runMemory(); });

        refreshMemoryTables();
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
        pageFaults_ = new QLabel("Total Page Faults: -", right);
        rightLayout->addWidget(pageTable_, 1);
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
        diskTotal_ = new QLabel("Total Head Movement: -", right);
        rightLayout->addWidget(diskOutput_, 1);
        rightLayout->addWidget(diskTotal_);
        layout->addWidget(right, 1);

        connect(run, &QPushButton::clicked, this, [this]() { runDisk(); });
        connect(diskSize_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
            diskHead_->setMaximum(std::max(0, size - 1));
        });
        return tab;
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

    void clearCpuResults() {
        resultTable_->setRowCount(0);
        metricsLabel_->setText("Averages: -");
        gantt_->setData({});
    }

    std::vector<Process> readProcesses() const {
        std::vector<Process> procs;
        int rows = procTable_->rowCount();
        procs.reserve(rows);
        for (int i = 0; i < rows; ++i) {
            Process p{};
            p.pid = i + 1;
            p.arrival = cellInt(procTable_, i, 1, 0);
            p.burst = std::max(1, cellInt(procTable_, i, 2, 1));
            p.priority = std::max(1, cellInt(procTable_, i, 3, 1));
            procs.push_back(p);
        }
        return procs;
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
            setCell(resultTable_, i, 1, QString::number(result.completion[i]));
            setCell(resultTable_, i, 2, QString::number(result.waiting[i]));
            setCell(resultTable_, i, 3, QString::number(result.turnaround[i]));
            setCell(resultTable_, i, 4, QString::number(result.response[i]));
            setCell(resultTable_, i, 5, QString::number(result.processes[i].priority));
        }

        metricsLabel_->setText(QString("Averages: Waiting=%1  Turnaround=%2  Response=%3")
                               .arg(result.metrics.avg_waiting, 0, 'f', 2)
                               .arg(result.metrics.avg_turnaround, 0, 'f', 2)
                               .arg(result.metrics.avg_response, 0, 'f', 2));
        gantt_->setData(result.gantt);
    }

    void runComparison() {
        auto procs = readProcesses();
        if (procs.empty()) return;

        std::vector<std::pair<QString, ScheduleResult>> results;
        if (cmpFcfs_->isChecked()) results.push_back({"FCFS", scheduleFCFS(procs)});
        if (cmpSjf_->isChecked()) results.push_back({"SJF Non-Preemptive", scheduleSJFNonPreemptive(procs)});
        if (cmpSrtf_->isChecked()) results.push_back({"SJF Preemptive", scheduleSJFPreemptive(procs)});
        if (cmpPrio_->isChecked()) results.push_back({"Priority", schedulePriority(procs)});
        if (cmpRr_->isChecked()) results.push_back({"Round Robin", scheduleRoundRobin(procs, cmpQuantum_->value())});

        cmpTable_->setRowCount(static_cast<int>(results.size()));
        std::vector<QString> labels;
        std::vector<double> values;
        for (int i = 0; i < static_cast<int>(results.size()); ++i) {
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
    }

    void refreshMemoryTables() {
        int blocks = memBlocks_->value();
        int procs = memProcs_->value();
        memBlocksTable_->setRowCount(blocks);
        memProcsTable_->setRowCount(procs);
        for (int i = 0; i < blocks; ++i) ensureItem(memBlocksTable_, i, 0, "100");
        for (int i = 0; i < procs; ++i) ensureItem(memProcsTable_, i, 0, "90");
    }

    void runMemory() {
        std::vector<int> blocks, procs;
        for (int i = 0; i < memBlocksTable_->rowCount(); ++i) blocks.push_back(cellInt(memBlocksTable_, i, 0, 0));
        for (int i = 0; i < memProcsTable_->rowCount(); ++i) procs.push_back(cellInt(memProcsTable_, i, 0, 0));

        MemoryResult res{};
        QString algo = memAlgo_->currentText();
        if (algo == "First Fit") res = firstFit(blocks, procs);
        else if (algo == "Best Fit") res = bestFit(blocks, procs);
        else res = nextFit(blocks, procs);

        memResult_->setRowCount(static_cast<int>(procs.size()));
        for (int i = 0; i < static_cast<int>(procs.size()); ++i) {
            setCell(memResult_, i, 0, QString::number(i + 1));
            setCell(memResult_, i, 1, QString::number(procs[i]));
            if (res.allocation[i] == -1) {
                setCell(memResult_, i, 2, "-");
                setCell(memResult_, i, 3, "-");
                setCell(memResult_, i, 4, "-");
            } else {
                setCell(memResult_, i, 2, QString::number(res.allocation[i] + 1));
                setCell(memResult_, i, 3, QString::number(blocks[res.allocation[i]]));
                setCell(memResult_, i, 4, QString::number(res.internal[i]));
            }
        }

        memTotals_->setText(QString("Totals: Internal=%1  External=%2  Unallocated=%3")
                            .arg(res.totalInternal)
                            .arg(res.totalExternal)
                            .arg(res.unallocated));
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
            setCell(pageTable_, i, 0, QString::number(i + 1));
            setCell(pageTable_, i, 1, QString::number(res.refs[i]));
            setCell(pageTable_, i, 2, res.hits[i] ? "HIT" : "FAULT");
            setCell(pageTable_, i, 3, framesToString(res.framesSteps[i]));
        }
        pageFaults_->setText(QString("Total Page Faults: %1").arg(res.faults));
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

        QString seq;
        for (int i = 0; i < static_cast<int>(res.sequence.size()); ++i) {
            if (i > 0) seq += " -> ";
            seq += QString::number(res.sequence[i]);
        }
        diskOutput_->setPlainText("Sequence:\n" + seq);
        diskTotal_->setText(QString("Total Head Movement: %1").arg(res.totalMovement));
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

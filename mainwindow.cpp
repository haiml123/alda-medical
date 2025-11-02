#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFont>
#include <QAbstractItemView>
#include <QListView>
#include <QStyledItemDelegate>
#include <QStyleFactory>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QHBoxLayout>
#include <QRandomGenerator>

// ============================================================
// RingBufferPlot Implementation
// ============================================================

RingBufferPlot::RingBufferPlot(QWidget *parent)
    : QWidget(parent),
      channelCount(64),          // Default: 64 channels
      channelHeight(12),         // Default: 12 pixels per channel (compact to fit all 64)
      bufferSize(500),
      writeIndex(0),
      bufferFilled(false),       // Start with unfilled buffer
      yMin(-50.0),               // Adjusted for noise range
      yMax(50.0),
      isSimulating(false) {      // Start with simulation OFF

    setMinimumHeight(400);  // Minimum height for visibility
    setMinimumWidth(600);

    // Initialize multi-channel buffers with ZEROS (clean start)
    channelBuffers.resize(channelCount);
    for (int i = 0; i < channelCount; i++) {
        channelBuffers[i].resize(bufferSize);
        channelBuffers[i].fill(0.0);  // All zeros = flat lines
    }

    // Timer for widget updates (always runs for display refresh)
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, QOverload<>::of(&RingBufferPlot::update));
    updateTimer->start(16); // 16ms = ~60 FPS (was 33ms for 30 FPS)

    // Timer for data simulation (only starts when user clicks "Start Recording")
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &RingBufferPlot::simulateData);
    // DON'T start simulationTimer here - wait for user action!
}

void RingBufferPlot::addDataPoint(double value) {
    // Add to first channel only (for backward compatibility)
    if (channelCount > 0) {
        channelBuffers[0][writeIndex] = value;
    }
}

void RingBufferPlot::addMultiChannelData(const QVector<double>& values) {
    // Add data to all channels
    int numChannels = qMin(values.size(), channelCount);
    for (int i = 0; i < numChannels; i++) {
        channelBuffers[i][writeIndex] = values[i];
    }
    writeIndex = (writeIndex + 1) % bufferSize;

    // Mark buffer as filled once we wrap around
    if (writeIndex == 0) {
        bufferFilled = true;
    }
}

void RingBufferPlot::setChannelCount(int count) {
    if (count > 0 && count <= 256) {  // Reasonable limit
        channelCount = count;
        channelBuffers.resize(channelCount);
        for (int i = 0; i < channelCount; i++) {
            channelBuffers[i].resize(bufferSize);
            channelBuffers[i].fill(0.0);
        }
        setMinimumHeight(channelCount * channelHeight + 100);
    }
}

void RingBufferPlot::setChannelHeight(int height) {
    if (height > 5) {  // Minimum 5 pixels per channel
        channelHeight = height;
        // No fixed minimum height - let it expand
    }
}

void RingBufferPlot::setBufferSize(int size) {
    if (size > 0) {
        bufferSize = size;
        for (int i = 0; i < channelCount; i++) {
            channelBuffers[i].resize(bufferSize);
            channelBuffers[i].fill(0.0);
        }
        writeIndex = 0;
    }
}

void RingBufferPlot::setYRange(double min, double max) {
    yMin = min;
    yMax = max;
}

void RingBufferPlot::setUpdateInterval(int msec) {
    updateTimer->setInterval(msec);
}

void RingBufferPlot::startSimulation() {
    isSimulating = true;
    simulationTimer->start(0);  // 0ms = as fast as possible = ~5000 Hz
}

void RingBufferPlot::stopSimulation() {
    isSimulating = false;
    simulationTimer->stop();
}

void RingBufferPlot::clearData() {
    for (int i = 0; i < channelCount; i++) {
        channelBuffers[i].fill(0.0);
    }
    writeIndex = 0;
    bufferFilled = false;  // Reset filled flag
}

void RingBufferPlot::simulateData() {
    // Simulate realistic EEG noise for all 64 channels
    QVector<double> samples(channelCount);

    // Generate noise for each channel
    for (int ch = 0; ch < channelCount; ch++) {
        // Base noise component (white noise)
        double whiteNoise = (QRandomGenerator::global()->bounded(1000) - 500) / 100.0;  // ±5 µV

        // Low frequency drift (simulates baseline wander)
        static QVector<double> driftPhase(64, 0.0);
        if (driftPhase.size() < channelCount) {
            driftPhase.resize(channelCount, 0.0);
        }
        double drift = 3.0 * qSin(driftPhase[ch]);
        driftPhase[ch] += 0.01 + (ch * 0.0001);  // Slightly different freq per channel

        // Occasional spike (simulates artifacts)
        double spike = 0.0;
        if (QRandomGenerator::global()->bounded(1000) < 5) {  // 0.5% chance
            spike = (QRandomGenerator::global()->bounded(2) == 0 ? 1 : -1) *
                    (QRandomGenerator::global()->bounded(20) + 10);  // ±10-30 µV spike
        }

        // Alpha-like activity (8-12 Hz) with varying amplitude per channel
        static QVector<double> alphaPhase(64, 0.0);
        if (alphaPhase.size() < channelCount) {
            alphaPhase.resize(channelCount, 0.0);
        }
        double alpha = (2.0 + (ch % 5) * 0.5) * qSin(alphaPhase[ch]);
        alphaPhase[ch] += 0.5 + (ch * 0.01);  // ~10 Hz, slight variation

        // Beta activity (13-30 Hz) - smaller amplitude
        static QVector<double> betaPhase(64, 0.0);
        if (betaPhase.size() < channelCount) {
            betaPhase.resize(channelCount, 0.0);
        }
        double beta = (1.0 + (ch % 3) * 0.3) * qSin(betaPhase[ch]);
        betaPhase[ch] += 1.2 + (ch * 0.02);  // ~20 Hz, slight variation

        // Combine all components
        samples[ch] = whiteNoise + drift + spike + alpha + beta;
    }

    // Add the multi-channel data
    addMultiChannelData(samples);
}

void RingBufferPlot::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(15, 23, 42)); // Dark background

    // Define plot area with margins
    int marginLeft = 60;
    int marginRight = 20;
    int marginTop = 50;
    int marginBottom = 20;

    int plotWidth = width() - marginLeft - marginRight;
    int totalHeight = height() - marginTop - marginBottom;

    // DYNAMIC: Calculate channel height to fit ALL channels exactly
    double exactChannelHeight = (double)totalHeight / (double)channelCount;

    // Draw title
    painter.setPen(QColor(226, 232, 240));
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(marginLeft, 20, QString("64-Channel EEG Display"));

    // Draw status
    QFont statusFont = painter.font();
    statusFont.setPointSize(9);
    painter.setFont(statusFont);
    QString status = isSimulating ? "● Recording" : "○ Stopped";
    QColor statusColor = isSimulating ? QColor(34, 197, 94) : QColor(148, 163, 184);
    painter.setPen(statusColor);
    painter.drawText(marginLeft + 350, 20, status);

    // Draw ALL 64 channels with precise positioning
    for (int ch = 0; ch < channelCount; ch++) {
        // Use floating point for precise positioning
        int yOffset = marginTop + (int)(ch * exactChannelHeight);
        int nextYOffset = marginTop + (int)((ch + 1) * exactChannelHeight);
        int chHeight = nextYOffset - yOffset;

        // Ensure minimum height of 3 pixels
        if (chHeight < 3) {
            chHeight = 3;
        }

        QRect channelArea(marginLeft, yOffset, plotWidth, chHeight);
        drawChannel(painter, ch, channelArea);
    }

    // Draw border
    painter.setPen(QPen(QColor(100, 116, 139), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(marginLeft, marginTop, plotWidth, totalHeight);
}

void RingBufferPlot::drawChannel(QPainter& painter, int channelIndex, const QRect& channelArea) {
    if (channelIndex >= channelCount) return;

    const QVector<double>& dataBuffer = channelBuffers[channelIndex];

    int marginLeft = channelArea.left();
    int plotWidth = channelArea.width();
    int chHeight = channelArea.height();
    int chTop = channelArea.top();
    int chBottom = channelArea.bottom();
    int chCenter = chTop + chHeight / 2;

    // Draw channel separator line
    painter.setPen(QPen(QColor(51, 65, 85), 1));
    painter.drawLine(marginLeft, chBottom, marginLeft + plotWidth, chBottom);

    // Draw channel label
    painter.setPen(QColor(100, 116, 139));
    QFont labelFont = painter.font();
    labelFont.setPointSize(7);
    painter.setFont(labelFont);
    painter.drawText(5, chCenter + 3, QString("Ch%1").arg(channelIndex + 1));

    // Only draw center line if recording
    if (isSimulating) {
        painter.setPen(QPen(QColor(51, 65, 85), 1, Qt::DotLine));
        painter.drawLine(marginLeft, chCenter, marginLeft + plotWidth, chCenter);
    }

    // Only draw signal if we're recording or have recorded data
    if (!isSimulating && writeIndex == 0 && !bufferFilled) {
        return;  // No data at all yet
    }

    // Draw signal
    if (bufferSize > 1) {
        int gapSize = 45;
        int sweepX = marginLeft + (plotWidth * writeIndex) / bufferSize;

        painter.setPen(QPen(QColor(34, 211, 238), 1));

        // Determine what to draw based on whether buffer has been filled
        int startIdx, endIdx;

        if (!bufferFilled) {
            // First fill: only draw from 0 to writeIndex
            startIdx = 0;
            endIdx = writeIndex;
        } else {
            // Buffer filled: draw entire buffer (continuous overwrite)
            startIdx = 0;
            endIdx = bufferSize;
        }

        // Draw the signal
        QPainterPath signalPath;
        bool firstPoint = true;

        for (int i = startIdx; i < endIdx; i++) {
            double value = dataBuffer[i];
            double normalizedValue = (value - yMin) / (yMax - yMin);
            normalizedValue = qBound(0.0, normalizedValue, 1.0);

            int x = marginLeft + (plotWidth * i) / bufferSize;

            // Skip drawing in the gap area
            if (x >= sweepX - gapSize/2 && x <= sweepX + gapSize/2) {
                if (!firstPoint) {
                    painter.drawPath(signalPath);
                    signalPath = QPainterPath();
                    firstPoint = true;
                }
                continue;
            }

            int y = chTop + chHeight - (int)(normalizedValue * chHeight);

            if (firstPoint) {
                signalPath.moveTo(x, y);
                firstPoint = false;
            } else {
                signalPath.lineTo(x, y);
            }
        }

        if (!firstPoint) {
            painter.drawPath(signalPath);
        }
    }
}

// ============================================================
// MainWindow Implementation
// ============================================================

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {

    setWindowTitle("ALDA Medical - 64-Channel EEG System");

    // Larger window for better 64-channel viewing
    resize(1400, 1000);  // Increased height for clearer channel display

    setupUI();
    applyModernStyles();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // ====== CONTROL BUTTONS AT TOP ======
    QHBoxLayout *plotControlLayout = new QHBoxLayout();
    plotControlLayout->setSpacing(12);

    startPlotButton = new QPushButton("▶ Start Recording", this);
    startPlotButton->setObjectName("plotButton");
    startPlotButton->setCursor(Qt::PointingHandCursor);
    startPlotButton->setMinimumHeight(45);
    plotControlLayout->addWidget(startPlotButton);

    stopPlotButton = new QPushButton("⏸ Stop Recording", this);
    stopPlotButton->setObjectName("plotButton");
    stopPlotButton->setCursor(Qt::PointingHandCursor);
    stopPlotButton->setMinimumHeight(45);
    stopPlotButton->setEnabled(false);
    plotControlLayout->addWidget(stopPlotButton);

    clearPlotButton = new QPushButton("⌫ Clear Data", this);
    clearPlotButton->setObjectName("plotButtonSecondary");
    clearPlotButton->setCursor(Qt::PointingHandCursor);
    clearPlotButton->setMinimumHeight(45);
    plotControlLayout->addWidget(clearPlotButton);

    plotControlLayout->addStretch();  // Push buttons to the left

    mainLayout->addLayout(plotControlLayout);

    // ====== OSCILLOSCOPE CHART (FILLS REMAINING SPACE) ======
    // Create the oscilloscope plot with 64 channels
    oscilloscopePlot = new RingBufferPlot(this);
    oscilloscopePlot->setChannelCount(64);      // 64 channels
    // Channel height is calculated dynamically based on window size!
    oscilloscopePlot->setBufferSize(500);       // 500 samples
    oscilloscopePlot->setYRange(-50.0, 50.0);   // ±50 µV range for noise
    oscilloscopePlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(oscilloscopePlot);

    connect(startPlotButton, &QPushButton::clicked, this, &MainWindow::onStartPlotClicked);
    connect(stopPlotButton, &QPushButton::clicked, this, &MainWindow::onStopPlotClicked);
    connect(clearPlotButton, &QPushButton::clicked, this, &MainWindow::onClearPlotClicked);
}

void MainWindow::applyModernStyles() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f8fafc;
        }

        QWidget#centralWidget {
            background-color: #f8fafc;
        }

        QPushButton#plotButton {
            background-color: #2563eb;
            color: white;
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton#plotButton:hover {
            background-color: #1d4ed8;
        }

        QPushButton#plotButton:pressed {
            background-color: #1e40af;
        }

        QPushButton#plotButton:disabled {
            background-color: #94a3b8;
            color: #cbd5e1;
        }

        QPushButton#plotButtonSecondary {
            background-color: #64748b;
            color: white;
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton#plotButtonSecondary:hover {
            background-color: #475569;
        }

        QPushButton#plotButtonSecondary:pressed {
            background-color: #334155;
        }
    )");
}

void MainWindow::onStartPlotClicked() {
    oscilloscopePlot->startSimulation();
    startPlotButton->setEnabled(false);
    stopPlotButton->setEnabled(true);
}

void MainWindow::onStopPlotClicked() {
    oscilloscopePlot->stopSimulation();
    startPlotButton->setEnabled(true);
    stopPlotButton->setEnabled(false);
}

void MainWindow::onClearPlotClicked() {
    oscilloscopePlot->clearData();
}
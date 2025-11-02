#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QVector>

// RingBufferPlot - Multi-channel oscilloscope display widget
class RingBufferPlot : public QWidget {
    Q_OBJECT

public:
    explicit RingBufferPlot(QWidget *parent = nullptr);

    void addDataPoint(double value);

    void setBufferSize(int size);
    void setYRange(double min, double max);

    void setUpdateInterval(int msec);

    void addMultiChannelData(const QVector<double>& values);  // Add data for all channels at once
    void setChannelCount(int count);                           // Set number of channels (e.g., 64)
    void setChannelHeight(int height);                         // Set height per channel in pixels
    void clearData();

    void startSimulation();
    void stopSimulation();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
            void simulateData();

private:
    QVector<QVector<double>> channelBuffers;  // Multi-channel buffers
    int channelCount;                          // Number of channels
    int channelHeight;                         // Height allocated per channel
    int bufferSize;
    int writeIndex;
    bool bufferFilled;                         // NEW: Track if buffer has wrapped around
    double yMin;
    double yMax;
    QTimer *updateTimer;
    QTimer *simulationTimer;
    bool isSimulating;

    void drawChannel(QPainter& painter, int channelIndex, const QRect& channelArea);
};

// MainWindow
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartPlotClicked();
    void onStopPlotClicked();
    void onClearPlotClicked();

private:
    // Oscilloscope controls
    QPushButton *startPlotButton;
    QPushButton *stopPlotButton;
    QPushButton *clearPlotButton;
    RingBufferPlot *oscilloscopePlot;

    void setupUI();
    void applyModernStyles();
    void fixDropdownContainer();
};

#endif // MAINWINDOW_H
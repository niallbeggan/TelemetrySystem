#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QMessageBox>
#include <serialportthread.h>
#include <QColor>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Main Tab
    void updateMainTemp(int temp);
    void updateMainVoltage(int voltage);
    void updateMainSpeed(int speed);
    void updateMainRunningTime();
    void updateMainPower(int power);

    void updateMainTab(int temp, int voltage, int speed, int power);

    // Graphs
    void clearData();
    void addPointsToGraph(QVector<QVector<double>> &graph, double x, double y);
    void plotGraphs();

    // Suspension
    void frontLeftPlot();
    void rearLeftPlot();
    void frontRightPlot();
    void rearRightPlot();

    // Battery
    void updateBatteryTab(QString voltage, QString current, QString temp);
    void batteryTempPlot();
    void batteryCurrentPlot();
    void batteryVoltagePlot();
    void plotBatteryGraphs();

    // Pedals
    void updatePedalTab(QString brake, QString accelerator);
    void pedalBrakePlot();
    void pedalAcceleratorPlot();
    void plotPedalGraph();

private slots:
    void scanSerialPorts();
    void on_refreshPorts_clicked();
    void on_startComms_clicked();
    void on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber);
    void updateGUI(QStringList sensors);
    void on_endComms_clicked();
    void clearComboBox();
    void showStartComms();
    void showEndComms();

private:
    Ui::MainWindow *ui;
    QVector <double> suspensionLeftFrontX, suspensionLeftFrontY;
    QVector <double> suspensionRightFrontX, suspensionRightFrontY;
    QVector <double> suspensionLeftRearX, suspensionLeftRearY;
    QVector <double> suspensionRightRearX, suspensionRightRearY;

    //Try to make QVector of QVectors, one for each graph
    QVector <QVector<double>> suspensionLeftFront;
    QVector <QVector<double>> suspensionRightFront;
    QVector <QVector<double>> suspensionLeftRear;
    QVector <QVector<double>> suspensionRightRear;

    // Battery plots
    QVector <QVector<double>> batteryTemp;
    QVector <QVector<double>> batteryVoltage;
    QVector <QVector<double>> batteryCurrent;
    double highestCurrent;
    double lowestVoltage;

    // Pedal position plots
    QVector <QVector<double>> brakePedal;
    QVector <QVector<double>> acceleratorPedal;

    // Running time
    QElapsedTimer runningTime;

signals:
    void startComms();
    void updateFromComboBox(QString PortDescriptionAndNumber);
    void closeComms();
    void timestamp(int millis, int seconds, int minutes, int hours);
};

#endif // MAINWINDOW_H

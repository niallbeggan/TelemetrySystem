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
    void updateMainPower(int power);
    void updateMainRunningTime();
    void updateMainGPSStatus(int noOfSatellites);
    void updateRadioStatus(int signalStrength);
    void updateCANStatus(int CANMessageReceived);
    void updateTempStatus(float temp);

    void updateMainTab(double temp, double voltage, double speed, double power);

    // Graphs
    void clearData(QVector <QVector<double>> &plotVectors);
    void addPointsToGraphVector(QVector<QVector<double>> &graphVector, double x, double y);
    void plotSuspensionGraphs();

    // Suspension
    void frontLeftPlot();
    void rearLeftPlot();
    void frontRightPlot();
    void rearRightPlot();

    // Battery
    void updateBatteryTab(double voltage, double current, double temp);
    void batteryTempPlot();
    void batteryCurrentPlot();
    void batteryVoltagePlot();
    void plotBatteryGraphs();

    // Pedals
    void updatePedalTab(double brake, double brakeTimestamp, double accelerator, double acceleratorTimestamp);
    void pedalBrakePlot();
    void pedalAcceleratorPlot();
    void plotPedalGraph();

    // Motor & Steering
    void updateMotorAndSteeringTab(double leftMotorVoltage, double leftMotorCurrent, double leftMotorCurrentTimestamp,
                                   double rightMotorVoltage, double rightMotorCurrent, double rightMotorCurrentTimestamp,
                                   double steeringInput, double steeringInputTimestamp);
    void motorDifferentialPlot();
    void steeringInputPlot();

private slots:
    void scanSerialPorts();
    void on_refreshPorts_clicked();
    void on_startComms_clicked();
    void on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber);
    void updateGUI(QVector <double> signalVector, QVector <double> timestampVector);
    void on_endComms_clicked();
    void clearComboBox();
    void showStartComms();
    void showEndComms();
    void showMessageBox(int type);
private:
    Ui::MainWindow *ui;

    QVector <double> usedForInitialisationOnly;
    //QVector of double QVectors, one for each suspension graph
    QVector <QVector<double>> suspensionLeftFront;
    QVector <QVector<double>> suspensionRightFront;
    QVector <QVector<double>> suspensionLeftRear;
    QVector <QVector<double>> suspensionRightRear;

    // Battery plots
    QVector <QVector<double>> batteryTemp;
    QVector <QVector<double>> batteryTempLimit;
    QVector <QVector<double>> batteryVoltage;
    QVector <QVector<double>> batteryCurrent;
    double highestCurrent;
    double lowestVoltage;

    // Pedal position plots
    QVector <QVector<double>> brakePedal;
    QVector <QVector<double>> acceleratorPedal;

    // Motor plot
    QVector <QVector<double>> motorDifferentialPower;

    // Steering plot
    QVector <QVector<double>> steeringInputPercent;

    // Running time
    QElapsedTimer runningTime;
    bool stopClicked; // Used to reset menu bar

signals:
    void startComms();
    void updateFromComboBox(QString PortDescriptionAndNumber);
    void closeComms();
    void timestamp(int millis, int seconds, int minutes, int hours);
};

#endif // MAINWINDOW_H

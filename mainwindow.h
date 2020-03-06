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

    void updateMainTab(int temp, int voltage, int speed);

    // Graphs
    void clearData();
    void frontLeftPlot();
    void rearLeftPlot();
    void frontRightPlot();
    void rearRightPlot();
    void addPointsToGraph(QVector<QVector<double>> &graph, double x, double y);
    void plotGraphs();
    void updatePowerLCD(int voltage, int current);

    // Battery
    void updateBatteryTab(QString voltage, QString current);

private slots:
    void scanSerialPorts();
    void on_refreshPorts_clicked();
    void on_startComms_clicked();
    void on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber);
    void updateGUI(QString msg);
    void on_endComms_clicked();
    void clearComboBox();
//    void on_clearPlot_clicked(); for a clear pot button i removed
    void on_pushButton_clicked();
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

signals:
    void startComms();
    void updateFromComboBox(QString PortDescriptionAndNumber);
    void closeComms();
};

#endif // MAINWINDOW_H

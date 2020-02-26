#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QMessageBox>
#include <serialportthread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //Graph
    void addPoint(double x, double y);
    void addPoint2Graph(double x, double y);
    void clearData();
    void plot();
    void plotGraph2();

private slots:
    void scanSerialPorts();
    void on_refreshPorts_clicked();
    void on_startComms_clicked();
    void on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber);
    void updateTextEdit(QString msg);
    void on_endComms_clicked();
    void clearComboBox();
//    void on_clearPlot_clicked(); for a clear pot button i removed
    void on_pushButton_clicked();
    void showStartComms();
    void showEndComms();

private:
    Ui::MainWindow *ui;
    QVector <double> qv_x, qv_y;
    QVector <double> qv_x2, qv_y2;
signals:
    void startComms();
    void updateFromComboBox(QString PortDescriptionAndNumber);
    void closeComms();
};

#endif // MAINWINDOW_H

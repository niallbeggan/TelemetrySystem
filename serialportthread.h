#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <mainwindow.h>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>

class SerialPortThread : public QObject
{
    Q_OBJECT

public:
    SerialPortThread();
    ~SerialPortThread();

private slots:
    void handleError(QSerialPort::SerialPortError errorSent);
    void sendDataToGUISlot();
    void selectPortFromComboBoxClick(QString PortDescriptionAndNumber);
    void startComms();
    void endCommsFromGUI();
    void telemRequestDataTimer();
    void updateTimestamp(int millis, int seconds, int minutes, int hours);

private:
    int closeComms(QSerialPort* &port);
    QSerialPort *TelemSerialPort;
    QString portNumber;
    QTimer * requestTimer;
    QString filename = "Data.txt";
    QMutex serialMutex;
    int serialErrorTimeoutCount;
    int milli, second, minute, hour;
    bool stopComms;

signals:
    void sendDataToGUI(QVector<double>, QVector<double>);
    void clearComboBox();
    void scanSerialPorts();
    void startTelem();
    void showStartComms();
    void showEndComms();
    void msgBoxSignal(int type);
};

#endif // SERIALPORTTHREAD_H

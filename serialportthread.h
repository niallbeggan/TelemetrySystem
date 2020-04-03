#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QMainWindow>
#include <QObject>
//#include <QQuickItem>
#include <QWidget>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <mainwindow.h>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>

class SerialPortThread : public QThread
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

    int milli, second, minute, hour;
signals:
    void sendDataToGUI(QStringList sensorValueList);
    void clearComboBox();
    void scanSerialPorts();
    void startTelem();
    void showStartComms();
    void showEndComms();
};

#endif // SERIALPORTTHREAD_H

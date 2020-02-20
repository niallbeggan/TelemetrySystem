#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QMainWindow>
#include <QObject>
//#include <QQuickItem>
#include <QWidget>
#include <QThread>
#include <QTimer>
#include <mainwindow.h>

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
private:
    int closeComms(QSerialPort* &port);
    QSerialPort *TelemSerialPort;
    QString portNumber;

signals:
    void sendDataToGUI(QString msg);
    void clearComboBox();
    void scanSerialPorts();
    void startTelem();
};

#endif // SERIALPORTTHREAD_H

//void MyObject::startWorkInAThread()
//{
//    WorkerThread *workerThread = new WorkerThread(this);
//    connect(workerThread, &WorkerThread::resultReady, this, &MyObject::handleResults);
//    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
//    workerThread->start();
//}

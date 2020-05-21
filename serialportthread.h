/***************************************************************************
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the MIT License                                 **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  **
**                                                                        **
**                                                                        **
****************************************************************************
**           Author: Niall Beggan                                         **
**           Contact: niallbeggan@gmail.com                               **
**           Date: 20.5.2020                                              **
**           Version: 1.0.0                                               **
****************************************************************************/

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
    float take8ByteArrayReturnSignal(QByteArray messageArray);
    double take8ByteArrayReturnTimestamp(QByteArray messageArray);

private slots:
    void handleError(QSerialPort::SerialPortError errorSent);
    void sendDataToGUISlot();
    void selectPortFromComboBoxClick(QString PortDescriptionAndNumber);
    void startComms();
    void endCommsFromGUI();
    void telemRequestDataTimer();
    void updateRunTime(int millis, int seconds, int minutes, int hours);

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

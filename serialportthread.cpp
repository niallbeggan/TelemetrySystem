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

#include "serialportthread.h"
#include <QDesktopServices>
#include <QStandardPaths>
#include <QCoreApplication>

#define REQUEST_TIME_MS 200
#define WAIT_FOR_REPLY_TIME 165 // Round trip time = 168-179mS @144 bytes. // 100mS @42 bytes
#define PACKET_SIZE_BYTES 144

SerialPortThread::SerialPortThread() {
    qRegisterMetaType<QVector<double> >("QVector<double>"); // Need to do this to send QVector <double> signals
    portNumber = ""; // Initiliase variables
    serialErrorTimeoutCount = 0;
    TelemSerialPort = NULL;
    connect(this, SIGNAL(startTelem()), this, SLOT(telemRequestDataTimer()), Qt::QueuedConnection);

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation); // Log files created here

    QString time_format = "yyyy-MM-dd__HH-mm-ss";
    QDateTime a = QDateTime::currentDateTime();
    QString as = a.toString(time_format);
    qDebug() << as;
    QString telem = "_Telemetry_Data.txt";

    filename = as + telem;
    qDebug() << "Filename: " << filename;
    filename = dataPath + "/" + filename;
    qDebug() << "Filename: " << filename;
    requestTimer = new QTimer(this);
    connect(requestTimer, SIGNAL(timeout()), SLOT(sendDataToGUISlot()));
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
          QTextStream stream(&file);
          stream << "EStop\t"
                 <<"UTC_Timestamp\t"
                 << "BMS Temperature (C)\t"
                 <<"UTC_Timestamp\t"
                 << "Car speed (kmph)\t"
                 <<"UTC_Timestamp\t"
                 << "BMS Voltage (V)\t"
                 <<"UTC_Timestamp\t"
                 << "BMS Current (A)\t"
                 <<"UTC_Timestamp\t"
                 << "Power (kW)\t"
                 <<"UTC_Timestamp\t"
                 << "Left Motor Voltage (V)\t"
                 <<"UTC_Timestamp\t"
                 << "Right Motor Voltage (A)\t"
                 <<"UTC_Timestamp\t"
                 << "Left Motor Current (V)\t"
                 <<"UTC_Timestamp\t"
                 << "Right Motor Current (A)\t"
                 <<"UTC_Timestamp\t"
                 << "Steering input (%)\t"
                 <<"UTC_Timestamp\t"
                 << "Acceleration pedal (%)\t"
                 <<"UTC_Timestamp\t"
                 << "Brake pedal (%)\t"
                 <<"UTC_Timestamp\t"
                 << "Left front suspension\t"
                 <<"UTC_Timestamp\t"
                 << "Right front suspension\t"
                 <<"UTC_Timestamp\t"
                 << "Left back suspension\t"
                 <<"UTC_Timestamp\t"
                 << "Right back suspension\t"
                 <<"UTC_Timestamp\t"
                 << "Number of Satellites\t"
                 <<"UTC_Timestamp";
          stream << "\n";
          file.close();
    }
}

SerialPortThread::~SerialPortThread() {
    closeComms(TelemSerialPort);
    delete requestTimer;
}

float SerialPortThread::take8ByteArrayReturnSignal(QByteArray messageArray) {
    // declare variables
    float signalValue = -100; // Could be neg or pos
    // Get signal
    int bigByte = static_cast < char > (messageArray[1]);                // This carries the int's sign
    int smallByte = static_cast < unsigned char > (messageArray[0]);   // this doesnt
    if(bigByte < 0)                                               // If neg change sign of small byte
        signalValue = (bigByte * 256) - smallByte;                // Add (-small) to big to get total neg value
    signalValue = (bigByte * 256) + smallByte;                    // If positive, add to get total pos value
    signalValue = signalValue/10;
    return signalValue;
}

double SerialPortThread::take8ByteArrayReturnTimestamp(QByteArray messageArray) {
    double timestampSeconds = 0; // Time of day in seconds
    double timestampMillis = 0; // Milliseconds between seconds
    double timestamp = 0;
    // get timestamp
    int x = 0;
    for (int z = 2; z < 6; z = z + 1) {
        timestampSeconds += static_cast < unsigned char > (messageArray[z]) <<(x*8);
        x = x + 1;
    }
    timestampSeconds += static_cast < char > (messageArray[5]) <<(x*8);

    int bigByte = static_cast < char > (messageArray[7]);             // this carries the int's sign
    int smallByte = static_cast < unsigned char > (messageArray[6]); // this doesnt
    timestampMillis = (bigByte * 256) + smallByte;                   // Add small to big to get total value
    timestampMillis = (timestampMillis/1000); // Divide by 1000 to convert to seconds
    timestamp = timestampSeconds + timestampMillis;
    return timestamp;
}

void SerialPortThread::startComms() {
    stopComms = false;
    serialErrorTimeoutCount = 0; // Reset serial port reset counter every time startcomms is clicked.
    int found = 0;
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        if(portNumber == port.portName()) {
            found = 1;
            qDebug() << __FILE__ << __LINE__ << port.portName();
        }
    }
    if(found == 0) {
        portNumber = "";
        emit scanSerialPorts();
    }
    if(portNumber != "") {
        if(TelemSerialPort == NULL) {
            TelemSerialPort = new QSerialPort();
            connect(TelemSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
            //connect(TelemSerialPort, SIGNAL(readyRead()), SLOT(sendDataToGUISlot())); Going with different method.
            qDebug() << __FILE__ << __LINE__ << "Creating new TelemSerialPort";
        }
        if(TelemSerialPort->isOpen() != true) {
            qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "Opening port";
            TelemSerialPort->setPortName(portNumber);
            TelemSerialPort->setParity(QSerialPort::NoParity);
            TelemSerialPort->setBaudRate(QSerialPort::Baud57600, QSerialPort::AllDirections);
            TelemSerialPort->setStopBits(QSerialPort::OneStop);
            TelemSerialPort->setFlowControl(QSerialPort::NoFlowControl);
            TelemSerialPort->open(QIODevice::ReadWrite);
            emit startTelem();
            emit showEndComms();
        }
    }
    else {
        emit msgBoxSignal(1);
    }
}

void SerialPortThread::handleError(QSerialPort::SerialPortError errorSent) {
    serialErrorTimeoutCount += 1;
    qDebug() << __FILE__ << __LINE__ << errorSent;
    int err = 0;
    if(errorSent == QSerialPort::NoError)
        err = 0;
    else if((errorSent == QSerialPort::NotOpenError)
        || (errorSent == QSerialPort::DeviceNotFoundError)) // These occur when USB suddenly unplugged at bad times.
        err = 1;
    else
        err = 1;
    if(serialErrorTimeoutCount > 5) // If stuck in loop, force shutdown. serialErrorTimeoutCount reset every time startComms clicked.
        err = 1;

    switch (err) {
    case 0:
        // Don't do anything. This is sent whenever the port is actually opened properly. NOT AN ERROR.
        break;
    case 1: // If you remove the usb quickly after starting comms prog will crash
        if((TelemSerialPort != NULL) && (stopComms == false)) {
            qDebug() << __FILE__ << __LINE__ << "stopComms == true.";
            stopComms = true;
            qDebug() << __FILE__ << __LINE__ << "Closing serial port due to error!";
            emit msgBoxSignal(2);
        }
        break;
    default:
        // For now, just close everything for any other error
        if((TelemSerialPort != NULL) && (stopComms == false)) {
            qDebug() << __FILE__ << __LINE__ << "stopComms == true.";
            stopComms = true;
            emit msgBoxSignal(2);
            qDebug() << __FILE__ << __LINE__ << "Closing serial port due to error!";
        }
        break;
    }
}

int SerialPortThread::closeComms(QSerialPort* &port) {
    if(requestTimer) {
        if(requestTimer->isActive() == true)
            requestTimer->stop(); // Ends requests
    }
    if((port != NULL) && (portNumber != "")) {
        if(port->isOpen())
            qDebug() << __FILE__ << __LINE__ << "Port is open";
            port->close();
            qDebug() << __FILE__ << __LINE__ << "Port is closed";
        if(port->isOpen()) {
            qDebug() << __FILE__ << __LINE__ << "FAILED TO CLOSE PORT!";
        }
        delete port;
        port = NULL;
        portNumber = ""; // Delete the portnumber
        qDebug() << __FILE__ << __LINE__ << "Serial port is closed and deleted!";
        emit clearComboBox();
        emit showStartComms();
        return 0;
    }
    portNumber = ""; // Without this portnumber can be assigned even with a cleared combo box
    qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "Serial port was already closed and deleted!";
    emit clearComboBox();
    emit showStartComms();
    return 0;
}

void SerialPortThread::sendDataToGUISlot() {
    if(stopComms == true) {
        qDebug() << __FILE__ << __LINE__ << "Calling closeComms.";
        closeComms(TelemSerialPort);
    }
    QStringList sensors;
    QVector <double> signalVector, timestampVector;
    //QElapsedTimer propagationDelayTestTimer; // UNCOMMENT FOR DELAY TEST
    //propagationDelayTestTimer.start(); // UNCOMMENT FOR DELAY TEST
    //double sendTime = 0; // UNCOMMENT FOR DELAY TEST
    //double receiveTime = 0; // UNCOMMENT FOR DELAY TEST
    if(TelemSerialPort != NULL) {
        if(TelemSerialPort->isOpen()) {
            //sendTime = propagationDelayTestTimer.elapsed(); // UNCOMMENT FOR DELAY TEST
            TelemSerialPort->write("1");
            TelemSerialPort->flush(); // This is actually needed. Otherwise request not sent until later.
            QByteArray datas;
            int loop = 0;
            QThread::msleep(WAIT_FOR_REPLY_TIME);
            while((TelemSerialPort->bytesAvailable() < 143) && (loop < 8)) { // If no full packet after 189mS move on. (normally 165-179mS with dummy.ino examples.Might be slower when reading CAN bus.
                QThread::msleep(3);
                loop++;
                QCoreApplication::processEvents(); // Need this to update bytesAvailable otherwise never gets connected to main thread
            }
            //receiveTime = propagationDelayTestTimer.elapsed(); // UNCOMMENT FOR DELAY TEST

            datas = TelemSerialPort->readAll(); // Need to emit an empty signal here if dont get full msg
            float signal = -1;
            double timestamp = -1;
            qDebug() << "datas.size()" << datas.size();
            if(datas.size() == PACKET_SIZE_BYTES) {
                for(int i = 0; i < datas.size() - 7; i = i + 8) {
                    QByteArray CAN_Message = datas.mid(i, 8);
                    signal = take8ByteArrayReturnSignal(CAN_Message);
                    timestamp = take8ByteArrayReturnTimestamp(CAN_Message);
                    // add to vector
                    timestampVector.append(timestamp);
                    signalVector.append(signal);
                    // loop
                }
                // Debug/Check section //
                //qDebug() << "sendTime: " << sendTime; // UNCOMMENT FOR DELAY TEST
                //qDebug() << "receiveTime :"<< receiveTime; // UNCOMMENT FOR DELAY TEST

                //qDebug() << "Time of day from GPS :" <<  (UTC_time_seconds/3600) <<":" << ((UTC_time_seconds%3600)/60) << ":" << UTC_time_seconds%60;

                //qDebug() << "signalVector :" << signalVector; //TMP
                //qDebug() << "timestampVector :" << timestampVector; //TMP

                // Send to GUI/Main thread section
                if(signalVector.length() > 17) // Full msg received
                    emit sendDataToGUI(signalVector, timestampVector);
                else {
                    for(int i = 0; i < 18; i++) {
                        signalVector.append(-1); // Send error values to GUI if somehow signalVector is too short
                        timestampVector.append(-1);
                    }
                }
            }
            else {
                for(int i = 0; i < 18; i++) { // Send error values to GUI if not enough bytes received
                    signalVector.append(-1);
                    timestampVector.append(-1);
                }
                emit sendDataToGUI(signalVector, timestampVector);
            }
            qDebug() << "signalVector :" << signalVector;
            qDebug() << "timestampVector :" << timestampVector;
            //qDebug() << QString::number(timestampVector[2], 'f', 3); // The gps speed time, to check it is correct

            QFile file(filename);  // Log to text file
            if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                  QTextStream stream(&file);
                  for(int i = 0; i < timestampVector.length(); i++) {
                      stream << QString::number(signalVector[i], 'f', 1); // Print to 1 decimal place
                      stream << "\t";
                      stream << QString::number(timestampVector[i], 'f', 3); // Print to 3 decimal places
                      stream << "\t";
                  }
                  stream << "\n";
                  file.close();
            }
            signalVector.clear();
            timestampVector.clear();
        }
        else {
            qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "OPEN ERROR: " << TelemSerialPort->errorString();
            handleError(QSerialPort::DeviceNotFoundError); // Should break loop and find errror
        }
    }
}

void SerialPortThread::selectPortFromComboBoxClick(QString PortDescriptionAndNumber) {
    if(TelemSerialPort != NULL) { // Close the port opened previously if it is open
        if(requestTimer->isActive() != true) { // If comms is not runnning
            qDebug() << __FILE__ << __LINE__ << "Calling closeComms.";
            closeComms(TelemSerialPort);
        }
    }
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        if(PortDescriptionAndNumber == (port.description() + " (" + port.portName() + ")")) {
            portNumber = port.portName();
            qDebug() << __FILE__ << __LINE__ << port.portName();
        }
    }
}

void SerialPortThread::endCommsFromGUI(){
    stopComms = true;
    // If this slot is called it interrupts
    // other functions using Telemserialport, deleting the pointer here will cause a crash.
    // It interrupts other functions even with a queued connection.
}

void SerialPortThread::telemRequestDataTimer() {
    QThread::msleep(1500); // Serial port needs time to be setup before it works. Otherwise first few messages fail.
    requestTimer->start(REQUEST_TIME_MS);
    qDebug() << "Starting timer";
}

void SerialPortThread::updateRunTime(int millis, int seconds, int minutes, int hours) {
    milli = millis;
    second = seconds; // Global variable assignment. Shoud find a better way
    minute = minutes;
    hour = hours;
}

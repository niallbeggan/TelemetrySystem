#include "serialportthread.h"
#include <QDesktopServices>
#include <QStandardPaths>
#include <QCoreApplication>

#define REQUEST_TIME_MS 180
#define WAIT_FOR_REPLY_TIME 162 // Round trip time = 100mS.
#define PACKET_SIZE_BYTES 144

SerialPortThread::SerialPortThread() {
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
          stream <<"UTC_Timestamp\t"
                 << "Time H:M:S.mS\t"
                 << "EStop\t"
                 << "BMS Temperature (C)\t"
                 << "Car speed (kmph)\t"
                 << "BMS Voltage (V)\t"
                 << "BMS Current (A)\t"
                 << "Power (kW)\t"
                 << "Left Motor Voltage (V)\t"
                 << "Right Motor Voltage (A)\t"
                 << "Left Motor Current (V)\t"
                 << "Right Motor Current (A)\t"
                 << "Steering input (%)\t"
                 << "Acceleration pedal (%)\t"
                 << "Brake pedal (%)\t"
                 << "Left front suspension\t"
                 << "Right front suspension\t"
                 << "Left back suspension\t"
                 << "Right back suspension";
          stream << "\n";
          file.close();
    }
}

SerialPortThread::~SerialPortThread() {
    closeComms(TelemSerialPort);
    delete requestTimer;
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
    qDebug() << errorSent;
    int err = 0;
    if(errorSent == QSerialPort::NoError)
        err = 0;
    if((errorSent == QSerialPort::NoError) // These all occur on the 'first pass' when a port is opened. Need to engineer around this.
            || (errorSent == QSerialPort::NotOpenError)
            || (errorSent == QSerialPort::DeviceNotFoundError)) // These occur when USB suddenly unplugged at bad times.
        err = 1;
    else
        err = 2;
    if(serialErrorTimeoutCount > 5) // If stuck in loop, force shutdown. serialErrorTimeoutCount reset every time startComms clicked.
        err = 2;

    switch (err) {
    case 1:
        // Don't do anything. This is sent whenever the port is actually opened properly. NOT AN ERROR.
        break;
    case 2: // If you remove the usb quickly after starting comms prog will crash
        if((TelemSerialPort != NULL) && (portNumber != "")) {
            delete TelemSerialPort;
            TelemSerialPort = NULL;
            portNumber = ""; //delete the portnumber
            qDebug() << __FILE__ << __LINE__ << "Serial port is closed and deleted!";
        }
        else
            qDebug() << __FILE__ << __LINE__ << "Serial port was already closed and deleted!";
        emit msgBoxSignal(2);
        qDebug() << __FILE__ << __LINE__ << errorSent;
        emit clearComboBox();
        break;
    default:
        // For now, just close everything for any error
        closeComms(TelemSerialPort);
        emit msgBoxSignal(2);
        qDebug() << __FILE__ << __LINE__ << errorSent;
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
        closeComms(TelemSerialPort);
    }
    QString msg = "";
    QStringList sensors;
    QVector <double> signalVector, timeStampSecondsVector, timeStampMillisVector;
    //QElapsedTimer propagationDelayTestTimer;
    //propagationDelayTestTimer.start();
    //double sendTime = 0;
    //double receiveTime = 0;
    if(TelemSerialPort != NULL) {
        if(TelemSerialPort->isOpen()) {
            //sendTime = propagationDelayTestTimer.elapsed();
            TelemSerialPort->write("1");
            TelemSerialPort->flush(); // This is actually needed. Otherwise not sent until function returns.
            QByteArray datas;
            int loop = 0;
            QThread::msleep(WAIT_FOR_REPLY_TIME);
            while((TelemSerialPort->bytesAvailable() < 138) && (loop < 5)) {
                QThread::msleep(5);
                loop++;
                QCoreApplication::processEvents(); // Need this to update bytesAvailable otherwise never gets connected to main thread
            }
            //receiveTime = propagationDelayTestTimer.elapsed();

            datas = TelemSerialPort->readAll(); // Need to emit an empty signal here if dont get full msg
            if(datas.size() == PACKET_SIZE_BYTES) {
                for(int i = 0; i < datas.size() - 7; i = i + 8) {
                    // Get signal
                    int bigByte = static_cast < char > (datas[i]);                // This carries the int's sign
                    int smallByte = static_cast < unsigned char > (datas[i+1]);   // this doesnt
                    float signalValue = -100;                                     // Could be neg or pos
                    if(bigByte < 0)                                               // If neg change sign of small byte
                        signalValue = (bigByte * 256) - smallByte;                // Add (-small) to big to get total neg value
                    signalValue = (bigByte * 256) + smallByte;                    // If positive, add to get total pos value
                    signalValue = signalValue/10;
                    msg += QString::number(signalValue) + ",";
                    // get timestamp
                    QByteArray UTC;
                    int x = 0;
                    double UTC_time_seconds = 0; // Time of day in seconds
                    double UTC_millis = 0; // Milliseconds between seconds
                    for (int z = i+2; z < (i+6); z = z + 1) {
                        UTC += datas[z];
                        UTC_time_seconds += static_cast < unsigned char > (UTC[x]) <<(x*8);
                        x = x + 1;
                    }
                    for (int z = i+6; z < (i+7); z = z + 1) {
                        int bigByte = static_cast < char > (datas[z]);             // this carries the int's sign
                        int smallByte = static_cast < unsigned char > (datas[z+1]); // this doesnt
                        UTC_millis = (bigByte * 256) + smallByte;                   // Add small to big to get total value
                        UTC_millis = (UTC_millis/10); // Divide by ten to get milliseconds
                    }
                    // add to vector
                    timeStampSecondsVector.append(UTC_time_seconds);
                    timeStampMillisVector.append(UTC_millis);
                    signalVector.append(signalValue);
                    // loop
                }
                // Debug/Check section
                sensors = msg.split(",");
                qDebug() << sensors;
                //qDebug() << "sendTime: " << sendTime; // Propagation test debugs
                //qDebug() << "arduino receive :"<< sensors[0];
                //qDebug() << "arduino send" << sensors[1];
                //qDebug() << "receiveTime :"<< receiveTime;
                //qDebug() << "Time of day from GPS :" <<  (UTC_time_seconds/3600) <<":" << ((UTC_time_seconds%3600)/60) << ":" << UTC_time_seconds%60;

                qDebug() << "timeStampSecondsVector" << timeStampSecondsVector;
                qDebug() << "timeStampMillisVector" << timeStampMillisVector;

                if(sensors.length() > 18) //Full msg received
                    emit sendDataToGUI(sensors);
            }
            else {
                msg = "-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100";
                sensors = msg.split(",");
                emit sendDataToGUI(sensors);
            }
            QFile file(filename);  // Log to text file
            if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                  QTextStream stream(&file);
                  for(int i = 0; i < timeStampSecondsVector.length(); i++) {
                      stream << signalVector[i] << "\t";
                      stream << timeStampSecondsVector[i] << ".";
                      stream << timeStampMillisVector[i] << "\t";
                  }
                  stream << "\n";
                  file.close();
            }
            signalVector.clear();
            timeStampMillisVector.clear();
            timeStampSecondsVector.clear();
        }
        else {
            qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "OPEN ERROR: " << TelemSerialPort->errorString();
            handleError(QSerialPort::DeviceNotFoundError); // should break loopand find errror
        }
    }
}

void SerialPortThread::selectPortFromComboBoxClick(QString PortDescriptionAndNumber) {
    if(TelemSerialPort != NULL) { //Close the port opened previously if it is open
        closeComms(TelemSerialPort);
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
    requestTimer->start(REQUEST_TIME_MS);
    qDebug() << "Starting timer";
}

void SerialPortThread::updateTimestamp(int millis, int seconds, int minutes, int hours) {
    milli = millis;
    second = seconds; // Global variable assignment. Shoud find a better way
    minute = minutes;
    hour = hours;
}

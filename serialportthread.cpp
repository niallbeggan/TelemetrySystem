#include "serialportthread.h"

#define REQUEST_TIME_MS 150
#define WAIT_FOR_REPLY_TIME 5

SerialPortThread::SerialPortThread() {
    portNumber = ""; // Initiliase variables
    serialErrorTimeoutCount = 0;
    TelemSerialPort = NULL;
    connect(this, SIGNAL(startTelem()), this, SLOT(telemRequestDataTimer()));

    QString time_format = "yyyy_MM_dd_HH-mm-ss";
    QDateTime a = QDateTime::currentDateTime();
    QString as = a.toString(time_format);
    qDebug() << as;
    QString telem = "_Telemetry_Data.txt";

    filename = as + telem;
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
    serialErrorTimeoutCount = 0; // Reset serial port hard reset counter every time startcomms is clicked.
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
            || (errorSent == QSerialPort::DeviceNotFoundError)) //These occur when USB suddenly unplugged at bad times.
        err = 1;
    else
        err = 2;
    if(serialErrorTimeoutCount > 9) // If stuck in loop, force shutdown. serialErrorTimeoutCount reset every time startComms clicked.
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
            requestTimer->stop();//ends requests
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
        portNumber = ""; //delete the portnumber
        qDebug() << __FILE__ << __LINE__ << "Serial port is closed and deleted!";
        emit clearComboBox();
        emit showStartComms();
        return 0;
    }
    portNumber = ""; //without this portnumber can be assigned even with a cleared combo box
    qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "Serial port was already closed and deleted!";
    emit clearComboBox();
    emit showStartComms();
    return 0;
}

void SerialPortThread::sendDataToGUISlot() {
    QString msg = "";
    QStringList sensors;
    if(TelemSerialPort != NULL) {
        if(TelemSerialPort->isOpen()) {
            TelemSerialPort->write("1");
            QByteArray datas;
            QThread::msleep(WAIT_FOR_REPLY_TIME);
            int store_I = 0; // Needed to parse UTC timestamp
            datas = TelemSerialPort->readAll(); // Need to emit an empty signal here if dont get full msg
            for (int i = 0; i < datas.size()-5; i = i + 2) {                  // datas.size - 1 because we advance 2 bytes at a time.
                int bigByte = static_cast < char > (datas[i]);                //this carries the int's sign
                int smallByte = static_cast < unsigned char > (datas[i+1]);   // this doesnt
                float signalValue = -100;                                   // Could be neg or pos
                if(bigByte < 0)                                                   // If neg change sign of small byte
                    signalValue = (bigByte * 256) - smallByte;                // Add (-small) to big to get total neg value
                signalValue = (bigByte * 256) + smallByte;                    // If positive, add to get total pos value
                signalValue = signalValue/10;
                msg += QString::number(signalValue) + ",";
                store_I = i;
            }
            // Parsing 4 byte UTC timestamp section, needed store_I for this;
            long UTC_time_seconds = 0;
            QByteArray UTC;
            int z = 0;
            for (int i = store_I+2; i < datas.size(); i = i + 1) {
                UTC += datas[i];
                UTC_time_seconds += static_cast < unsigned char > (UTC[z]) <<(z*8);
                z = z + 1;
            }
            sensors = msg.split(",");
            // qDebug() << sensors;
            if(sensors.length() > 17) //Full msg received
                emit sendDataToGUI(sensors);
            else {
                msg = "-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100";
                sensors = msg.split(",");
                emit sendDataToGUI(sensors); // Should this be shown?
            }
            QFile file(filename);
            if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                  QTextStream stream(&file);
                  stream << UTC_time_seconds << "\t" << hour << ":" << minute << ":" << second << "." << milli << "\t";
                  for (QStringList::Iterator it = sensors.begin(); it != sensors.end(); ++it)
                                  stream << *it << "\t";
                  stream << "\n";
                  file.close();
            }
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
    closeComms(TelemSerialPort);
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

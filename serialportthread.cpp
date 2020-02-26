#include "serialportthread.h"

#define REQUEST_TIME 90

SerialPortThread::SerialPortThread() {
    portNumber = ""; //Initiliase variables
    TelemSerialPort = NULL;
    connect(this, SIGNAL(startTelem()), this, SLOT(telemRequestDataTimer()));
    filename = "SuspensionData.txt";
    requestTimer = new QTimer(this);
    connect(requestTimer, SIGNAL(timeout()), SLOT(sendDataToGUISlot()));
}

SerialPortThread::~SerialPortThread() {
    closeComms(TelemSerialPort);
    delete requestTimer;
}

void SerialPortThread::startComms() {
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        if(portNumber == port.portName()) {
            break;
            qDebug() << __FILE__ << __LINE__ << port.portName();
        }
        else {
            portNumber = "";
            emit scanSerialPorts();
        }
    }
    if(portNumber != "") {
        if(TelemSerialPort == NULL) {
            TelemSerialPort = new QSerialPort();
            connect(TelemSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
            //connect(TelemSerialPort, SIGNAL(readyRead()), SLOT(sendDataToGUISlot()));
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
        QMessageBox msgWarning;
        msgWarning.setText("WARNING!\nPlease select an existing port before trying to start comms.");
        msgWarning.setIcon(QMessageBox::Warning);
        msgWarning.setWindowTitle("Caution");
        msgWarning.exec();
    }
}

void SerialPortThread::handleError(QSerialPort::SerialPortError errorSent) {
    QMessageBox msgWarning;
    msgWarning.setText("Failure!\nUSB connection has failed! No data can be recieved.");
    msgWarning.setIcon(QMessageBox::Critical);
    msgWarning.setWindowTitle("Critical!");
    int err = 0;
    if(errorSent == QSerialPort::NoError)
        err = 0;
    if((errorSent == QSerialPort::NoError)
            || (errorSent == QSerialPort::NotOpenError)
            || (errorSent == QSerialPort::DeviceNotFoundError)) //These occur when USB suddenly unplugged at bad times.
        err = 1;
    else
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
        msgWarning.exec();
        qDebug() << __FILE__ << __LINE__ << errorSent;
        emit clearComboBox();
        break;
    default:
        // For now, just close everything for any error
        closeComms(TelemSerialPort);
        msgWarning.exec();
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
        qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "Serial port is closed and deleted!";
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
            msleep(20);
            QElapsedTimer * timeout = new QElapsedTimer;
            timeout->start();
            datas = TelemSerialPort->readLine();//need to emit an empty signal here if dont get full msg
//            while(datas.isEmpty() && timeout->elapsed()<100) {
//                datas = TelemSerialPort->readLine();
//                qDebug() << "Waiting!";
//                qDebug() << datas;
//            }
//            while(datas.size() < 14 && timeout->elapsed()<100) {
//                qDebug() << datas;
//                datas += TelemSerialPort->readLine();
//            }
//            if(datas.isEmpty())
//                msg = "-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,\n";
//            else {
                for (int i = 0; i < datas.size(); i++)
                    msg += datas[i];
//            }
            qDebug() << msg;
            sensors = msg.split(",");
            if(sensors.length() > 14) //Full msg received
                emit sendDataToGUI(msg);
            else {
                msg = "-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,-100,\n";
                sensors = msg.split(",");
                emit sendDataToGUI(msg);
            }
            QFile file(filename);
            if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                  QTextStream stream(&file);
                  stream << sensors[12] << "," << sensors[13] << "\n";
                  file.close();
            }
        }
        else {
            qDebug() << QThread::currentThreadId() << __FILE__ << __LINE__ << "OPEN ERROR: " << TelemSerialPort->errorString();
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
    requestTimer->start(90);
    qDebug() << "Starting timer";
}

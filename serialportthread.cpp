#include "serialportthread.h"

SerialPortThread::SerialPortThread() {
    portNumber = ""; //Initiliase variables
    TelemSerialPort = NULL;
    connect(this, SIGNAL(startTelem()), this, SLOT(telemRequestDataTimer()));
}

SerialPortThread::~SerialPortThread() {
    closeComms(TelemSerialPort);
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
            qDebug() << __FILE__ << __LINE__ << "Opening port";
            TelemSerialPort->setPortName(portNumber);
            TelemSerialPort->setParity(QSerialPort::NoParity);
            TelemSerialPort->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);
            TelemSerialPort->setStopBits(QSerialPort::OneStop);
            TelemSerialPort->setFlowControl(QSerialPort::NoFlowControl);
            TelemSerialPort->open(QIODevice::ReadWrite);
            emit startTelem();
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
        return 0;
    }
    qDebug() << __FILE__ << __LINE__ << "Serial port was already closed and deleted!";
    emit clearComboBox();
    return 0;
}

void SerialPortThread::sendDataToGUISlot() {
    QString msg = "";
    if(TelemSerialPort != NULL) {
        if(TelemSerialPort->isOpen()) {
            TelemSerialPort->write("1");
            msleep(35);
            QByteArray datas;
            datas = TelemSerialPort->readLine();//need to emit an empty signal here if dont get full msg
            for (int i = 0; i < datas.size(); i++){
                if (datas.at(i)) {
                    msg += datas[i];
                }
            }
            QStringList sensors = msg.split(",");
            if(sensors.length() > 14) //Full msg received
                emit sendDataToGUI(msg);
        } else {
            qDebug() << __FILE__ << __LINE__ << "OPEN ERROR: " << TelemSerialPort->errorString();
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
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(sendDataToGUISlot()));
    timer->start(50);
    qDebug() << "Starting timer";
}

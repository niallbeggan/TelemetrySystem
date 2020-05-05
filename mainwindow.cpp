#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>

#define ESTOP signalVector[0] // Making a define of a variable that only exists in one
#define BMSTEMPERATURE signalVector[1] // function is a terrible idea but it makes the packet so much
#define CARSPEED signalVector[2] // easier to change, and makes the code a LOT more read-able. I might change the defines
#define BMSVOLTAGE signalVector[3] // to just the location in the array later.
#define BMSCURRENT signalVector[4]
#define POWER_KW signalVector[5]
#define LEFTMOTORVOLTAGE signalVector[6]
#define RIGHTMOTORVOLTAGE signalVector[7]
#define LEFTMOTORCURRENT signalVector[8]
#define RIGHTMOTORCURRENT signalVector[9]
#define STEERINGINPUT signalVector[10]
#define ACCELERATOR_PEDAL_POSITION signalVector[11]

// Not on CAN Bus yet. Just used for demo
#define BRAKE_PEDAL_POSITION signalVector[12]
#define SUSPENSION_FRONT_LEFT signalVector[13]
#define SUSPENSION_FRONT_RIGHT signalVector[14]
#define SUSPENSION_REAR_LEFT signalVector[15]
#define SUSPENSION_REAR_RIGHT signalVector[16]
#define NO_OF_SATELLITES signalVector[17]

// All timestamps
#define ESTOP_TIMESTAMP timestampVector[0]
#define BMSTEMPERATURE_TIMESTAMP timestampVector[1]
#define CARSPEED_TIMESTAMP timestampVector[2]
#define BMSVOLTAGE_TIMESTAMP timestampVector[3]
#define BMSCURRENT_TIMESTAMP timestampVector[4]
#define POWER_KW_TIMESTAMP timestampVector[5]
#define LEFTMOTORVOLTAGE_TIMESTAMP timestampVector[6]
#define RIGHTMOTORVOLTAGE_TIMESTAMP timestampVector[7]
#define LEFTMOTORCURRENT_TIMESTAMP timestampVector[8]
#define RIGHTMOTORCURRENT_TIMESTAMP timestampVector[9]
#define STEERINGINPUT_TIMESTAMP timestampVector[10]
#define ACCELERATOR_PEDAL_POSITION_TIMESTAMP timestampVector[11]
#define BRAKE_PEDAL_POSITION_TIMESTAMP timestampVector[12]
#define SUSPENSION_FRONT_LEFT_TIMESTAMP timestampVector[13]
#define SUSPENSION_FRONT_RIGHT_TIMESTAMP timestampVector[14]
#define SUSPENSION_REAR_LEFT_TIMESTAMP timestampVector[15]
#define SUSPENSION_REAR_RIGHT_TIMESTAMP timestampVector[16]
#define NO_OF_SATELLITES_TIMESTAMP timestampVector[17]

// Fixed values, likely to change at a later date
#define BATTERY_TEMP_LIMIT 60 // A red line is graphed at this value on the battery temp graph
#define MAX_VOLTAGE 75.6 // 17 cells. These are used to calculate % charge.
#define MIN_VOLTAGE 51 // 3v/cell

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPortThread *thread = new SerialPortThread(); // Create serial port object
    QThread* thread1 = new QThread; // Create thread
    thread->moveToThread(thread1); // Move object to the thread
    thread1->start(); // Start the thread event loop

    MainWindow::setWindowTitle("TUD Formula Student Telemetry");
    //setWindowIcon(QIcon(":/TUD_Logo.PNG")); // Official, but shit looking logo
    setWindowIcon(QIcon(":/TUD.ico")); // DIY MS Paint logo converted to .ico file at icoconverter.com (choose 128p max)

    QCoreApplication::setApplicationName("TUD Formula Student Telemetry"); // For log files location

    // Connects are used to send and receive data from the serial port thread
    connect(this, SIGNAL(startComms()), thread, SLOT(startComms()), Qt::QueuedConnection);
    connect(this, SIGNAL(updateFromComboBox(QString)), thread, SLOT(selectPortFromComboBoxClick(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(closeComms()), thread, SLOT(endCommsFromGUI()), Qt::QueuedConnection);
    connect(thread, SIGNAL(sendDataToGUI(QVector <double>, QVector <double>)), this, SLOT(updateGUI(QVector <double>, QVector <double>)), Qt::QueuedConnection);
    connect(thread, SIGNAL(clearComboBox()), this, SLOT(clearComboBox()), Qt::QueuedConnection);
    connect(thread, SIGNAL(scanSerialPorts()), this, SLOT(scanSerialPorts()), Qt::QueuedConnection);
    connect(thread, SIGNAL(showStartComms()), this, SLOT(showStartComms()), Qt::QueuedConnection);
    connect(thread, SIGNAL(showEndComms()), this, SLOT(showEndComms()), Qt::QueuedConnection);
    connect(this, SIGNAL(timestamp(int, int, int, int)), thread, SLOT(updateRunTime(int, int, int, int)), Qt::QueuedConnection);
    connect(thread, SIGNAL(msgBoxSignal(int)), this, SLOT(showMessageBox(int)), Qt::QueuedConnection);

    // ui setup
    showStartComms();

    //refresh button
    QPixmap pixmap(":/refresh.PNG");
    QIcon ButtonIcon(pixmap);
    ui->refreshPorts->setIconSize(QSize(28,28));
    ui->refreshPorts->setIcon(ButtonIcon);

    // Create a red colour line, to be added to graphs later
    QPen redPen;
    redPen.setWidth(1);
    redPen.setColor("red");

    // Main tab Gauges setup
    ui->Main_Battery_Temp_Gauge->setMinValue(0);
    ui->Main_Battery_Temp_Gauge->setMaxValue(100);
    ui->Main_Battery_Temp_Gauge->setThreshold(60);// Set these three parameters first
    ui->Main_Battery_Temp_Gauge->setValue(0);
    ui->Main_Battery_Temp_Gauge->setLabel("Temp");
    ui->Main_Battery_Temp_Gauge->setUnits("°C");
    ui->Main_Battery_Temp_Gauge->setSteps(30);

    ui->Main_Battery_Voltage_Gauge->setMinValue(0);
    ui->Main_Battery_Voltage_Gauge->setMaxValue(80); // 17 cells, this would be 4.3 volts per cell, overcharged
    ui->Main_Battery_Voltage_Gauge->setThresholdEnabled(false);
    ui->Main_Battery_Voltage_Gauge->setValue(0);
    ui->Main_Battery_Voltage_Gauge->setLabel("Voltage");
    ui->Main_Battery_Voltage_Gauge->setUnits("V");
    ui->Main_Battery_Voltage_Gauge->setSteps(30);

    ui->Main_Speed_Gauge->setMinValue(0);
    ui->Main_Speed_Gauge->setMaxValue(150);// A hopefully higher than feasible top speed
    ui->Main_Speed_Gauge->setThresholdEnabled(false);
    ui->Main_Speed_Gauge->setValue(0);
    ui->Main_Speed_Gauge->setLabel("Speed");
    ui->Main_Speed_Gauge->setUnits("km/h");
    ui->Main_Speed_Gauge->setCoverGlassEnabled(true);
    ui->Main_Speed_Gauge->setSteps(30);
    ui->Main_Speed_Gauge->setDigitCount(3);

    ui->Main_Power_Gauge->setMinValue(0.0);
    ui->Main_Power_Gauge->setMaxValue(80.0);
    ui->Main_Power_Gauge->setThresholdEnabled(false);
    ui->Main_Power_Gauge->setValue(0.0);
    ui->Main_Power_Gauge->setLabel("Power");
    ui->Main_Power_Gauge->setUnits("kW");
    ui->Main_Power_Gauge->setSteps(40);
    ui->Main_Power_Gauge->setDigitCount(3);

    // Graphing suspension
    // Initialise all QVectors
    suspensionLeftFront = QVector <QVector<double>> (2,usedForInitialisationOnly);
    suspensionRightFront = QVector <QVector<double>> (2,usedForInitialisationOnly);
    suspensionLeftRear = QVector <QVector<double>> (2,usedForInitialisationOnly);
    suspensionRightRear = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->suspensionTabFrontLeftGraph->addGraph();
    ui->suspensionTabFrontLeftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabFrontLeftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabFrontLeftGraph->xAxis->setRange(0, 300);
    ui->suspensionTabFrontLeftGraph->yAxis->setRange(0, 100);
    ui->suspensionTabFrontLeftGraph->update();

    ui->suspensionTabFrontLeftGraph->yAxis->setLabel("Position (%) (100% is fully compressed)");
    ui->suspensionTabFrontLeftGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->suspensionTabRearLeftGraph->addGraph();
    ui->suspensionTabRearLeftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearLeftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabRearLeftGraph->xAxis->setRange(0, 300);
    ui->suspensionTabRearLeftGraph->yAxis->setRange(0, 100);
    ui->suspensionTabRearLeftGraph->update();

    ui->suspensionTabRearLeftGraph->yAxis->setLabel("Position (%) (100% is fully compressed)");
    ui->suspensionTabRearLeftGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->suspensionTabFrontRightGraph->addGraph();
    ui->suspensionTabFrontRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabFrontRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabFrontRightGraph->xAxis->setRange(0, 300);
    ui->suspensionTabFrontRightGraph->yAxis->setRange(0, 100);
    ui->suspensionTabFrontRightGraph->update();

    ui->suspensionTabFrontRightGraph->yAxis->setLabel("Position (%) (100% is fully compressed)");
    ui->suspensionTabFrontRightGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->suspensionTabRearRightGraph->addGraph();
    ui->suspensionTabRearRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabRearRightGraph->xAxis->setRange(0, 300);
    ui->suspensionTabRearRightGraph->yAxis->setRange(0, 100);
    ui->suspensionTabRearRightGraph->update();

    ui->suspensionTabRearRightGraph->yAxis->setLabel("Position (%) (100% is fully compressed)");
    ui->suspensionTabRearRightGraph->xAxis->setLabel("Timestamp (seconds of the day)");

//    ui->frontLeftPlot->setBackground(Qt::black);
//    //ui->frontLeftPlot->axisRect()->setBackground(Qt::white);
//    ui->frontLeftPlot->xAxis->setTickLabelColor(Qt::white); // Needed if i go with black backgroud.
//    ui->frontLeftPlot->xAxis->setBasePen(QPen(Qt::white)); //
//    ui->frontLeftPlot->xAxis->setLabelColor(Qt::white);
//    ui->frontLeftPlot->xAxis->setTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->xAxis->setSubTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setTickLabelColor(Qt::white);
//    ui->frontLeftPlot->yAxis->setBasePen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setLabelColor(Qt::white);
//    ui->frontLeftPlot->yAxis->setTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setSubTickPen(QPen(Qt::white));

    // Battery tab settings

    ui->batteryTabStateOfChargeProgressBar->setValue(0);
    highestCurrent = 0; // Settings that should get overwritten immediately
    lowestVoltage = 100;

    batteryTemp = QVector <QVector<double>> (2,usedForInitialisationOnly);
    batteryTempLimit = QVector <QVector<double>> (2,usedForInitialisationOnly);
    batteryVoltage  = QVector <QVector<double>> (2,usedForInitialisationOnly);
    batteryCurrent = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->batteryTabTempGraph->addGraph();
    ui->batteryTabTempGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabTempGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabTempGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabTempGraph->xAxis->setRange(0, 300);
    ui->batteryTabTempGraph->yAxis->setRange(0, 100);
    ui->batteryTabTempGraph->yAxis->setLabel("Temperature (Degrees C)");
    ui->batteryTabTempGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->batteryTabTempGraph->addGraph();
    ui->batteryTabTempGraph->graph(1)->setPen(redPen);
    ui->batteryTabTempGraph->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabTempGraph->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabTempGraph->update();

    ui->batteryTabVoltageGraph->addGraph();
    ui->batteryTabVoltageGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabVoltageGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabVoltageGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabVoltageGraph->xAxis->setRange(0, 300);
    ui->batteryTabVoltageGraph->yAxis->setRange(0, 80);
    ui->batteryTabVoltageGraph->update();
    ui->batteryTabVoltageGraph->yAxis->setLabel("Battery voltage (V)");
    ui->batteryTabVoltageGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->batteryTabCurrentGraph->addGraph();
    ui->batteryTabCurrentGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabCurrentGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabCurrentGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabCurrentGraph->xAxis->setRange(0, 300);
    ui->batteryTabCurrentGraph->yAxis->setRange(0, 500);
    ui->batteryTabCurrentGraph->update();
    ui->batteryTabCurrentGraph->yAxis->setLabel("Battery current (A)");
    ui->batteryTabCurrentGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    // Pedal positions tab
    brakePedal = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->pedalTabGraph->addGraph();
    ui->pedalTabGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->pedalTabGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

    acceleratorPedal = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->pedalTabGraph->addGraph();
    ui->pedalTabGraph->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->pedalTabGraph->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->pedalTabGraph->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 60))); // Accelerator blue
    ui->pedalTabGraph->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 60))); // Brake red
    ui->pedalTabGraph->xAxis->setRange(0, 300);
    ui->pedalTabGraph->yAxis->setRange(0, 100);
    ui->pedalTabGraph->update();

    ui->pedalTabGraph->yAxis->setLabel("Pedal position (%) (100% is fully pressed)");
    ui->pedalTabGraph->xAxis->setLabel("Timestamp (seconds of the day)");

    ui->pedalTabGraph->graph(0)->setPen(redPen);
    ui->pedalTabGraph->graph(0)->setName("Brake pedal"); // Legend
    ui->pedalTabGraph->graph(1)->setName("Accelerator pedal");
    ui->pedalTabGraph->legend->setVisible(true);

    // Motor differential graph
    motorDifferentialPower = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->motorDiffPower->addGraph();
    ui->motorDiffPower->graph(0)->setName("Differential motor power");
    ui->motorDiffPower->legend->setVisible(true);
    ui->motorDiffPower->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->motorDiffPower->graph(0)->setValueAxis(ui->motorDiffPower->xAxis);
    ui->motorDiffPower->graph(0)->setKeyAxis(ui->motorDiffPower->yAxis);
    ui->motorDiffPower->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->motorDiffPower->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->motorDiffPower->xAxis->setRange(-1000, 1000);
    ui->motorDiffPower->yAxis->setRange(0, 500);
    ui->motorDiffPower->update();

    ui->motorDiffPower->xAxis->setLabel("Difference in power between motors (kW) (negative means left motor has more power)");
    ui->motorDiffPower->yAxis->setLabel("Timestamp (seconds of the day)");

    // Steering Input graph
    steeringInputPercent = QVector <QVector<double>> (2,usedForInitialisationOnly);

    ui->steeringInputGraph->addGraph();
    ui->steeringInputGraph->graph(0)->setPen(redPen);
    ui->steeringInputGraph->graph(0)->setName("Steering input "); // Legend
    ui->steeringInputGraph->legend->setVisible(true);
    ui->steeringInputGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->steeringInputGraph->graph(0)->setValueAxis(ui->steeringInputGraph->xAxis);
    ui->steeringInputGraph->graph(0)->setKeyAxis(ui->steeringInputGraph->yAxis);
    ui->steeringInputGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->steeringInputGraph->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 30)));
    ui->steeringInputGraph->xAxis->setRange(-55, 55);
    ui->steeringInputGraph->yAxis->setRange(0, 500);
    ui->steeringInputGraph->update();

    ui->steeringInputGraph->xAxis->setLabel("Steering position (%) (negative means left)");
    ui->steeringInputGraph->yAxis->setLabel("Timestamp (seconds of the day)");
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::showMessageBox(int type) {
    if(type == 1) {
        QMessageBox msgWarning;
        msgWarning.setText("WARNING!\nPlease select an existing port before trying to start comms.");
        msgWarning.setIcon(QMessageBox::Warning);
        msgWarning.setWindowTitle("Caution");
        msgWarning.exec();
    }
    else if(type == 2){
        QMessageBox msgCritical;
        msgCritical.setText("Failure!\nUSB connection has failed! No data can be recieved.");
        msgCritical.setIcon(QMessageBox::Critical);
        msgCritical.setWindowTitle("Critical!");
        msgCritical.exec();
    }
}

// ********************** Updating all tabs *************************** //

void MainWindow::updateGUI(QVector <double> signalVector, QVector <double> timestampVector) {
    //QApplication::processEvents(); // Careful with this.

    // Battery
    updateBatteryTab(BMSVOLTAGE, BMSCURRENT, BMSTEMPERATURE); // volt, current, tmp

    // Main Tab
    updateMainTab(BMSTEMPERATURE, BMSVOLTAGE, CARSPEED, POWER_KW);

    if(timestampVector.contains(-1) == true) {
        // Show user radio has no signal
        updateRadioStatus(0);
        updateMainGPSStatus(0);
        updateMainTab(0, 0, 0, 0);
    }

    if(timestampVector.contains(-1) != true) { // Only update graphs if message received
        updateRadioStatus(1);
        updateMainGPSStatus(NO_OF_SATELLITES);
        // Suspension
        addPointsToGraphVector(suspensionLeftFront, SUSPENSION_FRONT_LEFT_TIMESTAMP, SUSPENSION_FRONT_LEFT);
        addPointsToGraphVector(suspensionRightFront, SUSPENSION_FRONT_RIGHT_TIMESTAMP, SUSPENSION_FRONT_RIGHT);
        addPointsToGraphVector(suspensionLeftRear, SUSPENSION_REAR_LEFT_TIMESTAMP, SUSPENSION_REAR_LEFT);
        addPointsToGraphVector(suspensionRightRear, SUSPENSION_REAR_RIGHT_TIMESTAMP, SUSPENSION_REAR_RIGHT);
        plotSuspensionGraphs();


        addPointsToGraphVector(batteryTemp, BMSTEMPERATURE_TIMESTAMP, BMSTEMPERATURE);
        addPointsToGraphVector(batteryTempLimit, BMSTEMPERATURE_TIMESTAMP, BATTERY_TEMP_LIMIT); // Might remove,red limit line
        addPointsToGraphVector(batteryCurrent, BMSCURRENT_TIMESTAMP, BMSCURRENT);
        addPointsToGraphVector(batteryVoltage, BMSVOLTAGE_TIMESTAMP, BMSVOLTAGE);
        plotBatteryGraphs();

        // Pedal positions tab
        updatePedalTab(BRAKE_PEDAL_POSITION, BRAKE_PEDAL_POSITION_TIMESTAMP , ACCELERATOR_PEDAL_POSITION, ACCELERATOR_PEDAL_POSITION_TIMESTAMP);

        //Motor & Steering tab
        updateMotorAndSteeringTab(LEFTMOTORVOLTAGE, LEFTMOTORCURRENT, LEFTMOTORCURRENT_TIMESTAMP,
                                  RIGHTMOTORVOLTAGE, RIGHTMOTORCURRENT, RIGHTMOTORCURRENT_TIMESTAMP,
                                  STEERINGINPUT, STEERINGINPUT_TIMESTAMP);
    }
}

// ************************ MainTab functions ************************ //

void MainWindow::scanSerialPorts() {
    MainWindow::ui->comboBoxSerialPorts->clear();
    ui->comboBoxSerialPorts->addItem("");
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) { // Add all COM ports to the drop down.
        MainWindow::ui->comboBoxSerialPorts->addItem(port.description() + " (" + port.portName() + ")");
    }
}

void MainWindow::on_refreshPorts_clicked() {
    QPixmap pixmap(":/refresh.PNG");
    QPixmap reload(":/reload2.PNG");
    QIcon ButtonIcon(pixmap);
    QIcon ButtonIcon2(reload);
    ui->refreshPorts->setIcon(ButtonIcon2);
    QApplication::processEvents();
    scanSerialPorts();
    ui->comboBoxSerialPorts->setCurrentIndex(0);
    QThread::msleep(150);
    ui->refreshPorts->setIcon(ButtonIcon);
}

void MainWindow::showStartComms() {
    ui->endComms->hide();
    ui->startComms->show();
}

void MainWindow::showEndComms() {
    ui->startComms->hide();
    ui->endComms->show();
}

void MainWindow::on_endComms_clicked() { // Tells the serial port thread to stop the timer
    emit closeComms();
    ui->gpsStatusLabel->setText("GPS STATUS:"); // This could be added to a function
    ui->radioStatusLabel->setText("RADIO STATUS:");
    stopClicked = true;
    clearComboBox();
}

void MainWindow::on_startComms_clicked() { // Tells the serial port thread to start the timer
    // Reset GUI graphs
    clearData(suspensionLeftFront);
    clearData(suspensionRightFront);
    clearData(suspensionLeftRear);
    clearData(suspensionRightRear);
    clearData(batteryTemp);
    clearData(batteryVoltage);
    clearData(batteryCurrent);
    clearData(brakePedal);
    clearData(acceleratorPedal);
    clearData(motorDifferentialPower);
    clearData(steeringInputPercent);
    highestCurrent = 0;
    lowestVoltage = 100;
    emit startComms();
    stopClicked = false;
    runningTime.start();
}

void MainWindow::clearComboBox() { // clears USB port options
    MainWindow::ui->comboBoxSerialPorts->clear();
}

void MainWindow::on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber) {
    emit updateFromComboBox(PortDescriptionAndNumber);
}

void MainWindow::updateMainTemp(int temp) {
    ui->Main_Battery_Temp_Gauge->setValue(temp);
}

void MainWindow::updateMainVoltage(int voltage) {
    ui->Main_Battery_Voltage_Gauge->setValue(voltage);
}

void MainWindow::updateMainSpeed(int speed) {
    ui->Main_Speed_Gauge->setValue(speed);
}

void MainWindow::updateMainRunningTime() {
    int millis = runningTime.elapsed();
    int secs = (millis/1000) % 60;
    int mins = (millis / 60000) % 60;
    int hours = (millis / 3600000);
    int hundredths = 0;
    millis = millis % 1000;
    hundredths = round(millis/10);

    ui->hoursLcdNumber->display(hours);
    ui->minutesLcdNumber->display(mins);
    ui->secondsLcdNumber->display(secs);
    ui->hundredthsSecondsLcdNumber->display(hundredths);

    emit timestamp(millis, secs, mins, hours);
}

void MainWindow::updateMainGPSStatus(int noOfSatellites) {
    if(stopClicked != true) {
        if(noOfSatellites < 4)
            ui->gpsStatusLabel->setText("GPS STATUS: CONNECTING");
        if(noOfSatellites == 4)
            ui->gpsStatusLabel->setText("GPS STATUS: WEAK SIGNAL");
        if(noOfSatellites == 5)
            ui->gpsStatusLabel->setText("GPS STATUS: OK SIGNAL");
        if(noOfSatellites > 5)
            ui->gpsStatusLabel->setText("GPS STATUS: GOOD SIGNAL");
    }
}

void MainWindow::updateRadioStatus(int signalStrength) {
    if(stopClicked != true) {
        if(signalStrength == 0)
            ui->radioStatusLabel->setText("RADIO STATUS: NO SIGNAL");
        if(signalStrength == 1)
            ui->radioStatusLabel->setText("RADIO STATUS: GOOD SIGNAL");
    }
}

void MainWindow::updateMainPower(int power) {
  ui->Main_Power_Gauge->setValue(power);
}

void MainWindow::updateMainTab(double temp, double voltage, double speed, double power) { // Cant display decimal places
    updateMainTemp(temp);
    updateMainVoltage(voltage);
    updateMainSpeed(speed);
    updateMainRunningTime();
    updateMainPower(power);
    updateMainRunningTime();
}

//********************** Suspension functions **********************//

void MainWindow::addPointsToGraphVector(QVector<QVector<double> > &graphVector, double x, double y) {
    graphVector[0].append(x); // Add two new points
    graphVector[1].append(y);
    if(graphVector[0].length() > 1000) { // TMP Add variable in for graph length?
        graphVector[1].removeFirst(); // If more than 1000 values, remove oldest every time u add a new 1.
        graphVector[0].removeFirst();
    }
}

void MainWindow::plotSuspensionGraphs() {
   frontLeftPlot();
   rearLeftPlot();
   frontRightPlot();
   rearRightPlot();
}

void MainWindow::clearData(QVector <QVector<double>> &plotVector) { // Still need to reset GUI when start is clicked
    plotVector[0].clear();
    plotVector[1].clear();
}

void MainWindow::frontLeftPlot() {
    ui->suspensionTabFrontLeftGraph->graph(0)->setData(suspensionLeftFront[0], suspensionLeftFront[1]);
    ui->suspensionTabFrontLeftGraph->replot();
    double startOfXAxis = 0;
    if(suspensionLeftFront[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionLeftFront[0][suspensionLeftFront[0].length()-1];
    ui->suspensionTabFrontLeftGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));//tmp
    ui->suspensionTabFrontLeftGraph->yAxis->setRange(1,100);
    ui->suspensionTabFrontLeftGraph->update();
}

void MainWindow::rearLeftPlot() {
    ui->suspensionTabRearLeftGraph->graph(0)->setData(suspensionRightFront[0], suspensionRightFront[1]);
    ui->suspensionTabRearLeftGraph->replot();
    double startOfXAxis = 0;
    if(suspensionRightFront[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionRightFront[0][suspensionRightFront[0].length()-1];
    ui->suspensionTabRearLeftGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    ui->suspensionTabRearLeftGraph->yAxis->setRange(1,100);
    ui->suspensionTabRearLeftGraph->update();
}

void MainWindow::frontRightPlot() {
    ui->suspensionTabFrontRightGraph->graph(0)->setData(suspensionLeftRear[0], suspensionLeftRear[1]);
    ui->suspensionTabFrontRightGraph->replot();
    double startOfXAxis = 0;
    if(suspensionLeftRear[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionLeftRear[0][suspensionLeftRear[0].length()-1];
    ui->suspensionTabFrontRightGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120)); // -120 here means show the last two minutes data
    ui->suspensionTabFrontRightGraph->yAxis->setRange(1,100);
    ui->suspensionTabFrontRightGraph->update();
}

void MainWindow::rearRightPlot() {
    ui->suspensionTabRearRightGraph->graph(0)->setData(suspensionRightRear[0], suspensionRightRear[1]);
    ui->suspensionTabRearRightGraph->replot();
    double startOfXAxis = 0;
    if(suspensionRightRear[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionRightRear[0][suspensionRightRear[0].length()-1];
    ui->suspensionTabRearRightGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    ui->suspensionTabRearRightGraph->yAxis->setRange(1,100);
    ui->suspensionTabRearRightGraph->update();
}

//void MainWindow::on_clearPlot_clicked() {
//    clearData();
//    plot();
//}

//void MainWindow::plot(QVector<QVector<double> > &graph) { // Tried to make a generic graph function
//    ui->plot->graph(0)->setData(graph1[0], graph1[1]);
//    ui->plot->replot();
//    double startOfXAxis = 0;
//    if(graph[0].length() == 0) {
//        startOfXAxis = 0;
//    }
//    else
//        startOfXAxis = graph[0][graph[0].length()-1];
//    ui->plot->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
//    ui->plot->yAxis->setRange(1,1030);
//    ui->plot->update();
//}

//*********************************Battery tab functions *************************//

void MainWindow::updateBatteryTab(double voltage, double current, double temp) {    
    double stateOfCharge = 100*(voltage-MIN_VOLTAGE)/(MAX_VOLTAGE-MIN_VOLTAGE);
    if((voltage < lowestVoltage) && (voltage > 0))
        lowestVoltage = voltage;
    if(current > highestCurrent)
        highestCurrent = current;
    ui->batteryTabStateOfChargeProgressBar->setValue(stateOfCharge); // this needs to be updated with more comlplex state of charge calculation
    ui->batteryTabTempLCD->display(temp);
    ui->batteryTabChargeLCD->display(stateOfCharge);
    ui->batteryTabCurrentLCD->display(current);
    ui->batteryTabVoltageLCD->display(voltage);
    ui->batteryTabMaxCurrentLCD->display(highestCurrent);
    ui->batteryTabLowestVoltageLCD->display(lowestVoltage);
}

void MainWindow::batteryTempPlot() {
    ui->batteryTabTempGraph->graph(0)->setData(batteryTemp[0], batteryTemp[1]);
    ui->batteryTabTempGraph->graph(1)->setData(batteryTempLimit[0], batteryTempLimit[1]);
    ui->batteryTabTempGraph->replot();
    double startOfXAxis = 0;
    if(batteryTemp[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = batteryTemp[0][batteryTemp[0].length()-1];
    ui->batteryTabTempGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    ui->batteryTabTempGraph->yAxis->setRange(0,100);
    ui->batteryTabTempGraph->update();
}

void MainWindow::batteryCurrentPlot() {
    ui->batteryTabCurrentGraph->graph(0)->setData(batteryCurrent[0], batteryCurrent[1]);
    ui->batteryTabCurrentGraph->replot();
    double startOfXAxis = 0;
    if(batteryCurrent[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = batteryCurrent[0][batteryCurrent[0].length()-1];
    ui->batteryTabCurrentGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    double min = *std::min_element(batteryCurrent[1].constBegin(), batteryCurrent[1].constEnd());
    if(0 < min)
        min = 0;
    if(min == -100) // need to fix this -100 lark by plotting against time
        min = 0;
    double max = *std::max_element(batteryCurrent[1].constBegin(), batteryCurrent[1].constEnd());
    if(300 > max)
        max = 300;
    ui->batteryTabCurrentGraph->yAxis->setRange(min-1, max+1);
    ui->batteryTabCurrentGraph->update();
}

void MainWindow::batteryVoltagePlot() {
    ui->batteryTabVoltageGraph->graph(0)->setData(batteryVoltage[0], batteryVoltage[1]);
    ui->batteryTabVoltageGraph->replot();
    double startOfXAxis = 0;
    if(batteryVoltage[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = batteryVoltage[0][batteryVoltage[0].length()-1];
    ui->batteryTabVoltageGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    double min = *std::min_element(batteryVoltage[1].constBegin(), batteryVoltage[1].constEnd());
    double max = *std::max_element(batteryVoltage[1].constBegin(), batteryVoltage[1].constEnd());
    if(min == -100)
        min = 0;
    if(80 > max)
        max = 80;
    ui->batteryTabVoltageGraph->yAxis->setRange(min-1, max+1);
    ui->batteryTabVoltageGraph->update();
}

void MainWindow::plotBatteryGraphs() {
    batteryTempPlot();
    batteryCurrentPlot();
    batteryVoltagePlot();
}

// ********************** Pedal functions *************************** //

void MainWindow::updatePedalTab(double brake, double brakeTimestamp, double accelerator, double acceleratorTimestamp) {
    addPointsToGraphVector(brakePedal, brakeTimestamp, brake);
    addPointsToGraphVector(acceleratorPedal, acceleratorTimestamp, accelerator); //remove this
    plotPedalGraph();
}

void MainWindow::pedalBrakePlot() {
    ui->pedalTabGraph->graph(0)->setData(brakePedal[0], brakePedal[1]);
    ui->pedalTabGraph->replot();
    double startOfXAxis = 0;
    if(brakePedal[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = brakePedal[0][brakePedal[0].length()-1];
    ui->pedalTabGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    ui->pedalTabGraph->yAxis->setRange(0,100);
    ui->pedalTabGraph->update();
}

void MainWindow::pedalAcceleratorPlot() {
    ui->pedalTabGraph->graph(1)->setData(acceleratorPedal[0], acceleratorPedal[1]);
    ui->pedalTabGraph->replot();
    double startOfXAxis = 0;
    if(acceleratorPedal[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = acceleratorPedal[0][acceleratorPedal[0].length()-1];
    ui->pedalTabGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-120));
    ui->pedalTabGraph->yAxis->setRange(0,100);
    ui->pedalTabGraph->update();
}

void MainWindow::plotPedalGraph() {
    pedalAcceleratorPlot();
    pedalBrakePlot();
}

// ********************** Motor and Steering functions *************************** //

void MainWindow::updateMotorAndSteeringTab(double leftMotorVoltage, double leftMotorCurrent, double leftMotorCurrentTimestamp,
                                           double rightMotorVoltage, double rightMotorCurrent, double rightMotorCurrentTimestamp,
                                           double steeringInput, double steeringInputTimestamp) {
    double leftMotorPower = leftMotorCurrent * leftMotorVoltage;
    double rightMotorPower = rightMotorCurrent * rightMotorVoltage;
    if(leftMotorVoltage == -100) {
        leftMotorVoltage = 0;
        leftMotorCurrent = 0;
        rightMotorCurrent = 0;
        rightMotorVoltage = 0;
        steeringInput = 0;
    }
    double differentialPower = rightMotorPower - leftMotorPower; // If left power is bigger, result is neg, plot more left power on left side of graph.
    addPointsToGraphVector(motorDifferentialPower, differentialPower, leftMotorCurrentTimestamp); // differentialPower TIMESTAMPS ASSUME TO BE THE SAME!!!
    addPointsToGraphVector(steeringInputPercent, steeringInput, steeringInputTimestamp);
    motorDifferentialPlot();
    steeringInputPlot();
}

void MainWindow::motorDifferentialPlot() {
    ui->motorDiffPower->graph(0)->setData(motorDifferentialPower[1], motorDifferentialPower[0]);

    int pxy = ui->motorDiffPower->xAxis->coordToPixel(0);
    ui->motorDiffPower->yAxis->setOffset(ui->motorDiffPower->axisRect()->left()-pxy); // Internet code

    ui->motorDiffPower->replot();  

    double startOfYAxis = 0;

    startOfYAxis = motorDifferentialPower[1][motorDifferentialPower[1].length()-1];
    ui->motorDiffPower->yAxis->setRange(startOfYAxis,(startOfYAxis-120));
    double min = *std::min_element(motorDifferentialPower[0].constBegin(), motorDifferentialPower[0].constEnd());
    double max = *std::max_element(motorDifferentialPower[0].constBegin(), motorDifferentialPower[0].constEnd());
    if(max < 1000)
        max = 1000;
    if(min > -1000)
        min = -1000;
    if((min*-1) > max)
        max = min* -1;
    else
        min = max* -1;
    ui->motorDiffPower->xAxis->setRange(min-100,max+100);
    ui->motorDiffPower->update();
}

void MainWindow::steeringInputPlot() {
    ui->steeringInputGraph->graph(0)->setData(steeringInputPercent[1], steeringInputPercent[0]);

    int pxy = ui->steeringInputGraph->xAxis->coordToPixel(0);
    ui->steeringInputGraph->yAxis->setOffset(ui->steeringInputGraph->axisRect()->left()-pxy);

    ui->steeringInputGraph->replot();
    double startOfYAxis = 0;
    startOfYAxis = steeringInputPercent[1][steeringInputPercent[1].length()-1];

    ui->steeringInputGraph->yAxis->setRange(startOfYAxis,(startOfYAxis-120));
    ui->steeringInputGraph->xAxis->setRange(-55, 55);
    ui->steeringInputGraph->update();
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ESTOP sensors[0] // Makes the code read-able
#define BMSTEMPERATURE sensors[1]
#define CARSPEED sensors[2]
#define BMSVOLTAGE sensors[3]
#define BMSCURRENT sensors[4]
#define POWER_KW sensors[5]
#define LEFTMOTORVOLTAGE sensors[6]
#define RIGHTMOTORVOLTAGE sensors[7]
#define LEFTMOTORCURRENT sensors[8]
#define RIGHTMOTORCURRENT sensors[9]
#define STEERINGINPUT sensors[10]
#define ACCELERATOR_PEDAL_POSITION sensors[11]
#define BRAKE_PEDAL_POSITION sensors[12]
#define SUSPENSION_FRONT_LEFT sensors[13]
#define SUSPENSION_FRONT_RIGHT sensors[14]
#define SUSPENSION_REAR_LEFT sensors[15]
#define SUSPENSION_REAR_RIGHT sensors[16]

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPortThread *thread = new SerialPortThread();
    QThread* thread1 = new QThread;
    thread->moveToThread(thread1);
    thread1->start();

    MainWindow::setWindowTitle("TUD Formula Student Telemetry");
    setWindowIcon(QIcon(":/TUD_Logo.PNG"));

    connect(this, SIGNAL(startComms()), thread, SLOT(startComms()));
    connect(this, SIGNAL(updateFromComboBox(QString)), thread, SLOT(selectPortFromComboBoxClick(QString)));
    connect(this, SIGNAL(closeComms()), thread, SLOT(endCommsFromGUI()));
    connect(thread, SIGNAL(sendDataToGUI(QStringList)), this, SLOT(updateGUI(QStringList)));
    connect(thread, SIGNAL(clearComboBox()), this, SLOT(clearComboBox()));
    connect(thread, SIGNAL(scanSerialPorts()), this, SLOT(scanSerialPorts()));
    connect(thread, SIGNAL(showStartComms()), this, SLOT(showStartComms()));
    connect(thread, SIGNAL(showEndComms()), this, SLOT(showEndComms()));
    connect(this, SIGNAL(timestamp(int, int, int, int)), thread, SLOT(updateTimestamp(int, int, int, int)));
    connect(thread, SIGNAL(msgBoxSignal(int)), this, SLOT(showMessageBox(int)));

    // ui setup
    showStartComms();

    //refresh button
    QPixmap pixmap(":/refresh.PNG");
    QIcon ButtonIcon(pixmap);
    ui->refreshPorts->setIconSize(QSize(28,28));
    ui->refreshPorts->setIcon(ButtonIcon);

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
    ui->Main_Battery_Voltage_Gauge->setLabel("Volts");
    ui->Main_Battery_Voltage_Gauge->setUnits("V");
    ui->Main_Battery_Voltage_Gauge->setSteps(30);

    ui->Main_Speed_Gauge->setMinValue(0);
    ui->Main_Speed_Gauge->setMaxValue(150);// A hopefully higher than feasible top speed
    ui->Main_Speed_Gauge->setThresholdEnabled(false);
    ui->Main_Speed_Gauge->setValue(0);
    ui->Main_Speed_Gauge->setLabel("Speed");
    ui->Main_Speed_Gauge->setUnits("Km/h");
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
    suspensionLeftFront.append(suspensionLeftFrontX);
    suspensionLeftFront.append(suspensionLeftFrontY);

    suspensionRightFront.append(suspensionLeftFrontX);
    suspensionRightFront.append(suspensionLeftFrontY);

    suspensionLeftRear.append(suspensionLeftFrontX);
    suspensionLeftRear.append(suspensionLeftFrontY);

    suspensionRightRear.append(suspensionLeftFrontX);
    suspensionRightRear.append(suspensionLeftFrontY);// i need to get rid of all these, they are empty...

    ui->suspensionTabFrontLeftGraph->addGraph();
    ui->suspensionTabFrontLeftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabFrontLeftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabFrontLeftGraph->xAxis->setRange(0, 300);
    ui->suspensionTabFrontLeftGraph->yAxis->setRange(0, 100);
    ui->suspensionTabFrontLeftGraph->update();

    ui->suspensionTabRearLeftGraph->addGraph();
    ui->suspensionTabRearLeftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearLeftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabRearLeftGraph->xAxis->setRange(0, 300);
    ui->suspensionTabRearLeftGraph->yAxis->setRange(0, 100);
    ui->suspensionTabRearLeftGraph->update();

    ui->suspensionTabFrontRightGraph->addGraph();
    ui->suspensionTabFrontRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabFrontRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabFrontRightGraph->xAxis->setRange(0, 300);
    ui->suspensionTabFrontRightGraph->yAxis->setRange(0, 100);
    ui->suspensionTabFrontRightGraph->update();

    ui->suspensionTabRearRightGraph->addGraph();
    ui->suspensionTabRearRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->suspensionTabRearRightGraph->xAxis->setRange(0, 300);
    ui->suspensionTabRearRightGraph->yAxis->setRange(0, 100);
    ui->suspensionTabRearRightGraph->update();

//    ui->frontLeftPlot->setBackground(Qt::black);
//    //ui->frontLeftPlot->axisRect()->setBackground(Qt::white);
//    ui->frontLeftPlot->xAxis->setTickLabelColor(Qt::white); // Needed if i go with black backgroud. hopefully
//    ui->frontLeftPlot->xAxis->setBasePen(QPen(Qt::white)); // i will be able to switch colours live time by end of project
//    ui->frontLeftPlot->xAxis->setLabelColor(Qt::white);
//    ui->frontLeftPlot->xAxis->setTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->xAxis->setSubTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setTickLabelColor(Qt::white);
//    ui->frontLeftPlot->yAxis->setBasePen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setLabelColor(Qt::white);
//    ui->frontLeftPlot->yAxis->setTickPen(QPen(Qt::white));
//    ui->frontLeftPlot->yAxis->setSubTickPen(QPen(Qt::white));

    // Battery tab settings
    highestCurrent = 0; // Settings that should get overwritten immediately
    lowestVoltage = 100;

    batteryTemp.append(suspensionLeftFrontX);
    batteryTemp.append(suspensionLeftFrontY);// i need to get rid of all these, they are empty... but need to initialise graph vector somehow

    batteryVoltage.append(suspensionLeftFrontX);
    batteryVoltage.append(suspensionLeftFrontY);

    batteryCurrent.append(suspensionLeftFrontX);
    batteryCurrent.append(suspensionLeftFrontY);

    ui->batteryTabTempGraph->addGraph();
    ui->batteryTabTempGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabTempGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabTempGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabTempGraph->xAxis->setRange(0, 300);
    ui->batteryTabTempGraph->yAxis->setRange(0, 100);
    ui->batteryTabTempGraph->update();

    ui->batteryTabVoltageGraph->addGraph();
    ui->batteryTabVoltageGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabVoltageGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabVoltageGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabVoltageGraph->xAxis->setRange(0, 300);
    ui->batteryTabVoltageGraph->yAxis->setRange(0, 80);
    ui->batteryTabVoltageGraph->update();

    ui->batteryTabCurrentGraph->addGraph();
    ui->batteryTabCurrentGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabCurrentGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabCurrentGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->batteryTabCurrentGraph->xAxis->setRange(0, 300);
    ui->batteryTabCurrentGraph->yAxis->setRange(0, 500);
    ui->batteryTabCurrentGraph->update();

    // Pedal positions tab
    brakePedal.append(suspensionLeftFrontX);
    brakePedal.append(suspensionLeftFrontY);

    ui->pedalTabGraph->addGraph();
    ui->pedalTabGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->pedalTabGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

    acceleratorPedal.append(suspensionLeftFrontX);
    acceleratorPedal.append(suspensionLeftFrontY);

    ui->pedalTabGraph->addGraph();
    ui->pedalTabGraph->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->pedalTabGraph->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->pedalTabGraph->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 60))); // Accelerator blue
    ui->pedalTabGraph->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 60))); // Brake red
    ui->pedalTabGraph->xAxis->setRange(0, 300);
    ui->pedalTabGraph->yAxis->setRange(0, 100);
    ui->pedalTabGraph->update();

    QPen redPen;
    redPen.setWidth(1);
    redPen.setColor("red");
    ui->pedalTabGraph->graph(0)->setPen(redPen);
    ui->pedalTabGraph->graph(0)->setName("Brake pedal"); // Legend
    ui->pedalTabGraph->graph(1)->setName("Accelerator pedal");
    ui->pedalTabGraph->legend->setVisible(true);

    // Motor differential graph
    motorDifferentialPower.append(suspensionLeftFrontX);
    motorDifferentialPower.append(suspensionLeftFrontY);

    ui->motorDiffPower->addGraph();
    ui->motorDiffPower->graph(0)->setName("Differential motor power");
    ui->motorDiffPower->legend->setVisible(true);
    ui->motorDiffPower->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->motorDiffPower->graph(0)->setValueAxis(ui->motorDiffPower->xAxis);
    ui->motorDiffPower->graph(0)->setKeyAxis(ui->motorDiffPower->yAxis);
    ui->motorDiffPower->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->motorDiffPower->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));
    ui->motorDiffPower->xAxis->setRange(0, 300);
    ui->motorDiffPower->yAxis->setRange(-1000, 1000);
    ui->motorDiffPower->update();

    // Steering Input graph
    steeringInputPercent.append(suspensionLeftFrontX);
    steeringInputPercent.append(suspensionLeftFrontY);

    ui->steeringInputGraph->addGraph();
    ui->steeringInputGraph->graph(0)->setPen(redPen);
    ui->steeringInputGraph->graph(0)->setName("Steering input "); // Legend
    ui->steeringInputGraph->legend->setVisible(true);
    ui->steeringInputGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->steeringInputGraph->graph(0)->setValueAxis(ui->steeringInputGraph->xAxis);
    ui->steeringInputGraph->graph(0)->setKeyAxis(ui->steeringInputGraph->yAxis);
    ui->steeringInputGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->steeringInputGraph->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 30)));
    ui->steeringInputGraph->xAxis->setRange(0, 300);
    ui->steeringInputGraph->yAxis->setRange(-100, 100);
    ui->steeringInputGraph->update();
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
    //else
        //NOP yet
}

// ********************** Updating all tabs *************************** //

void MainWindow::updateGUI(QStringList sensors) {
    //QApplication::processEvents(); // Careful with this.
    if(sensors.length() > 17) {
        // qDebug() << sensors;

        // Suspension
        addPointsToGraph(suspensionLeftFront, suspensionLeftFront[0].length()+1, SUSPENSION_FRONT_LEFT.toDouble());
        addPointsToGraph(suspensionRightFront, suspensionRightFront[0].length()+1, SUSPENSION_FRONT_RIGHT.toDouble());
        addPointsToGraph(suspensionLeftRear, suspensionLeftRear[0].length()+1, SUSPENSION_REAR_LEFT.toDouble());
        addPointsToGraph(suspensionRightRear, suspensionRightRear[0].length()+1, SUSPENSION_REAR_RIGHT.toDouble());
        plotGraphs();

        // Battery
        updateBatteryTab(BMSVOLTAGE, BMSCURRENT, BMSTEMPERATURE); // volt, current, tmp
        addPointsToGraph(batteryTemp, batteryTemp[0].length()+1, BMSTEMPERATURE.toDouble());
        addPointsToGraph(batteryCurrent, batteryCurrent[0].length()+1, BMSCURRENT.toDouble());
        addPointsToGraph(batteryVoltage, batteryVoltage[0].length()+1, BMSVOLTAGE.toDouble());
        plotBatteryGraphs();

        // Main Tab
        updateMainTab(BMSTEMPERATURE.toDouble(), BMSVOLTAGE.toDouble(), CARSPEED.toDouble(), POWER_KW.toDouble());

        // Pedal positions tab
        updatePedalTab(BRAKE_PEDAL_POSITION, ACCELERATOR_PEDAL_POSITION);

        //Motor & Steering tab
        updateMotorAndSteeringTab(LEFTMOTORVOLTAGE.toDouble(), LEFTMOTORCURRENT.toDouble(),
                                  RIGHTMOTORVOLTAGE.toDouble(), RIGHTMOTORCURRENT.toDouble(),
                                  STEERINGINPUT.toDouble());
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
    QThread::msleep(80);
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

    ui->minutesLcdNumber->display(hours);
    ui->minutesLcdNumber->display(mins);
    ui->secondsLcdNumber->display(secs);
    ui->hundredthsSecondsLcdNumber->display(hundredths);

    emit timestamp(millis, secs, mins, hours);
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

void MainWindow::addPointsToGraph(QVector<QVector<double> > &graph, double x, double y) {
    graph[0].append(x);
    graph[1].append(y);
}

void MainWindow::plotGraphs() {
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
    ui->suspensionTabFrontLeftGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->suspensionTabRearLeftGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->suspensionTabFrontRightGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->suspensionTabRearRightGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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

void MainWindow::updateBatteryTab(QString voltage, QString current, QString temp) {
    double Voltage = voltage.toDouble();
    double Current = current.toDouble();
    double Temp = temp.toDouble();

    double maxVoltage = 75.6; // 17 cellS
    double minVoltage = 51; // 3v/cell
    double stateOfCharge = 100*(Voltage-minVoltage)/(maxVoltage-minVoltage);
    if((Voltage < lowestVoltage) && (Voltage > 0))
        lowestVoltage = Voltage;
    if(Current > highestCurrent)
        highestCurrent = Current;
    ui->batteryTabStateOfChargeProgressBar->setValue(stateOfCharge); // this needs to be updated with more comlplex state of charge calculation
    ui->batteryTabTempLCD->display(Temp);
    ui->batteryTabChargeLCD->display(stateOfCharge);
    ui->batteryTabCurrentLCD->display(Current);
    ui->batteryTabVoltageLCD->display(Voltage);
    ui->batteryTabMaxCurrentLCD->display(highestCurrent);
    ui->batteryTabLowestVoltageLCD->display(lowestVoltage);
}

void MainWindow::batteryTempPlot() {
    ui->batteryTabTempGraph->graph(0)->setData(batteryTemp[0], batteryTemp[1]);
    ui->batteryTabTempGraph->replot();
    double startOfXAxis = 0;
    if(batteryTemp[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = batteryTemp[0][batteryTemp[0].length()-1];
    ui->batteryTabTempGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->batteryTabCurrentGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->batteryTabVoltageGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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

void MainWindow::updatePedalTab(QString brake, QString accelerator) {
    double Brake = brake.toDouble();
    double Accelerator = accelerator.toDouble();
    addPointsToGraph(brakePedal, brakePedal[0].length()+1, Brake);
    addPointsToGraph(acceleratorPedal, acceleratorPedal[0].length()+1, Accelerator); //remove this
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
    ui->pedalTabGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
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
    ui->pedalTabGraph->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->pedalTabGraph->yAxis->setRange(0,100);
    ui->pedalTabGraph->update();
}

void MainWindow::plotPedalGraph() {
    pedalAcceleratorPlot();
    pedalBrakePlot();
}

// ********************** Motor and Steering functions *************************** //

void MainWindow::updateMotorAndSteeringTab(double leftMotorVoltage, double leftMotorCurrent,
                                           double rightMotorVoltage, double rightMotorCurrent, double steeringInput) {
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
    addPointsToGraph(motorDifferentialPower, differentialPower, motorDifferentialPower[1].length()+1); // differentialPower
    addPointsToGraph(steeringInputPercent, steeringInput, steeringInputPercent[1].length()+1);
    motorDifferentialPlot();
    steeringInputPlot();
}

void MainWindow::motorDifferentialPlot() {
    ui->motorDiffPower->graph(0)->setData(motorDifferentialPower[1], motorDifferentialPower[0]);

    int pxy = ui->motorDiffPower->xAxis->coordToPixel(0);
    ui->motorDiffPower->yAxis->setOffset(ui->motorDiffPower->axisRect()->left()-pxy); // Internet code

    ui->motorDiffPower->replot();
    double startOfXAxis = 0;
    if(motorDifferentialPower[1].length() < 1000) {
        startOfXAxis = 1000;
    }
    else
        startOfXAxis = motorDifferentialPower[1][motorDifferentialPower[1].length()-1];
    ui->motorDiffPower->yAxis->setRange(startOfXAxis,(startOfXAxis-1000));
    double min = *std::min_element(motorDifferentialPower[0].constBegin(), motorDifferentialPower[0].constEnd());
    double max = *std::max_element(motorDifferentialPower[0].constBegin(), motorDifferentialPower[0].constEnd());
    ui->motorDiffPower->xAxis->setRange(min-1,max+1);
    ui->motorDiffPower->update();
}

void MainWindow::steeringInputPlot() {
    ui->steeringInputGraph->graph(0)->setData(steeringInputPercent[1], steeringInputPercent[0]);

    int pxy = ui->steeringInputGraph->xAxis->coordToPixel(0);
    ui->steeringInputGraph->yAxis->setOffset(ui->steeringInputGraph->axisRect()->left()-pxy); // Internet code

    ui->steeringInputGraph->replot();
    double startOfXAxis = 0;
    if(steeringInputPercent[1].length() < 1000) {
        startOfXAxis = 1000;
    }
    else
        startOfXAxis = steeringInputPercent[1][steeringInputPercent[1].length()-1];
    ui->steeringInputGraph->yAxis->setRange(startOfXAxis,(startOfXAxis-1000));
    double min = *std::min_element(steeringInputPercent[0].constBegin(), steeringInputPercent[0].constEnd());
    if(min == -100) {
        min = -100;
    }
    double max = *std::max_element(steeringInputPercent[0].constBegin(), steeringInputPercent[0].constEnd());
    ui->steeringInputGraph->xAxis->setRange(min-1, max+1);
    ui->steeringInputGraph->update();
}

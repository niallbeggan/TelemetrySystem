#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ESTOP sensors[0] // Makes the code read-able
#define BMSTEMPERATURE sensors[1]
#define CARSPEED sensors[2]
#define BMSVOLTAGE sensors[3]
#define BMSCURRENT sensors[4]
#define MOTORVOLTAGE sensors[5]
#define MOTORCURRENT sensors[6]
#define POWER_KW sensors[7]
#define ACCELERATOR_PEDAL_POSITION sensors[8]
#define BRAKE_PEDAL_POSITION sensors[9]
#define SUSPENSION_FRONT_LEFT sensors[10]
#define SUSPENSION_FRONT_RIGHT sensors[11]
#define SUSPENSION_REAR_LEFT sensors[12]
#define SUSPENSION_REAR_RIGHT sensors[13]

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPortThread *thread = new SerialPortThread();
    thread->start();
    connect(this, SIGNAL(startComms()), thread, SLOT(startComms()));
    connect(this, SIGNAL(updateFromComboBox(QString)), thread, SLOT(selectPortFromComboBoxClick(QString)));
    connect(this, SIGNAL(closeComms()), thread, SLOT(endCommsFromGUI()));
    connect(thread, SIGNAL(sendDataToGUI(QStringList)), this, SLOT(updateGUI(QStringList)));
    connect(thread, SIGNAL(clearComboBox()), this, SLOT(clearComboBox()));
    connect(thread, SIGNAL(scanSerialPorts()), this, SLOT(scanSerialPorts()));
    connect(thread, SIGNAL(showStartComms()), this, SLOT(showStartComms()));
    connect(thread, SIGNAL(showEndComms()), this, SLOT(showEndComms()));
    connect(this, SIGNAL(timestamp(int, int, int, int)), thread, SLOT(updateTimestamp(int, int, int, int)));

    // ui setup
    showStartComms();

    //refresh button
    QPixmap pixmap("C:/Users/Owner/Documents/college/2019-2020/sem2/project/Telem_V1.1.0/refresh"); //FIX PATH FOR INSTALLABLE VERSION
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

    ui->Main_Battery_Voltage_Gauge->setMinValue(0); // 17 series cells, 34 would be 2 volts per cell aka dead
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

    ui->Main_Power_Gauge->setMinValue(0);
    ui->Main_Power_Gauge->setMaxValue(80);
    ui->Main_Power_Gauge->setThresholdEnabled(false);
    ui->Main_Power_Gauge->setValue(0);
    ui->Main_Power_Gauge->setLabel("Power");
    ui->Main_Power_Gauge->setUnits("kW");
    ui->Main_Power_Gauge->setSteps(40);
    ui->Main_Power_Gauge->setDigitCount(3);
    //ui->Main_Power_Meter->setForeground(QColor("white"));
    //ui->Main_Speed_Gauge->setBackground(QColor("black"));

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

    ui->suspensionTabRearLeftGraph->addGraph();
    ui->suspensionTabRearLeftGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearLeftGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->suspensionTabFrontRightGraph->addGraph();
    ui->suspensionTabFrontRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabFrontRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->suspensionTabRearRightGraph->addGraph();
    ui->suspensionTabRearRightGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->suspensionTabRearRightGraph->graph(0)->setLineStyle(QCPGraph::lsLine);

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

    ui->batteryTabVoltageGraph->addGraph();
    ui->batteryTabVoltageGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabVoltageGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabVoltageGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));

    ui->batteryTabCurrentGraph->addGraph();
    ui->batteryTabCurrentGraph->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->batteryTabCurrentGraph->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->batteryTabCurrentGraph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 30)));

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

    QPen redPen;
    redPen.setWidth(1);
    redPen.setColor("red");
    ui->pedalTabGraph->graph(0)->setPen(redPen);
    ui->pedalTabGraph->graph(0)->setName("Brake pedal"); // Legend
    ui->pedalTabGraph->graph(1)->setName("Accelerator pedal");
    ui->pedalTabGraph->legend->setVisible(true);
}

MainWindow::~MainWindow() {
    delete ui;
}

// ********************** Updating all tabs *************************** //

void MainWindow::updateGUI(QStringList sensors) {
    QApplication::processEvents();
    //ui->textEdit_2->append(msg); used for debugging
    if(sensors.length() > 14) {
        //qDebug() << sensors;

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
        updateMainTab(BMSTEMPERATURE.toInt(), BMSVOLTAGE.toInt(), CARSPEED.toInt(), POWER_KW.toInt());

        // Pedal positions tab
        updatePedalTab(BRAKE_PEDAL_POSITION, ACCELERATOR_PEDAL_POSITION);
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
    scanSerialPorts();
    ui->comboBoxSerialPorts->setCurrentIndex(0);
}

void MainWindow::showStartComms() {
    ui->endComms->hide();
    ui->startComms->show();
}

void MainWindow::showEndComms() {
    ui->startComms->hide();
    ui->endComms->show();
}

void MainWindow::on_endComms_clicked() {
    emit closeComms();
    clearComboBox();
}

void MainWindow::on_startComms_clicked() {
    emit startComms();
    runningTime.start();
}

void MainWindow::clearComboBox() {
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

void MainWindow::updateMainTab(int temp, int voltage, int speed, int power) {
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

//void MainWindow::clearData() {
//    suspensionLeftFrontX.clear();
//    suspensionLeftFrontY.clear();
//}

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

    double maxVoltage = 71.4; // 17 cells 4.2 volts
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
    ui->batteryTabCurrentGraph->yAxis->setRange(0,500); // PUT HIGHER THAN MAX CURRENT HERE
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
    ui->batteryTabVoltageGraph->yAxis->setRange(40,90);
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
    ui->pedalTabGraph->yAxis->setRange(1,100);
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
    ui->pedalTabGraph->yAxis->setRange(1,100);
    ui->pedalTabGraph->update();
}

void MainWindow::plotPedalGraph() {
    pedalAcceleratorPlot();
    pedalBrakePlot();
}

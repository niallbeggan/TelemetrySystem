#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    connect(thread, SIGNAL(sendDataToGUI(QString)), this, SLOT(updateGUI(QString)));
    connect(thread, SIGNAL(clearComboBox()), this, SLOT(clearComboBox()));
    connect(thread, SIGNAL(scanSerialPorts()), this, SLOT(scanSerialPorts()));
    connect(thread, SIGNAL(showStartComms()), this, SLOT(showStartComms()));
    connect(thread, SIGNAL(showEndComms()), this, SLOT(showEndComms()));

    // ui setup
    showStartComms();

    // Main Gauges Set Default values to temp voltage speed
    ui->Main_Battery_Temp_Gauge->setMinValue(0);
    ui->Main_Battery_Temp_Gauge->setMaxValue(100);
    ui->Main_Battery_Temp_Gauge->setThreshold(60);// Set these three parameters first
    ui->Main_Battery_Temp_Gauge->setValue(0);
    ui->Main_Battery_Temp_Gauge->setLabel("Temp");
    ui->Main_Battery_Temp_Gauge->setUnits("°C");
    ui->Main_Battery_Temp_Gauge->setSteps(30);

    ui->Main_Battery_Voltage_Gauge->setMinValue(0); // 17 series cells, 34 would be 2 volts per cell aka dead
    ui->Main_Battery_Voltage_Gauge->setMaxValue(73); // 17 cells, this would be 4.3 volts per cell, overcharged
    ui->Main_Battery_Voltage_Gauge->setThresholdEnabled(false);
    ui->Main_Battery_Voltage_Gauge->setValue(0);
    ui->Main_Battery_Voltage_Gauge->setLabel("Volts");
    ui->Main_Battery_Voltage_Gauge->setUnits("V");
    ui->Main_Battery_Voltage_Gauge->setSteps(30);

    ui->Main_Speed_Gauge->setMinValue(0);
    ui->Main_Speed_Gauge->setMaxValue(150);// A hopefully higher than feasible top speed
    ui->Main_Speed_Gauge->setThresholdEnabled(false);
    //ui->Main_Speed_Gauge->setThreshold(60); // Arbitrary value. Easy to change
    ui->Main_Speed_Gauge->setValue(0);
    ui->Main_Speed_Gauge->setLabel("Speed");
    ui->Main_Speed_Gauge->setUnits("Km/h");
    ui->Main_Speed_Gauge->setCoverGlassEnabled(true);
    ui->Main_Speed_Gauge->setSteps(30);
    ui->Main_Speed_Gauge->setDigitCount(3);

    //ui->Main_Speed_Gauge->setBackground(QColor("black"));

    // Graphing suspension
    suspensionLeftFront.append(suspensionLeftFrontX);
    suspensionLeftFront.append(suspensionLeftFrontY);

    suspensionRightFront.append(suspensionLeftFrontX);
    suspensionRightFront.append(suspensionLeftFrontY);

    suspensionLeftRear.append(suspensionLeftFrontX);
    suspensionLeftRear.append(suspensionLeftFrontY);

    suspensionRightRear.append(suspensionLeftFrontX);
    suspensionRightRear.append(suspensionLeftFrontY);

    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->plot2->addGraph();
    ui->plot2->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot2->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->plot3->addGraph();
    ui->plot3->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot3->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->plot4->addGraph();
    ui->plot4->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot4->graph(0)->setLineStyle(QCPGraph::lsLine);
}

MainWindow::~MainWindow() {
    delete ui;
}

//********************** Suspension graphing code here **********************//

void MainWindow::addPointsToGraph(QVector<QVector<double> > &graph, double x, double y) {
    graph[0].append(x);
    graph[1].append(y);
}

void MainWindow::clearData() {
    suspensionLeftFrontX.clear();
    suspensionLeftFrontY.clear();
}

void MainWindow::plotGraph1() {
    ui->plot->graph(0)->setData(suspensionLeftFront[0], suspensionLeftFront[1]);
    ui->plot->replot();
    double startOfXAxis = 0;
    if(suspensionLeftFront[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionLeftFront[0][suspensionLeftFront[0].length()-1];
    ui->plot->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot->yAxis->setRange(1,100);
    ui->plot->update();
}

void MainWindow::plotGraph2() {
    ui->plot2->graph(0)->setData(suspensionRightFront[0], suspensionRightFront[1]);
    ui->plot2->replot();
    double startOfXAxis = 0;
    if(suspensionRightFront[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionRightFront[0][suspensionRightFront[0].length()-1];
    ui->plot2->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot2->yAxis->setRange(1,100);
    ui->plot2->update();
}

void MainWindow::plotGraph3() {
    ui->plot3->graph(0)->setData(suspensionLeftRear[0], suspensionLeftRear[1]);
    ui->plot3->replot();
    double startOfXAxis = 0;
    if(suspensionLeftRear[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionLeftRear[0][suspensionLeftRear[0].length()-1];
    ui->plot3->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot3->yAxis->setRange(1,100);
    ui->plot3->update();
}

void MainWindow::plotGraph4() {
    ui->plot4->graph(0)->setData(suspensionRightRear[0], suspensionRightRear[1]);
    ui->plot4->replot();
    double startOfXAxis = 0;
    if(suspensionRightRear[0].length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = suspensionRightRear[0][suspensionRightRear[0].length()-1];
    ui->plot4->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot4->yAxis->setRange(1,100);
    ui->plot4->update();
}

//void MainWindow::plot() {
//    ui->plot->graph(0)->setData(suspensionLeftFrontX, suspensionLeftFrontY);
//    ui->plot->replot();
//    double startOfXAxis = 0;
//    if(suspensionLeftFrontX.length() == 0) {
//        startOfXAxis = 0;
//    }
//    else
//        startOfXAxis = suspensionLeftFrontX[suspensionLeftFrontX.length()-1];
//    ui->plot->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
//    ui->plot->yAxis->setRange(1,1030);
//    ui->plot->update();
//}

//void MainWindow::on_clearPlot_clicked() {
//    clearData();
//    plot();
//}

//void MainWindow::plot(QVector<QVector<double> > &graph) {
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

//************************MainTab functions************************//

void MainWindow::scanSerialPorts() {
    MainWindow::ui->comboBoxSerialPorts->clear();
    ui->comboBoxSerialPorts->addItem("");
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) { //Add all COM ports to the drop down.
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

void MainWindow::clearComboBox() {
    MainWindow::ui->comboBoxSerialPorts->clear();
}

void MainWindow::on_startComms_clicked() {
    emit startComms();
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

void MainWindow::updateMainTab(int temp, int voltage, int speed) {
    updateMainTemp(temp);
    updateMainVoltage(voltage);
    updateMainSpeed(speed);
}

//**********************Updating all displays ***************************//

void MainWindow::updateGUI(QString msg) {
    QApplication::processEvents();
    ui->textEdit_2->append(msg);
    if(msg != "") {

        // Suspension
        QStringList sensors = msg.split(",");
        //qDebug() << sensors;
        addPointsToGraph(suspensionLeftFront, suspensionLeftFront[0].length()+1, sensors[10].toDouble());
        addPointsToGraph(suspensionRightFront, suspensionRightFront[0].length()+1, sensors[11].toDouble());
        addPointsToGraph(suspensionLeftRear, suspensionLeftRear[0].length()+1, sensors[12].toDouble());
        addPointsToGraph(suspensionRightRear, suspensionRightRear[0].length()+1, sensors[13].toDouble());
        plotGraph1();
        plotGraph2();
        plotGraph3();
        plotGraph4();

        // Battery
        updateBatteryTab(sensors[1],sensors[2]);

        // Main Tab
        updateMainTab(sensors[10].toInt(),sensors[10].toInt(),sensors[10].toInt());
    }
}

//*********************************Battery tab functions *************************//

void MainWindow::updateBatteryTab(QString voltage, QString current) {
    ui->batteryVoltage->setValue(voltage.toInt());
    ui->batteryCurrent->setValue(current.toInt());
    ui->lcdVoltageNumber->display(voltage.toInt());
    ui->lcdCurrentNumber->display(current);
}

void MainWindow::on_pushButton_clicked() {
    ui->gaugetest->setValue(15);
    ui->gaugetest->setLabel("Temp");
    ui->gaugetest->setUnits("C");
    ui->gaugetest->setThreshold(10);
}

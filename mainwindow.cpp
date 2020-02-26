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
    connect(thread, SIGNAL(sendDataToGUI(QString)), this, SLOT(updateTextEdit(QString)));
    connect(thread, SIGNAL(clearComboBox()), this, SLOT(clearComboBox()));
    connect(thread, SIGNAL(scanSerialPorts()), this, SLOT(scanSerialPorts()));
    connect(thread, SIGNAL(showStartComms()), this, SLOT(showStartComms()));
    connect(thread, SIGNAL(showEndComms()), this, SLOT(showEndComms()));

    //ui setup
    showStartComms();

    //Graph
    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->plot2->addGraph();
    ui->plot2->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot2->graph(0)->setLineStyle(QCPGraph::lsLine);
}
MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addPoint(double x, double y) {
    qv_x.append(x);
    qv_y.append(y);
}

void MainWindow::addPoint2Graph(double x, double y) {
    qv_x2.append(x);
    qv_y2.append(y);
}

void MainWindow::clearData() {
    qv_x.clear();
    qv_y.clear();
}

void MainWindow::plot() {
    ui->plot->graph(0)->setData(qv_x, qv_y);
    ui->plot->replot();
    double startOfXAxis = 0;
    if(qv_x.length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = qv_x[qv_x.length()-1];
    ui->plot->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot->yAxis->setRange(1,1030);
    ui->plot->update();
}

void MainWindow::plotGraph2() {
    ui->plot2->graph(0)->setData(qv_x2, qv_y2);
    ui->plot2->replot();
    double startOfXAxis = 0;
    if(qv_x2.length() == 0) {
        startOfXAxis = 0;
    }
    else
        startOfXAxis = qv_x2[qv_x2.length()-1];
    ui->plot2->xAxis->setRange(startOfXAxis,(startOfXAxis-300));
    ui->plot2->yAxis->setRange(1,1030);
    ui->plot2->update();
}

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

void MainWindow::on_startComms_clicked() {
    emit startComms();
}

void MainWindow::on_comboBoxSerialPorts_activated(const QString &PortDescriptionAndNumber) {
    emit updateFromComboBox(PortDescriptionAndNumber);
}

void MainWindow::updateTextEdit(QString msg) {
    QApplication::processEvents();
    ui->textEdit_2->append(msg);
    if(msg != "") {
        QStringList sensors = msg.split(",");
        //qDebug() << sensors;
        double LeftBackSuspensionDisplacement13 = sensors[12].toDouble();
        double RightBackSuspensionDisplacement14 = sensors[13].toDouble();//add in other graph
        addPoint(qv_x.length()+1,LeftBackSuspensionDisplacement13);
        addPoint2Graph(qv_x2.length()+1,RightBackSuspensionDisplacement14);
        //qDebug() << "Plotting this value" << qv_x.length()+1 << LeftBackSuspensionDisplacement13;
        plot();
        //qDebug() << "Plotting this value" << qv_x2.length()+1 << RightBackSuspensionDisplacement14;
        plotGraph2();
    }
}

void MainWindow::on_endComms_clicked() {
    emit closeComms();
    clearComboBox();
}

void MainWindow::clearComboBox() {
    MainWindow::ui->comboBoxSerialPorts->clear();
}

//void MainWindow::on_clearPlot_clicked() {
//    clearData();
//    plot();
//}

void MainWindow::on_pushButton_clicked() {
    ui->gaugetest->setValue(15);
    ui->gaugetest->setLabel("Temp");
    ui->gaugetest->setUnits("C");
    ui->gaugetest->setThreshold(10);
}

void MainWindow::showStartComms() {
    ui->endComms->hide();
    ui->startComms->show();
}

void MainWindow::showEndComms() {
    ui->startComms->hide();
    ui->endComms->show();
}

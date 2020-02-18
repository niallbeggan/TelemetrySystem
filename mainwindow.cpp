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

    //Graph
    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsLine);
}
MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addPoint(double x, double y) {
    qv_x.append(x);
    qv_y.append(y);
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

void MainWindow::scanSerialPorts() {
    MainWindow::ui->comboBoxSerialPorts->clear();
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) { //Add all COM ports to the drop down.
        MainWindow::ui->comboBoxSerialPorts->addItem(port.description() + " (" + port.portName() + ")");
    }
}

void MainWindow::on_refreshPorts_clicked() {
    scanSerialPorts();
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
    double y = msg.toDouble();
    addPoint(qv_x.length()+1,y);
    qDebug() << "Plotting this value" << qv_x.length()+1 << y;
    plot();
}

void MainWindow::on_endComms_clicked() {
    emit closeComms();
    clearComboBox();
}

void MainWindow::clearComboBox() {
    MainWindow::ui->comboBoxSerialPorts->clear();
}

void MainWindow::on_clearPlot_clicked() {
    clearData();
    plot();
}

/* Arduino code for this.
 * void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
}
int x=0;
void loop() {
  x=x+1;
  Serial.write("Hello");
  Serial.print(x,DEC);
  Serial.write("\n");
  delay(50);
}
*/

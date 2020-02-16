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
}

MainWindow::~MainWindow() {
    delete ui;
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
    ui->textEdit_2->append(msg);
}

void MainWindow::on_endComms_clicked() {
    emit closeComms();
    clearComboBox();
}

void MainWindow::clearComboBox() {
    MainWindow::ui->comboBoxSerialPorts->clear();
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

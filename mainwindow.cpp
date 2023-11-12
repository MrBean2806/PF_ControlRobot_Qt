#include <QSerialPort>
#include <QSerialPortInfo>
#include <string>
#include <QChar>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"

typedef struct{
    float v_x;
    float v_y;
    float w_z;
    uint8_t crc;
}s_Trama_rx;

typedef union{
  s_Trama_rx trama_struct;
  char trama_char[13];
}u_Trama_rx;

typedef struct{
  uint32_t timestamp;       //contador que indica el ms en que se hizo la medicion
  uint16_t u_m   [4];          //velocidad angular en el eje de cada motor
  uint16_t a_m   [3];          //aceleraciÃ³n lineal medida en cada eje
  uint16_t phi_m [3];          //giro en cada eje
  uint16_t i_m   [4];          //corriente medida en cada motor
  uint16_t v_bat;
  uint16_t status;
  uint8_t crc;
}s_Trama_tx;

typedef union{
  s_Trama_tx trama_struct;
  char trama_char[37];
}u_Trama_tx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    puertoSerie = new QSerialPort(this);
    data_file = new QFile(data_file_name);
    csv_file = new QFile(csv_file_name);
    chart = new QChart();
    chartView = new QChartView(chart);

//    ui->button->setIcon(QIcon(":/icons/up.png"));
//    ui->button->setIconSize(QSize(65, 65));

    connect(ui->slider_velocidad, SIGNAL(valueChanged(int)), ui->label_duty, SLOT(setNum(int)));
    connect(ui->slider_velocidad, SIGNAL(valueChanged(int)), this, SLOT(enviarTrama()));
    connect(ui->button_up,    SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_down,  SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_right, SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_left,  SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_cw,    SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_ccw,   SIGNAL(clicked(bool)), this, SLOT(enviarTrama()));
    connect(ui->button_conectarPuertoSerie, SIGNAL(clicked(bool)), this, SLOT(abrirPuertoSerie()));
    connect(puertoSerie,      SIGNAL(readyRead()),   this, SLOT(leerTrama()));

//    abrirPuertoSerie();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::abrirPuertoSerie(){

    if(puertoSerieAbierto == true){
        puertoSerie->close();
        puertoSerieAbierto = false;
    }
    /*
     *  Testing code, prints the description, vendor id, and product id of all ports.
     *  Used it to determine the values for the sasori.
     *
     *
    qDebug() << "Number of ports: " << QSerialPortInfo::availablePorts().length() << "\n";
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        qDebug() << "Description: " << serialPortInfo.description() << "\n";
        qDebug() << "Has vendor id?: " << serialPortInfo.hasVendorIdentifier() << "\n";
        qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier() << "\n";
        qDebug() << "Has product id?: " << serialPortInfo.hasProductIdentifier() << "\n";
        qDebug() << "Product ID: " << serialPortInfo.productIdentifier() << "\n";
    }
    */
    bool sasori_is_available = false;

    QString sasori_port_name;
    //  For each available serial port
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        //  check if the serialport has both a product identifier and a vendor identifier
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            //  check if the product ID and the vendor ID match those of the sasori uno
            if((serialPortInfo.productIdentifier() == sasori_product_id)
                    && (serialPortInfo.vendorIdentifier() == sasori_vendor_id)){
                sasori_is_available = true; //    sasori uno is available on this port
               sasori_port_name = serialPortInfo.portName();
            }
        }
    }

    if(sasori_is_available){

        qDebug() << "Puerto encontrado...\n";
        puertoSerie->setPortName(sasori_port_name);
        puertoSerie->open(QSerialPort::ReadWrite);
        puertoSerie->setBaudRate(2000000);
        puertoSerie->setDataBits(QSerialPort::Data8);
        puertoSerie->setFlowControl(QSerialPort::NoFlowControl);
        puertoSerie->setParity(QSerialPort::NoParity);
        puertoSerie->setStopBits(QSerialPort::OneStop);
        puertoSerieAbierto = true;

//        enviarTrama();
        ui->label_puertoSerieAbierto->setText("Puerto Conectado");


    }else{
        qDebug() << "No pude encontrarse el puerto\n";
        puertoSerieAbierto = false;
        ui->label_puertoSerieAbierto->setText("Puerto Desconectado");

//        QMessageBox::information(this, "Serial Port Error", "Couldn't open serial port");
    }



}

void MainWindow::enviarTrama(){
    Trama_tx trama_tx;
//    trama_tx.data.start = 0xFF;
    trama_tx.data.duty_pwm = ui->slider_velocidad->value();
    trama_tx.data.freq_pwm = 1000;
    if(puertoSerieAbierto){
        puertoSerie->write(trama_tx.string, sizeof(s_Trama_tx));
        qDebug() << "Enviado" << trama_tx.data.freq_pwm << trama_tx.data.duty_pwm;
    }

}

void MainWindow::leerTrama(){
//    if(puertoSerieAbierto){
//        QByteArray serialBuffer = puertoSerie->readAll();
////        qDebug() << serialBuffer;
//        if(graficarVelocidad){
//            //Leo todo lo que haya en el puerto serie y lo guardo en el archivo
////            qDebug() << serialBuffer;
//            QTextStream stream(data_file);
//            stream << serialBuffer;

//        }else{
//            QByteArray vel_medida;
//            int i_final = serialBuffer.lastIndexOf('\n');
//            int i_espacio = serialBuffer.lastIndexOf(' ', i_final);
//            if(i_final > 0){
//                vel_medida = serialBuffer.sliced(i_espacio+1, i_final-i_espacio-1);
////                QString segundaPalabra = inputString.mid(espacioPos + 1, inputString.indexOf('\n') - espacioPos - 1);
////                ui->lcd_w_m->display(vel_medida.toInt());
//            }
//        }
//    }

}

void MainWindow::iniciarEnsayo(){
//    abrirPuertoSerie();
//    graficarVelocidad = true;
//    data_file->remove();
//    abrirArchivo(data_file);
//    ui->slider_velocidad->setValue(100);
//    QTimer::singleShot(1500, Qt::PreciseTimer, [&]() {
//        ui->slider_velocidad->setValue(0);
//    });
}

void MainWindow::detenerEnsayo(){
//    graficarVelocidad = false;
//    ui->slider_velocidad->setValue(0);
////    puertoSerie->close();
//    data_file->close();
//    graficarDatos();
}

void MainWindow::abrirArchivo(QFile *file){
//    qDebug() << "Abriendo archivo";

//    if(!file->open(QIODevice::ReadWrite | QIODevice::Text)){
////    if(!file->open(QIODevice::ReadWrite)){
//        qCritical() << "No se pudo abrir el archivo :(";
//        qCritical() << file->errorString();
//        return;
//    }
//    qInfo() << "Archivo abierto ...";
}

void MainWindow::graficarDatos(){

//    abrirArchivo(data_file);
//    QTextStream stream(data_file);
//    QLineSeries *serie_duty = new QLineSeries();
//    QLineSeries *serie_vel_medida = new QLineSeries();
//    int n = 0;  //numero de muestra

//    while(!stream.atEnd()){
//        QString line = stream.readLine();
////        qDebug() << line;
//        QStringList list = line.split(' ');
//        if(list.size() > 1){
//            QString duty = list.at(0);
//            QString vel_medida = list.at(1);
//            serie_duty->append(n, duty.toInt());
//            serie_vel_medida->append(n, vel_medida.toInt());
//            n++;
//        }
//    }

//    chart->setAnimationOptions(QChart::AllAnimations);
//    chart->removeAllSeries();
//    serie_duty->setName("Velocidad de referencia");
//    serie_vel_medida->setName("Velocidad medida");

//    chart->addSeries(serie_duty);
//    chart->addSeries(serie_vel_medida);
//    chart->createDefaultAxes();
//    chart->legend()->setVisible(true);
//    chartView->setRenderHint(QPainter::Antialiasing);

//    exportarArchivo();
}

void MainWindow::exportarArchivo(){
//    //convierte el archivo .txt a formato .csv para exportar los graficos a excel
//    int n = 1;


//    if( !data_file->isOpen() ){
//        abrirArchivo(data_file);
//    }
//    data_file->seek(0);
//    csv_file->remove();
//    abrirArchivo(csv_file);

//    QTextStream stream_out(data_file);
//    QTextStream stream_in(csv_file);
////    qDebug() << data_file->pos();
//    stream_in << "time,duty,wm\n";
//    stream_in << " ,Interp:linear,Interp:linear\n";
//    stream_in << "Unit:s, ,Unit: rpm\n";

//    while(!stream_out.atEnd()){
//        QString line = stream_out.readLine();
//        line = line.replace(' ', ',');
//        stream_in << n/1000.0 << ',' << line << '\n';
//        n++;
//    }
//    data_file->close();
//    csv_file->close();

}


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



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    puertoSerie = new QSerialPort(this);
    trama_tx = new u_Trama_tx;

    data_file = new QFile(data_file_name);
    csv_file = new QFile(csv_file_name);
    chart = new QChart();
    chartView = new QChartView(chart);

//    ui->button->setIcon(QIcon(":/icons/up.png"));
//    ui->button->setIconSize(QSize(65, 65));

    connect(ui->slider_velocidad, SIGNAL(valueChanged(int)), ui->label_velocidad, SLOT(setNum(int)));
    connect(ui->slider_velocidad, SIGNAL(valueChanged(int)), this, SLOT(enviarTrama()));
    connect(ui->button_up,    &QPushButton::clicked, [this](){  this->setVel( 1, 0, 0);  });
    connect(ui->button_down,  &QPushButton::clicked, [this](){  this->setVel(-1, 0, 0);  });
    connect(ui->button_right, &QPushButton::clicked, [this](){  this->setVel( 0,-1, 0);  });
    connect(ui->button_left,  &QPushButton::clicked, [this](){  this->setVel( 0, 1, 0);  });
    connect(ui->button_cw,    &QPushButton::clicked, [this](){  this->setVel( 0, 0, 1);  });
    connect(ui->button_ccw,   &QPushButton::clicked, [this](){  this->setVel( 0, 0,-1);  });

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

void  MainWindow::setVel(float v_x, float v_y, float w_z){
    float vel = ui->slider_velocidad->value()/100.0;
    trama_tx->data.v_x = v_x*vel;
    trama_tx->data.v_y = v_y*vel;
    trama_tx->data.w_z = w_z*vel;
    qDebug() << "Enviado" <<  trama_tx->data.v_x <<  trama_tx->data.v_y <<  trama_tx->data.w_z;
    enviarTrama();
}

void MainWindow::enviarTrama(){
//    Trama_tx trama_tx;
//    trama_tx.data.freq_pwm = 1000;

    if(puertoSerieAbierto){
        puertoSerie->write(trama_tx->string, sizeof(s_Trama_tx));
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


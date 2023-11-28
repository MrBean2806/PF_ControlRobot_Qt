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
#include <QtMath>
#include <iostream>
#include <vector>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    puertoSerie = new QSerialPort(this);
    trama_tx = new u_Trama_tx;
    timer = new QTimer(this);
    vel_espacial_actual = new Vel_espacial();
    vel_espacial_final = new Vel_espacial();

//    data_file = new QFile(data_file_name);
//    csv_file = new QFile(csv_file_name);
//    chart = new QChart();
//    chartView = new QChartView(chart);

//    QPixmap imagen(":/icons/RobotOmni.png");
//    imagen = imagen.scaled(300, 200, Qt::KeepAspectRatio);
//    ui->label_ImagenRobot->setPixmap(imagen);
//    ui->label_ImagenRobot->show();


    connect(ui->slider_vx, SIGNAL(valueChanged(int)), ui->label_vx, SLOT(setNum(int)));
    connect(ui->slider_vy, SIGNAL(valueChanged(int)), ui->label_vy, SLOT(setNum(int)));
    connect(ui->slider_wz, SIGNAL(valueChanged(int)), ui->label_wz, SLOT(setNum(int)));

    connect(ui->slider_vx, SIGNAL(sliderReleased()), this, SLOT(setVel()));
    connect(ui->slider_vy, SIGNAL(sliderReleased()), this, SLOT(setVel()));
    connect(ui->slider_wz, SIGNAL(sliderReleased()), this, SLOT(setVel()));


//    connect(ui->button_up,    &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });
//    connect(ui->button_down,  &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });
//    connect(ui->button_right, &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });
//    connect(ui->button_left,  &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });
//    connect(ui->button_cw,    &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });
//    connect(ui->button_ccw,   &QPushButton::clicked, [this](){  this->setVel( 0, 0, 0);  });

//    connect(&timer, &QTimer::timeout, this, SLOT(enviarTrama()));

    connect(ui->button_conectarPuertoSerie, SIGNAL(clicked(bool)), this, SLOT(abrirPuertoSerie()));
    connect(puertoSerie, SIGNAL(readyRead()), this, SLOT(leerTrama()));

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

void MainWindow::setVel(){
    if(velocidadEstable){
        float l = 0.15;
        float w = 0.125;
        float r = 0.05;

        float vx_ref = ui->slider_vx->value()/100.0;
        float vy_ref = ui->slider_vy->value()/100.0;
        float wz_ref = ui->slider_wz->value()/100.0;

        ui->label_u_ref_1->setText(QString::number((30.0/3.1415)*(1.0/r)*((-l-w)*wz_ref + vx_ref - vy_ref)));
        ui->label_u_ref_2->setText(QString::number((30.0/3.1415)*(1.0/r)*(( l+w)*wz_ref + vx_ref + vy_ref)));
        ui->label_u_ref_3->setText(QString::number((30.0/3.1415)*(1.0/r)*(( l+w)*wz_ref + vx_ref - vy_ref)));
        ui->label_u_ref_4->setText(QString::number((30.0/3.1415)*(1.0/r)*((-l-w)*wz_ref + vx_ref + vy_ref)));

        vel_espacial_final->setVel(vx_ref, vy_ref, wz_ref);
        qDebug() << "Vel inicial";
        vel_espacial_actual->imprimirVelocidades();
        qDebug() << "Vel final";
        vel_espacial_final->imprimirVelocidades();

//        velocidadEstable = false;
        generarRampaVelocidad(vel_espacial_actual, vel_espacial_final);
        qDebug() << "Sali";
//        vel_espacial_actual = vel_espacial_final;
        vel_espacial_actual->setVel(vel_espacial_final->vx, vel_espacial_final->vy, vel_espacial_final->wz);
//        enviarTrama();
    }
}

void MainWindow::generarRampaVelocidad(Vel_espacial * v_inicial, Vel_espacial * v_final){
    float a_max   = 0.44;         // [m/s^2]
    int data_freq = 50;      //frecuencia [Hz] con que se hacen las actualizaciones de velocidad
    float t_step  = 1.0/data_freq;
    float Delta_t = 0;
    int N = 0;
    float vel_inicial = 0, vel_final = 0;

    if(v_inicial->vx != v_final->vx){
        vel_inicial = v_inicial->vx;
        vel_final   = v_final->vx;
    }else if(v_inicial->vy != v_final->vy){
        vel_inicial = v_inicial->vy;
        vel_final   = v_final->vy;
    }else if(v_inicial->wz != v_final->wz){
        vel_inicial = v_inicial->wz;
        vel_final   = v_final->wz;
    }

//    if(vel_final - vel_inicial < 0)  a_max = -a_max;
//    Delta_t = (vel_final-vel_inicial)/a_max;
//    Delta_t = qRound(Delta_t/t_step)*t_step; //me aseguro que el delta sea multiplo de t_step
//    N = Delta_t/t_step + 1;       //Cantidad de pasos de la rampa

    if(vel_final<vel_inicial)  a_max=-a_max;
    Delta_t = (vel_final-vel_inicial)/a_max;
    Delta_t = qRound(Delta_t/t_step)*t_step; //me aseguro que el delta sea multiplo de t_step
    N = Delta_t/t_step + 1;

//    qDebug() << "N: " << N << "Delta_t: " << Delta_t << "t_step: " << t_step;
//    qDebug() << "N: " << N;
    rampa_vx = new std::vector<float>(N, 0);
    rampa_vy = new std::vector<float>(N, 0);
    rampa_wz = new std::vector<float>(N, 0);
    std::vector<float> rampa(N, 0);

    for(int n = 0; n < N; n++){
        rampa[n] = a_max*t_step*n - a_max*Delta_t + vel_final;
//        qDebug() << rampa_vel[n];
    }
    if(v_inicial->vx != v_final->vx){
        std::copy(rampa.begin(), rampa.end(), rampa_vx->begin());
    }else if(v_inicial->vy != v_final->vy){
        std::copy(rampa.begin(), rampa.end(), rampa_vy->begin());
    }else if(v_inicial->wz != v_final->wz){
        std::copy(rampa.begin(), rampa.end(), rampa_wz->begin());
    }

    qDebug() << "Rampa vx";
    for (auto iter = rampa_vx->begin(); iter != rampa_vx->end(); ++iter) {
        qDebug() << *iter;
    }
//    qDebug() << "Rampa vy";
//    for (auto iter = rampa_vy->begin(); iter != rampa_vy->end(); ++iter) {
//        qDebug() << *iter;
//    }
//    qDebug() << "Rampa wz";
//    for (auto iter = rampa_wz->begin(); iter != rampa_wz->end(); ++iter) {
//        qDebug() << *iter;
//    }
}

void MainWindow::enviarTrama(){
    static int n = 0;
    if(puertoSerieAbierto){
//        if(n != ){
//            trama_tx->data.v_x = rampa_vx[n];
//            trama_tx->data.v_y = rampa_vy[n];
//            trama_tx->data.w_z = rampa_wz[n];

//            puertoSerie->write(trama_tx->string, sizeof(s_Trama_tx));
//            qDebug() << "Enviado" <<  trama_tx->data.v_x <<  trama_tx->data.v_y <<  trama_tx->data.w_z;
//            n++;
//        }
//        delete rampa_vx;
//        delete rampa_vy;
//        delete rampa_wz;
    }

}

void MainWindow::leerTrama(){
//    u_Trama_rx trama_rx;
//    if(puertoSerieAbierto){
//        QByteArray serialBuffer = puertoSerie->readAll();
////        qDebug() << serialBuffer;
//        int i_inicio = serialBuffer.indexOf('[');
//        int i_final  = serialBuffer.indexOf(']', i_inicio);
//        while(i_inicio >= 0 && i_final >= 0){
//                memcpy(trama_rx.string, serialBuffer.sliced(i_inicio, i_final-i_inicio-1), sizeof(s_Trama_rx));
////                qDebug() << "Extraido: " << serialBuffer.sliced(i_inicio, i_final-i_inicio-1), sizeof(s_Trama_rx);
//                i_inicio = serialBuffer.indexOf('[', i_final);
//                i_final  = serialBuffer.indexOf(']', i_inicio);
//        }
//        ui->label_u_ref_1->setText(trama_rx.data.u_m[0]);
//        ui->label_u_ref_2->setText(trama_rx.data.u_m[1]);
//        ui->label_u_ref_3->setText(trama_rx.data.u_m[2]);
//        ui->label_u_ref_4->setText(trama_rx.data.u_m[3]);
////        enviarTrama();
//        if(graficarVelocidad){
//            //Leo todo lo que haya en el puerto serie y lo guardo en el archivo
////            qDebug() << serialBuffer;
////            QTextStream stream(data_file);
////            stream << serialBuffer;

//        }else{
////            QByteArray vel_medida;
//            QByteArray duty = 0;

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


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
#include <QPixmap>
#include <QImageReader>
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

    //Agrego vista superior del robot
    QImageReader::setAllocationLimit(0);
    QImage imagen("D:/Qt_Projects/PF_ControlRobot/icons/RobotOmni.jpeg");
    QPixmap imagenRobot = QPixmap::fromImage(imagen);
    imagenRobot = imagenRobot.scaled(300, 200, Qt::KeepAspectRatio);
    ui->label_ImagenRobot->setPixmap(imagenRobot);
    ui->label_ImagenRobot->show();

    //// CONEXIONES ENTRE OBJETOS Y SLOTS
    //Al mover el slider se muestra el valor elegido en m/s en un label
    connect(ui->slider_vx, &QSlider::sliderMoved, [this](){  ui->label_vx->setText(QString::number(ui->slider_vx->value()/100.0, 'f', 2)); });
    connect(ui->slider_vy, &QSlider::sliderMoved, [this](){  ui->label_vy->setText(QString::number(ui->slider_vy->value()/100.0, 'f', 2)); });
    connect(ui->slider_wz, &QSlider::sliderMoved, [this](){  ui->label_wz->setText(QString::number(ui->slider_wz->value()/100.0, 'f', 2)); });

    //Reseteo de slider con botones
    connect(ui->button_reset_vx,  &QPushButton::clicked, [this](){  ui->slider_vx->setValue(0); });
    connect(ui->button_reset_vy,  &QPushButton::clicked, [this](){  ui->slider_vy->setValue(0); });
    connect(ui->button_reset_wz,  &QPushButton::clicked, [this](){  ui->slider_wz->setValue(0); });
    connect(ui->button_reset_vx, SIGNAL(clicked(bool)), this, SLOT(setVel()));
    connect(ui->button_reset_vy, SIGNAL(clicked(bool)), this, SLOT(setVel()));
    connect(ui->button_reset_wz , SIGNAL(clicked(bool)), this, SLOT(setVel()));

    //Cuando se suelta el slider se actualiza la velocidad seteada
    connect(ui->slider_vx, SIGNAL(sliderReleased()), this, SLOT(setVel()));
    connect(ui->slider_vy, SIGNAL(sliderReleased()), this, SLOT(setVel()));
    connect(ui->slider_wz, SIGNAL(sliderReleased()), this, SLOT(setVel()));

    //Timer
    connect(timer, SIGNAL(timeout()), this, SLOT(enviarTrama()));

    //Señales puerto serie
    connect(ui->button_conectarPuertoSerie, SIGNAL(clicked(bool)), this, SLOT(abrirPuertoSerie()));
    connect(puertoSerie, SIGNAL(readyRead()), this, SLOT(leerTrama()));

    //Abro el puerto serie al iniciar el programa
    abrirPuertoSerie();

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
    //Descomentando el siguiente codigo se puede encontrar los datos mencionados del dispositivo USB a conectar
    //Una vez obtenidos, se tienen que agregar en los miembros de la clase MainWindow
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
    // parametros cinematicos del robot
    float l = 0.15;
    float w = 0.125;
    float r = 0.05;

    float vx_ref = ui->slider_vx->value()/100.0;
    float vy_ref = ui->slider_vy->value()/100.0;
    float wz_ref = ui->slider_wz->value()/100.0;

    ui->label_vx->setText(QString::number(vx_ref, 'f', 2));
    ui->label_vy->setText(QString::number(vy_ref, 'f', 2));
    ui->label_wz->setText(QString::number(wz_ref, 'f', 2));

    // Muestro la velocidad seteada para cada motor
    ui->label_u_ref_1->setText(QString::number((30.0/3.1415)*(1.0/r)*((-l-w)*wz_ref + vx_ref - vy_ref), 'f', 2) + " rpm");
    ui->label_u_ref_2->setText(QString::number((30.0/3.1415)*(1.0/r)*(( l+w)*wz_ref + vx_ref + vy_ref), 'f', 2) + " rpm");
    ui->label_u_ref_3->setText(QString::number((30.0/3.1415)*(1.0/r)*(( l+w)*wz_ref + vx_ref - vy_ref), 'f', 2) + " rpm");
    ui->label_u_ref_4->setText(QString::number((30.0/3.1415)*(1.0/r)*((-l-w)*wz_ref + vx_ref + vy_ref), 'f', 2) + " rpm");

    vel_espacial_final->setVel(vx_ref, vy_ref, wz_ref);
    qDebug() << "Vel inicial";
    vel_espacial_actual->imprimirVelocidades();
    qDebug() << "Vel final";
    vel_espacial_final->imprimirVelocidades();

    generarRampaVelocidad(vel_espacial_actual, vel_espacial_final); //Genero la rampa de velocidad para cada componente
    vel_espacial_actual->setVel(vel_espacial_final->vx, vel_espacial_final->vy, vel_espacial_final->wz);    //Actualizo la velocidad actual
    timer->start(data_freq);    //Llama a enviarTrama() con un periodo de data_freq para enviar las velocidades intermedias de las rampas
}

// Se encarga de generar una transicion lineal entre las velocidades iniciales y finales seteadas
// Los valores intermedios son guardados en vectores miembros de la clase MainWindow, y transmitidos de a uno
void MainWindow::generarRampaVelocidad(Vel_espacial * v_inicial, Vel_espacial * v_final){
    float a_max   = 0.5;    // [m/s^2]
    data_freq = 20;         // [ms] periodo con el que se envian los datos de la rampa
    float t_step  = data_freq/1000.0;
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

    if(vel_final<vel_inicial)  a_max = -a_max;
    Delta_t = (vel_final-vel_inicial)/a_max;
    Delta_t = qRound(Delta_t/t_step)*t_step; //me aseguro que el delta sea multiplo de t_step
    N       = Delta_t/t_step + 1;

    rampa_vx = new std::vector<float>(N, v_inicial->vx);
    rampa_vy = new std::vector<float>(N, v_inicial->vy);
    rampa_wz = new std::vector<float>(N, v_inicial->wz);
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

}

// Un timer llama esta funcion con una frecuencia determinada, se envian los 3 componentes de la velocidad espacial hasta terminar la rampa
void MainWindow::enviarTrama(){
    static int n = 0;
    int N = rampa_vx->size();
    if(puertoSerieAbierto){
        if(n < N){
            trama_tx->data.v_x = rampa_vx->at(n);
            trama_tx->data.v_y = rampa_vy->at(n);
            trama_tx->data.w_z = rampa_wz->at(n);

            puertoSerie->write(trama_tx->string, sizeof(s_Trama_tx));
            qDebug() << "Enviado" <<  trama_tx->data.v_x <<  trama_tx->data.v_y <<  trama_tx->data.w_z;
            ui->textBrowser_terminal->append(">> " + QString::number(trama_tx->data.v_x)+"  "+ QString::number(trama_tx->data.v_y)+"  "+QString::number(trama_tx->data.w_z));
            n++;
        }else{
            timer->stop();
            n = 0;
            delete rampa_vx;
            delete rampa_vy;
            delete rampa_wz;
        }
    }
}

// Leo de a bloques de caracteres desde el puerto serie
// Identifico y extraigo la trama buscando los caracteres delimitadores '[' y ']'
void MainWindow::leerTrama(){
    u_Trama_rx trama_rx;
    bool trama_valida = false;
    if(puertoSerieAbierto){
        QByteArray serialBuffer = puertoSerie->readAll();
//        qDebug() << "\nLlego: " << serialBuffer;
//        qDebug() << "Largo: " << serialBuffer.length();
        int i_inicio = serialBuffer.indexOf('[');
        int i_final  = serialBuffer.indexOf(']', i_inicio);
        while(i_inicio >= 0 && i_final >= 0){
                if(i_final-i_inicio == sizeof(s_Trama_rx)-1){
                    trama_valida = true;
//                    qDebug() << "Trama valida";
                    memcpy(trama_rx.string, serialBuffer.sliced(i_inicio, i_final-i_inicio-1), sizeof(s_Trama_rx));
//                    qDebug() << "Extraido: " << serialBuffer.sliced(i_inicio, i_final-i_inicio-1), sizeof(s_Trama_rx);
                }
                i_inicio = serialBuffer.indexOf('[', i_final);
                i_final  = serialBuffer.indexOf(']', i_inicio);
        }
        if(trama_valida){
            //Actualizo las etiquetas de la UI
            float u1 = (ui->label_u_ref_1->text().contains("-")) ? -trama_rx.data.u_m[0] : trama_rx.data.u_m[0];
            float u2 = (ui->label_u_ref_2->text().contains("-")) ? -trama_rx.data.u_m[1] : trama_rx.data.u_m[1];
            float u3 = (ui->label_u_ref_3->text().contains("-")) ? -trama_rx.data.u_m[2] : trama_rx.data.u_m[2];
            float u4 = (ui->label_u_ref_4->text().contains("-")) ? -trama_rx.data.u_m[3] : trama_rx.data.u_m[3];

            ui->label_u_m_1->setText(QString::number(u1));
            ui->label_u_m_2->setText(QString::number(u2));
            ui->label_u_m_3->setText(QString::number(u3));
            ui->label_u_m_4->setText(QString::number(u4));

            ui->lcd_i_m->display(trama_rx.data.i_m);
            ui->lcd_vbat->display(trama_rx.data.v_bat);
            ui->lcd_am1->display(trama_rx.data.a_m[0]);
            ui->lcd_am2->display(trama_rx.data.a_m[1]);
            ui->lcd_am3->display(trama_rx.data.a_m[2]);
            ui->lcd_phim1->display(trama_rx.data.phi_m[0]);
            ui->lcd_phim2->display(trama_rx.data.phi_m[1]);
            ui->lcd_phim3->display(trama_rx.data.phi_m[2]);
        }
    }

}


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QByteArray>
#include <QFile>
#include <QTimer>
#include <QtCharts>
#include <iostream>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct{
    float v_x;
    float v_y;
    float w_z;
    uint8_t crc;
}s_Trama_tx;

typedef union{
  s_Trama_tx data;
  char string[16];
}u_Trama_tx;

typedef struct{
    uint32_t  start;
    uint32_t  timestamp;       //contador que indica el ms en que se hizo la medicion
    float     u_m   [4];          //velocidad angular en el eje de cada motor
    int16_t   a_m   [3];          //aceleraciÃ³n lineal medida en cada eje
    int16_t   phi_m [3];          //giro en cada eje
    uint16_t  i_m;          //corriente medida en cada motor
    uint16_t  v_bat;
    uint16_t  status;
    uint8_t   crc;
    uint8_t   stop;
}s_Trama_rx;

typedef union{
  s_Trama_rx data;
  char string[44];
}u_Trama_rx;

class Vel_espacial {
public:
    float vx;  // Velocidad lineal en x
    float vy;  // Velocidad lineal en y
    float wz;  // Velocidad angular alrededor del eje z

    // Constructor por defecto
    Vel_espacial() : vx(0.0), vy(0.0), wz(0.0) {}
    // Constructor con parámetros
    Vel_espacial(float vx, float vy, float wz) : vx(vx), vy(vy), wz(wz) {}
    // Contructor con otro objeto
    Vel_espacial(const Vel_espacial& otro) : vx(otro.vx), vy(otro.vy), wz(otro.wz) {}

    // Imprime las velocidades espaciales
    void imprimirVelocidades() const {
        qDebug() << "vx = " << vx << ", vy = " << vy << ", wz = " << wz ;
    }
    // Setea las velocidades individuales
    void setVel(float _vx, float _vy, float _wz){ vx = _vx; vy = _vy; wz = _wz;}

    // Sobrecarga del operador de asignación '='
    Vel_espacial& operator=(const Vel_espacial& otro) {
        if (this != &otro) {
            // Evitar autoasignación
            vx = otro.vx;
            vy = otro.vy;
            wz = otro.wz;
        }
        return *this;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void abrirPuertoSerie();
    void leerTrama();
    void enviarTrama();
    void setVel();

private:
    Ui::MainWindow *ui;
    QSerialPort *puertoSerie;
    u_Trama_tx *trama_tx;
    QTimer *timer;
    Vel_espacial *vel_espacial_actual;
    Vel_espacial *vel_espacial_final;
    std::vector<float> *rampa_vx;
    std::vector<float> *rampa_vy;
    std::vector<float> *rampa_wz;

    static const quint16 sasori_vendor_id = 1027;   //parametros propios del conversor USB
    static const quint16 sasori_product_id = 24577;
    float data_freq = 20;         // [ms] periodo con el que se envian los datos de la rampa
    bool puertoSerieAbierto = false;
    bool graficarVelocidad  = false;
    void abrirArchivo(QFile *);
    void generarRampaVelocidad(Vel_espacial *, Vel_espacial *);

};


#endif // MAINWINDOW_H

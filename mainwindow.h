#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QByteArray>
#include <QFile>
#include <QTimer>
#include <QtCharts>


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
    uint32_t start;
    uint32_t timestamp;       //contador que indica el ms en que se hizo la medicion
    float    u_m   [4];          //velocidad angular en el eje de cada motor
    float    a_m   [3];          //aceleraciÃ³n lineal medida en cada eje
    float    phi_m [3];          //giro en cada eje
    uint16_t i_m;          //corriente medida en cada motor
    uint16_t v_bat;
    uint16_t status;
    uint8_t  crc;
    uint8_t  stop;
}s_Trama_rx;

typedef union{
  s_Trama_rx data;
  char string[52];
}u_Trama_rx;

class Vel_espacial {
public:
    float vx;  // Componente en la dirección x
    float vy;  // Componente en la dirección y
    float wz;  // Componente de rotación alrededor del eje z

    // Constructor por defecto
    Vel_espacial() : vx(0.0), vy(0.0), wz(0.0) {}

    // Constructor con parámetros
    Vel_espacial(float vx, float vy, float wz) : vx(vx), vy(vy), wz(wz) {}

    // Método para imprimir las velocidades espaciales
    void imprimirVelocidades() const {
        qDebug() << "vx: " << vx << ", vy: " << vy << ", wz: " << wz ;
    }
    void setVel(float _vx, float _vy, float _wz){ vx = _vx; vy = _vy; wz = _wz;}
    float getMag(){ return vx*vx + vy*vy + wz*wz; }
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
    void iniciarEnsayo();
    void detenerEnsayo();
    void graficarDatos();
    void exportarArchivo();


private:
    Ui::MainWindow *ui;
    QSerialPort *puertoSerie;
    u_Trama_tx *trama_tx;
    QTimer *timer;
    Vel_espacial *vel_espacial_actual;
    Vel_espacial *vel_espacial_final;
    float *rampaVelEspacial;
    float *rampa_vx;
    float *rampa_vy;
    float *rampa_wz;
    bool puertoSerieAbierto = false;
    bool graficarVelocidad  = false;
    bool velocidadEstable   = true;
    static const quint16 sasori_vendor_id = 1027;
    static const quint16 sasori_product_id = 24577;
    void abrirArchivo(QFile *);
    void setVel(float, float, float);
    float * generarRampaVelocidad(float, float);

};


#endif // MAINWINDOW_H

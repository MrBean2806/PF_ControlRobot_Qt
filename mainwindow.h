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
  char string[13];
}u_Trama_tx;

typedef struct{
  uint32_t timestamp;       //contador que indica el ms en que se hizo la medicion
  uint16_t u_m   [4];          //velocidad angular en el eje de cada motor
  uint16_t a_m   [3];          //aceleraciÃ³n lineal medida en cada eje
  uint16_t phi_m [3];          //giro en cada eje
  uint16_t i_m   [4];          //corriente medida en cada motor
  uint16_t v_bat;
  uint16_t status;
  uint8_t crc;
}s_Trama_rx;

typedef union{
  s_Trama_rx data;
  char string[37];
}u_Trama_rx;

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
    QFile *data_file;
    QFile *csv_file;
    QString data_file_name = "data.txt";
    QString csv_file_name = "data.csv";
    QChart *chart;
    QChartView *chartView;
    QTimer *timer;
    bool puertoSerieAbierto = false;
    bool graficarVelocidad = false;
    static const quint16 sasori_vendor_id = 1027;
    static const quint16 sasori_product_id = 24577;
    void abrirArchivo(QFile *);
    void setVel(float, float, float);

//    void writeFile(QString);
};
#endif // MAINWINDOW_H

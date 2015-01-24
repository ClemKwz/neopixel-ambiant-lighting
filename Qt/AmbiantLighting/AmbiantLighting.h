#ifndef AMBIANTLIGHTING_H
#define AMBIANTLIGHTING_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

class QGraphicsScene;

namespace Ui
{
    class AMBIANT_LIGHTING;
}

#define LEDS_COUNT              30

class AMBIANT_LIGHTING : public QMainWindow
{
    Q_OBJECT

public:
    explicit                    AMBIANT_LIGHTING(QWidget *pParent = 0);
                                ~AMBIANT_LIGHTING();

private slots:
    void                        Loop();

protected:
    void                        OpenSerialPort();
    void                        CleanLeds();
    void                        UpdateLed(int nID, int nR, int nG, int nB);
    void                        ProcessAverage();
    void                        SendValues();

private:
    Ui::AMBIANT_LIGHTING*       _pUi;
    QGraphicsScene*             _pGraphicsScene;
    QSerialPort*                _pSerial;
    QList<QSerialPortInfo>      _PortInfoList;
    int                         _pLeds[LEDS_COUNT][4];
};

#endif // AMBIANTLIGHTING_H

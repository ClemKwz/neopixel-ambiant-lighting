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

    enum PIXELS_ANALYSIS_MODE
    {
        ALL_SCREEN_MODE = 0,
        CINEMASCOPE_MODE
    };

public:
    explicit                    AMBIANT_LIGHTING(QWidget *pParent = 0);
                                ~AMBIANT_LIGHTING();

private slots:
    void                        Loop();
    void                        OnModeChanged(bool);

protected:
    void                        OpenSerialPort();
    void                        CleanLeds();
    void                        UpdateLed(int nID, int nR, int nG, int nB);
    void                        ProcessAverage();
    void                        SendValues();
    void                        ProcessLeftSide(int nY, int nR, int nG, int nB);
    void                        ProcessRightSide(int nY, int nR, int nG, int nB);
    void                        ProcessTopSide(int nX, int nR, int nG, int nB);

private:
    Ui::AMBIANT_LIGHTING*       _pUi;
    QSerialPort*                _pSerial;
    QList<QSerialPortInfo>      _PortInfoList;

    PIXELS_ANALYSIS_MODE        _eMode;

    int                         _nWidth;
    int                         _nHeight;
    int                         _nCinemaScopeBlackBarHeight;

    int                         _pLeds[LEDS_COUNT][4];
};

#endif // AMBIANTLIGHTING_H

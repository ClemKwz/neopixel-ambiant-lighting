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

private:
    Ui::AMBIANT_LIGHTING*       _pUi;
    QGraphicsScene*             _pGraphicsScene;
    QSerialPort*                _pSerial;
    QList<QSerialPortInfo>      _PortInfoList;
};

#endif // AMBIANTLIGHTING_H

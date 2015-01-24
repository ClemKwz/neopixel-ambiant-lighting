#include "AmbiantLighting.h"
#include "ui_AmbiantLighting.h"
#include <QDesktopWidget>
#include <QScreen>
#include <QGraphicsScene>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include "windows.h"

//-----------------------------------------------------------
AMBIANT_LIGHTING::AMBIANT_LIGHTING(QWidget *pParent)
    : QMainWindow       (pParent)
    , _pUi              (new Ui::AMBIANT_LIGHTING)
    , _pGraphicsScene   (NULL)
{
    _pUi->setupUi(this);

    _pSerial = new QSerialPort(this);
    _PortInfoList = QSerialPortInfo::availablePorts();

    OpenSerialPort();

    _pGraphicsScene = new QGraphicsScene(this);
    _pUi->graphicsView->setScene(_pGraphicsScene);

    QTimer::singleShot(0, this, SLOT(Loop()));
}

//-----------------------------------------------------------
AMBIANT_LIGHTING::~AMBIANT_LIGHTING()
{
    delete _pUi;
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::OpenSerialPort()
{
    int nIndex = 0;
    int nFound = -1;
    foreach(const QSerialPortInfo& info, QSerialPortInfo::availablePorts())
    {
        if(info.description() == "Arduino Uno")
        {
            nFound = nIndex;
            break;
        }
        ++nIndex;
    }

    if (nFound == -1)		// failed !
        return;

    _pSerial->setPort(_PortInfoList[nFound]);

    _pSerial->setBaudRate(QSerialPort::Baud57600);
    _pSerial->setFlowControl(QSerialPort::NoFlowControl);
    _pSerial->setParity(QSerialPort::NoParity);
    _pSerial->setDataBits(QSerialPort::Data8);
    _pSerial->setStopBits(QSerialPort::OneStop);

    if(_pSerial->open(QIODevice::ReadWrite))
    {
        _pSerial->setBaudRate(QSerialPort::Baud57600);
        _pSerial->setFlowControl(QSerialPort::NoFlowControl);
        _pSerial->setParity(QSerialPort::NoParity);
        _pSerial->setDataBits(QSerialPort::Data8);
        _pSerial->setStopBits(QSerialPort::OneStop);
        _pSerial->setDataTerminalReady(true);
        _pSerial->setRequestToSend(true);

        QString sMsg = "Connected to : ";
        sMsg += _PortInfoList[nFound].description();
        sMsg += " (" + _PortInfoList[nFound].portName() + ")";
        _pUi->lineEdit->setText(sMsg);
    }
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::Loop()
{
    const QPixmap& Pixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());

    const QImage& Image = Pixmap.toImage();
    const QSize& ImageSize = Image.size();

    const int nWidth = ImageSize.width();
    const int nHeight = ImageSize.height();

    int nDelta = 4;

    CleanLeds();

    for(int nCptWidth = 0;nCptWidth < ImageSize.width();nCptWidth+=nDelta)
    {
        for(int nCptHeight = 0;nCptHeight < ImageSize.height();nCptHeight+=nDelta)
        {
            const QRgb& PixelColor = Image.pixel(nCptWidth, nCptHeight);
            int nR = qRed(PixelColor);
            int nG = qGreen(PixelColor);
            int nB = qBlue(PixelColor);

            if(nCptWidth < nWidth/4)
            {
                UpdateLed(0, nR, nG, nB);
                UpdateLed(1, nR, nG, nB);
                UpdateLed(2, nR, nG, nB);
                UpdateLed(3, nR, nG, nB);
                UpdateLed(4, nR, nG, nB);
                UpdateLed(5, nR, nG, nB);
                UpdateLed(6, nR, nG, nB);
                UpdateLed(7, nR, nG, nB);
            }
            if(nCptWidth > ((nWidth/4)*3))
            {
                UpdateLed(19, nR, nG, nB);
                UpdateLed(20, nR, nG, nB);
                UpdateLed(21, nR, nG, nB);
                UpdateLed(22, nR, nG, nB);
                UpdateLed(23, nR, nG, nB);
                UpdateLed(24, nR, nG, nB);
                UpdateLed(25, nR, nG, nB);
                UpdateLed(26, nR, nG, nB);
            }
            if(nCptHeight > ((nHeight/3)*2))
            {
                UpdateLed(8, nR, nG, nB);
                UpdateLed(9, nR, nG, nB);
                UpdateLed(10, nR, nG, nB);
                UpdateLed(11, nR, nG, nB);
                UpdateLed(12,nR, nG, nB);
                UpdateLed(13, nR, nG, nB);
                UpdateLed(14, nR, nG, nB);
                UpdateLed(15, nR, nG, nB);
                UpdateLed(16, nR, nG, nB);
                UpdateLed(17, nR, nG, nB);
                UpdateLed(18, nR, nG, nB);
                UpdateLed(19, nR, nG, nB);
            }
        }
    }

    ProcessAverage();

    if(_pSerial->isOpen())
    {
        SendValues();
    }

    QTimer::singleShot(0, this, SLOT(Loop()));
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::CleanLeds()
{
    for(int nCpt = 0;nCpt < LEDS_COUNT;++nCpt)
    {
        _pLeds[nCpt][0] = 0;
        _pLeds[nCpt][1] = 0;
        _pLeds[nCpt][2] = 0;
        _pLeds[nCpt][3] = 0;
    }
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::UpdateLed(int nID, int nR, int nG, int nB)
{
    _pLeds[nID][0]++;
    _pLeds[nID][1] += nR;
    _pLeds[nID][2] += nG;
    _pLeds[nID][3] += nB;
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::ProcessAverage()
{
    for(int nCpt = 0;nCpt < LEDS_COUNT;++nCpt)
    {
        int nValue = _pLeds[nCpt][0];
        if(nValue > 1)
        {
            _pLeds[nCpt][1] /= nValue;
            _pLeds[nCpt][2] /= nValue;
            _pLeds[nCpt][3] /= nValue;
        }
    }
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::SendValues()
{
    QByteArray dataOutA;
    dataOutA.append('A');
    for(int nCpt = 0;nCpt < 15;++nCpt)
    {
        byte bR = _pLeds[nCpt][1];
        byte bG = _pLeds[nCpt][2];
        byte bB = _pLeds[nCpt][3];

        dataOutA.append(bR);
        dataOutA.append(bG);
        dataOutA.append(bB);
    }
    _pSerial->write(dataOutA);

    QByteArray dataOutB;
    dataOutB.append('B');
    for(int nCpt = 15;nCpt < 30;++nCpt)
    {
        byte bR = _pLeds[nCpt][1];
        byte bG = _pLeds[nCpt][2];
        byte bB = _pLeds[nCpt][3];

        dataOutB.append(bR);
        dataOutB.append(bG);
        dataOutB.append(bB);
    }
    _pSerial->write(dataOutB);
}

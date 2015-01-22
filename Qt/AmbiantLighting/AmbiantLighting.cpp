#include "AmbiantLighting.h"
#include "ui_AmbiantLighting.h"
#include <QDesktopWidget>
#include <QScreen>
#include <QGraphicsScene>
#include <QTimer>
#include <QTime>
#include <QDebug>

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
    int nR = 0.f;
    int nG = 0.f;
    int nB = 0.f;

    const QPixmap& Pixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());

    const QImage& Image = Pixmap.toImage();
    const QSize& ImageSize = Image.size();

    const int nWidth = ImageSize.width();
    const int nHeight = ImageSize.height();

    int nDelta = 4;

    for(int nCptWidth = 0;nCptWidth < ImageSize.width();nCptWidth+=nDelta)
    {
        for(int nCptHeight = 0;nCptHeight < ImageSize.height();nCptHeight+=nDelta)
        {
            const QRgb& PixelColor = Image.pixel(nCptWidth, nCptHeight);
            nR += qRed(PixelColor);
            nG += qGreen(PixelColor);
            nB += qBlue(PixelColor);
        }
    }

    nR = nR / ((nWidth/nDelta)*(nHeight/nDelta));
    nG = nG / ((nWidth/nDelta)*(nHeight/nDelta));
    nB = nB / ((nWidth/nDelta)*(nHeight/nDelta));

    QColor AverageColor(nR, nG, nB);
    QBrush Brush(AverageColor);
    _pGraphicsScene->setBackgroundBrush(Brush);

    if(_pSerial->isOpen())
    {
        byte bR = nR;
        byte bG = nG;
        byte bB = nB;
        QByteArray dataOut;
        dataOut.append('C');
        dataOut.append(bR);
        dataOut.append(bG);
        dataOut.append(bB);
        _pSerial->write(dataOut);
    }

    QTimer::singleShot(0, this, SLOT(Loop()));
}

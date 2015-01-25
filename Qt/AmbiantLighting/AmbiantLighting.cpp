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
    : QMainWindow                   (pParent)
    , _pUi                          (new Ui::AMBIANT_LIGHTING)
    , _eMode                        (ALL_SCREEN_MODE)
    , _nWidth                       (0)
    , _nHeight                      (0)
    , _nCinemaScopeBlackBarHeight   (0)
{
    _pUi->setupUi(this);

    _pSerial = new QSerialPort(this);
    _PortInfoList = QSerialPortInfo::availablePorts();

    connect(_pUi->radioButtonAllScreen, SIGNAL(toggled(bool)), this, SLOT(OnModeChanged(bool)));
    connect(_pUi->radioButtonCinemaScope, SIGNAL(toggled(bool)), this, SLOT(OnModeChanged(bool)));

    OpenSerialPort();

    const QPixmap& Pixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
    const QImage& Image = Pixmap.toImage();
    const QSize& ImageSize = Image.size();
    _nWidth = ImageSize.width();
    _nHeight = ImageSize.height();

    float fScreenRatio = (float )_nWidth /(float)_nHeight;
    float fBar = (1 - (fScreenRatio / 2.39f)) / 2; // 2.39/1 is CinemaScope ratio
    _nCinemaScopeBlackBarHeight = _nHeight * fBar;

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

    const int nX1 = _nWidth / 3;
    const int nX2 = nX1 * 2;
    const int nY1 = _nHeight / 3;
    int nTopPixelsToSkip = 0; // For CinemaScope mode

    int nDelta = 4;

    if(_eMode == CINEMASCOPE_MODE)
    {
        nTopPixelsToSkip = _nCinemaScopeBlackBarHeight;
    }

    CleanLeds();
    for(int nCptWidth = 0;nCptWidth < ImageSize.width();nCptWidth+=nDelta)
    {
        for(int nCptHeight = 0;nCptHeight < ImageSize.height();nCptHeight+=nDelta)
        {
            const QRgb& PixelColor = Image.pixel(nCptWidth, nCptHeight);
            int nR = qRed(PixelColor);
            int nG = qGreen(PixelColor);
            int nB = qBlue(PixelColor);
            if(nCptWidth < nX1)
            {
                ProcessLeftSide(nCptHeight, nR, nG, nB);
            }
            else if(nCptWidth > nX2)
            {
                ProcessRightSide(nCptHeight, nR, nG, nB);
            }
            if(nCptHeight >= nTopPixelsToSkip && nCptHeight < nY1 + nTopPixelsToSkip)
            {
                ProcessTopSide(nCptWidth, nR, nG, nB);
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
    if(nID >= 0)
    {
        _pLeds[nID][0]++;
        _pLeds[nID][1] += nR;
        _pLeds[nID][2] += nG;
        _pLeds[nID][3] += nB;
    }
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::ProcessLeftSide(int nY, int nR, int nG, int nB)
{
    const int nSideLedHeight = _nHeight / 8;
    int nRelatedLedID = -1;
    if(nY >= nSideLedHeight && nY < nSideLedHeight*2)
    {
        nRelatedLedID = 6;
    }
    else if(nY >= nSideLedHeight*2 && nY < nSideLedHeight*3)
    {
        nRelatedLedID = 5;
    }
    else if(nY >= nSideLedHeight*3 && nY < nSideLedHeight*4)
    {
        nRelatedLedID = 4;
    }
    else if(nY >= nSideLedHeight*4 && nY < nSideLedHeight*5)
    {
        nRelatedLedID = 3;
    }
    else if(nY >= nSideLedHeight*5 && nY < nSideLedHeight*6)
    {
        nRelatedLedID = 2;
    }
    else if(nY >= nSideLedHeight*6 && nY < nSideLedHeight*7)
    {
        nRelatedLedID = 1;
    }
    else if(nY >= nSideLedHeight*7 && nY < nSideLedHeight*8)
    {
        nRelatedLedID = 0;
    }
    UpdateLed(nRelatedLedID, nR, nG, nB);
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::ProcessRightSide(int nY, int nR, int nG, int nB)
{
    const int nSideLedHeight = _nHeight / 8;
    int nRelatedLedID = -1;
    if(nY >= nSideLedHeight && nY < nSideLedHeight*2)
    {
        nRelatedLedID = 19;
    }
    else if(nY >= nSideLedHeight*2 && nY < nSideLedHeight*3)
    {
        nRelatedLedID = 20;
    }
    else if(nY >= nSideLedHeight*3 && nY < nSideLedHeight*4)
    {
        nRelatedLedID = 21;
    }
    else if(nY >= nSideLedHeight*4 && nY < nSideLedHeight*5)
    {
        nRelatedLedID = 22;
    }
    else if(nY >= nSideLedHeight*5 && nY < nSideLedHeight*6)
    {
        nRelatedLedID = 23;
    }
    else if(nY >= nSideLedHeight*6 && nY < nSideLedHeight*7)
    {
        nRelatedLedID = 24;
    }
    else if(nY >= nSideLedHeight*7 && nY < nSideLedHeight*8)
    {
        nRelatedLedID = 25;
    }
    UpdateLed(nRelatedLedID, nR, nG, nB);
}

//-----------------------------------------------------------
void AMBIANT_LIGHTING::ProcessTopSide(int nX, int nR, int nG, int nB)
{
    const int nTopLedWidth = _nWidth / 12;
    int nRelatedLedID = -1;
    if(nX < nTopLedWidth)
    {
       nRelatedLedID = 7;
    }
    else if(nX >= nTopLedWidth && nX < nTopLedWidth*2)
    {
        nRelatedLedID = 8;
    }
    else if(nX >= nTopLedWidth*2 && nX < nTopLedWidth*3)
    {
        nRelatedLedID = 9;
    }
    else if(nX >= nTopLedWidth*3 && nX < nTopLedWidth*4)
    {
        nRelatedLedID = 10;
    }
    else if(nX >= nTopLedWidth*4 && nX < nTopLedWidth*5)
    {
        nRelatedLedID = 11;
    }
    else if(nX >= nTopLedWidth*5 && nX < nTopLedWidth*6)
    {
        nRelatedLedID = 12;
    }
    else if(nX >= nTopLedWidth*6 && nX < nTopLedWidth*7)
    {
        nRelatedLedID = 13;
    }
    else if(nX >= nTopLedWidth*7 && nX < nTopLedWidth*8)
    {
        nRelatedLedID = 14;
    }
    else if(nX >= nTopLedWidth*8 && nX < nTopLedWidth*9)
    {
        nRelatedLedID = 15;
    }
    else if(nX >= nTopLedWidth*9 && nX < nTopLedWidth*10)
    {
        nRelatedLedID = 16;
    }
    else if(nX >= nTopLedWidth*10 && nX < nTopLedWidth*11)
    {
        nRelatedLedID = 17;
    }
    else if(nX >= nTopLedWidth*11 && nX < nTopLedWidth*12)
    {
        nRelatedLedID = 18;
    }
    UpdateLed(nRelatedLedID, nR, nG, nB);
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

//-----------------------------------------------------------
void AMBIANT_LIGHTING::OnModeChanged(bool bChecked)
{
    if(bChecked)
    {
        if(sender() == _pUi->radioButtonAllScreen)
        {
            _eMode = ALL_SCREEN_MODE;
        }
        else if(sender() == _pUi->radioButtonCinemaScope)
        {
            _eMode = CINEMASCOPE_MODE;
        }
    }
}

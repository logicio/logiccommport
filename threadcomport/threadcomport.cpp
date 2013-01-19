/*

	threadcomport v.0.0.1
	author Golubkin Egor (Gorin), Russia
	///////////////////////////////////////////////

	Multiplatform asynchronous Serial Port Extension
	based on Wayne Roth's QExtSerialPort
	

*/  
#include <threadcomport.h>
#include <qextserialport.h>

#include <QtCore>
#include <QtCore/QtDebug>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QTime>

//#define DEBUG_STATISTIC


/*
==============
<CONSTRUCTORS>
==============
*/
Qthreadcomport::Qthreadcomport (const PortSettings *settings):
        QextSerialPort(*settings)
{
    thread = NULL;
    ftimeout = 0;
}

ReceiveThread::ReceiveThread()
{
    lastcount = 0;
    ftimeout = 0;
}

QMainComThread::QMainComThread(QString name, const PortSettings *settings)
{
    comsettings = new PortSettings();
    *comsettings = *settings;
    comname = name;
    comport = NULL;
}
/*
==============
<DESTRSTRUCTORS>
==============
*/
Qthreadcomport::~Qthreadcomport()
{
    stopThread();
}
QMainComThread::~QMainComThread()
{
    terminate();
    wait();
    delete comport;
    comport = NULL;
    delete comsettings;
    comsettings = NULL;
}
/*
==============
<METHODS>
==============
*/
/*TMainComThread*/
Qthreadcomport* QMainComThread::getPort()
{
    return comport;
}

void QMainComThread::run()
{
    Qthreadcomport *port = new Qthreadcomport(comsettings);
    port->setPortName(comname);
    comport = port;

    exec();
}

/*QReceiveThread*/
void ReceiveThread::run()
{
    int count;
    QTime timerTimeout;
    timerTimeout.start();
    forever
    {
        msleep(1);
        mutex.lock();
        count = comport->bytesAvailable();
        mutex.unlock();

        if (count > 0)
        {

            if (timerTimeout.elapsed() > ftimeout)
                {
                    emit newDataInPortThread(count);
                    QTime timedb;
#ifdef DEBUG_STATISTIC                    
                    qDebug()<<"thread count= "<<count<<"time= "<<"\t"<<timedb.currentTime().second()<<" "<<timedb.currentTime().msec();
#endif //#ifdef DEBUG_STATISTIC                    
                    timerTimeout.restart();
                }
        }
        if (timerTimeout.elapsed() > ftimeout)
            timerTimeout.restart();
    }
}

void ReceiveThread::setPort(Qthreadcomport *port)
{
    comport = port;
}

/*Qthreadcomport*/
void Qthreadcomport::close()
{
    stopThread();
    QextSerialPort::close();
}

bool Qthreadcomport::open(QIODevice::OpenMode mode)
{
    thread = new ReceiveThread();
    thread->setPort(this);
    connect(thread,SIGNAL(newDataInPortThread(int)),this,SLOT(newDataInPortSlot(int)));
    bool bopen = QextSerialPort::open(mode);
    thread->start();
    thread->setTimeout(ftimeout);

    return bopen;
}

qint64 Qthreadcomport::readData(char *data, qint64 maxSize)
{
    qint64 count = QextSerialPort::readData(data, maxSize);
    return count;
}

void Qthreadcomport::stopThread()
{
    if (thread != NULL)
    {
        thread->terminate();
        thread->wait();
        delete thread;
        thread = NULL;
    }
}

void Qthreadcomport::setTimeout(int timeout)
{
    if (thread != NULL)
        thread->setTimeout(timeout);
    ftimeout = timeout;
}

void Qthreadcomport::rxDataSet(char *data, int size)
{
    QByteArray lvStr(data,size);

    rxDataMutex.lock();
    rxData.append(lvStr);
    rxDataMutex.unlock();
}

void Qthreadcomport::rxDataGet(QByteArray &data)
{
    rxDataMutex.lock();
    data = rxData;
    rxData.clear();
    rxDataMutex.unlock();
}

/*
==============
<SLOTS>
==============
*/
void Qthreadcomport::newDataInPortSlot(int count)
{
    if ((bytesAvailable() == 0) || (!(isOpen()))) return;
    const int size = 4096;
    char data[size];
    int readcount = readData(data, count);
    QTime timedb;
    timedb = timedb.currentTime();
#ifdef DEBUG_STATISTIC    
    qDebug()<<"readData"<<"time= "<<"\t\t\t"<<timedb.second()<<" "<<timedb.msec();
//#endif //#ifdef DEBUG_STATISTIC    
    qDebug() <<"readCount=" << readcount;
    data[readcount] = 0;
    qDebug() << "****readMesg=" << data;
#endif //#ifdef DEBUG_STATISTIC        
//    data[readcount] = '\0';
    rxDataSet(data, readcount);

    unsigned char *dataPtr_dummy;
    int readcount_dummy = readcount;
    dataPtr_dummy = (unsigned char *)&data;
    emit newDataInPortSignal(timedb, dataPtr_dummy,readcount_dummy);
}


#include <QtGui>
#include <QtCore>

#include <QtDebug>
#include <QRegExp>
#include <QTextCodec>
#include <QTime>
#include <QString>
#include <QTextCodec>

#include "mainwindow.h"
#include <threadcomport.h>
#include "ui_main_window_form.h"

//#define DEBUG_MESG
//#define DEBUG_STATISTIC

/*
==============
<CONSTRUCTORS>
==============
*/
MainWindow::MainWindow()
{
    QString prefix = "";
#ifdef Q_WS_WIN 
    const QString portname = "Com";
    int minnum = 1;
    int maxnum = 20;
    prefix = "\\\\.\\";
#else
    const QString portname = "/dev/ttyS";
    int minnum = 0;
    int maxnum = 19;
#endif

    setupUi(this);
    port =  NULL;
    timerout = NULL;
    timeoutTime = NULL;
    counter_in = 0;
    counter_out = 0;

    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
    createMainMenu();
    setWindowIcon(QIcon(":/images/plats.ico"));

    labelReceive = new QLabel("                  ");
    labelReceive->setIndent(10);
    labelTransmit = new QLabel("                  ");
    labelTransmit->setIndent(10);
    statusBar()->addWidget(labelReceive);
    statusBar()->addWidget(labelTransmit);

#ifndef Q_WS_WIN
    const QString portnameUSB = "/dev/ttyUSB";
    comboBox_port->addItem(portnameUSB+QString("%1").arg(minnum));
#endif    
    for (; minnum <= maxnum; minnum++)
        if (minnum>=10)
            comboBox_port->addItem(prefix+portname+QString("%1").arg(minnum));//for windows xp and vista port prefix
    else
        comboBox_port->addItem(portname+QString("%1").arg(minnum));
    comboBox_baudrate->addItem("BAUD110",BAUD110);
    comboBox_baudrate->addItem("BAUD300",BAUD300);
    comboBox_baudrate->addItem("BAUD600",BAUD600);
    comboBox_baudrate->addItem("BAUD1200",BAUD1200);
    comboBox_baudrate->addItem("BAUD2400",BAUD2400);
    comboBox_baudrate->addItem("BAUD4800",BAUD4800);
    comboBox_baudrate->addItem("BAUD9600",BAUD9600);
    comboBox_baudrate->addItem("BAUD19200",BAUD19200);
    comboBox_baudrate->addItem("BAUD38400",BAUD38400);
    comboBox_baudrate->addItem("BAUD57600",BAUD57600);
    comboBox_baudrate->addItem("BAUD115200",BAUD115200);

    comboBox_parity->addItem("PAR_NONE",PAR_NONE);
    comboBox_parity->addItem("PAR_ODD",PAR_ODD);
    comboBox_parity->addItem("PAR_EVEN",PAR_EVEN);
    comboBox_parity->addItem("PAR_SPACE",PAR_SPACE);

    comboBox_flcntrl->addItem("FLOW_OFF",FLOW_OFF);
    comboBox_flcntrl->addItem("FLOW_HARDWARE",FLOW_HARDWARE);
    comboBox_flcntrl->addItem("FLOW_XONXOFF",FLOW_XONXOFF);

    PortSettings settings;
    readSettings(&settings);

    mainComThread = new QMainComThread(comboBox_port->currentText(),&settings);
    mainComThread->start();
    while (!port)
    {
        port = mainComThread->getPort();
        qApp->processEvents();
    }

    bt_stopsend->setEnabled(false);
    bt_closeport->setEnabled(false);
    progressBar->setValue(0);
    timerout = new QTimer(this);

    SetCurComboBState();
    //connecting
    connect(bt_openport,SIGNAL(clicked()),this, SLOT(openPort()));
    connect(bt_closeport,SIGNAL(clicked()),this, SLOT(closePort()));
    connect(comboBox_baudrate,SIGNAL(currentIndexChanged(int)),this,SLOT(enabledPortBt()));
    connect(comboBox_parity,SIGNAL(currentIndexChanged(int)),this,SLOT(enabledPortBt()));
    connect(comboBox_flcntrl,SIGNAL(currentIndexChanged(int)),this,SLOT(enabledPortBt()));
    connect(bt_apply,SIGNAL(clicked(bool)),this,SLOT(applyPortOptions()));
    connect(bt_cancel,SIGNAL(clicked(bool)),this,SLOT(SetCurComboBState()));
    connect(bt_send,SIGNAL(clicked(bool)), this, SLOT(btsendClick()));
    connect(bt_stopsend,SIGNAL(clicked(bool)), this, SLOT(btstopsendClick()));
    connect(timerout, SIGNAL(timeout()), this, SLOT(transmitMsg()));
    connect(bt_clear, SIGNAL(clicked(bool)), textBr_inp, SLOT(clear()));
    connect(bt_clear, SIGNAL(clicked(bool)), textBr_mess, SLOT(clear()));
    connect(port,SIGNAL(newDataInPortSignal(QTime,const unsigned char *, const int)),this, SLOT(receiveMsg(QTime,const unsigned char *, const int)));
    connect(spinBox_Period, SIGNAL(valueChanged(int)), this, SLOT(intervalChange(int)));
    connect(bt_open, SIGNAL(clicked(bool)), this, SLOT(open()));
    connect(bt_save, SIGNAL(clicked(bool)), this, SLOT(save()));

    dlgCRC = 0;
}
/*
==============
<DESTRUCTORS>
==============
*/
MainWindow::~MainWindow()
{
    delete timerout;
    timerout = NULL;

    delete mainComThread;
    mainComThread = NULL;
}
/*
==============
<METHODS>
==============
*/
void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::getdataout(QByteArray *data)
{
    data->clear();
    QString str = (textEd_out->toPlainText()).toAscii();

    if(rbt_Rawout->isChecked())
    {
        *data = fileData;
    }
    if (rbt_ASCIIout->isChecked())//ASCII
    {
        str = textEd_out->toPlainText();
        *data = str.toAscii();
    }
    if (rbt_Decout->isChecked())//Dec
    {
        QRegExp rx("(\\d+)");
        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1)
        {
            data->append((rx.cap(1)).toInt());
            pos += rx.matchedLength();
        }
    }
    if (rbt_BINout->isChecked())//BIN
    {
        QRegExp rx("([01]+)");
        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1)
        {
            bool ok;
            data->append((rx.cap(1)).toInt(&ok, 2));
            pos += rx.matchedLength();
        }
    }
    if (rbt_HEXout->isChecked())//HEX
    {
        QRegExp rx("([0-9a-fA-F]+)");
        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1)
        {
            bool ok;
            data->append((rx.cap(1)).toInt(&ok, 16));
            pos += rx.matchedLength();
        }
    }
}

void MainWindow::readSettings(PortSettings *portsettings)
{
#ifdef Q_WS_WIN 
    QSettings settings("QT_comport.ini" ,QSettings::IniFormat);
#else
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Gorin", "QT_comport");
#endif
    settings.beginGroup("window");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(700, 400)).toSize();
    settings.endGroup();

    resize(size);
    move(pos);

    settings.beginGroup("port");
    int portparam;
    portparam = comboBox_port->findText( (settings.value("name")).toString(),
                                         Qt::MatchExactly | Qt::MatchFixedString);
    if (-1 == portparam)
        portparam = 0;
    comboBox_port->setCurrentIndex(portparam);
    portsettings->BaudRate = (BaudRateType)settings.value("baudrate",BAUD9600).toInt();
    portsettings->Parity = (ParityType)settings.value("parity",PAR_NONE).toInt();
    portsettings->FlowControl = (FlowType)settings.value("flowcontrol",FLOW_OFF).toInt();
    portsettings->DataBits = (DataBitsType)settings.value("databits",DATA_8).toInt();
    portsettings->StopBits = (StopBitsType)settings.value("flowcontrol",STOP_1).toInt();
    bDTR = settings.value("DTR",false).toBool();
    bRTS = settings.value("RTS",false).toBool();
    portsettings->Timeout_Millisec = 500;
    portsettings->Timeout_Sec = 0;
    settings.endGroup();

    settings.beginGroup("parameters");
    spinBox_Period->setValue(settings.value("periodvalue",0).toInt());
    TypeDataInOut type = (TypeDataInOut)(settings.value("typedataout",0).toInt());
    switch (type)
    {
    case RAW_:
        rbt_Rawout->setChecked(true);
        break;
    case DEC_:
        rbt_Decout->setChecked(true);
        break;
    case ASCII_:
        rbt_ASCIIout->setChecked(true);
        break;
    case BIN_:
        rbt_BINout->setChecked(true);
        break;
    case HEX_:
        rbt_HEXout->setChecked(true);
        break;
	case NMEA_:
		break;
    }
    type = (TypeDataInOut)(settings.value("typedatain",0).toInt());
    switch (type)
    {
    case RAW_:
        break;
    case DEC_:
        rbt_Decin->setChecked(true);
        break;
    case ASCII_:
        rbt_ASCIIin->setChecked(true);
        break;
    case NMEA_:
        rbt_NMEAin->setChecked(true);
        break;
    case BIN_:
        rbt_BINin->setChecked(true);
        break;
    case HEX_:
        rbt_HEXin->setChecked(true);
        break;
    }
    textEd_out->setPlainText(settings.value("dataout","").toString());
    calcTimeoutAct->setChecked(settings.value("calctimeout",false).toBool());
    settings.endGroup();
}

void MainWindow::writeSettings()
{
#ifdef Q_WS_WIN 
    QSettings settings("QT_comport.ini" ,QSettings::IniFormat);
#else
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Gorin", "QT_comport");
#endif
    settings.beginGroup("window");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.endGroup();

    settings.beginGroup("port");
    settings.setValue("name", port->portName());
    settings.setValue("baudrate", (int)port->baudRate());
    settings.setValue("parity", (int)port->parity());
    settings.setValue("flowcontrol", (int)port->flowControl());
    settings.setValue("databits", (int)port->dataBits());
    settings.setValue("stopbits", (int)port->stopBits());
    settings.setValue("DTR",bDTR);
    settings.setValue("RTS",bRTS);
    settings.endGroup();

    settings.beginGroup("parameters");
    settings.setValue("periodvalue",spinBox_Period->value());
    TypeDataInOut type;
    if(rbt_Rawout->isChecked())
        type = RAW_;
    if (rbt_Decout->isChecked())
        type = DEC_;
    if (rbt_ASCIIout->isChecked())
        type = ASCII_;
    if (rbt_BINout->isChecked())
        type = BIN_;
    if (rbt_HEXout->isChecked())
        type = HEX_;
    settings.setValue("typedataout",type);
    if (rbt_Decin->isChecked())
        type = DEC_;
    if (rbt_ASCIIin->isChecked())
        type = ASCII_;
    if (rbt_NMEAin->isChecked())
        type = NMEA_;
    if (rbt_BINin->isChecked())
        type = BIN_;
    if (rbt_HEXin->isChecked())
        type = HEX_;
    settings.setValue("typedatain",type);
    settings.setValue("dataout",textEd_out->toPlainText());
    settings.setValue("calctimeout", calcTimeoutAct->isChecked());
    settings.endGroup();
}
/*
==============
<SLOTS>
==============
*/
void MainWindow::about()
{
    QMessageBox::about(this, "About "+windowTitle(),
                       "<B>"+windowTitle()+"</B><BR>"
                       "<br>"
                       "author: tkcheng<br>"
                       "<a href='https://github.com/logicio/logiccommport'>Source at github.com</a><br>"
                       "<a href='mailto:ctkiann@yahoo.com'>ctkiann@yahoo.com</a><br>"
                       "<br>"
                       "Based on QT_comport from<br>"                    
                       "author: Egor Golubkin (Gorin)<br>"
                       "<a href='mailto:Gorin_mind@rambler.ru'>Gorin_mind@rambler.ru</a>"
                       "<br><br>"
                       "<small>based on qextserialport</small>");
}

void MainWindow::applyPortOptions()
{
    int portparam = (comboBox_baudrate->itemData(comboBox_baudrate->currentIndex())).toInt();
    port->setBaudRate((BaudRateType)portparam);
    portparam = (comboBox_flcntrl->itemData(comboBox_flcntrl->currentIndex())).toInt();
    port->setFlowControl((FlowType)portparam);
    portparam = (comboBox_parity->itemData(comboBox_parity->currentIndex())).toInt();
    port->setParity((ParityType)portparam);
    bt_apply->setEnabled(false);
    bt_cancel->setEnabled(false);
}

void MainWindow::btsendClick()
{
    if (spinBox_Period->value() <= 0)
        transmitMsg();
    else
    {
        timerout->start(spinBox_Period->value());
        bt_send->setEnabled(false);
        bt_stopsend->setEnabled(true);
    }
}

void MainWindow::btstopsendClick()
{
    timerout->stop();
    bt_send->setEnabled(true);
    bt_stopsend->setEnabled(false);
}

void MainWindow::createMainMenu()
{
    //File actions
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Alt+F4"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    //Help actions
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setShortcut(tr("CTRL+A"));
    aboutAct->setStatusTip(tr("About application"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    //calcTimeoutAct
    calcTimeoutAct = new QAction (tr("&Calc Timeout"), this);
    calcTimeoutAct->setCheckable(true);
    connect(calcTimeoutAct, SIGNAL(toggled(bool)), this, SLOT(setTimeoutTimer(bool)));
    //calcCRCAct
    calcCRCAct = new QAction (tr("Ca&lc CRC"), this);
    connect(calcCRCAct, SIGNAL(triggered()), this, SLOT(calcCRC()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(calcTimeoutAct);
    editMenu->addAction(calcCRCAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::closePort()
{
    if (! port) return;
    port->close();
    textBr_mess->append("Port close");
    bt_closeport->setEnabled(false);
    bt_openport->setEnabled(true);
}

void MainWindow::enabledPortBt()
{
    bt_apply->setEnabled(true);
    bt_cancel->setEnabled(true);
}

void MainWindow::intervalChange(int value)
{
    timerout->setInterval(value);
}

void MainWindow::openPort()
{
    if (! port) return;
    if (port->isOpen())
        port->close();
    port->setPortName(comboBox_port->currentText());
    setTimeoutTimer(false);
    port->open(QIODevice::ReadWrite);
    if (port->isOpen())
        textBr_mess->append("Port open");
    else
        textBr_mess->append("Port not open");
    port->setDtr(bDTR);
    port->setRts(bRTS);
    bt_closeport->setEnabled(true);
    bt_openport->setEnabled(false);
    counter_in = 0;
    counter_out = 0;
}

void MainWindow::receiveMsg(const QTime timesl, const unsigned char *data_dummy, const int size_dummy)
{

    //point data and size to local variable
    QByteArray lvRxData;
    unsigned char* data;
    int size;

    //remove warning only
    data = (unsigned char *)data_dummy;
    size = size_dummy;

    port->rxDataGet(lvRxData);
//    qDebug() << "***Receiving:" << lvRxData;
    data = (unsigned char *)lvRxData.data();
    size = lvRxData.size();

    textBr_mess->append(QString("read %1").arg(size));

    QTime timedb;
    timedb = timesl ;

#ifdef DEBUG_STATISTIC    
    qDebug()<<"mainwindow "<<"inputtime= "<<"\t"<<timedb.second()<<" "<<timedb.msec();
    qDebug()<<"mainwindow "<<"time= "<<"\t\t\t"<<timedb.currentTime().second()<<" "<<timedb.currentTime().msec()<<"-------------------------";
#endif //#ifdef DEBUG_STATISTIC

    if (calcTimeoutAct->isChecked())
    {
        if (timeoutTime)
            textBr_inp->append(QString("timeout %1 ms").arg(timeoutTime->msecsTo(timesl)));
        else timeoutTime = new QTime();
        *timeoutTime = timesl;
    }
    if (rbt_NMEAin->isChecked())
    {
        QTextCursor *tempcursor;
//        tempcursor = &textBr_inp->textCursor();
        QTextCursor tmpcursor;
        tmpcursor = textBr_inp->textCursor();
        tempcursor = &tmpcursor;

        tempcursor->movePosition(QTextCursor::End);
        tempcursor->insertText(transformInpData(data, size));
        textBr_inp->setTextCursor(*tempcursor);
    }
    else
    {
        textBr_inp->append(transformInpData(data, size));
    }
    counter_in += size;
    labelReceive->setText(QString("receive %1").arg(counter_in));   
}

void MainWindow::SetCurComboBState()
{
    if (!port) return;
    comboBox_port->setCurrentIndex(comboBox_port->findText(port->portName()));
    int portparam = comboBox_baudrate->findData( port->baudRate());
    comboBox_baudrate->setCurrentIndex(portparam);
    portparam = comboBox_parity->findData( port->parity());
    comboBox_parity->setCurrentIndex(portparam);
    portparam = comboBox_flcntrl->findData(port->flowControl());
    comboBox_flcntrl->setCurrentIndex(portparam);

    bt_apply->setEnabled(false);
    bt_cancel->setEnabled(false);
}

void MainWindow::setTimeoutTimer(bool state)
{
    if (state);
    else
    {
        delete timeoutTime;
        timeoutTime = NULL;
    }
}

QString MainWindow::transformInpData(const unsigned char *data, int size)
{
    const QString tab = "  ";
    QString res;
    if (rbt_Decin->isChecked())
    {
        for (int i = 0;i < size;i++)
            res = res+QString("%1").arg((int)data[i])+tab;
    }

    if (rbt_NMEAin->isChecked())
    {
        int i = 0;
        for (;i < size;i++)
            switch (data[i])
            {
                case 0:
                    res += "\'0\'";
                    break;
                case 0x0A:
                    res += "<LF>";
                    res += data[i];
                    break;
                case 0x0D:
                    res += "<CR>";
                    break;
                //test cheng
                case 0x1B:
                    res += "\\0x1b";
                    break;
                default:
                    res += data[i];
            }
    }

    if (rbt_ASCIIin->isChecked())
    {
        int i = 0;
        for (;i < size;i++)
            switch (data[i])
            {
                case 0:
                    res += "\'0\'";
                    break;
                default:
                    res += data[i];
            }
    }

    if (rbt_BINin->isChecked())
    {
        for (int i = 0;i < size;i++)
            res = res+QString("%1").arg((int)data[i],0,2)+tab;
    }
    if (rbt_HEXin->isChecked())
    {
        for (int i = 0;i < size;i++)
            res = res+QString("%1").arg((int)data[i],0,16)+tab;
    }

    return res;
}

void MainWindow::transmitMsg()
{
    QByteArray data;
    getdataout(&data);
    int count = port->write(data, data.length());
    textBr_mess->append(QString("transmited %1").arg(count));

    if (progressBar->value() >= progressBar->maximum())
        progressBar->reset();
    else
        progressBar->setValue(progressBar->value()+1);
    if (count == -1) count = 0;
    counter_out+=count;
    labelTransmit->setText(QString("transmit %1").arg(counter_out));
}

void MainWindow::calcCRC()
{
    if (dlgCRC == 0)
    {
        dlgCRC = new QDialogCRC(this);
        connect(dlgCRC, SIGNAL(textCopy (QString &)), this, SLOT(setTextCopy (QString &)));
    }
    dlgCRC->show();
}

void MainWindow::setTextCopy (QString & str)
{
    textEd_out->setText(str);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    textEd_out->clear();
    
    while (!in.atEnd())
    {
        QString line = in.readLine();
        textEd_out->append(line);
    }
    file.close();

    //re read-file and save as raw data, above 
    if (!file.open(QIODevice::ReadOnly /*| QIODevice::Text*/))
        return;

    QDataStream in_raw(&file);

    if(file.seek(0) == false)
    {
        qDebug() << "File Seek Rewind NG";
        return;        
    }
    int totalSize=0;

    int readLen, maxLen=LEN_FILE_DATA_PER_READ;
    char data[LEN_FILE_DATA_PER_READ];
    fileData.clear();
    do
    {
        readLen = 0;
        readLen = in_raw.readRawData(data, maxLen);
        fileData.append(data, readLen);
        totalSize += readLen;

        if(readLen == -1)       //EOF
            break;
    }while(readLen);

#ifdef DEBUG_MESG
    qDebug () << "FileSize:" << file.size();
    qDebug () << "TotalFileLen:" << totalSize;
#endif //#ifdef DEBUG_MESG

    file.close();
}

void MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save file..."), "",
                                                    tr("Text File (*.txt);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    out << textEd_out->toPlainText();
    file.close();
}

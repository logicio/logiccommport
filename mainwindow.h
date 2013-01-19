#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_main_window_form.h"
#include <threadcomport.h>
#include "crc_form.h"

#define KBYTES                      (1024)
#define LEN_FILE_DATA_PER_READ      (1*KBYTES)//

enum TypeDataInOut {RAW_, DEC_, ASCII_, NMEA_, HEX_, BIN_};

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event);

private:
    QDialogCRC *dlgCRC;

    Qthreadcomport *port;
    QMainComThread *mainComThread;
    QTimer *timerout;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *editMenu;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *calcTimeoutAct;
    QAction *calcCRCAct;
    QTime *timeoutTime;
    QLabel *labelReceive;
    QLabel *labelTransmit;
    QByteArray   fileData;


    int counter_in,counter_out;
    bool bRTS;
    bool bDTR;
    void createMainMenu();
    void getdataout(QByteArray *data);
    void readSettings(PortSettings *portsettings);
    QString transformInpData(const unsigned char *data, int size);
    void writeSettings();

private slots:
    void about();
    void applyPortOptions();
    void btsendClick();
    void btstopsendClick();
    void closePort();
    void enabledPortBt();
    void openPort();
    void receiveMsg(const QTime timesl, const unsigned char *data, const int size);
    void SetCurComboBState();
    void setTimeoutTimer(bool state);
    void transmitMsg();
    void intervalChange(int value);
    void calcCRC();
    void setTextCopy (QString & str);
    void open();
    void save();

};

#endif

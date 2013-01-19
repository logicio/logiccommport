/*

	crc_form.h
	author Golubkin Egor (Gorin), Russia
	///////////////////////////////////////////////
	dialog for calc CRC
*/  
#ifndef CRCDIALOG_H	
 #define CRCDIALOG_H

 #include <QtGui>

 #include "ui_crc_form.h"

class QDialogCRC : public QDialog, private Ui::DialogCRC{
	Q_OBJECT
public:
	QDialogCRC(QWidget * parent = 0);
	~QDialogCRC();
private:
	void getdataIn (QByteArray &data);
	unsigned short calcCRC16 (const QByteArray &data);
        unsigned short calcCRC_MODBUS (const QByteArray &data);
        unsigned short calcCRC_NMEA (const QByteArray &data);
signals:
	void textCopy (QString & str);

private slots:
	void calcCRC();
	void copy();

};


 #endif

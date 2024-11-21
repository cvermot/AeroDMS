/******************************************************************************\
<AeroDms : logiciel de gestion compta section a�ronautique>
Copyright (C) 2023-2024 Cl�ment VERMOT-DESROCHES (clement@vermot.net)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
/******************************************************************************/
#ifndef PDFDOWNLOADER_H
#define PDFDOWNLOADER_H

#include <QWidget>
#include <QtNetwork>

#include "AeroDmsTypes.h"

class PdfDownloader : public QWidget {
	Q_OBJECT

public:
	PdfDownloader();

private:
	QNetworkAccessManager* networkManager;
	QNetworkCookieJar* cookies2;
	QNetworkAccessManager mgr2;

	QNetworkAccessManager mgr;

private slots:
	void serviceRequestFinished(QNetworkReply*);
	void saveFile(QNetworkReply* rep);
};

#endif PDFDOWNLOADER_H

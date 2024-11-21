/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2024 Clément VERMOT-DESROCHES (clement@vermot.net)

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

	enum Etape
	{
		Etape_INITIALISATION = 0,
		Etape_CONNEXION = 1,
		Etape_ATTENTE_TELECHARGEMENT = 2,
		Etape_FINALISER_TELECHARGEMENT = 3,
		Etape_TERMINE = 4
	};

	Etape phaseTraitement = Etape_INITIALISATION;

private slots:
	void serviceRequestFinished(QNetworkReply*);
	//void saveFile(QNetworkReply* rep);
};

#endif PDFDOWNLOADER_H

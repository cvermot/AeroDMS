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
#ifndef PDFEXTRACTOR_H
#define PDFEXTRACTOR_H

#include <QWidget>
#include <QWebEnginePage>

#include <podofo/podofo.h>

#include "AeroDmsTypes.h"

class PdfExtractor : public QWidget {
	Q_OBJECT

public:
	PdfExtractor();

	static AeroDmsTypes::ListeDonneesFacture  recupererLesDonneesDuPdf( const QString p_fichier,
		                                                                const AeroDmsTypes::Aeroclub p_aeroclub = AeroDmsTypes::Aeroclub_INCONNU);

private:
	static AeroDmsTypes::DonneesFacture extraireDonneesOpenFlyer(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::DonneesFacture extraireDonneesACAndernos(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesDaca(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);

	static const QDate extraireDate(const QString p_date);
	static const QTime extraireDuree(const QString p_duree);
	static const float recupererMontantAca(QString p_chaine);
};

#endif // PDFEXTRACTOR_H


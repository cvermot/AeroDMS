/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2025 Clément VERMOT-DESROCHES (clement@vermot.net)

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
	static AeroDmsTypes::ListeDonneesFacture  recupererLesDonneesDuPdf( const QString p_fichier,
		                                                                const AeroDmsTypes::Aeroclub p_aeroclub = AeroDmsTypes::Aeroclub_INCONNU);
	static AeroDmsTypes::ListeDonneesFacture  recupererLesDonneesDuCsv(const QString p_fichier);

private:
	static AeroDmsTypes::DonneesFacture extraireDonneesOpenFlyer(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::DonneesFacture extraireDonneesAerogest(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::DonneesFacture extraireDonneesACAndernos(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesDaca(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesSepavia(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesUaca(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesAtvv(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesGenerique(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	static AeroDmsTypes::ListeDonneesFacture extraireDonneesGenerique1Passe(std::vector<PoDoFo::PdfTextEntry> p_entries, const unsigned p_noPage);
	
	static const double recupererMontantAca(QString p_chaine);

	static const QDate extraireDateRegex(const QString p_date);
	static const QDate extraireDateTiretsUsRegex(const QString p_date);
	static const QTime extraireDureeRegex(const QString p_duree);
};

#endif // PDFEXTRACTOR_H

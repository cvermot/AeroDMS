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
#ifndef PDFPRINTER_H
#define PDFPRINTER_H

#include <QWidget>
#include <QtPrintSupport>

#include "AeroDmsTypes.h"
#include "DialogueProgressionImpression.h"

class PdfPrinter : public QWidget {
	Q_OBJECT

public:
	PdfPrinter();
	AeroDmsTypes::EtatImpression imprimerDossier(const QString p_dossier,
		const AeroDmsTypes::ParametresImpression p_parametresImpression);
	AeroDmsTypes::EtatImpression imprimerFichier(const QString p_fichier,
		const AeroDmsTypes::ParametresImpression p_parametresImpression);

private:
	void imprimerLaDemande();
	void imprimerLaDemandeAgrafage();
	bool selectionnerImprimante(QPrinter& p_printer, 
		const AeroDmsTypes::ParametresImpression p_parametresImpression);
	void imprimer(QPrinter &p_printer, 
		const bool p_forcerImpressionRecto);
	void ouvrirFenetreProgressionImpression(const int p_nombreDeFichiersAImprimer);

	DialogueProgressionImpression *progressionImpression;
	QString fichierAImprimer = "";
	QString dossierAImprimer = "";
	bool demandeImpressionEstConfirmee = false;

	QPrinter imprimante;
	QFileInfoList liste;
	AeroDmsTypes::ParametresImpression parametresImpression;

private slots:
	void imprimerFichierSuivant();
	void detruireFenetreProgressionImpression();
	
};

#endif // PDFPRINTER_H

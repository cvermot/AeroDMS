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
#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QWidget>
#include <QWebEnginePage>
#include "ManageDb.h"

class PdfRenderer : public QWidget {
	Q_OBJECT

public:
	PdfRenderer();
	PdfRenderer(ManageDb *p_db, QWidget* parent = nullptr);

	int imprimerLesDemandesDeSubvention( const QString p_nomTresorier,
		                                 const QString p_cheminSortieFichiersGeneres);

private :
	QWebEnginePage* view;
	ManageDb *db;

	QString cheminSortieFichiersGeneres;

	QStringList listeDesFichiers;
	AeroDmsTypes::DemandeEnCoursDeTraitement demandeEnCours;
	QList<int> listeAnnees;

	int nombreFacturesTraitees;
	int nombreFacturesATraiter;

	void imprimerLaProchaineDemandeDeSubvention();
	void remplirLeChampMontant(QString& p_html, const float p_montant);
	void produireFichierPdfGlobal();

signals:
	void mettreAJourNombreFactureTraitees(int nombreFacturesATraiter, int nombreFactureTraitees);

public slots:
    void chargementTermine(bool retour);
    void impressionTerminee(const QString& filePath, bool success);
    void statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode);

};

#endif // PDFRENDERER_H
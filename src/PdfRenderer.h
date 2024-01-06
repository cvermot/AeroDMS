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
		                                 const QString p_cheminSortieFichiersGeneres,
		                                 const QString p_cheminStockageFactures);

private :
	QWebEnginePage* view;
	ManageDb *db;

	QString cheminSortieFichiersGeneres;
	QString repertoireDesFactures;

	QStringList listeDesFichiers;
	AeroDmsTypes::DemandeEnCoursDeTraitement demandeEnCours;
	QList<int> listeAnnees;

	int nombreFacturesTraitees;
	int nombreFacturesATraiter;
	int indiceFichier;

	void imprimerLaProchaineDemandeDeSubvention();
	void remplirLeChampMontant(QString& p_html, const float p_montant);
	void produireFichierPdfGlobal();

	void recopierFacture(const QString p_nomFacture);
	void recopierFactures(const QStringList p_listeFactures);
	QString numeroFichierSur3Digits();

signals:
	void mettreAJourNombreFacturesTraitees(int p_nombreFacturesTraitees);
	void mettreAJourNombreFacturesATraiter(int p_nombreFacturesATraiter);
	void generationTerminee(QString p_cheminFichiers);

public slots:
    void chargementTermine(bool retour);
    void impressionTerminee(const QString& filePath, bool success);

};

#endif // PDFRENDERER_H
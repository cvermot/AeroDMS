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
	PdfRenderer( ManageDb *p_db,
		         QString p_cheminTemplatesHtml, 
		         QWidget* parent = nullptr);

	void imprimerLesDemandesDeSubvention( const QString p_nomTresorier,
		                                  const QString p_cheminSortieFichiersGeneres,
		                                  const QString p_cheminStockageFactures,
		                                  const AeroDmsTypes::TypeGenerationPdf p_typeGenerationPdf = AeroDmsTypes::TypeGenerationPdf_TOUTES,
		                                  const AeroDmsTypes::Signature p_signature = AeroDmsTypes::Signature_SANS,
		                                  const bool p_mergerTousLesPdf = true,
										  const bool p_recapHdVAvecRecettes = false,
										  const bool p_recapHdvAvecBaladesEtSorties = false,
		                                  const int p_valeurGraphAGenerer = 0);
	void imprimerLeRecapitulatifDesHeuresDeVol( const int p_annee,
		                                        const QString p_cheminSortieFichiersGeneres,
		                                        const QString p_cheminStockageFactures,
		                                        const AeroDmsTypes::Signature p_signature,
		                                        const int p_graphAGenerer);

private :
	QWebEnginePage* view;
	ManageDb *db;
	QUrl ressourcesHtml;

	QString cheminSortieFichiersGeneres;
	QString repertoireDesFactures;
	bool laDemandeEstPourUnDocumentUnique;

	QStringList listeDesFichiers;
	AeroDmsTypes::DemandeEnCoursDeTraitement demandeEnCours;
	QList<int> listeAnnees;

	int nombreEtapesEffectuees;
	int indiceFichier;

	void imprimerLaProchaineDemandeDeSubvention();
	void remplirLeChampMontant( QString& p_html, 
		                        const float p_montant);
	void remplirLeChampSignature(QString& p_html);

	void imprimerLeFichierPdfDeRecapAnnuel( const int p_annee, 
		                                    const AeroDmsTypes::ListeSubventionsParPilotes p_listePilotesDeCetteAnnee, 
		                                    const AeroDmsTypes::SubventionsParPilote p_totaux);
	QString genererImagesStatistiques(const int p_annee);

	void produireFichierPdfGlobal();

	QString genererHtmlRecapBaladesSorties(const int p_annee);

	static int calculerNbEtapesGenerationRecapHdV(const int p_graphAGenerer);

	QString numeroFichierSur3Digits();

	void mergerPdf();

	static AeroDmsTypes::ResolutionEtParametresStatistiques convertirResolution(const int p_resolution);

	void enregistrerImage( QWidget* p_widget,
						   QString p_urlImage,
						   QString p_titre);

signals:
	void mettreAJourNombreFacturesTraitees(int p_nombreFacturesTraitees);
	void mettreAJourNombreFacturesATraiter(int p_nombreFacturesATraiter);
	void generationTerminee(QString p_cheminFichiers);

public slots:
    void chargementTermine(bool retour);
    void impressionTerminee(const QString& filePath, bool success);

};

#endif // PDFRENDERER_H
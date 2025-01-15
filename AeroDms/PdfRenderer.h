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
#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QWidget>
#include <QWebEnginePage>
#include <QMarginsF>
#include "ManageDb.h"

class PdfRenderer : public QWidget {
	Q_OBJECT

public:
	PdfRenderer( ManageDb *p_db,
		         const QString p_cheminTemplatesHtml,
				 const QMarginsF p_marges,
		         QWidget* parent = nullptr);

	void mettreAJourMarges(QMarginsF p_marges);
	void mettreAJourCheminRessourcesHtml(const QString p_ressources);

	void imprimerLesDemandesDeSubvention( const QString p_nomTresorier,
		const QString p_cheminSortieFichiersGeneres,
		const QString p_cheminStockageFactures,
	    const AeroDmsTypes::TypeGenerationPdf p_typeGenerationPdf,
		const AeroDmsTypes::Signature p_signature,
		const bool p_mergerTousLesPdf,
		const bool p_recapHdVAvecRecettes,
		const bool p_recapHdvAvecBaladesEtSorties,
		const bool p_virementEstAutorise,
		const int p_valeurGraphAGenerer,
		const int p_annee);
	void imprimerLeRecapitulatifDesHeuresDeVol( const int p_annee,
		const QString p_cheminSortieFichiersGeneres,
		const QString p_cheminStockageFactures,
		const AeroDmsTypes::Signature p_signature,
		const int p_graphAGenerer,
		const QString p_nomTresorier);

private :
	QWebEnginePage* view;
	ManageDb *db;
	QMarginsF marges;
	QUrl ressourcesHtml;
	QString cheminSignature = "";

	QString cheminSortieFichiersGeneres;
	QString repertoireDesFactures;
	bool laDemandeEstPourUnDocumentUnique;

	QStringList listeDesFichiers;
	AeroDmsTypes::DemandeEnCoursDeTraitement demandeEnCours;
	QList<int> listeAnnees;

	int nombreEtapesEffectuees;
	int indiceFichier;

	AeroDmsTypes::EtatGeneration imprimerLaProchaineDemandeDeSubvention();
	void remplirLeChampMontant( QString& p_html, 
		                        const float p_montant) const;
	void remplirLeChampSignature(QString& p_html) const;

	AeroDmsTypes::EtatGeneration imprimerLeFichierPdfDeRecapAnnuel( const int p_annee,
		                                                            const AeroDmsTypes::ListeSubventionsParPilotes p_listePilotesDeCetteAnnee, 
		                                                            const AeroDmsTypes::SubventionsParPilote p_totaux);
	QString genererImagesStatistiques(const int p_annee);

	const QString  produireFichierPdfGlobal();

	QString genererHtmlRecapBaladesSorties( const int p_annee, 
		                                    AeroDmsTypes::EtatGeneration& p_etatGenerationARetourner);

	QString mergerPdf();

	QString numeroFichierSur3Digits();

	static int calculerNbEtapesGenerationRecapHdV(const int p_graphAGenerer);

	static AeroDmsTypes::ResolutionEtParametresStatistiques convertirResolution(const int p_resolution, const QMarginsF p_marges);

	static void enregistrerImage( QWidget &p_widget,
						          const QString p_urlImage,
							      const QString p_titre);
	static void copierFichierSvgDansHtml( const QString p_fichier,
									      QString& p_html);

	static void convertirEnPng(const QString p_fichierSvg,
		const QString p_fichierPng,
		const QSize targetSize);

	void rincerInfosIban(QString& p_templateCe);

signals:
	void mettreAJourNombreFacturesTraitees(int p_nombreFacturesTraitees);
	void mettreAJourNombreFacturesATraiter(int p_nombreFacturesATraiter);
	void generationTerminee(QString p_cheminFichiers, QString p_cheminFichierPdfGlobal);
	void echecGeneration();

public slots:
    void chargementTermine(bool retour);
    void impressionTerminee(const QString& filePath, bool success);

};

#endif // PDFRENDERER_H

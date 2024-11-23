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
	PdfDownloader(const QString p_cheminFacturesATraiter);

	void telechargerDonneesDaca(const QString p_identifiant, const QString p_motDePasse);
	void telechargerFactureDaca(const QString p_identifiant, const QString p_motDePasse, const AeroDmsTypes::IdentifiantFacture p_identifiantFacture);
	AeroDmsTypes::DonneesFacturesDaca recupererDonneesDaca();
	const QString recupererCheminDernierFichierTelecharge();

private:
	QNetworkAccessManager* networkManager;

	QString identifiantConnexion = "";
	QString motDePasse = "";
	AeroDmsTypes::IdentifiantFacture facture;

	QString repertoireFacturesATraiter = "";
	QString derniereFactureTelechargee = "";

	int nombreEssais = 0;

	AeroDmsTypes::DonneesFacturesDaca donneesDaca;

	enum Etape
	{
		Etape_INITIALISATION,
		Etape_CONNEXION,
		Etape_CONNECTE,
		Etape_ATTENTE_TELECHARGEMENT,
		Etape_TERMINE,
		Etape_ECHEC_CONNEXION,
		Etape_ECHEC_ENREGISTREMENT_FICHIER,
		Etape_ECHEC_TELECHARGEMENT
	};

	enum Demande
	{
		Demande_RECUPERE_INFOS_COMPTES,
		Demande_TELECHARGE_FACTURE
	};

	Etape phaseTraitement = Etape_INITIALISATION;
	Demande demandeEnCours = Demande_TELECHARGE_FACTURE;


	void telechargerFichier();
	void connecter();
	void parserDonneesDaca(const QByteArray& p_donnees);

private slots:
	void serviceRequestFinished(QNetworkReply*);
	//void saveFile(QNetworkReply* rep);

signals:
	void etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures);
};

#endif PDFDOWNLOADER_H

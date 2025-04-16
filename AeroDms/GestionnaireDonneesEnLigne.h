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
#ifndef GESTIONNAIREDONNEESENLIGNE_H
#define GESTIONNAIREDONNEESENLIGNE_H

#include <QWidget>
#include <QtNetwork>

#include "AeroDmsTypes.h"

class GestionnaireDonneesEnLigne : public QWidget {
	Q_OBJECT

public:
	GestionnaireDonneesEnLigne(const AeroDmsTypes::ParametresSysteme & p_parametres);

	void activer();
	const bool estActif();
	void recupererSha256Bdd();
	void envoyerFichier(QString p_chemin);
	void envoyerBdd(QString p_chemin);
	void telechargerBdd();
	void telechargerFacture(const QString p_nomFacture);

private:
	QNetworkAccessManager* networkManager = nullptr;
	AeroDmsTypes::ParametresSysteme parametres;
	QUrl serviceUrl;
	QString nomFichierEnTelechargement = "";

	int nombreEssais = 0;
	bool estActive = false;

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
		Demande_RECUPERER_SHA256_BDD,
		Demande_TELECHARGER_FACTURE,
		Demande_ENVOI_FACTURE,
		Demande_ENVOI_BDD,
		Demande_TELECHARGER_BDD,
	};

	Etape phaseTraitement = Etape_INITIALISATION;
	Demande demandeEnCours = Demande_TELECHARGER_FACTURE;
	QString cheminFichier = "";

	bool uneDemandeEstEnCours = false;

	struct DemandeBufferisee
	{
		Demande demande;
		QString cheminFichier = "";
	};
	QVector<DemandeBufferisee> listeDemandes;
	

	bool connecter();
	void libererMutex();

private slots:
	void serviceRequestFinished(QNetworkReply*);
	void demandeAuthentificationProxy(const QNetworkProxy& p_proxy,
		QAuthenticator* p_authenticator);
	void fournirIdentifiants(QNetworkReply* reply, QAuthenticator* authenticator);

signals:
	void etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures);
	void receptionSha256Bdd(QString);
	void baseDeDonneesTelechargeeEtDisponible();
	void finDEnvoiBdd();
};

#endif GESTIONNAIREDONNEESENLIGNE_H



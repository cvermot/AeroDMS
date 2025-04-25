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

#include "GestionnaireDonneesEnLigne.h"
#include "AeroDmsServices.h"

GestionnaireDonneesEnLigne::GestionnaireDonneesEnLigne(const AeroDmsTypes::ParametresSysteme & p_parametres)
{
    // On demande a utiliser le proxy système, si défini
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(serviceRequestFinished(QNetworkReply*)));
    connect(networkManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
        this, SLOT(demandeAuthentificationProxy(const QNetworkProxy&, QAuthenticator*)));

    connect(networkManager, &QNetworkAccessManager::authenticationRequired, this, &GestionnaireDonneesEnLigne::fournirIdentifiants);

    parametres = p_parametres;
}

void GestionnaireDonneesEnLigne::fournirIdentifiants(QNetworkReply* reply, QAuthenticator* authenticator)
{
    Q_UNUSED(reply); // Non utilisé
    authenticator->setUser(parametres.loginServeurModeExterne); 
    authenticator->setPassword(parametres.motDePasseServeurModeExterne); 
}

bool GestionnaireDonneesEnLigne::connecter()
{
    if (nombreEssais < 3)
    {
        nombreEssais = nombreEssais + 1;

        QNetworkRequest request(serviceUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        switch (demandeEnCours)
        {
            //Cas de demandes "simples" (get)
            case Demande_RECUPERER_SHA256_BDD :
            case Demande_TELECHARGER_BDD:
            case Demande_TELECHARGER_FACTURE:
            { 
                networkManager->get(request);
                phaseTraitement = Etape_CONNEXION;
            }
            break;
            //Cas des téléchargements de gros fichiers
            case Demande_TELECHARGER_MISE_A_JOUR :
            {
                reponseFichierEnCoursDeTelechargement = networkManager->get(request);
                phaseTraitement = Etape_CONNEXION;

                connect(reponseFichierEnCoursDeTelechargement, &QNetworkReply::readyRead, 
                    this, &GestionnaireDonneesEnLigne::enregistrerDonneesRecues);
                connect(reponseFichierEnCoursDeTelechargement, &QNetworkReply::downloadProgress, 
                    this, &GestionnaireDonneesEnLigne::gererProgressionTelechargement);
                //connect(reponseFichierEnCoursDeTelechargement, &QNetworkReply::finished, 
                //    this, &GestionnaireDonneesEnLigne::finaliserTelechargement);

                // Ouvrir le fichier de destination
                const QString fichierDeSortie = parametres.cheminStockageBdd + "/update.zip";
                fichierEnCoursDeTelechargement = new QFile(fichierDeSortie, this);
                if (!fichierEnCoursDeTelechargement->open(QIODevice::WriteOnly)) {
                    qWarning() << "Impossible d'ouvrir le fichier pour écrire:" << fichierDeSortie;
                    nombreEssais = 3;
                    reponseFichierEnCoursDeTelechargement->abort();  
                }
            }
            break;
            //Cas des envois de fichiers
            case Demande_ENVOI_FACTURE:
            {
                phaseTraitement = Etape_CONNEXION;
                QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                // Ajoutez le fichier
                QFile* file = new QFile(cheminFichier);
                if (!file->open(QIODevice::ReadOnly)) 
                {
                    qDebug() << "Erreur : Impossible d'ouvrir le fichier.";
                    delete file;
                    return false;
                }

                QHttpPart filePart;
                request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + multiPart->boundary());
                filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                    QVariant("form-data; name=\"fileToUpload\"; filename=\"" +
                        file->fileName() + "\""));
                filePart.setBodyDevice(file); // Attache le fichier au multipart
                file->setParent(multiPart);   // Permet au multiPart de gérer la suppression de l'objet file
                multiPart->append(filePart);

                // Envoyer la requête POST
                QNetworkReply* reply = networkManager->post(request, multiPart);
                multiPart->setParent(reply); // Le multiPart sera supprimé avec la réponse

            }
            break;
            case Demande_ENVOI_BDD:
            {
                phaseTraitement = Etape_CONNEXION;
                QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

                // Ajoutez le fichier
                QFile* file = new QFile(cheminFichier);
                if (!file->open(QIODevice::ReadOnly)) 
                {
                    qDebug() << "Erreur : Impossible d'ouvrir le fichier.";
                    delete file;
                    return false;
                }

                QHttpPart filePart;
                request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + multiPart->boundary());
                filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                    QVariant("form-data; name=\"fileToUpload\"; filename=\"" +
                        file->fileName() + "\""));
                filePart.setBodyDevice(file); // Attache le fichier au multipart
                file->setParent(multiPart);   // Permet au multiPart de gérer la suppression de l'objet file
                multiPart->append(filePart);

                // Envoyer la requête POST
                QNetworkReply* reply = networkManager->post(request, multiPart);
                multiPart->setParent(reply); // Le multiPart sera supprimé avec la réponse

                emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD_ENVOI);
            }
            break;
        }
        return true;
    }
    else
    {
        phaseTraitement = Etape_ECHEC_CONNEXION;
        //emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION);

        /*QMessageBox::critical(
            this, QApplication::applicationName() + " - " + tr("Erreur de connexion"),
            "Erreur de connexion au gestionnaire des données en ligne, à l'adresse \n"
            + parametres.adresseServeurModeExterne
            + ".");*/

        //3 erreurs sur cette requete. On passe à la suivante
        libererMutex();

        return false;
    }
}

void GestionnaireDonneesEnLigne::enregistrerDonneesRecues()
{
    switch(demandeEnCours)
    {
        case Demande_TELECHARGER_MISE_A_JOUR:
        {
            if (fichierEnCoursDeTelechargement != nullptr)
            {
                fichierEnCoursDeTelechargement->write(reponseFichierEnCoursDeTelechargement->readAll());
            }  
        }
        break;
        default:
        {
            //rien a faire dans les autres cas de demandes
        }
        break;
    }
}

void GestionnaireDonneesEnLigne::finaliserTelechargement()
{
    switch (demandeEnCours)
    {
        case Demande_TELECHARGER_MISE_A_JOUR:
        {
            if (fichierEnCoursDeTelechargement != nullptr) 
            {
                fichierEnCoursDeTelechargement->close();
                delete fichierEnCoursDeTelechargement;
                fichierEnCoursDeTelechargement = nullptr;
            }
            disconnect(reponseFichierEnCoursDeTelechargement);
            reponseFichierEnCoursDeTelechargement->deleteLater();
        }
        break;
        default:
        {
            //rien a faire dans les autres cas de demandes
        }
        break;
    }
}

void GestionnaireDonneesEnLigne::gererProgressionTelechargement(const qint64 p_nbOctetsRecus, const qint64 p_nbOctetsTotal)
{
    emit notifierProgressionTelechargement(p_nbOctetsRecus, p_nbOctetsTotal);
}

void GestionnaireDonneesEnLigne::demandeAuthentificationProxy(const QNetworkProxy& p_proxy,
    QAuthenticator* p_authenticator)
{
    QMessageBox::critical(
        this, QApplication::applicationName() + " - " + tr("Erreur proxy"),
        "Le proxy exige une authentification\n"
        "Authentification non supportée par l'application.");
}

void GestionnaireDonneesEnLigne::serviceRequestFinished(QNetworkReply* rep)
{
    if (phaseTraitement == Etape_CONNEXION)
    {
        if (rep->error() == QNetworkReply::NoError)
        {
            nombreEssais = 0;

            if (demandeEnCours == Demande_RECUPERER_SHA256_BDD)
            {
                const QByteArray sdata = rep->readAll();
                emit receptionSha256Bdd(sdata);
                emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256_RECU);
                libererMutex();
            }
            else if (demandeEnCours == Demande_ENVOI_FACTURE)
            {
                const QByteArray sdata = rep->readAll();
                if (sdata != "Le fichier PDF été téléchargé avec succès.")
                {
                    QMessageBox dialogueErreurVersionBdd;
                    dialogueErreurVersionBdd.setText(tr("L'envoi du fichier PDF en ligne a échoué pour la raison suivante :\n")
                        + sdata);
                    dialogueErreurVersionBdd.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur d'envoi du PDF en ligne"));
                    dialogueErreurVersionBdd.setIcon(QMessageBox::Critical);
                    dialogueErreurVersionBdd.setStandardButtons(QMessageBox::Close);
                    dialogueErreurVersionBdd.exec();
                }

                libererMutex();
            }
            else if (demandeEnCours == Demande_ENVOI_BDD)
            {
                const QByteArray sdata = rep->readAll();
                if (sdata != "La base de données a été téléchargée avec succès.")
                {
                    QMessageBox dialogueErreurVersionBdd;
                    dialogueErreurVersionBdd.setText(tr("L'envoi de la BDD en ligne a échoué pour la raison suivante :\n")
                    + sdata);
                    dialogueErreurVersionBdd.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur d'envoi de la BDD en ligne"));
                    dialogueErreurVersionBdd.setIcon(QMessageBox::Critical);
                    dialogueErreurVersionBdd.setStandardButtons(QMessageBox::Close);
                    dialogueErreurVersionBdd.exec();
                }
                
                emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD_FIN);
                emit finDEnvoiBdd();
                libererMutex();
            }
            else if (demandeEnCours == Demande_TELECHARGER_BDD)
            {
                const QByteArray sdata = rep->readAll();
                QMimeDatabase db;
                QMimeType type = db.mimeTypeForData(sdata);
                if (type.name() == "application/x-sqlite3")
                {
                    QString nomFichier = parametres.cheminStockageBdd
                        + "/"
                        + "AeroDMS_new.sqlite";

                    QFile localFile(nomFichier);
                    if (!localFile.open(QIODevice::WriteOnly))
                    {
                        phaseTraitement = Etape_ECHEC_ENREGISTREMENT_FICHIER;
                        //emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE);
                    }
                    else
                    {
                        localFile.write(sdata);
                        localFile.close();
                        phaseTraitement = Etape_CONNECTE;
                        emit baseDeDonneesTelechargeeEtDisponible();
                    }
                    emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_RECU);
                }
                libererMutex();
            }
            else if (demandeEnCours == Demande_TELECHARGER_FACTURE)
            {
                const QByteArray sdata = rep->readAll();
                QMimeDatabase db;
                QMimeType type = db.mimeTypeForData(sdata);
                if (type.name() == "application/pdf")
                {
                    QString nomFichier = parametres.cheminStockageFacturesTraitees
                        + "/"
                        + nomFichierEnTelechargement;

                    QFile localFile(nomFichier);
                    if (!localFile.open(QIODevice::WriteOnly))
                    {
                        phaseTraitement = Etape_ECHEC_ENREGISTREMENT_FICHIER;
                        //emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE);
                    }
                    else
                    {
                        localFile.write(sdata);
                        localFile.close();
                        phaseTraitement = Etape_CONNECTE;
                    }
                }
                else
                {
                    QMessageBox dialogueErreurTelechargementFacture;
                    dialogueErreurTelechargementFacture.setText(tr("Impossible de télécharger la facture ") +
                        nomFichierEnTelechargement +".\n");
                    dialogueErreurTelechargementFacture.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur de téléchargement d'une facture"));
                    dialogueErreurTelechargementFacture.setIcon(QMessageBox::Critical);
                    dialogueErreurTelechargementFacture.setStandardButtons(QMessageBox::Close);
                    dialogueErreurTelechargementFacture.exec();
                }
                libererMutex();
            }
            else if (demandeEnCours == Demande_TELECHARGER_MISE_A_JOUR)
            {
                finaliserTelechargement();
                emit zipMiseAJourDisponible();
                libererMutex();
            }
        }
        else
        {
            if (!connecter())
            {
                AeroDmsServices::afficherErreur(rep);
            }
        }
    }   
    else
    {
        //Rien à faire dans ces états...
    }
}

void GestionnaireDonneesEnLigne::activer()
{
    estActive = true;
}

const bool GestionnaireDonneesEnLigne::estActif()
{
    return estActive;
}

void GestionnaireDonneesEnLigne::envoyerFichier(QString p_chemin)
{
    if (!uneDemandeEstEnCours)
    {
        uneDemandeEstEnCours = true;

        serviceUrl = QUrl(parametres.adresseServeurModeExterne + "/upload.php");
        nombreEssais = 0;
        demandeEnCours = Demande_ENVOI_FACTURE;
        cheminFichier = p_chemin;

        connecter();
    }
    else
    {
        DemandeBufferisee demandeBufferisee;
        demandeBufferisee.demande = Demande_ENVOI_FACTURE;
        demandeBufferisee.cheminFichier = p_chemin;
        listeDemandes.append(demandeBufferisee);
    }
}

void GestionnaireDonneesEnLigne::envoyerBdd(QString p_chemin)
{
    if (estActive)
    {
        if (!uneDemandeEstEnCours)
        {
            uneDemandeEstEnCours = true;

            serviceUrl = QUrl(parametres.adresseServeurModeExterne + "/uploadBdd.php");
            nombreEssais = 0;
            demandeEnCours = Demande_ENVOI_BDD;
            cheminFichier = p_chemin;

            emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_ENVOI_BDD);

            connecter();
        }
        else
        {
            DemandeBufferisee demandeBufferisee;
            demandeBufferisee.demande = Demande_ENVOI_BDD;
            demandeBufferisee.cheminFichier = p_chemin;
            listeDemandes.append(demandeBufferisee);
        }
    }
}

void GestionnaireDonneesEnLigne::telechargerBdd()
{
    if (estActive)
    {
        if (!uneDemandeEstEnCours)
        {
            uneDemandeEstEnCours = true;

            serviceUrl = QUrl(parametres.adresseServeurModeExterne + "/telechargerBdd.php");
            nombreEssais = 0;
            demandeEnCours = Demande_TELECHARGER_BDD;
            emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_BDD_CONNEXION);

            connecter();
        }
        else
        {
            DemandeBufferisee demandeBufferisee;
            demandeBufferisee.demande = Demande_TELECHARGER_BDD;
            listeDemandes.append(demandeBufferisee);
        }
    } 
}

void GestionnaireDonneesEnLigne::telechargerMiseAJour(const QString p_url)
{
    if (estActive)
    {
        if (!uneDemandeEstEnCours)
        {
            uneDemandeEstEnCours = true;

            serviceUrl = QUrl(p_url);
            nombreEssais = 0;
            demandeEnCours = Demande_TELECHARGER_MISE_A_JOUR;

            connecter();
        }
        else
        {
            DemandeBufferisee demandeBufferisee;
            demandeBufferisee.demande = Demande_TELECHARGER_MISE_A_JOUR;
            demandeBufferisee.cheminFichier = p_url;
            listeDemandes.append(demandeBufferisee);
        }
    }
}

void GestionnaireDonneesEnLigne::telechargerFacture(const QString p_nomFacture)
{
    if (estActive)
    {
        if (!uneDemandeEstEnCours)
        {
            uneDemandeEstEnCours = true;

            serviceUrl = QUrl(parametres.adresseServeurModeExterne + "/uploads/" + p_nomFacture);
            nombreEssais = 0;
            demandeEnCours = Demande_TELECHARGER_FACTURE;
            nomFichierEnTelechargement = p_nomFacture;

            connecter();
        }
        else
        {
            DemandeBufferisee demandeBufferisee;
            demandeBufferisee.demande = Demande_TELECHARGER_FACTURE;
            demandeBufferisee.cheminFichier = p_nomFacture;
            listeDemandes.append(demandeBufferisee);
        }
    }
}

void GestionnaireDonneesEnLigne::recupererSha256Bdd()
{
    if (estActive)
    {
        if (!QSslSocket::supportsSsl()
            || QSslSocket::sslLibraryVersionString() == ""
            || QSslSocket::sslLibraryBuildVersionString() == "")
        {
            //emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION_SSL_TLS_INDISPONIBLE);
            QMessageBox::critical(this,
                QApplication::applicationName() + " - " + tr("Absence de support SSL/TLS"),
                tr("<b>Le support SSL/TLS est inactif pour votre version de Qt.</b>")
                + "<br /><br />"
                + tr("Les DLL nécessaires sont introuvables ou le support SSL est désactivé.")
                + "<br />"
                + tr("Les DLL <code>qopensslbackend.dll</code> ou <code>qschannelbackend.dll</code> doivent se trouver dans le répetoire <code>tls</code> au côté de l'application.")
                + "<br /><br />"
                + tr("Support OpenSSL : ")
                + (QSslSocket::supportsSsl() ? "Oui" : "Non")
                + "<br />"
                + tr("Version librairie SSL : ")
                + QSslSocket::sslLibraryVersionString()
                + "<br />"
                + tr("Build librairie SSL : ")
                + QSslSocket::sslLibraryBuildVersionString()
                + "<br />"
                + "<br /><br />"
                + tr("Impossible de se connecter au site de partage des données. Arret."));
        }
        else
        {
            if (!uneDemandeEstEnCours)
            {
                uneDemandeEstEnCours = true;

                serviceUrl = QUrl(parametres.adresseServeurModeExterne + "/db_sha256.php");
                nombreEssais = 0;
                demandeEnCours = Demande_RECUPERER_SHA256_BDD;
                emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_SHA256_CONNEXION);

                connecter();
            }
            else
            {
                DemandeBufferisee demandeBufferisee;
                demandeBufferisee.demande = Demande_RECUPERER_SHA256_BDD;
                listeDemandes.append(demandeBufferisee);
            }
        }
    }
}

void GestionnaireDonneesEnLigne::libererMutex()
{
    uneDemandeEstEnCours = false;

    if (listeDemandes.size() != 0)
    {
        //FIFO : on prend le premier item du vecteur
        DemandeBufferisee demande = listeDemandes.at(0);
        listeDemandes.pop_front();

        switch (demande.demande)
        {
            case Demande_RECUPERER_SHA256_BDD:
            {
                recupererSha256Bdd();
            }
            break;
            case Demande_TELECHARGER_BDD:
            {
                telechargerBdd();
            }
            break;
            case Demande_TELECHARGER_FACTURE:
            {
                telechargerFacture(demande.cheminFichier);
            }
            break;
            case Demande_ENVOI_FACTURE:
            {
                envoyerFichier(demande.cheminFichier);
            }
            break;
            case Demande_ENVOI_BDD:
            {
                envoyerBdd(demande.cheminFichier);
            }
            break;
            case Demande_TELECHARGER_MISE_A_JOUR:
            {
                telechargerMiseAJour(demande.cheminFichier);
            }
            break; 
        }         
    }
}
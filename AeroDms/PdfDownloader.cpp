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

#include "PdfDownloader.h"
#include "AeroDmsServices.h"

PdfDownloader::PdfDownloader(const QString p_cheminFacturesATraiter, 
    ManageDb* p_db)
{
    db = p_db;

    // On demande a utiliser le proxy système, si défini
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(serviceRequestFinished(QNetworkReply*)));
    connect(networkManager, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), 
        this, SLOT(demandeAuthentificationProxy(const QNetworkProxy&, QAuthenticator*)));

    repertoireFacturesATraiter = p_cheminFacturesATraiter;
}

void PdfDownloader::telechargerDonneesDaca(const QString p_identifiant,
    const QString p_motDePasse)
{
    if (!QSslSocket::supportsSsl()
        || QSslSocket::sslLibraryVersionString() == ""
        || QSslSocket::sslLibraryBuildVersionString() == "")
    {
        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION_SSL_TLS_INDISPONIBLE);
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
            + tr("Impossible de se connecter au site du DACA. Arret."));
    }
    else
    {
        identifiantConnexion = p_identifiant;
        motDePasse = p_motDePasse;

        nombreEssais = 0;
        demandeEnCours = Demande_RECUPERE_INFOS_COMPTES;

        connecter();
    }
}

void PdfDownloader::telechargerFactureDaca( const QString p_identifiant, 
    const QString p_motDePasse, 
    const AeroDmsTypes::IdentifiantFacture p_identifiantFacture)
{
    identifiantConnexion = p_identifiant;
    motDePasse = p_motDePasse;
    facture = p_identifiantFacture;
    derniereFactureTelechargee = "";

    nombreEssais = 0;
    demandeEnCours = Demande_TELECHARGE_FACTURE;

    if (phaseTraitement == Etape_CONNECTE)
    {
        telechargerFichier();
    }
    else
    {
        //Etat indéterminé : on redémarre depuis le début
        phaseTraitement = Etape_INITIALISATION;
        //networkManager->disconnect();
        connecter();
    }
}

const AeroDmsTypes::IdentifiantFacture PdfDownloader::recupererIdentifiantFactureTelechargee()
{
    return facture;
}

bool PdfDownloader::connecter()
{
    QUrl serviceUrl = QUrl("https://daca.fr/site5/adm_register.php");

    if (nombreEssais < 3)
    {
        nombreEssais = nombreEssais + 1;

        QUrlQuery postData;
        postData.addQueryItem("upseudo", identifiantConnexion);
        postData.addQueryItem("ppass", motDePasse);
        postData.addQueryItem("lng", "fr");

        QNetworkRequest request(serviceUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply* cnxReply = networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
        phaseTraitement = Etape_CONNEXION;
        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_CONNEXION_EN_COURS);

        return true;
    }
    else
    {
        phaseTraitement = Etape_ECHEC_CONNEXION;
        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION);

        return false;
    }  
}

void PdfDownloader::telechargerFichier()
{
    if (demandeEnCours == Demande_TELECHARGE_FACTURE)
    {
        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_RECUPERATION_FACTURE_EN_COURS);
        nombreEssais = nombreEssais + 1;
        phaseTraitement = Etape_ATTENTE_TELECHARGEMENT;
        QNetworkRequest req(QUrl(QString("https://daca.fr/site5/plugins/daca/html2pdf/releve_mensuel.php?compte_id=%1&mois=%2&annee=%3")
            .arg(facture.pilote, QString::number(facture.moisAnnee.month()), QString::number(facture.moisAnnee.year()))));

        QNetworkReply* reply = networkManager->get(req);
    }
    else if(demandeEnCours == Demande_RECUPERE_INFOS_COMPTES)
    {
        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_RECUPERATION_DONNEES_EN_COURS);
        nombreEssais = nombreEssais + 1;
        phaseTraitement = Etape_ATTENTE_TELECHARGEMENT;
        QNetworkRequest req(QUrl(QString("https://daca.fr/site5/plugins/daca/daca.php?ecran=compta_releves")));

        QNetworkReply* reply = networkManager->get(req);
    }
}

void PdfDownloader::serviceRequestFinished(QNetworkReply* rep)
{
    if (phaseTraitement == Etape_CONNEXION)
    {
        if (rep->error() == QNetworkReply::NoError)
        {
            nombreEssais = 0;

            if (rep->readAll().contains("Responsable CE"))
            {
                telechargerFichier();
            }
            else
            {
                QMessageBox::critical(this, 
                    QApplication::applicationName() + " - " + tr("Erreur de connexion au site du DACA"),
                    tr("La connexion au site du DACA est impossible.\n\nPlusieurs causes sont possibles :\n")
                    + tr("    -l'identifiant est invalide\n")
                    + tr("    -l'identifiant utilisé n'est pas celui d'un responsable CE (pas d'item \"Responsable CE\" dans la colonne \"Menu DACA\" du site)\n")
                    + tr("    -site du DACA hors ligne ou indisponible\n"));
                emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonnnesFactures_ECHEC_CONNEXION);
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
    else if ( phaseTraitement == Etape_ATTENTE_TELECHARGEMENT)
    {
        if (rep->size() != 0)
        {
            if (demandeEnCours == Demande_TELECHARGE_FACTURE)
            {
                const QByteArray sdata = rep->readAll();
                if (sdata.contains("%PDF-1.7"))
                {
                    QString nomFichier = repertoireFacturesATraiter
                        + "/" 
                        + facture.pilote
                        + "_"
                        + QString::number(facture.moisAnnee.year())
                        + "_"
                        + QString::number(facture.moisAnnee.month()).rightJustified(2, '0')
                        + "_"
                        + QApplication::applicationName()
                        + ".pdf";

                    QFile localFile(nomFichier);
                    if (!localFile.open(QIODevice::WriteOnly))
                    {
                        phaseTraitement = Etape_ECHEC_ENREGISTREMENT_FICHIER;
                        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE);
                    }
                    else
                    {
                        localFile.write(sdata);
                        localFile.close();
                        derniereFactureTelechargee = nomFichier;
                        phaseTraitement = Etape_CONNECTE;
                        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_FACTURE_RECUPEREE);
                    }
                    //networkManager->disconnect();
                }
                else
                {
                    if (nombreEssais < 3)
                    {
                        telechargerFichier();
                    }
                    else
                    {
                        //networkManager->disconnect();
                        phaseTraitement = Etape_ECHEC_TELECHARGEMENT;
                        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE);
                        AeroDmsServices::afficherErreur(rep);
                    }
                }
            }
            else if (demandeEnCours == Demande_RECUPERE_INFOS_COMPTES)
            {
                const QByteArray sdata = rep->readAll();
                phaseTraitement = Etape_CONNECTE;
                parserDonneesDaca(sdata);
            }
        }
    }
    else
    {
        //Rien à faire dans ces états...
    }
}

void PdfDownloader::parserDonneesDaca(const QByteArray &p_donnees)
{
    donneesDaca.listeMoisAnnees.clear();
    donneesDaca.listePilotes.clear();

    //Le site nous renvoie du Windows 1252... on convertit.
    const QString donnees = QString::fromLocal8Bit(p_donnees);

    //On récupère les numéros de comptes et le nom du pilote associé
    QRegularExpression regexCompte("<option value='(411\\..*?)'>Compte pilote de (.*?)</option>");
    QRegularExpressionMatchIterator iterateurCompte = regexCompte.globalMatch(donnees);

    while (iterateurCompte.hasNext()) 
    {
        QRegularExpressionMatch match = iterateurCompte.next();
        AeroDmsTypes::CleStringValeur item;
        item.cle = (match.captured(1));
        item.texte = (match.captured(2));

        QStringList nomPrenomPilote = item.texte.split(" ");
        if (nomPrenomPilote.size() == 2)
        {
            item.idPilote = db->piloteExiste(nomPrenomPilote.at(0), nomPrenomPilote.at(1));
            if (item.idPilote != "")
            {
                donneesDaca.listePilotes.append(item);
            }
            else
            {
                donneesDaca.listePilotesNonConnus.append(item);
            }
        }
        else
        {
            donneesDaca.listePilotesNonConnus.append(item);
        }      
    }

    //On récupère les dates disponibles
    QRegularExpression regexDate("<option value='(\\d{2})-(\\d{4})'[^>]*>(.*?)</option>");
    QRegularExpressionMatchIterator iterateurDate = regexDate.globalMatch(donnees);

    AeroDmsTypes::ListeCleStringValeur listeItem;
    QList<QDate> listeDates;
    while (iterateurDate.hasNext())
    {
        QRegularExpressionMatch match = iterateurDate.next();
        AeroDmsTypes::CleStringValeur item;
        item.cle = (match.captured(1));
        item.texte = (match.captured(3));

        listeItem.append(item);
        listeDates.append(QDate(match.captured(2).toInt(), match.captured(1).toInt(), 1));
    }

    QListIterator<QDate> dateIt(listeDates);
    dateIt.toBack();
    int nbMaxMois = 12;
    while (dateIt.hasPrevious() && nbMaxMois > 0)
    {
        donneesDaca.listeMoisAnnees.append(dateIt.previous());
        nbMaxMois--;
    }

    emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_DONNEES_RECUPEREES);
}

const AeroDmsTypes::DonneesFacturesDaca PdfDownloader::recupererDonneesDaca()
{
    return donneesDaca;
}

const QString PdfDownloader::recupererCheminDernierFichierTelecharge()
{
    return derniereFactureTelechargee;
}

void PdfDownloader::demandeAuthentificationProxy(const QNetworkProxy& p_proxy, 
    QAuthenticator* p_authenticator)
{
    QMessageBox::critical(
        this, QApplication::applicationName() + " - " + tr("Erreur proxy"),
        "Le proxy exige une authentification\n"
        "Authentification non supportée par l'application.");
}

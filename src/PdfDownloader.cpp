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

#include "PdfDownloader.h"

PdfDownloader::PdfDownloader(const QString p_cheminFacturesATraiter, ManageDb* p_db)
{
    db = p_db;

    // On demande a utiliser le proxy système, si défini
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(serviceRequestFinished(QNetworkReply*)));

    repertoireFacturesATraiter = p_cheminFacturesATraiter;
}

void PdfDownloader::telechargerDonneesDaca(const QString p_identifiant,
    const QString p_motDePasse)
{
    identifiantConnexion = p_identifiant;
    motDePasse = p_motDePasse;

    nombreEssais = 0;
    demandeEnCours = Demande_RECUPERE_INFOS_COMPTES;

    connecter();
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
                afficherErreur(rep->error());
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
                    //qDebug() << "OK fichier PDF";
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
                    //qDebug() << "Fichier non PDF. Redemande telech";

                    if (nombreEssais < 3)
                    {
                        telechargerFichier();
                    }
                    else
                    {
                        //networkManager->disconnect();
                        phaseTraitement = Etape_ECHEC_TELECHARGEMENT;
                        emit etatRecuperationDonnees(AeroDmsTypes::EtatRecuperationDonneesFactures_ECHEC_RECUPERATION_FACTURE);
                        afficherErreur(rep->error());
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
            if (db->piloteExiste(nomPrenomPilote.at(0), nomPrenomPilote.at(1)))
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

AeroDmsTypes::DonneesFacturesDaca PdfDownloader::recupererDonneesDaca()
{
    return donneesDaca;
}

const QString PdfDownloader::recupererCheminDernierFichierTelecharge()
{
    return derniereFactureTelechargee;
}

void PdfDownloader::afficherErreur(const QNetworkReply::NetworkError p_erreur)
{
   
    switch (p_erreur)
    {
        case QNetworkReply::ConnectionRefusedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The remote server refused the connection (the server is not accepting "
                "requests)\n Erreur remontée : QNetworkReply::ConnectionRefusedError ("
                + QString::number(QNetworkReply::ConnectionRefusedError) + ")");
        }
        break;
        case QNetworkReply::RemoteHostClosedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The remote server closed the connection prematurely, before the entire "
                "reply was received and processed\n Erreur remontée : "
                "QNetworkReply::RemoteHostClosedError ("
                + QString::number(QNetworkReply::RemoteHostClosedError) + ")");
        }
        break;
        case QNetworkReply::HostNotFoundError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The remote host name was not found (invalid hostname)\n Erreur remontée "
                ": QNetworkReply::HostNotFoundError ("
                + QString::number(QNetworkReply::HostNotFoundError) + ")");
        }
        break;
        case QNetworkReply::TimeoutError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The connection to the remote server timed out\n Erreur remontée : "
                "QNetworkReply::TimeoutError ("
                + QString::number(QNetworkReply::TimeoutError) + ")");
        }
        break;
        case QNetworkReply::OperationCanceledError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The operation was canceled via calls to abort() or close() before it "
                "was finished.\n Erreur remontée : QNetworkReply::OperationCanceledError ("
                + QString::number(QNetworkReply::OperationCanceledError) + ")");
        }
        break;
        case QNetworkReply::SslHandshakeFailedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The SSL/TLS handshake failed and the encrypted channel could not be "
                "established. The sslErrors() signal should have been emitted.\n Erreur "
                "remontée : QNetworkReply::SslHandshakeFailedError ("
                + QString::number(QNetworkReply::SslHandshakeFailedError) + ")");
        }
        break;
        case QNetworkReply::TemporaryNetworkFailureError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The connection was broken due to disconnection from the network, "
                "however the system has initiated roaming to another access point. The "
                "request should be resubmitted and will be processed as soon as the "
                "connection is re-established.\n Erreur remontée : "
                "QNetworkReply::TemporaryNetworkFailureError ("
                + QString::number(QNetworkReply::TemporaryNetworkFailureError)
                + ")");
        }
        break;
        case QNetworkReply::NetworkSessionFailedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The connection was broken due to disconnection from the network or "
                "failure to start the network.\n Erreur remontée : "
                "QNetworkReply::NetworkSessionFailedError ("
                + QString::number(QNetworkReply::NetworkSessionFailedError) + ")");
        }
        break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The background request is not currently allowed due to platform "
                "policy.\n Erreur remontée : "
                "QNetworkReply::BackgroundRequestNotAllowedError ("
                + QString::number(QNetworkReply::BackgroundRequestNotAllowedError)
                + ")");
        }
        break;
        case QNetworkReply::TooManyRedirectsError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "While following redirects, the maximum limit was reached. The limit is "
                "by default set to 50 or as set by "
                "QNetworkRequest::setMaxRedirectsAllowed(). (This value was introduced "
                "in 5.6.)\n Erreur remontée : QNetworkReply::TooManyRedirectsError ("
                + QString::number(QNetworkReply::TooManyRedirectsError) + ")");
        }
        break;
        case QNetworkReply::InsecureRedirectError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "While following redirects, the network access API detected a redirect "
                "from a encrypted protocol (https) to an unencrypted one (http). (This "
                "value was introduced in 5.6.)\n Erreur remontée : "
                "QNetworkReply::InsecureRedirectError ("
                + QString::number(QNetworkReply::InsecureRedirectError) + ")");
        }
        break;
        case QNetworkReply::ProxyConnectionRefusedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The connection to the proxy server was refused (the proxy server is not "
                "accepting requests)\n Erreur remontée : "
                "QNetworkReply::ProxyConnectionRefusedError ("
                + QString::number(QNetworkReply::ProxyConnectionRefusedError) + ")");
        }
        break;
        case QNetworkReply::ProxyConnectionClosedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The proxy server closed the connection prematurely, before the entire "
                "reply was received and processed\n Erreur remontée : "
                "QNetworkReply::ProxyConnectionClosedError ("
                + QString::number(QNetworkReply::ProxyConnectionClosedError) + ")");
        }
        break;
        case QNetworkReply::ProxyNotFoundError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The proxy host name was not found (invalid proxy hostname)\n Erreur "
                "remontée : QNetworkReply::ProxyNotFoundError ("
                + QString::number(QNetworkReply::ProxyNotFoundError) + ")");
        }
        break;
        case QNetworkReply::ProxyTimeoutError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The connection to the proxy timed out or the proxy did not reply in "
                "time to the request sent\n Erreur remontée : "
                "QNetworkReply::ProxyTimeoutError ("
                + QString::number(QNetworkReply::ProxyTimeoutError) + ")");
        }
        break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The proxy requires authentication in order to honour the request but "
                "did not accept any credentials offered (if any)\n Erreur remontée : "
                "QNetworkReply::ProxyAuthenticationRequiredError ("
                + QString::number(QNetworkReply::ProxyAuthenticationRequiredError)
                + ")");
        }
        break;
        case QNetworkReply::ContentAccessDenied:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The access to the remote content was denied (similar to HTTP error "
                "403)\n Erreur remontée : QNetworkReply::ContentAccessDenied ("
                + QString::number(QNetworkReply::ContentAccessDenied) + ")");
        }
        break;
        case QNetworkReply::ContentOperationNotPermittedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The operation requested on the remote content is not permitted\n Erreur "
                "remontée : QNetworkReply::ContentOperationNotPermittedError ("
                + QString::number(QNetworkReply::ContentOperationNotPermittedError)
                + ")");
        }
        break;
        case QNetworkReply::ContentNotFoundError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The remote content was not found at the server (similar to HTTP error "
                "404)\n Erreur remontée : QNetworkReply::ContentNotFoundError ("
                + QString::number(QNetworkReply::ContentNotFoundError) + ")");
        }
        break;
        case QNetworkReply::AuthenticationRequiredError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The remote server requires authentication to serve the content but the "
                "credentials provided were not accepted (if any)\n Erreur remontée : "
                "QNetworkReply::AuthenticationRequiredError ("
                + QString::number(QNetworkReply::AuthenticationRequiredError) + ")");
        }
        break;
        case QNetworkReply::ContentReSendError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The request needed to be sent again, but this failed for example "
                "because the upload data could not be read a second time.\n Erreur "
                "remontée : QNetworkReply::ContentReSendError ("
                + QString::number(QNetworkReply::ContentReSendError) + ")");
        }
        break;
        case QNetworkReply::ContentConflictError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The request could not be completed due to a conflict with the current "
                "state of the resource.\n Erreur remontée : "
                "QNetworkReply::ContentConflictError ("
                + QString::number(QNetworkReply::ContentConflictError) + ")");
        }
        break;
        case QNetworkReply::ContentGoneError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The requested resource is no longer available at the server.\n Erreur "
                "remontée : QNetworkReply::ContentGoneError ("
                + QString::number(QNetworkReply::ContentGoneError) + ")");
        }
        break;
        case QNetworkReply::InternalServerError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The server encountered an unexpected condition which prevented it from "
                "fulfilling the request.\n Erreur remontée : "
                "QNetworkReply::InternalServerError ("
                + QString::number(QNetworkReply::InternalServerError) + ")");
        }
        break;
        case QNetworkReply::OperationNotImplementedError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The server does not support the functionality required to fulfill the "
                "request.\n Erreur remontée : "
                "QNetworkReply::OperationNotImplementedError ("
                + QString::number(QNetworkReply::OperationNotImplementedError)
                + ")");
        }
        break;
        case QNetworkReply::ServiceUnavailableError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The server is unable to handle the request at this time.\n Erreur "
                "remontée : QNetworkReply::ServiceUnavailableError ("
                + QString::number(QNetworkReply::ServiceUnavailableError) + ")");
        }
        break;
        case QNetworkReply::ProtocolUnknownError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The Network Access API cannot honor the request because the protocol is "
                "not known\n Erreur remontée : QNetworkReply::ProtocolUnknownError ("
                + QString::number(QNetworkReply::ProtocolUnknownError) + ")");
        }
        break;
        case QNetworkReply::ProtocolInvalidOperationError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "The requested operation is invalid for this protocol\n Erreur remontée "
                ": QNetworkReply::ProtocolInvalidOperationError ("
                + QString::number(QNetworkReply::ProtocolInvalidOperationError)
                + ")");
        }
        break;
        case QNetworkReply::UnknownNetworkError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "An unknown network-related error was detected\n Erreur remontée : "
                "QNetworkReply::UnknownNetworkError ("
                + QString::number(QNetworkReply::UnknownNetworkError) + ")");
        }
        break;
        case QNetworkReply::UnknownProxyError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "An unknown proxy-related error was detected\n Erreur remontée : "
                "QNetworkReply::UnknownProxyError ("
                + QString::number(QNetworkReply::UnknownProxyError) + ")");
        }
        break;
        case QNetworkReply::UnknownContentError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "An unknown error related to the remote content was detected\n Erreur "
                "remontée : QNetworkReply::UnknownContentError ("
                + QString::number(QNetworkReply::UnknownContentError) + ")");
        }
        break;
        case QNetworkReply::ProtocolFailure:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "A breakdown in protocol was detected (parsing error, invalid or "
                "unexpected responses, etc.)\n Erreur remontée : "
                "QNetworkReply::ProtocolFailure ("
                + QString::number(QNetworkReply::ProtocolFailure) + ")");
        }
        break;
        case QNetworkReply::UnknownServerError:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "An unknown error related to the server response was detected\n Erreur "
                "remontée : QNetworkReply::UnknownServerError ("
                + QString::number(QNetworkReply::UnknownServerError) + ")");
        }
        break;
        default:
        {
            QMessageBox::information(
                this, QApplication::applicationName() + " - " + tr("Erreur réseau"),
                "Erreur indéterminée\n"
                "Code d'erreur remonté : "
                + QString::number(p_erreur));
        }
        break;
    }
}

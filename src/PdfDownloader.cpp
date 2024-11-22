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

PdfDownloader::PdfDownloader()
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(serviceRequestFinished(QNetworkReply*)));
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

void PdfDownloader::telechargerFactureDaca(const QString p_identifiant, 
    const QString p_motDePasse, 
    const QString p_nomFacture)
{
    identifiantConnexion = p_identifiant;
    motDePasse = p_motDePasse;
    factureATelecharger = p_nomFacture;

    nombreEssais = 0;
    demandeEnCours = Demande_TELECHARGE_FACTURE;

    connecter();
}

void PdfDownloader::connecter()
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
    }
    else
    {
        phaseTraitement = Etape_ECHEC_CONNEXION;
    }  
}

void PdfDownloader::telechargerFichier()
{
    if (demandeEnCours == Demande_TELECHARGE_FACTURE)
    {
        nombreEssais = nombreEssais + 1;
        phaseTraitement = Etape_ATTENTE_TELECHARGEMENT;
        QNetworkRequest req(QUrl(QString("https://daca.fr/site5/plugins/daca/html2pdf/releve_mensuel.php?compte_id=%1&mois=%2&annee=%3")
            .arg(factureATelecharger, "09", "2024")));

        QNetworkReply* reply = networkManager->get(req);
    }
    else if(demandeEnCours == Demande_RECUPERE_INFOS_COMPTES)
    {
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

            telechargerFichier();
        }
        else
        {
            connecter();
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
                    qDebug() << "OK fichier PDF";
                    QFile localFile("downloadedfile2.pdf");
                    if (!localFile.open(QIODevice::WriteOnly))
                    {
                        phaseTraitement = Etape_ECHEC_ENREGISTREMENT_FICHIER;
                    }
                    else
                    {
                        localFile.write(sdata);
                        localFile.close();
                        phaseTraitement = Etape_TERMINE;
                    }
                    networkManager->disconnect();
                }
                else
                {
                    qDebug() << "Fichier non PDF. Redemande telech";

                    if (nombreEssais < 3)
                    {
                        telechargerFichier();
                    }
                    else
                    {
                        networkManager->disconnect();
                        phaseTraitement = Etape_ECHEC_TELECHARGEMENT;
                    }
                }
            }
            else if (demandeEnCours == Demande_RECUPERE_INFOS_COMPTES)
            {
                const QByteArray sdata = rep->readAll();
                recupererDonneesDaca(sdata);
            }
        }
    }
    else
    {
        qDebug() << "terminé";
    }
}

void PdfDownloader::recupererDonneesDaca(const QByteArray &p_donnees)
{
    donneesDaca.listeMois.clear();
    donneesDaca.listePilotes.clear();

    //Le site nous renvoie du Windows 1252... on convertit.
    const QString donnees = QString::fromLocal8Bit(p_donnees);

    //On récupère les numéros de comptes et le nom du pilote associé
    QRegularExpression regexCompte("<option value='(411\\..*?)'>Compte pilote de (.*?)</option>");
    QRegularExpressionMatchIterator iterateurCompte = regexCompte.globalMatch(donnees);

    while (iterateurCompte.hasNext()) {
        QRegularExpressionMatch match = iterateurCompte.next();
        AeroDmsTypes::CleStringValeur item;
        item.cle = (match.captured(1));
        item.texte = (match.captured(2));
        donneesDaca.listePilotes.append(item);

        qDebug() << item.cle << item.texte;
    }

    //On récupère les dates disponibles
    QRegularExpression regexDate("<option value='(\\d{2}-\\d{4})'>(.*?)</option>");
    QRegularExpressionMatchIterator iterateurDate = regexDate.globalMatch(donnees);

    while (iterateurDate.hasNext())
    {
        QRegularExpressionMatch match = iterateurDate.next();
        AeroDmsTypes::CleStringValeur item;
        item.cle = (match.captured(1));
        item.texte = (match.captured(2));
        donneesDaca.listeMois.append(item);

        qDebug() << item.cle << item.texte;
    }

}

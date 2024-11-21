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
    QUrl serviceUrl = QUrl("https://daca.fr/site5/adm_register.php");
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(serviceRequestFinished(QNetworkReply*)));

    QUrlQuery postData;
    postData.addQueryItem("upseudo", "fakelogin");
    postData.addQueryItem("ppass", "fakepasswd");
    postData.addQueryItem("lng", "fr");

    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply* cnxReply = networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    qDebug() << "connexion" << cnxReply->error() << cnxReply->readAll();

    qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkCookieJar* cookies = networkManager->cookieJar();

    QEventLoop eventLoop;

    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(serviceRequestFinished(QNetworkReply*)));
    phaseTraitement = Etape_CONNEXION;
    QNetworkRequest req(QUrl("https://daca.fr"));
    QNetworkReply* reply = networkManager->get(req);
 
    eventLoop.exec();


    

}

void PdfDownloader::serviceRequestFinished(QNetworkReply* rep)
{
    qDebug() << "reply" << rep->error() << rep->size()  <<  phaseTraitement;
    //qDebug() << rep->readAll();

    if (phaseTraitement == Etape_CONNEXION)
    {
        qDebug() << "réponse" << rep->error() << rep->size();
        if (rep->error() == QNetworkReply::NoError)
        {
            //mgr.setCookieJar(cookies);
            //QMessageBox::information(this, "Result", "Result");
            qDebug() << "reponse get" << rep->readAll();

            QThread::sleep(2);
            phaseTraitement = Etape_ATTENTE_TELECHARGEMENT;
            QNetworkRequest req(QUrl("https://daca.fr/site5/plugins/daca/html2pdf/releve_mensuel.php?compte_id=411.pilote.infopilote&mois=09&annee=2024"));
            QNetworkReply* reply = networkManager->get(req);
            //eventLoop.exec();
            qDebug() << "demande download";
        }
        else
        {
            //QMessageBox::critical(this, "Erreur", "Erreur");
            qDebug() << "erruer connexion" << rep->readAll();
            phaseTraitement = Etape_TERMINE;
        }
        

    }
    else if (phaseTraitement == Etape_ATTENTE_TELECHARGEMENT)
    {
        qDebug() << "réponse download" << rep->size();

        if (rep->size() != 0)
        {
            QFile localFile("downloadedfile.pdf");
            if (!localFile.open(QIODevice::WriteOnly))
                return;
            const QByteArray sdata = rep->readAll();
            localFile.write(sdata);
            qDebug() << sdata;
            localFile.close();

            phaseTraitement = Etape_FINALISER_TELECHARGEMENT;
        }
    }
    else if (phaseTraitement == Etape_FINALISER_TELECHARGEMENT)
    {
        qDebug() << "réponse download" << rep->size();

        if (rep->size() != 0)
        {
            QFile localFile("downloadedfile2.pdf");
            if (!localFile.open(QIODevice::WriteOnly))
                return;
            const QByteArray sdata = rep->readAll();
            localFile.write(sdata);
            qDebug() << sdata;
            localFile.close();

            phaseTraitement = Etape_TERMINE;
        }

    }
    else
    {
        qDebug() << "terminé";
    }


    
    //QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(saveFile(QNetworkReply*)));

    

}

/*void PdfDownloader::saveFile(QNetworkReply* rep)
{
    qDebug() << "réponse download" << rep->size();

    QFile localFile("downloadedfile.pdf");
    if (!localFile.open(QIODevice::WriteOnly))
        return;
    const QByteArray sdata = rep->readAll();
    localFile.write(sdata);
    qDebug() << sdata;
    localFile.close();
}*/
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

    
    //mgr.setCookieJar(cookies);
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QNetworkRequest req(QUrl("https://daca.fr"));
    QNetworkReply* reply = mgr.get(req);
    eventLoop.exec();

    qDebug() << "réponse" << reply->error() << reply->size();
    if (reply->error() == QNetworkReply::NoError)
    {
        mgr.setCookieJar(cookies);
        QMessageBox::information(this, "Result", "Result");
        qDebug() << "reponse get" << reply->readAll();
    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Erreur");
    }

}

void PdfDownloader::serviceRequestFinished(QNetworkReply* rep)
{
    qDebug() << "reply" << rep->error();
    qDebug() << rep->readAll();

    //QEventLoop eventLoop;


    
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(saveFile(QNetworkReply*)));

    QNetworkRequest req(QUrl("https://daca.fr/fakeUrl"));
    QNetworkReply* reply = mgr.get(req);
    //eventLoop.exec();
    qDebug() << "demande download";

}

void PdfDownloader::saveFile(QNetworkReply* rep)
{
    qDebug() << "réponse download" << rep->size();

    QFile localFile("downloadedfile.pdf");
    if (!localFile.open(QIODevice::WriteOnly))
        return;
    const QByteArray sdata = rep->readAll();
    localFile.write(sdata);
    qDebug() << sdata;
    localFile.close();
}
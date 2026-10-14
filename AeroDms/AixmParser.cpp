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

#include "AixmParser.h"
#include "AeroDmsServices.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

AixmParser::AixmParser(ManageDb* p_db,
	QWidget* p_parent)
{
	db = p_db;
}

void  AixmParser::mettreAJourAerodromes(const QString p_fichierAixm)
{
    QFile file(p_fichierAixm);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
    {
        QMessageBox::critical(this,
            QApplication::applicationName() + " - " + tr("Mise à jour de la liste des aérodromes"),
            tr("Impossible d'ouvrir le fichier ")+ p_fichierAixm +".");
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) 
    {
        QMessageBox::critical(this,
            QApplication::applicationName() + " - " + tr("Mise à jour de la liste des aérodromes"),
            tr("Impossible de parser le fichier XML ") + p_fichierAixm + ".");
        file.close();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();

    if (root.attribute("version") != "4.5") 
    {
        QMessageBox::critical(this,
            QApplication::applicationName() + " - " + tr("Mise à jour de la liste des aérodromes"),
            tr("Erreur de lecture du fichier XML<br><br>Le fichier ") + p_fichierAixm + tr(" n'est pas un fichier AIXM 4.5.<br><br>Arrêt."));
        file.close();
        return;
    }

    int nombreCree = 0;
	int nombreMisAJour = 0;

    // Parcourir les aérodromes
    QDomNodeList aerodromes = root.elementsByTagName("Ahp");

    for (int i = 0; i < aerodromes.count(); ++i) 
    {
        QDomNode aerodromeNode = aerodromes.at(i);
        if (aerodromeNode.isElement()) 
        {
            QDomElement aerodromeElement = aerodromeNode.toElement();

            QDomNodeList nameNodes = aerodromeElement.elementsByTagName("txtName");
            QDomNodeList typeNodes = aerodromeElement.elementsByTagName("codeType");
            QDomNodeList icaoCodeNodes = aerodromeElement.elementsByTagName("codeIcao");
            QDomNodeList latitude = aerodromeElement.elementsByTagName("geoLat");
            QDomNodeList longitude = aerodromeElement.elementsByTagName("geoLong");

            if (nameNodes.count() == 2
                && typeNodes.count() == 1
                && icaoCodeNodes.count() == 1
                && nameNodes.at(0).toElement().text() == "FRANCE"
                && typeNodes.at(0).toElement().text() == "AD")
            {
                const QString name = AeroDmsServices::capitaliserTexte(nameNodes.at(1).toElement().text());
                const QString icaoCode = icaoCodeNodes.at(0).toElement().text();

                const AeroDmsTypes::Status status = db->mettreAJourAerodrome(icaoCode, 
                    name, 
                    conversionSexagesimalVersDecimal(latitude.at(0).toElement().text()),
                    conversionSexagesimalVersDecimal(longitude.at(0).toElement().text()));
                switch (status)
                {
				    case AeroDmsTypes::Status_CREATION:
					    nombreCree++;
					break;
				    case AeroDmsTypes::Status_MIS_A_JOUR:
					    nombreMisAJour++;
					break;
				    default:
					break;
                }
            }
            emit signalerMiseAJourAerodrome(aerodromes.count(), i + 1, nombreCree, nombreMisAJour);
            qApp->processEvents();
        }
    }
}

double AixmParser::conversionSexagesimalVersDecimal(QString p_coordonnees) 
{
    // 1) Longitude : DDDMMSS(.sss)?[EW]
    static const QRegularExpression reLon(
        R"(^(\d{3})([0-5]\d)([0-5]\d(?:\.\d+)?)([EW])$)"
    );

    // 2) Latitude : DDMMSS(.sss)?[NS]
    static const QRegularExpression reLat(
        R"(^(\d{2})([0-5]\d)([0-5]\d(?:\.\d+)?)([NS])$)"
    );

    QRegularExpressionMatch m = reLon.match(p_coordonnees);
    if (!m.hasMatch())
    {
        m = reLat.match(p_coordonnees);
    } 

    if (!m.hasMatch())
    {
        return 0.0; // format invalide
    }   

    double negatif = 1.0;
    if (m.captured(4) == "W" 
        || m.captured(4) == "S")
    {
        negatif = -1.0;
    }
    const double degres = m.captured(1).toDouble();
    const double minutes = m.captured(2).toDouble();
    const double secondes = m.captured(3).toDouble();

    return negatif * (degres + (minutes / 60.0) + (secondes / 3600.0));
}

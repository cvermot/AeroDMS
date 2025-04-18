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

            if (nameNodes.count() == 2
                && typeNodes.count() == 1
                && icaoCodeNodes.count() == 1
                && nameNodes.at(0).toElement().text() == "FRANCE"
                && typeNodes.at(0).toElement().text() == "AD")
            {
                const QString name = AeroDmsServices::capitaliserTexte(nameNodes.at(1).toElement().text());
                const QString icaoCode = icaoCodeNodes.at(0).toElement().text();

                const AeroDmsTypes::Status status = db->mettreAJourAerodrome(icaoCode, name);
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

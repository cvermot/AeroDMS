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

#include "AeroDmsServices.h"

const QString AeroDmsServices::convertirMinutesEnHeuresMinutes(const int p_minutes)
{
    const int heures = p_minutes / 60;
    const int minutes = p_minutes % 60;
    QString minutesString = QString::number(minutes);
    if (minutesString.size() == 1)
    {
        minutesString = QString("0").append(minutesString);
    }
    QString heuresMinutes = QString::number(heures).append("h").append(minutesString);
    return heuresMinutes;
}

void AeroDmsServices::ajouterIconesComboBox(QComboBox& p_activite)
{
    for (int i = 0; i < p_activite.count(); i++)
    {
        if (p_activite.itemText(i) == "ULM")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/feather.svg"));
        }
        else if (p_activite.itemText(i) == "Avion")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/airplane.svg"));
        }
        else if (p_activite.itemText(i) == "Planeur")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/airplane-landing.svg"));
        }
        else if (p_activite.itemText(i) == "Hélicoptère")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/helicopter.svg"));
        }
        else if (p_activite.itemText(i) == "Balade")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/account-group.svg"));
        }
        else if (p_activite.itemText(i) == "Sortie")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/bag-checked.svg"));
        }
        else if (p_activite.itemText(i) == "Entrainement")
        {
            p_activite.setItemIcon(i, QIcon("./ressources/airport.svg"));
        }
    }
}

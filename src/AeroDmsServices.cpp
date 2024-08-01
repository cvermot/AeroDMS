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

const QTime AeroDmsServices::convertirMinutesEnQTime(const int p_minutes)
{
    const int heures = p_minutes / 60;
    const int minutes = p_minutes % 60;
    return QTime(heures, minutes, 0);
}

const QTime AeroDmsServices::convertirHeuresDecimalesEnQTime(const double p_heureDecimale)
{
    QTime heureHhmm;
    int heure = floor(p_heureDecimale);
    int minutes = 60 * (p_heureDecimale - floor(p_heureDecimale));
    heureHhmm.setHMS(heure, minutes, 0);

    return heureHhmm;
}

QIcon AeroDmsServices::recupererIcone(const QString& p_icone)
{
        if (p_icone == "ULM")
        {
            return QIcon("./ressources/feather.svg");
        }
        else if (p_icone == "Avion")
        {
            return QIcon("./ressources/airplane.svg");
        }
        else if (p_icone == "Planeur")
        {
            return QIcon("./ressources/airplane-landing.svg");
        }
        else if (p_icone == "Hélicoptère")
        {
            return QIcon("./ressources/helicopter.svg");
        }
        else if (p_icone == "Avion électrique")
        {
            return QIcon("./ressources/lightning-bolt.svg");
        }
        else if (p_icone == "Balade")
        {
            return QIcon("./ressources/account-group.svg");
        }
        else if (p_icone == "Sortie")
        {
            return QIcon("./ressources/bag-checked.svg");
        }
        else if (p_icone == "Entrainement")
        {
            return QIcon("./ressources/airport.svg");
        }
        else if (p_icone == "Total")
        {
            return QIcon("./ressources/sigma.svg");
        }
        else if (p_icone == "Oui")
        {
            return QIcon("./ressources/check.svg");
        }
        else if (p_icone == "Non")
        {
            return QIcon("./ressources/close.svg");
        }
        else if (p_icone == "Tous")
        {
            return QIcon("./ressources/all-inclusive.svg");
        }

        

        else if (p_icone == "A" || p_icone == "a")
        {
            return QIcon("./ressources/alpha-a.svg");
        }
        else if (p_icone == "B" || p_icone == "b")
        {
            return QIcon("./ressources/alpha-b.svg");
        }
        else if (p_icone == "C" || p_icone == "c")
        {
            return QIcon("./ressources/alpha-c.svg");
        }
        else if (p_icone == "D" || p_icone == "d")
        {
            return QIcon("./ressources/alpha-d.svg");
        }
        else if (p_icone == "E" || p_icone == "e")
        {
            return QIcon("./ressources/alpha-e.svg");
        }
        else if (p_icone == "F" || p_icone == "f")
        {
            return QIcon("./ressources/alpha-f.svg");
        }
        else if (p_icone == "G" || p_icone == "g")
        {
            return QIcon("./ressources/alpha-g.svg");
        }
        else if (p_icone == "H" || p_icone == "h")
        {
            return QIcon("./ressources/alpha-h.svg");
        }
        else if (p_icone == "I" || p_icone == "i")
        {
            return QIcon("./ressources/alpha-i.svg");
        }
        else if (p_icone == "J" || p_icone == "j")
        {
            return QIcon("./ressources/alpha-j.svg");
        }
        else if (p_icone == "K" || p_icone == "k")
        {
            return QIcon("./ressources/alpha-k.svg");
        }
        else if (p_icone == "L" || p_icone == "l")
        {
            return QIcon("./ressources/alpha-l.svg");
        }
        else if (p_icone == "M" || p_icone == "m")
        {
            return QIcon("./ressources/alpha-m.svg");
        }
        else if (p_icone == "N" || p_icone == "n")
        {
            return QIcon("./ressources/alpha-n.svg");
        }
        else if (p_icone == "O" || p_icone == "o")
        {
            return QIcon("./ressources/alpha-o.svg");
        }
        else if (p_icone == "P" || p_icone == "p")
        {
            return QIcon("./ressources/alpha-p.svg");
        }
        else if (p_icone == "Q" || p_icone == "q")
        {
            return QIcon("./ressources/alpha-q.svg");
        }
        else if (p_icone == "R" || p_icone == "r")
        {
            return QIcon("./ressources/alpha-r.svg");
        }
        else if (p_icone == "S" || p_icone == "s")
        {
            return QIcon("./ressources/alpha-s.svg");
        }
        else if (p_icone == "T" || p_icone == "t")
        {
            return QIcon("./ressources/alpha-t.svg");
        }
        else if (p_icone == "U" || p_icone == "u")
        {
            return QIcon("./ressources/alpha-u.svg");
        }
        else if (p_icone == "V" || p_icone == "v")
        {
            return QIcon("./ressources/alpha-v.svg");
        }
        else if (p_icone == "W" || p_icone == "w")
        {
            return QIcon("./ressources/alpha-w.svg");
        }
        else if (p_icone == "X" || p_icone == "x")
        {
            return QIcon("./ressources/alpha-x.svg");
        }
        else if (p_icone == "Y" || p_icone == "y")
        {
            return QIcon("./ressources/alpha-y.svg");
        }
        else if (p_icone == "Z" || p_icone == "z")
        {
            return QIcon("./ressources/alpha-z.svg");
        }

        return QIcon("./ressources/help.svg");

}

void AeroDmsServices::ajouterIconesComboBox(QComboBox& p_activite)
{
    for (int i = 0; i < p_activite.count(); i++)
    {
        p_activite.setItemIcon(i, recupererIcone(p_activite.itemText(i)));
    }
}

void AeroDmsServices::normaliser(QString & p_texte)
{
    p_texte.replace(QRegularExpression("[ ']"), "");

    p_texte.replace(QRegularExpression("[éèëê]"), "e");
    p_texte.replace(QRegularExpression("[ÉÈËÊ]"), "E");

    p_texte.replace(QRegularExpression("[aâä]"), "a");
    p_texte.replace(QRegularExpression("[AÂÄ]"), "A");

    p_texte.replace(QRegularExpression("[îï]"), "i");
    p_texte.replace(QRegularExpression("[ÎÏ]"), "I");

    p_texte.replace(QRegularExpression("[ôö]"), "o");
    p_texte.replace(QRegularExpression("[ÔÖ]"), "O");

    p_texte.replace(QRegularExpression("[ùûü]"), "u");
    p_texte.replace(QRegularExpression("[ÙÛÜ]"), "U");

    p_texte.replace(QRegularExpression("[ÿ]"), "y");
    p_texte.replace(QRegularExpression("[Ÿ]"), "Y");

    p_texte.replace(QRegularExpression("[ç]"), "c");
    p_texte.replace(QRegularExpression("[Ç]"), "C");

    p_texte.replace(QRegularExpression("[æ]"), "ae");
    p_texte.replace(QRegularExpression("[Æ]"), "AE");
    p_texte.replace(QRegularExpression("[œ]"), "oe");
    p_texte.replace(QRegularExpression("[Œ]"), "OE");
}
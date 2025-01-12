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

#ifndef AERODMSSERVICES_H
#define AERODMSSERVICES_H

#include <QtWidgets>
#include "AeroDmsTypes.h"

class AeroDmsServices
{
public:
    static const QString convertirMinutesEnHeuresMinutes(const int p_minutes);
    static const QTime convertirMinutesEnQTime(const int p_minutes);
    static const QTime convertirHeuresDecimalesEnQTime(const double p_heureDecimale);

    static const QIcon recupererIcone(const QString& p_icone);
    static const QIcon recupererIcone(const QChar p_caractere);
    static const QIcon recupererIcone(const AeroDmsTypes::Icone p_icone);
    static void ajouterIconesComboBox(QComboBox &p_activite);

    static void normaliser(QString &p_texte);
    static QString capitaliserTexte(const QString& p_input);

    static QString recupererCheminFichierImageSignature();

    static QFile fichierDepuisQUrl(QUrl& p_url, 
        QString p_nomFichier);
    static void calculerTailleQResources(AeroDmsTypes::TailleFichiers& p_tailleFichiers, 
        const QString& prefix = "/AeroDms/");

    static QString mergerPdf(const QString p_cheminRepertoireFichiersAMerger,
        const QString p_cheminFichierPdfDeSortie,
        const QStringList p_listeFichiersPdfAMerger,
        const QString p_nomTresorier,
        const QString p_titreDuFichuer,
        const QString p_sujetDuFichier);

    static QString chiffrerDonnees(QString& data);
    static QString dechiffrerDonnees(QString& encryptedData);
};

#endif // AERODMSSERVICES_H

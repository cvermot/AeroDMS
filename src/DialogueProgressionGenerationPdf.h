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

#ifndef DIALOGUEPROGRESSIONGENERATIONPDF_H
#define DIALOGUEPROGRESSIONGENERATIONPDF_H
#include <QDialog>
#include <QtGui>
#include <QtWidgets>

class DialogueProgressionGenerationPdf : public QDialog
{
    Q_OBJECT

public:
    DialogueProgressionGenerationPdf(QWidget* parent = nullptr);

    void setMaximum(const int p_maximum);
    void setValue(const int p_valeur);
    void generationEstTerminee(const bool p_fichierMergeEstDisponible);

private:
    QPushButton *boutonFermer = nullptr;
    QPushButton *boutonOuvrirPdf = nullptr;
    QPushButton *boutonImprimer = nullptr;
    QPushButton* boutonImprimerAgrafage = nullptr;
    QPushButton *boutonOuvrirDossier = nullptr;
    QProgressBar *barreDeProgression = nullptr;
    QLabel* label = nullptr;

public slots:
    void demanderImpression();
    void demanderImpressionAgrafage();
    void demanderOuvrirLeDossier();

signals:
    void imprimer();
    void imprimerAgrafage();
    void ouvrirLeDossier();
};

#endif // DIALOGUEPROGRESSIONGENERATIONPDF_H

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

#ifndef DIALOGUEPROGRESSIONIMPRESSION_H
#define DIALOGUEPROGRESSIONIMPRESSION_H
#include <QDialog>
#include <QtGui>
#include <QtWidgets>

class DialogueProgressionImpression : public QDialog
{
    Q_OBJECT

public:
    DialogueProgressionImpression(QWidget* parent = nullptr);

    void setMaximumFichier(const int p_nbFichier);
    void traitementFichierSuivant();
    void traitementPageSuivante(int currentPage, int totalPages);

private:
    QPushButton* boutonFermer = nullptr;
    QProgressBar* barreDeProgressionFichier = nullptr;
    QLabel* labelFichier = nullptr;
    QProgressBar* barreDeProgressionPage = nullptr;
    QLabel* labelPage = nullptr;
    QLabel* label = nullptr;

public slots:

signals:

};

#endif // DIALOGUEPROGRESSIONIMPRESSION_H

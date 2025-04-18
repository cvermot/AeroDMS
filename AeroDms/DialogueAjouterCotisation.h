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
#ifndef DIALOGUEAJOUTERCOTISATION_H
#define DIALOGUEAJOUTERCOTISATION_H
#include <QDialog>
#include "ManageDb.h"

class DialogueAjouterCotisation : public QDialog
{
    Q_OBJECT

public:
    DialogueAjouterCotisation( ManageDb* db, 
                               const double p_montantCotisation, 
                               const double p_budgetEntrainement, 
                               QWidget* parent = nullptr);

    void mettreAJourLeContenuDeLaFenetre();
    const AeroDmsTypes::CotisationAnnuelle recupererInfosCotisationAAjouter();

    void editerLaCotisation( const QString p_pilote, 
                             const int p_annee,
                             const double p_montantSubventionDejaAlloue);

private:
    QComboBox* listePilote = nullptr;
    QComboBox* annee = nullptr;
    QDoubleSpinBox* montant = nullptr;
    QDoubleSpinBox* montantSubventionAnnuelle = nullptr;
    QLineEdit* remarque = nullptr;

    ManageDb* database = nullptr;

    QDialogButtonBox* buttonBox = nullptr;
    QPushButton* cancelButton = nullptr;
    QPushButton* okButton = nullptr;

    double montantCotisation = 0;
    double budgetEntrainement = 0;

    bool modeEdition = false;

    void peuplerListePilote();

public slots:
    void prevaliderDonnnesSaisies();
    void rincerFenetre();
};

#endif // DIALOGUEAJOUTERCOTISATION_H

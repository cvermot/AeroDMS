/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2026 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#ifndef DIALOGUEMODIFIERIMPUTATION_H
#define DIALOGUEMODIFIERIMPUTATION_H
#include <QDialog>
#include "ManageDb.h"

class DialogueModifierImputation : public QDialog
{
    Q_OBJECT

public:
    DialogueModifierImputation(ManageDb* db,
        QWidget* parent = nullptr);

    void modifierImputation(const int p_idVol);

    //void mettreAJourLeContenuDeLaFenetre();
    //const AeroDmsTypes::CotisationAnnuelle recupererInfosCotisationAAjouter();

    //void editerLaCotisation(const QString p_pilote,
    //    const int p_annee,
    //    const double p_montantSubventionDejaAlloue);

private:
    QComboBox* listeNomsImputations = nullptr;
    QLineEdit* nomImputation = nullptr;
    QLabel* nomImputationLabel = nullptr;
    QTextEdit* descriptionImputation = nullptr;

    ManageDb* database = nullptr;

    QDialogButtonBox* buttonBox = nullptr;
    QPushButton* cancelButton = nullptr;
    QPushButton* okButton = nullptr;

    int idVol = 0;

    void peuplerListeNomsImputations();

public slots:
    //void prevaliderDonnnesSaisies();
    //void rincerFenetre();
    void enregistrerLImputation();
    void mettreAJourDonnees();
    void modificationNomImputation();
};

#endif // DIALOGUEMODIFIERIMPUTATION_H


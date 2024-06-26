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
#ifndef DIALOGUEGESTIONAERONEFS_H
#define DIALOGUEGESTIONAERONEFS_H
#include <QDialog>
#include "ManageDb.h"

class DialogueGestionAeronefs : public QDialog
{
    Q_OBJECT

public:
    DialogueGestionAeronefs();
    DialogueGestionAeronefs(ManageDb* db, QWidget* parent = nullptr);

    //AeroDmsTypes::Pilote recupererInfosPilote();
    //void preparerMiseAJourPilote(const QString p_piloteId);

    void peuplerListeAeronefs();

private:
    ManageDb* database;

    QTableWidget* vueAeronefs;

    /*QLineEdit* nom;
    QLineEdit* prenom;
    QLineEdit* aeroclub;
    QCheckBox* estAyantDroit;
    QComboBox* activitePrincipale;
    QLineEdit* mail;
    QLineEdit* telephone;
    QLineEdit* remarque;
    QCheckBox* estActif;
    QCheckBox* estBrevete;

    QString idPilote;

    

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;

    void peuplerActivitePrincipale();*/

private slots:
    void sauvegarderDonneesSaisies(int p_ligne, int p_colonne);
    /*void prevaliderDonneesSaisies();
    void annulationOuFinSaisie(); */
};

#endif // DIALOGUEGESTIONAERONEFS_H
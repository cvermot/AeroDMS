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
#ifndef DIALOGUEGESTIONAEROCLUB_H
#define DIALOGUEGESTIONAEROCLUB_H
#include <QDialog>
#include "ManageDb.h"

class DialogueGestionAeroclub : public QDialog
{
    Q_OBJECT

public:
    DialogueGestionAeroclub(ManageDb* db, QWidget* parent = nullptr);

    const AeroDmsTypes::Club recupererInfosClub();
    void peuplerListeAeroclub();
    void peuplerListeAerodrome();
    void ouvrirFenetre(const bool p_modeEdition);
    //void preparerMiseAJourClub(const int p_aerolcubId);

private:
    QComboBox* selectionAeroclub = nullptr;
    QComboBox* selectionAerodrome = nullptr;
    QLabel* selectionAeroclubLabel = nullptr;
    QLineEdit* nomAeroclub = nullptr;
    QLineEdit* raisonSociale = nullptr;
    QLineEdit* iban = nullptr;
    QLineEdit* bic = nullptr;

    int idAeroclub = -1;

    ManageDb* database = nullptr;

    QDialogButtonBox* buttonBox = nullptr;
    QPushButton* cancelButton = nullptr;
    QPushButton* okButton = nullptr;

    bool validerIbanFrancais(const QString& p_iban);

private slots:
    void prevaliderDonneesSaisies();
    void annulationOuFinSaisie();
    void chargerDonneesAeroclub();
    void validerIban();
};

#endif // DIALOGUEGESTIONAEROCLUB_H

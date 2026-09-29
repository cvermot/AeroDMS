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
#ifndef DIALOGUEGESTIONTYPESAERONEFS_H
#define DIALOGUEGESTIONTYPESAERONEFS_H
#include <QDialog>
#include "ManageDb.h"

class DialogueGestionTypesAeronefs : public QDialog
{
    Q_OBJECT

public:
    DialogueGestionTypesAeronefs(ManageDb* db,
        QWidget* parent = nullptr);

    void peuplerListeTypesAeronefs();

private:
    ManageDb* database = nullptr;

    QTableWidget* vueTypesAeronefs = nullptr;
    QVector<QComboBox*> *listeUniteTypeConso = nullptr;
    QVector<QComboBox*> *listeTypeDecompte = nullptr;
    QVector<QDoubleSpinBox*> *listeConsommation = nullptr;

private slots:
    void changementConsommation();
    void changementUnite();
    void changementRecurrence();
    void changementDonneesTexte(const int p_ligne,
        const int p_colonne);
};

#endif // DIALOGUEGESTIONTYPESAERONEFS_H

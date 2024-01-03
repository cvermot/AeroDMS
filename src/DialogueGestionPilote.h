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
#ifndef DIALOGUEGESTIONPILOTE_H
#define DIALOGUEGESTIONPILOTE_H
#include <QDialog>
#include "ManageDb.h"

class DialogueGestionPilote : public QDialog
{
    Q_OBJECT

public:
    DialogueGestionPilote();
    DialogueGestionPilote(ManageDb* db, QWidget* parent = nullptr);

    AeroDmsTypes::Pilote recupererInfosPilote();

private:
    QLineEdit *nom;
    QLineEdit* prenom;
    QLineEdit* aeroclub;
    QCheckBox *estAyantDroit;
    QLineEdit* mail;
    QLineEdit* telephone;
    QLineEdit* remarque;

    QString idPilote;

    ManageDb* database;

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;
    //QWidget* extension;

private slots:
    void prevaliderDonneesSaisies();
};

#endif // DIALOGUEGESTIONPILOTE_H


/******************************************************************************\
<QUas : a Free Software logbook for UAS operators>
Copyright (C) 2023 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#include "DialogueGestionPilote.h"

#include <QtWidgets>

DialogueGestionPilote::DialogueGestionPilote()
{

}

DialogueGestionPilote::DialogueGestionPilote(ManageDb* db, QWidget* parent) : QDialog(parent)
{
    database = db;

    cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    okButton = new QPushButton(tr("&Ajouter"), this);
    okButton->setDisabled(true);
    okButton->setDefault(true);
    okButton->setToolTip("Remplir à minima les champs nom, prénom et aéroclub");
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    nom = new QLineEdit(this);
    QLabel* nomLabel = new QLabel(tr("Nom : "), this);
    connect(nom, &QLineEdit::textChanged, this, &DialogueGestionPilote::prevaliderDonneesSaisies);

    prenom = new QLineEdit(this);
    QLabel* prenomLabel = new QLabel(tr("Prénom : "), this);
    connect(prenom, &QLineEdit::textChanged, this, &DialogueGestionPilote::prevaliderDonneesSaisies);

    aeroclub = new QLineEdit(this);
    QLabel* aeroclubLabel = new QLabel(tr("Aéroclub : "), this);
    connect(aeroclub, &QLineEdit::textChanged, this, &DialogueGestionPilote::prevaliderDonneesSaisies);

    estAyantDroit = new QCheckBox(this);
    QLabel* estAyantDroitLabel = new QLabel(tr("Ayant droit : "), this);

    mail = new QLineEdit(this);
    QLabel* mailLabel = new QLabel(tr("Mail : "), this);

    telephone = new QLineEdit(this);
    QLabel* telephoneLabel = new QLabel(tr("Téléphone : "), this);

    remarque = new QLineEdit(this);
    QLabel* remarqueLabel = new QLabel(tr("Remarque : "), this);

    idPilote = "";


    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(nomLabel, 0, 0);
    mainLayout->addWidget(nom, 0, 1);

    mainLayout->addWidget(prenomLabel, 1, 0);
    mainLayout->addWidget(prenom, 1, 1);

    mainLayout->addWidget(aeroclubLabel, 2, 0);
    mainLayout->addWidget(aeroclub, 2, 1);

    mainLayout->addWidget(estAyantDroitLabel, 3, 0);
    mainLayout->addWidget(estAyantDroit, 3, 1);

    mainLayout->addWidget(mailLabel, 4, 0);
    mainLayout->addWidget(mail, 4, 1);

    mainLayout->addWidget(telephoneLabel, 5, 0);
    mainLayout->addWidget(telephone, 5, 1);

    mainLayout->addWidget(remarqueLabel, 6, 0);
    mainLayout->addWidget(remarque, 6, 1);


    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

   // mainLayout->setRowStretch(2, 1);

    setLayout(mainLayout);

    setWindowTitle(tr("Ajouter un pilote"));
}

AeroDmsTypes::Pilote DialogueGestionPilote::recupererInfosPilote()
{
    AeroDmsTypes::Pilote pilote;

    pilote.idPilote = idPilote;
    pilote.nom = nom->text();
    pilote.prenom = prenom->text();
    pilote.estAyantDroit = estAyantDroit->checkState() == Qt::Checked ;
    pilote.aeroclub = aeroclub->text();
    pilote.mail = mail->text();
    pilote.telephone = telephone->text();
    pilote.remarque = remarque->text();

    //On rince en vue de l'eventuel ajout du pilote suivant
    idPilote = "";
    nom->clear();
    prenom->clear();
    estAyantDroit->setChecked(Qt::Unchecked);
    aeroclub->clear();
    mail->clear();
    telephone->clear();
    remarque->clear();

    return pilote;
}

void DialogueGestionPilote::prevaliderDonneesSaisies()
{
    okButton->setDisabled(false);

    if (nom->text() == ""
        || prenom->text() == ""
        || aeroclub->text() == "")
    {
        okButton->setDisabled(true);
    }
}
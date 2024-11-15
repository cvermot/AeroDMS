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
#include "DialogueAjouterSortie.h"

#include <QtWidgets>

DialogueAjouterSortie::DialogueAjouterSortie()
{
    
}

DialogueAjouterSortie::DialogueAjouterSortie(QWidget* parent) : QDialog(parent)
{
    cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    okButton = new QPushButton(tr("&Ajouter"), this);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    nomSortie = new QLineEdit(this);
    QLabel* nomSortieLabel = new QLabel(tr("Nom de la sortie : "), this);
    connect(nomSortie, &QLineEdit::textChanged, this, &DialogueAjouterSortie::prevaliderDonneesSaisies);

    date = new QDateEdit(this);
    date->setDisplayFormat("dd/MM/yyyy");
    date->setCalendarPopup(true);
    date->setDate(QDate::currentDate());
    QLabel* dateLabel = new QLabel(tr("Date : "), this);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(nomSortieLabel, 0, 0);
    mainLayout->addWidget(nomSortie, 0, 1);

    mainLayout->addWidget(dateLabel, 1, 0);
    mainLayout->addWidget(date, 1, 1);

    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

    setLayout(mainLayout);

    setWindowTitle(QApplication::applicationName() + " - " + tr("Ajouter une sortie"));
    prevaliderDonneesSaisies();
}

AeroDmsTypes::Sortie DialogueAjouterSortie::recupererInfosSortieAAjouter()
{
    AeroDmsTypes::Sortie sortie;

    sortie.nom = nomSortie->text();
    sortie.date = date->date();

    nomSortie->clear();

    return sortie;
}

void DialogueAjouterSortie::prevaliderDonneesSaisies()
{
    okButton->setDisabled(false);

    if (nomSortie->text() == "")
    {
        okButton->setDisabled(true);
    }
}
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
#include "DialogueAjouterCotisation.h"

#include <QtWidgets>

DialogueAjouterCotisation::DialogueAjouterCotisation()
{
}

DialogueAjouterCotisation::DialogueAjouterCotisation( ManageDb* db, 
                                                      const float p_montantCotisation, 
                                                      const float p_budgetEntrainement, 
                                                      QWidget* parent) : QDialog(parent)
{
    database = db;
    montantCotisation = p_montantCotisation;
    budgetEntrainement = p_budgetEntrainement;

    cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    okButton = new QPushButton(tr("&Ajouter"), this);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    listePilote = new QComboBox(this);
    QLabel* listePiloteLabel = new QLabel(tr("Pilote : "), this);
    connect(listePilote, &QComboBox::currentIndexChanged, this, &DialogueAjouterCotisation::prevaliderDonnnesSaisies);

    annee = new QComboBox(this);
    QLabel* anneeLabel = new QLabel(tr("Année : "), this);
    const int anneeCourante = QDate::currentDate().year();
    annee->addItem(QString::number(anneeCourante - 1));
    annee->addItem(QString::number(anneeCourante));
    annee->addItem(QString::number(anneeCourante + 1));
    annee->setCurrentIndex(1);
    annee->setItemIcon(0, QIcon("./ressources/numeric-negative-1.svg"));
    annee->setItemIcon(1, QIcon("./ressources/numeric-0.svg"));
    annee->setItemIcon(2, QIcon("./ressources/numeric-positive-1.svg"));

    montant = new QDoubleSpinBox(this);
    QLabel* montantLabel = new QLabel(tr("Montant : "), this);
    montant->setRange(0.0, 2000.0);
    montant->setSingleStep(1.0);
    montant->setDecimals(2);
    montant->setValue(montantCotisation);
    montant->setSuffix("€");

    montantSubventionAnnuelle = new QDoubleSpinBox(this);
    QLabel* montantSubventionAnnuelleLabel = new QLabel(tr("Montant de la subvention annuelle : "), this);
    montantSubventionAnnuelle->setRange(0.0, 2000.0);
    montantSubventionAnnuelle->setSingleStep(1.0);
    montantSubventionAnnuelle->setDecimals(2);
    montantSubventionAnnuelle->setValue(budgetEntrainement);
    montantSubventionAnnuelle->setSuffix("€");

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(listePiloteLabel, 0, 0);
    mainLayout->addWidget(listePilote, 0, 1);

    mainLayout->addWidget(anneeLabel, 1, 0);
    mainLayout->addWidget(annee, 1, 1);

    mainLayout->addWidget(montantLabel, 2, 0);
    mainLayout->addWidget(montant, 2, 1);

    mainLayout->addWidget(montantSubventionAnnuelleLabel, 3, 0);
    mainLayout->addWidget(montantSubventionAnnuelle, 3, 1);

    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

    setLayout(mainLayout);

    setWindowTitle(tr("Ajouter une cotisation pilote"));

    peuplerListePilote();
    prevaliderDonnnesSaisies();
}

void DialogueAjouterCotisation::peuplerListePilote()
{
    listePilote->clear();

    const AeroDmsTypes::ListePilotes listePilotes = database->recupererPilotes();

    listePilote->addItem("", "");
    for (int i = 0; i < listePilotes.size(); i++)
    {
        const AeroDmsTypes::Pilote pilote = listePilotes.at(i);
        listePilote->addItem(QString(pilote.prenom).append(" ").append(pilote.nom), listePilotes.at(i).idPilote);
    }
}

void DialogueAjouterCotisation::mettreAJourLeContenuDeLaFenetre()
{
    peuplerListePilote();
}

AeroDmsTypes::CotisationAnnuelle DialogueAjouterCotisation::recupererInfosCotisationAAjouter()
{
    AeroDmsTypes::CotisationAnnuelle infosCotisation;

    infosCotisation.idPilote = listePilote->currentData().toString();
    infosCotisation.annee = annee->currentText().toInt();
    infosCotisation.montant = montant->value();
    infosCotisation.montantSubvention = montantSubventionAnnuelle->value();

    listePilote->setCurrentIndex(0);
    annee->setCurrentIndex(1);
    montant->setValue(montantCotisation);
    montantSubventionAnnuelle->setValue(budgetEntrainement);

    return infosCotisation;
}

void DialogueAjouterCotisation::prevaliderDonnnesSaisies()
{
    okButton->setEnabled(true);

    if (listePilote->currentIndex() == 0)
    {
        okButton->setEnabled(false);
    }
}
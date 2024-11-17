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
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(rincerFenetre()));
    connect(this, SIGNAL(rejected()), this, SLOT(rincerFenetre()));

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
    annee->setItemIcon(0, AeroDmsServices::recupererIcone(AeroDmsServices::Icone_MOINS_1));
    annee->setItemIcon(1, AeroDmsServices::recupererIcone(AeroDmsServices::Icone_0));
    annee->setItemIcon(2, AeroDmsServices::recupererIcone(AeroDmsServices::Icone_1));

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

    rincerFenetre();

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
        AeroDmsTypes::Pilote pilote = listePilotes.at(i);
        AeroDmsServices::normaliser(pilote.prenom);
        listePilote->addItem( AeroDmsServices::recupererIcone(pilote.prenom.at(0)),
                              QString(pilote.prenom).append(" ").append(pilote.nom),
                              listePilotes.at(i).idPilote);
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
    infosCotisation.estEnEdition = modeEdition;

    rincerFenetre();

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

void DialogueAjouterCotisation::editerLaCotisation( const QString p_pilote,
                                                    const int p_annee, 
                                                    const float p_montantSubventionDejaAlloue)
{
    int index = listePilote->findData(p_pilote);
    if (index != -1) {
        listePilote->setCurrentIndex(index);
    }

    listePilote->setEnabled(false);
    annee->addItem(QString::number(p_annee));
    annee->setItemIcon(3, QIcon("./ressources/pencil.svg"));
    annee->setCurrentIndex(3);
    annee->setEnabled(false);
    montant->setEnabled(false);
    montantSubventionAnnuelle->setMinimum(p_montantSubventionDejaAlloue);
    montantSubventionAnnuelle->setValue(database->recupererSubventionEntrainement(p_pilote, p_annee));

    setWindowTitle(QApplication::applicationName() + " - " + tr("Modifier une cotisation pilote"));
    okButton->setText("&Modifier");

    modeEdition = true;

}

void DialogueAjouterCotisation::rincerFenetre()
{
    //On remet aux valeur par défaut les champs changées en cas d'édition :
    listePilote->setEnabled(true);
    annee->setEnabled(true);
    montant->setEnabled(true);
    montantSubventionAnnuelle->setMinimum(0.0);
    modeEdition = false;
    okButton->setText("&Ajouter");

    setWindowTitle(QApplication::applicationName() + " - " + tr("Ajouter une cotisation pilote"));

    //On rince les autres valeurs
    listePilote->setCurrentIndex(0);
    annee->setCurrentIndex(1);
    montant->setValue(montantCotisation);
    montantSubventionAnnuelle->setValue(budgetEntrainement);

    //Si on a 4 item c'est qu'on est en mode édition => on supprime l'item additionnel ajouté par simplicité en mode édition
    if (annee->count() == 4)
    {
        annee->removeItem(3);
    }
}

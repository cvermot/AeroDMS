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
#include "DialogueModifierImputation.h"
#include "AeroDmsServices.h"

#include <QtWidgets>

DialogueModifierImputation::DialogueModifierImputation(ManageDb* db,
    QWidget* parent) : QDialog(parent)
{
    database = db;

    QGridLayout* mainLayout = new QGridLayout(this);
    resize(600, 220);

    cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    okButton = new QPushButton(tr("&Valider"), this);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(enregistrerLImputation()));

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    listeNomsImputations = new QComboBox(this);
    QLabel* listeNomsImputationsLabel = new QLabel(tr("Imputation : "), this);
    connect(listeNomsImputations, &QComboBox::currentIndexChanged, this, &DialogueModifierImputation::mettreAJourDonnees);

    nomImputation = new QLineEdit(this);
    nomImputationLabel = new QLabel(tr("Nom de l'imputation : "), this);
    connect(nomImputation, &QLineEdit::textChanged, this, &DialogueModifierImputation::modificationNomImputation);

    descriptionImputation = new QTextEdit(this);
    QLabel* descriptionImputationLabel = new QLabel(tr("Description de l'imputation : "), this);

    mainLayout->addWidget(listeNomsImputationsLabel, 0, 0);
    mainLayout->addWidget(listeNomsImputations, 0, 1);

    mainLayout->addWidget(nomImputationLabel, 3, 0);
    mainLayout->addWidget(nomImputation, 3, 1);

    mainLayout->addWidget(descriptionImputationLabel, 4, 0);
    mainLayout->addWidget(descriptionImputation, 4, 1);

    mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

    setLayout(mainLayout);
}

void DialogueModifierImputation::modifierImputation(const int p_idVol)
{
    idVol = p_idVol;

    peuplerListeNomsImputations();

    //On récupère l'imputation du vol
    const AeroDmsTypes::Imputation imputation = database->recupererDetailsImputationVol(idVol);

    //On sélectionne la bonne imputation dans la liste déroulante
    int nItem = 0;
    while (nItem < listeNomsImputations->count()
           && listeNomsImputations->itemData(nItem).value<AeroDmsTypes::Imputation>().id != imputation.id)
    {
        nItem++;
    }
    listeNomsImputations->setCurrentIndex(nItem);
    nomImputation->setText(imputation.nom);
    descriptionImputation->setText(imputation.description);

    nomImputation->setHidden(true);
    nomImputationLabel->setHidden(true);
}

void DialogueModifierImputation::mettreAJourDonnees()
{
    const AeroDmsTypes::Imputation imputation = listeNomsImputations->currentData().value<AeroDmsTypes::Imputation>();

    if (imputation.id == AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        nomImputation->setHidden(false);
        nomImputationLabel->setHidden(false);
        okButton->setEnabled(false);
    }
    else
    {
        nomImputation->setHidden(true);
        nomImputationLabel->setHidden(true);
        okButton->setEnabled(true);
    }
    nomImputation->setText(imputation.nom);
    descriptionImputation->setText(imputation.description);
}

void DialogueModifierImputation::peuplerListeNomsImputations()
{
    const AeroDmsTypes::ListeImputations liste = database->recupereListeImputations();

    listeNomsImputations->clear();

    //On met la première ligne en icone d'imputation section, ensuite on écrasera dans la boucle pour mettre la seconde icone.
    QIcon icone = AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_IMPUTATION_SECTION);

    for (AeroDmsTypes::Imputation imputation : liste)
    {
        QVariant donnee;
        donnee.setValue(imputation);
        listeNomsImputations->addItem(icone, imputation.nom, donnee);
        icone = AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_IMPUTATION_AUTRE_SECTION);
    }
    
    QVariant donnee;
    donnee.setValue(AeroDmsTypes::K_INIT_IMPUTATION);
    listeNomsImputations->addItem(AeroDmsServices::recupererIcone(AeroDmsTypes::Icone_NOUVELLE_IMPUTATION), "Nouvelle imputation...", donnee);
}

void DialogueModifierImputation::enregistrerLImputation()
{
    AeroDmsTypes::Imputation imputation;
    imputation.id = listeNomsImputations->currentData().value<AeroDmsTypes::Imputation>().id;
    imputation.nom = nomImputation->text();
    imputation.description = descriptionImputation->toHtml();
    database->enregistrerImputation(imputation, idVol);
    accept();
}

void DialogueModifierImputation::modificationNomImputation()
{
    okButton->setEnabled(nomImputation->text() != "");
}

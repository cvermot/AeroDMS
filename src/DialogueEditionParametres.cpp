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

#include "DialogueEditionParametres.h"

#include <QtWidgets>
#include <QPrintDialog>
#include <QPrinter>

DialogueEditionParametres::DialogueEditionParametres()
{

}

DialogueEditionParametres::DialogueEditionParametres(const AeroDmsTypes::ParametresMetier p_parametresMetiers,
    const AeroDmsTypes::ParametresSysteme p_parametresSysteme,
    QWidget* parent) : QDialog(parent)
{
    QGridLayout* mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    onglets = new QTabWidget(this);
    mainLayout->addWidget(onglets, 0, 0, 10, 2);

    //Mailing
    QGridLayout* mailingLayout = new QGridLayout();
    QWidget* mailingWidget = new QWidget(this);
    mailingWidget->setLayout(mailingLayout);
    onglets->addTab(mailingWidget, QIcon("./ressources/email-multiple.svg"), "Mailing");

    int ligneActuelle = mailingLayout->rowCount();
    texteChequeDispo = new QTextEdit(this);
    QLabel* texteChequeDispoLabel = new QLabel(tr("Chèques disponibles : "), this);
    mailingLayout->addWidget(texteChequeDispoLabel, ligneActuelle, 0);
    mailingLayout->addWidget(texteChequeDispo, ligneActuelle, 1);

    texteChequeDispo->setText(p_parametresMetiers.texteMailDispoCheques);

    ligneActuelle = mailingLayout->rowCount();
    texteSubventionRestantes = new QTextEdit(this);
    QLabel* texteSubventionRestantesLabel = new QLabel(tr("Subvention restante : "), this);
    mailingLayout->addWidget(texteSubventionRestantesLabel, ligneActuelle, 0);
    mailingLayout->addWidget(texteSubventionRestantes, ligneActuelle, 1);

    texteSubventionRestantes->setText(p_parametresMetiers.texteMailSubventionRestante);

    //Parametres système
    QGridLayout* systemeLayout = new QGridLayout();
    QWidget* systemeWidget = new QWidget(this);
    systemeWidget->setLayout(systemeLayout);
    onglets->addTab(systemeWidget, QIcon("./ressources/database-cog.svg"), "Parametres système");

    cheminBdd = new QLineEdit(this);
    QLabel* cheminBddLabel = new QLabel(tr("Dossier BDD : "), this);
    systemeLayout->addWidget(cheminBddLabel, ligneActuelle, 0);
    systemeLayout->addWidget(cheminBdd, ligneActuelle, 1);
    ligneActuelle = systemeLayout->rowCount();
    nomBdd = new QLineEdit(this);
    QLabel* nomBddLabel = new QLabel(tr("Fichier BDD : "), this);
    systemeLayout->addWidget(nomBddLabel, ligneActuelle, 0);
    systemeLayout->addWidget(nomBdd, ligneActuelle, 1);
    QPushButton* boutonSelectionBdd = new QPushButton("Séléctionner", this);
    systemeLayout->addWidget(boutonSelectionBdd, ligneActuelle, 2, 1, 2);

    cheminBdd->setText(p_parametresSysteme.cheminStockageBdd);
    nomBdd->setText(p_parametresSysteme.nomBdd);

    ligneActuelle = systemeLayout->rowCount();
    delaisGardeBdd = new QSpinBox(this);
    QLabel* delaisGardeBddLabel = new QLabel(tr("Délais de garde BDD : "), this);
    systemeLayout->addWidget(delaisGardeBddLabel, ligneActuelle, 0);
    systemeLayout->addWidget(delaisGardeBdd, ligneActuelle, 1);
    QLabel* delaisGardeBddLabelUnite = new QLabel(tr("ms"), this);
    systemeLayout->addWidget(delaisGardeBddLabelUnite, ligneActuelle, 2);

    delaisGardeBdd->setValue(p_parametresMetiers.delaisDeGardeBdd);

    ligneActuelle = systemeLayout->rowCount();
    factureATraiter = new QLineEdit(this);
    QLabel* factureATraiterLabel = new QLabel(tr("Dossier d'entrée des factures : "), this);
    systemeLayout->addWidget(factureATraiterLabel, ligneActuelle, 0);
    systemeLayout->addWidget(factureATraiter, ligneActuelle, 1);
    QPushButton* boutonSelectionFactureATraiter = new QPushButton("Séléctionner", this);
    systemeLayout->addWidget(boutonSelectionFactureATraiter, ligneActuelle, 2);

    factureATraiter->setText(p_parametresSysteme.cheminStockageFacturesATraiter);

    ligneActuelle = systemeLayout->rowCount();
    facturesSaisies = new QLineEdit(this);
    QLabel* facturesSaisiesLabel = new QLabel(tr("Dossier d'enregistrement des factures : "), this);
    systemeLayout->addWidget(facturesSaisiesLabel, ligneActuelle, 0);
    systemeLayout->addWidget(facturesSaisies, ligneActuelle, 1);
    QPushButton* boutonSelectionFacturesSaisies = new QPushButton("Séléctionner", this);
    systemeLayout->addWidget(boutonSelectionFacturesSaisies, ligneActuelle, 2);

    facturesSaisies->setText(p_parametresSysteme.cheminStockageFacturesTraitees);

    ligneActuelle = systemeLayout->rowCount();
    sortieFichiersGeneres = new QLineEdit(this);
    QLabel* sortieFichiersGeneresLabel = new QLabel(tr("Dossier de sortie des fichiers générés : "), this);
    systemeLayout->addWidget(sortieFichiersGeneresLabel, ligneActuelle, 0);
    systemeLayout->addWidget(sortieFichiersGeneres, ligneActuelle, 1);
    QPushButton* boutonSelectionSortieFichiersGeneres = new QPushButton("Séléctionner", this);
    systemeLayout->addWidget(boutonSelectionSortieFichiersGeneres, ligneActuelle, 2);

    sortieFichiersGeneres->setText(p_parametresSysteme.cheminSortieFichiersGeneres);

    //Elements financiers
    QGridLayout* financeLayout = new QGridLayout();
    QWidget* financeWidget = new QWidget(this);
    financeWidget->setLayout(financeLayout);
    onglets->addTab(financeWidget, QIcon("./ressources/cash-multiple.svg"), "Parametres financiers");

    ligneActuelle = financeLayout->rowCount();
    montantCotisationPilote = new QDoubleSpinBox(this);
    QLabel* montantCotisationPiloteLabel = new QLabel(tr("Montant cotisation pilote : "), this);
    financeLayout->addWidget(montantCotisationPiloteLabel, ligneActuelle, 0);
    financeLayout->addWidget(montantCotisationPilote, ligneActuelle, 1);
    QLabel* montantCotisationPiloteUnite = new QLabel(tr("€"), this);
    financeLayout->addWidget(montantCotisationPiloteUnite, ligneActuelle, 2);

    montantCotisationPilote->setValue(p_parametresMetiers.montantCotisationPilote);

    ligneActuelle = financeLayout->rowCount();
    montantSubventionEntrainement = new QDoubleSpinBox(this);
    montantSubventionEntrainement->setMaximum(10000);
    QLabel* montantSubventionEntrainementLabel = new QLabel(tr("Subvention annuelle entrainement : "), this);
    financeLayout->addWidget(montantSubventionEntrainementLabel, ligneActuelle, 0);
    financeLayout->addWidget(montantSubventionEntrainement, ligneActuelle, 1);
    QLabel* montantSubventionEntrainementUnite = new QLabel(tr("€"), this);
    financeLayout->addWidget(montantSubventionEntrainementUnite, ligneActuelle, 2);

    montantSubventionEntrainement->setValue(p_parametresMetiers.montantSubventionEntrainement);

    ligneActuelle = financeLayout->rowCount();
    plafondHoraireRemboursementEntrainement = new QDoubleSpinBox(this);
    plafondHoraireRemboursementEntrainement->setMaximum(10000);
    QLabel* plafondHoraireRemboursementEntrainementLabel = new QLabel(tr("Plafond coût horaire subvention entrainement : "), this);
    financeLayout->addWidget(plafondHoraireRemboursementEntrainementLabel, ligneActuelle, 0);
    financeLayout->addWidget(plafondHoraireRemboursementEntrainement, ligneActuelle, 1);
    QLabel* plafondHoraireRemboursementEntrainementUnite = new QLabel(tr("€"), this);
    financeLayout->addWidget(plafondHoraireRemboursementEntrainementUnite, ligneActuelle, 2);

    plafondHoraireRemboursementEntrainement->setValue(p_parametresMetiers.plafondHoraireRemboursementEntrainement);

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementEntrainement = new QDoubleSpinBox(this);
    proportionRemboursementEntrainement->setMaximum(100);
    QLabel* proportionRemboursementEntrainementLabel = new QLabel(tr("Proportion rembousement entrainement : "), this);
    financeLayout->addWidget(proportionRemboursementEntrainementLabel, ligneActuelle, 0);
    financeLayout->addWidget(proportionRemboursementEntrainement, ligneActuelle, 1);
    QLabel* proportionRemboursementEntrainementUnite = new QLabel(tr("%"), this);
    financeLayout->addWidget(proportionRemboursementEntrainementUnite, ligneActuelle, 2);

    proportionRemboursementEntrainement->setValue(p_parametresMetiers.proportionRemboursementEntrainement*100);  

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementBalade = new QDoubleSpinBox(this);
    proportionRemboursementBalade->setMaximum(100);
    QLabel* proportionRemboursementBaladeLabel = new QLabel(tr("Proportion rembousement balades : "), this);
    financeLayout->addWidget(proportionRemboursementBaladeLabel, ligneActuelle, 0);
    financeLayout->addWidget(proportionRemboursementBalade, ligneActuelle, 1);
    QLabel* proportionRemboursementBaladeUnite = new QLabel(tr("%"), this);
    financeLayout->addWidget(proportionRemboursementBaladeUnite, ligneActuelle, 2);

    proportionRemboursementBalade->setValue(p_parametresMetiers.proportionRemboursementBalade *100);

    ligneActuelle = financeLayout->rowCount();
    proportionParticipationBalade = new QDoubleSpinBox(this);
    proportionParticipationBalade->setMaximum(100);
    QLabel* proportionParticipationBaladeLabel = new QLabel(tr("Proportion rembousement balades : "), this);
    financeLayout->addWidget(proportionParticipationBaladeLabel, ligneActuelle, 0);
    financeLayout->addWidget(proportionParticipationBalade, ligneActuelle, 1);
    QLabel* proportionParticipationBaladeUnite = new QLabel(tr("%"), this);
    financeLayout->addWidget(proportionParticipationBaladeUnite, ligneActuelle, 2);

    proportionParticipationBalade->setValue(p_parametresMetiers.proportionParticipationBalade * 100);

    ligneActuelle = financeLayout->rowCount();
    nomTresorier = new QLineEdit(this);
    QLabel* nomTresorierLabel = new QLabel(tr("Nom du trésorier : "), this);
    financeLayout->addWidget(nomTresorierLabel, ligneActuelle, 0);
    financeLayout->addWidget(nomTresorier, ligneActuelle, 1, 1, 2);

    nomTresorier->setText(p_parametresMetiers.nomTresorier);

    //Elements impression
    QGridLayout* impressionLayout = new QGridLayout();
    QWidget* impressionWidget = new QWidget(this);
    impressionWidget->setLayout(impressionLayout);
    onglets->addTab(impressionWidget, QIcon("./ressources/printer-pos-cog.svg"), "Parametres d'impression");

    ligneActuelle = impressionLayout->rowCount();
    imprimante = new QLineEdit(this);
    QLabel* imprimanteLabel = new QLabel(tr("Imprimante : "), this);
    impressionLayout->addWidget(imprimanteLabel, ligneActuelle, 0);
    impressionLayout->addWidget(imprimante, ligneActuelle, 1);
    QPushButton* boutonSelectionImprimante = new QPushButton("Séléctionner l'imprimante", this);
    impressionLayout->addWidget(boutonSelectionImprimante, ligneActuelle, 2);
    connect(boutonSelectionImprimante, SIGNAL(clicked()), this, SLOT(selectionnerImprimante()));

    imprimante->setText(p_parametresSysteme.imprimante);

    ligneActuelle = impressionLayout->rowCount();
    impressionCouleur = new QComboBox(this);
    QLabel* impressionCouleurLabel = new QLabel(tr("Couleur : "), this);
    impressionLayout->addWidget(impressionCouleurLabel, ligneActuelle, 0);
    impressionLayout->addWidget(impressionCouleur, ligneActuelle, 1);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors-off.svg"), tr("Noir et blanc"), QPrinter::GrayScale);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors.svg"), tr("Couleur"), QPrinter::Color);

    impressionCouleur->setCurrentIndex(p_parametresSysteme.modeCouleurImpression);

    QPushButton *cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QPushButton* okButton = new QPushButton(tr("&Enregistrer"), this);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(enregistrerParametres()));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);

    mainLayout->addWidget(buttonBox, mainLayout->rowCount(), 0, 1, 2);

    connect(onglets, &QTabWidget::currentChanged, this, &DialogueEditionParametres::gererChangementOnglet);
}

void DialogueEditionParametres::selectionnerImprimante()
{
    QPrinter printer;

    QPrintDialog dialog(&printer, this);

    dialog.setOption(QAbstractPrintDialog::PrintCollateCopies, false);
    dialog.setOption(QAbstractPrintDialog::PrintCurrentPage, false);
    dialog.setOption(QAbstractPrintDialog::PrintPageRange, false);
    dialog.setOption(QAbstractPrintDialog::PrintSelection, false);
    dialog.setOption(QAbstractPrintDialog::PrintShowPageSize, false);
    dialog.setOption(QAbstractPrintDialog::PrintToFile, false);

    if (dialog.exec() == QDialog::Accepted)
    {
        imprimante->setText(printer.printerName());
    }
}

void DialogueEditionParametres::gererChangementOnglet()
{
    //qDebug() << onglets->widget(onglets->currentIndex())->sizeHint();
    //onglets->setFixedHeight(onglets->widget(onglets->currentIndex())->sizeHint().height());
    //resize(onglets->sizeHint());
}

void DialogueEditionParametres::enregistrerParametres()
{
    AeroDmsTypes::ParametresMetier parametresMetiers;
    
    parametresMetiers.montantSubventionEntrainement = montantSubventionEntrainement->value();
    parametresMetiers.montantCotisationPilote = montantCotisationPilote->value();
    parametresMetiers.proportionRemboursementEntrainement = proportionRemboursementEntrainement->value()/100;
    parametresMetiers.proportionRemboursementBalade = proportionRemboursementBalade->value()/100;
    parametresMetiers.plafondHoraireRemboursementEntrainement = plafondHoraireRemboursementEntrainement->value();
    parametresMetiers.proportionParticipationBalade = proportionParticipationBalade->value()/100;
    parametresMetiers.nomTresorier = nomTresorier->text();
    parametresMetiers.delaisDeGardeBdd = delaisGardeBdd->value();
    parametresMetiers.texteMailDispoCheques = texteChequeDispo->toPlainText();
    parametresMetiers.texteMailSubventionRestante = texteSubventionRestantes->toPlainText();
    
    AeroDmsTypes::ParametresSysteme parametresSysteme;
    parametresSysteme.cheminStockageBdd = cheminBdd->text();
    parametresSysteme.cheminStockageFacturesTraitees = facturesSaisies->text();
    parametresSysteme.cheminStockageFacturesATraiter = factureATraiter->text();
    parametresSysteme.cheminSortieFichiersGeneres = sortieFichiersGeneres->text();
    parametresSysteme.nomBdd = nomBdd->text();
    parametresSysteme.imprimante = imprimante->text();
    parametresSysteme.modeCouleurImpression = static_cast<QPrinter::ColorMode>(impressionCouleur->currentData().toInt());

    emit envoyerParametres(parametresMetiers, parametresSysteme);
    accept();
}


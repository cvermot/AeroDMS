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
    const bool p_editionParametresCritiques,
    QWidget* parent) : QDialog(parent)
{
    const int K_COLONNE_LABEL = 0;
    const int K_COLONNE_CHAMP = 1;
    const int K_COLONNE_UNITE_BOUTON = 2;

    setWindowTitle(tr("AeroDms - Paramètres"));

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
    mailingLayout->addWidget(new QLabel(tr("Chèques disponibles : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(texteChequeDispo, ligneActuelle, K_COLONNE_CHAMP);

    texteChequeDispo->setText(p_parametresMetiers.texteMailDispoCheques);

    ligneActuelle = mailingLayout->rowCount();
    texteSubventionRestantes = new QTextEdit(this);
    mailingLayout->addWidget(new QLabel(tr("Subvention restante : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(texteSubventionRestantes, ligneActuelle, K_COLONNE_CHAMP);

    texteSubventionRestantes->setText(p_parametresMetiers.texteMailSubventionRestante);

    //Elements financiers
    QGridLayout* financeLayout = new QGridLayout();
    QWidget* financeWidget = new QWidget(this);
    financeWidget->setLayout(financeLayout);
    onglets->addTab(financeWidget, QIcon("./ressources/cash-multiple.svg"), "Financiers");

    ligneActuelle = financeLayout->rowCount();
    montantCotisationPilote = new QDoubleSpinBox(this);
    financeLayout->addWidget(new QLabel(tr("Montant cotisation pilote : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(montantCotisationPilote, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("€"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    montantCotisationPilote->setValue(p_parametresMetiers.montantCotisationPilote);

    ligneActuelle = financeLayout->rowCount();
    montantSubventionEntrainement = new QDoubleSpinBox(this);
    montantSubventionEntrainement->setMaximum(10000);
    financeLayout->addWidget(new QLabel(tr("Subvention annuelle entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(montantSubventionEntrainement, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("€"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    montantSubventionEntrainement->setValue(p_parametresMetiers.montantSubventionEntrainement);

    ligneActuelle = financeLayout->rowCount();
    plafondHoraireRemboursementEntrainement = new QDoubleSpinBox(this);
    plafondHoraireRemboursementEntrainement->setMaximum(10000);
    financeLayout->addWidget(new QLabel(tr("Plafond coût horaire subvention entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(plafondHoraireRemboursementEntrainement, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("€"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    plafondHoraireRemboursementEntrainement->setValue(p_parametresMetiers.plafondHoraireRemboursementEntrainement);

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementEntrainement = new QDoubleSpinBox(this);
    proportionRemboursementEntrainement->setMaximum(100);
    financeLayout->addWidget(new QLabel(tr("Proportion remboursement entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionRemboursementEntrainement, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("%"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    proportionRemboursementEntrainement->setValue(p_parametresMetiers.proportionRemboursementEntrainement*100);  

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementBalade = new QDoubleSpinBox(this);
    proportionRemboursementBalade->setMaximum(100);
    financeLayout->addWidget(new QLabel(tr("Proportion remboursement balades : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionRemboursementBalade, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("%"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    proportionRemboursementBalade->setValue(p_parametresMetiers.proportionRemboursementBalade *100);

    ligneActuelle = financeLayout->rowCount();
    proportionParticipationBalade = new QDoubleSpinBox(this);
    proportionParticipationBalade->setMaximum(100);
    financeLayout->addWidget(new QLabel(tr("Proportion remboursement balades : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionParticipationBalade, ligneActuelle, K_COLONNE_CHAMP);
    financeLayout->addWidget(new QLabel(tr("%"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);

    proportionParticipationBalade->setValue(p_parametresMetiers.proportionParticipationBalade * 100);

    ligneActuelle = financeLayout->rowCount();
    nomTresorier = new QLineEdit(this);
    financeLayout->addWidget(new QLabel(tr("Nom du trésorier : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(nomTresorier, ligneActuelle, K_COLONNE_CHAMP, 1, 2);

    nomTresorier->setText(p_parametresMetiers.nomTresorier);

    //Elements impression
    QGridLayout* impressionLayout = new QGridLayout();
    QWidget* impressionWidget = new QWidget(this);
    impressionWidget->setLayout(impressionLayout);
    onglets->addTab(impressionWidget, QIcon("./ressources/printer-pos-cog.svg"), "Impression");

    ligneActuelle = impressionLayout->rowCount();
    imprimante = new QLineEdit(this);
    impressionLayout->addWidget(new QLabel(tr("Imprimante : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(imprimante, ligneActuelle, K_COLONNE_CHAMP);
    QPushButton* boutonSelectionImprimante = new QPushButton("Sélectionner l'imprimante", this);
    impressionLayout->addWidget(boutonSelectionImprimante, ligneActuelle, K_COLONNE_UNITE_BOUTON);
    connect(boutonSelectionImprimante, SIGNAL(clicked()), this, SLOT(selectionnerImprimante()));

    ligneActuelle = impressionLayout->rowCount();
    resolutionImpression = new QComboBox(this);
    impressionLayout->addWidget(new QLabel(tr("Résolution : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(resolutionImpression, ligneActuelle, K_COLONNE_CHAMP);

    imprimante->setText(p_parametresSysteme.imprimante);
    QPrinter printer;
    printer.setPrinterName(p_parametresSysteme.imprimante);
    if (p_parametresSysteme.imprimante != ""
        && printer.isValid())
    {
        peuplerResolutionImpression(printer);
    }
    else
    {
        resolutionImpression->addItem("600 DPI", 600);
        resolutionImpression->setEnabled(false);
    }
    //TODO imprimante->setText(p_parametresSysteme.resolutionImpression);

    ligneActuelle = impressionLayout->rowCount();
    impressionCouleur = new QComboBox(this);
    impressionLayout->addWidget(new QLabel(tr("Couleur : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(impressionCouleur, ligneActuelle, K_COLONNE_CHAMP);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors-off.svg"), tr("Noir et blanc"), QPrinter::GrayScale);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors.svg"), tr("Couleur"), QPrinter::Color);

    impressionCouleur->setCurrentIndex(p_parametresSysteme.modeCouleurImpression);

    //Parametres système
    QGridLayout* systemeLayout = new QGridLayout();
    QWidget* systemeWidget = new QWidget(this);
    systemeWidget->setLayout(systemeLayout);
    onglets->addTab(systemeWidget, QIcon("./ressources/database-cog.svg"), "Système");

    cheminBdd = new QLineEdit(this);
    systemeLayout->addWidget(new QLabel(tr("Dossier BDD : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(cheminBdd, ligneActuelle, K_COLONNE_CHAMP);
    QPushButton* boutonSelectionBdd = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionBdd, ligneActuelle, K_COLONNE_UNITE_BOUTON, 1, 1);
    cheminBdd->setEnabled(p_editionParametresCritiques);
    boutonSelectionBdd->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionBdd, SIGNAL(clicked()), this, SLOT(selectionnerBdd()));

    cheminBdd->setText(p_parametresSysteme.cheminStockageBdd + "/" + p_parametresSysteme.nomBdd);

    ligneActuelle = systemeLayout->rowCount();
    delaisGardeBdd = new QSpinBox(this);
    QLabel* delaisGardeBddLabel = new QLabel(tr("Délais de garde BDD : "), this);
    systemeLayout->addWidget(delaisGardeBddLabel, ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(delaisGardeBdd, ligneActuelle, K_COLONNE_CHAMP);
    systemeLayout->addWidget(new QLabel(tr("ms"), this), ligneActuelle, K_COLONNE_UNITE_BOUTON);
    delaisGardeBdd->setEnabled(p_editionParametresCritiques);

    delaisGardeBdd->setValue(p_parametresMetiers.delaisDeGardeBdd);

    ligneActuelle = systemeLayout->rowCount();
    factureATraiter = new QLineEdit(this);
    systemeLayout->addWidget(new QLabel(tr("Dossier d'entrée des factures : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(factureATraiter, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionFactureATraiter = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionFactureATraiter, ligneActuelle, K_COLONNE_UNITE_BOUTON);
    connect(boutonSelectionFactureATraiter, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    factureATraiter->setText(p_parametresSysteme.cheminStockageFacturesATraiter);

    ligneActuelle = systemeLayout->rowCount();
    facturesSaisies = new QLineEdit(this);
    systemeLayout->addWidget(new QLabel(tr("Dossier d'enregistrement des factures : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(facturesSaisies, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionFacturesSaisies = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionFacturesSaisies, ligneActuelle, K_COLONNE_UNITE_BOUTON);
    facturesSaisies->setEnabled(p_editionParametresCritiques);
    boutonSelectionFacturesSaisies->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionFacturesSaisies, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    facturesSaisies->setText(p_parametresSysteme.cheminStockageFacturesTraitees);

    ligneActuelle = systemeLayout->rowCount();
    sortieFichiersGeneres = new QLineEdit(this);
    systemeLayout->addWidget(new QLabel(tr("Dossier de sortie des fichiers générés : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(sortieFichiersGeneres, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionSortieFichiersGeneres = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionSortieFichiersGeneres, ligneActuelle, K_COLONNE_UNITE_BOUTON);
    sortieFichiersGeneres->setEnabled(p_editionParametresCritiques);
    boutonSelectionSortieFichiersGeneres->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionFacturesSaisies, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    sortieFichiersGeneres->setText(p_parametresSysteme.cheminSortieFichiersGeneres);

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

    resize(800, onglets->widget(0)->sizeHint().height());
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

        peuplerResolutionImpression(printer);
    }
}

void DialogueEditionParametres::peuplerResolutionImpression(QPrinter &printer)
{
    resolutionImpression->clear();
    resolutionImpression->setEnabled(true);
    QList<int> resolutions = printer.supportedResolutions();
    for (int resolution : resolutions)
    {
        resolutionImpression->addItem(QString::number(resolution) + " DPI", resolution);
    }
}

void DialogueEditionParametres::selectionnerBdd()
{
    QFileDialog dialog(this ,tr("Sélectionner le fichier base de données"), cheminBdd->text(), tr("Base de données SQLite(*.sqlite)"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted)
    {
        cheminBdd->setText(dialog.selectedFiles().at(0));
    }
}

void DialogueEditionParametres::selectionnerRepertoire()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);

    if (sender() == boutonSelectionFactureATraiter)
    {
        dialog.setDirectory(factureATraiter->text());
    }
    else if (sender() == boutonSelectionFacturesSaisies)
    {
        dialog.setDirectory(facturesSaisies->text());
    }
    else if (sender() == boutonSelectionSortieFichiersGeneres)
    {
        dialog.setDirectory(sortieFichiersGeneres->text());
    }

    if (dialog.exec() == QDialog::Accepted)
    {
        if (sender() == boutonSelectionFactureATraiter)
        {
            factureATraiter->setText(dialog.selectedFiles().at(0));
        }
        else if (sender() == boutonSelectionFacturesSaisies)
        {
            facturesSaisies->setText(dialog.selectedFiles().at(0));
        }
        else if (sender() == boutonSelectionSortieFichiersGeneres)
        {
            sortieFichiersGeneres->setText(dialog.selectedFiles().at(0));
        }
    }
}

void DialogueEditionParametres::gererChangementOnglet()
{
    //qDebug() << onglets->widget(onglets->currentIndex())->sizeHint();
    //onglets->setFixedHeight(onglets->widget(onglets->currentIndex())->sizeHint().height());
    //resize(800, onglets->widget(onglets->currentIndex())->sizeHint().height());
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

    QFileInfo fichier(cheminBdd->text());
    parametresSysteme.cheminStockageBdd = fichier.absolutePath();
    parametresSysteme.nomBdd = fichier.fileName();

    parametresSysteme.cheminStockageFacturesTraitees = facturesSaisies->text();
    parametresSysteme.cheminStockageFacturesATraiter = factureATraiter->text();
    parametresSysteme.cheminSortieFichiersGeneres = sortieFichiersGeneres->text();
    parametresSysteme.imprimante = imprimante->text();
    parametresSysteme.resolutionImpression = resolutionImpression->currentData().toInt();
    parametresSysteme.modeCouleurImpression = static_cast<QPrinter::ColorMode>(impressionCouleur->currentData().toInt());

    emit envoyerParametres(parametresMetiers, parametresSysteme);
    accept();
}


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

DialogueEditionParametres::DialogueEditionParametres(const AeroDmsTypes::ParametresMetier p_parametresMetiers,
    const AeroDmsTypes::ParametresSysteme p_parametresSysteme,
    const bool p_editionParametresCritiques,
    QWidget* parent) : QDialog(parent)
{
    const int K_COLONNE_LABEL = 0;
    const int K_COLONNE_CHAMP = 1;
    const int K_COLONNE_BOUTON = 2;

    setWindowTitle(QApplication::applicationName() + " - " + tr("Paramètres"));

    QGridLayout* mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    onglets = new QTabWidget(this);
    mainLayout->addWidget(onglets, 0, 0, 10, 2);

    //Mailing
    QGridLayout* mailingLayout = new QGridLayout();
    QWidget* mailingWidget = new QWidget(this);
    mailingWidget->setLayout(mailingLayout);
    onglets->addTab(mailingWidget, 
        AeroDmsServices::recupererIcone(AeroDmsServices::Icone_MAILING), 
        tr("Mailing"));

    int ligneActuelle = mailingLayout->rowCount();
    objetChequeDispo = new QLineEdit(this);
    objetChequeDispo->setToolTip(tr("Objet par défaut du mail qui sera envoyé lors d'un envoi de mailing via les fonctions\n\"Envoyer un mail aux pilotes concernés par la dernière demande de subvention\" ou \"Envoyer un mail aux pilotes concernés par une demande de subvention\"."));
    mailingLayout->addWidget(new QLabel(tr("Chèques disponibles (objet) : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(objetChequeDispo, ligneActuelle, K_COLONNE_CHAMP);

    objetChequeDispo->setText(p_parametresMetiers.objetMailDispoCheques);

    ligneActuelle = mailingLayout->rowCount();
    texteChequeDispo = new QTextEdit(this);
    texteChequeDispo->setToolTip(tr("Corps par défaut du mail qui sera envoyé lors d'un envoi de mailing via les fonctions\n\"Envoyer un mail aux pilotes concernés par la dernière demande de subvention\" ou \"Envoyer un mail aux pilotes concernés par une demande de subvention\"."));
    mailingLayout->addWidget(new QLabel(tr("Chèques disponibles (texte) : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(texteChequeDispo, ligneActuelle, K_COLONNE_CHAMP);

    texteChequeDispo->setText(p_parametresMetiers.texteMailDispoCheques);

    ligneActuelle = mailingLayout->rowCount();
    objetSubventionRestantes = new QLineEdit(this);
    objetSubventionRestantes->setToolTip(tr("Objet par défaut du mail qui sera envoyé lors d'un envoi de mailing via la fonction \"Envoyer un mail aux pilotes n'ayant pas épuisé leur subvention entrainement\"."));
    mailingLayout->addWidget(new QLabel(tr("Subvention restante (objet) : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(objetSubventionRestantes, ligneActuelle, K_COLONNE_CHAMP);

    objetSubventionRestantes->setText(p_parametresMetiers.objetMailSubventionRestante);

    ligneActuelle = mailingLayout->rowCount();
    texteSubventionRestantes = new QTextEdit(this);
    texteSubventionRestantes->setToolTip(tr("Corps par défaut du mail qui sera envoyé lors d'un envoi de mailing via la fonction \"Envoyer un mail aux pilotes n'ayant pas épuisé leur subvention entrainement\"."));
    mailingLayout->addWidget(new QLabel(tr("Subvention restante (texte) : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(texteSubventionRestantes, ligneActuelle, K_COLONNE_CHAMP);

    texteSubventionRestantes->setText(p_parametresMetiers.texteMailSubventionRestante);

    ligneActuelle = mailingLayout->rowCount();
    objetAutresMailings = new QLineEdit(this);
    objetAutresMailings->setToolTip(tr("Objet par défaut des autres mailings."));
    mailingLayout->addWidget(new QLabel(tr("Autres mailings (objet) : "), this), ligneActuelle, K_COLONNE_LABEL);
    mailingLayout->addWidget(objetAutresMailings, ligneActuelle, K_COLONNE_CHAMP);

    objetAutresMailings->setText(p_parametresMetiers.objetMailAutresMailings);

    //Elements financiers
    QGridLayout* financeLayout = new QGridLayout();
    QWidget* financeWidget = new QWidget(this);
    financeWidget->setLayout(financeLayout);
    onglets->addTab(financeWidget, AeroDmsServices::recupererIcone(AeroDmsServices::Icone_FINANCIER), "Financiers");

    ligneActuelle = financeLayout->rowCount();
    montantCotisationPilote = new QDoubleSpinBox(this);
    montantCotisationPilote->setSuffix(" €");
    financeLayout->addWidget(new QLabel(tr("Montant cotisation pilote : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(montantCotisationPilote, ligneActuelle, K_COLONNE_CHAMP);

    montantCotisationPilote->setValue(p_parametresMetiers.montantCotisationPilote);

    ligneActuelle = financeLayout->rowCount();
    montantSubventionEntrainement = new QDoubleSpinBox(this);
    montantSubventionEntrainement->setMaximum(10000);
    montantSubventionEntrainement->setSuffix(" €");
    financeLayout->addWidget(new QLabel(tr("Subvention annuelle entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(montantSubventionEntrainement, ligneActuelle, K_COLONNE_CHAMP);

    montantSubventionEntrainement->setValue(p_parametresMetiers.montantSubventionEntrainement);

    ligneActuelle = financeLayout->rowCount();
    plafondHoraireRemboursementEntrainement = new QDoubleSpinBox(this);
    plafondHoraireRemboursementEntrainement->setMaximum(10000);
    plafondHoraireRemboursementEntrainement->setSuffix(" €");
    financeLayout->addWidget(new QLabel(tr("Plafond coût horaire subvention entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(plafondHoraireRemboursementEntrainement, ligneActuelle, K_COLONNE_CHAMP);

    plafondHoraireRemboursementEntrainement->setValue(p_parametresMetiers.plafondHoraireRemboursementEntrainement);

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementEntrainement = new QDoubleSpinBox(this);
    proportionRemboursementEntrainement->setMaximum(100);
    proportionRemboursementEntrainement->setSuffix(" %");
    financeLayout->addWidget(new QLabel(tr("Proportion remboursement entraînement : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionRemboursementEntrainement, ligneActuelle, K_COLONNE_CHAMP);

    proportionRemboursementEntrainement->setValue(p_parametresMetiers.proportionRemboursementEntrainement*100);  

    ligneActuelle = financeLayout->rowCount();
    proportionRemboursementBalade = new QDoubleSpinBox(this);
    proportionRemboursementBalade->setToolTip(tr("Proportion du coût du vol qui sera remboursée au pilote pour les vols de balades.\nUne proportion inférieure à 100% est nécessaire pour le respect des règles de vols en partage de frais (le pilote participe aux frais directs du vol). "));
    proportionRemboursementBalade->setMaximum(100);
    proportionRemboursementBalade->setSuffix(" %");
    financeLayout->addWidget(new QLabel(tr("Proportion remboursement balades : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionRemboursementBalade, ligneActuelle, K_COLONNE_CHAMP);

    proportionRemboursementBalade->setValue(p_parametresMetiers.proportionRemboursementBalade *100);

    ligneActuelle = financeLayout->rowCount();
    proportionParticipationBalade = new QDoubleSpinBox(this);
    proportionParticipationBalade->setToolTip(tr("Proportion du coût du vol qui doit être payé par les passagers."));
    proportionParticipationBalade->setMaximum(100);
    proportionParticipationBalade->setSuffix(" %");
    financeLayout->addWidget(new QLabel(tr("Proportion participation passagers balades : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(proportionParticipationBalade, ligneActuelle, K_COLONNE_CHAMP);

    proportionParticipationBalade->setValue(p_parametresMetiers.proportionParticipationBalade * 100);

    ligneActuelle = financeLayout->rowCount();
    nomTresorier = new QLineEdit(this);
    nomTresorier->setToolTip(tr("Nom qui sera utilisé pour le champ \"Nom du demandeur\" des formulaires de demande de subventions."));
    financeLayout->addWidget(new QLabel(tr("Nom du trésorier : "), this), ligneActuelle, K_COLONNE_LABEL);
    financeLayout->addWidget(nomTresorier, ligneActuelle, K_COLONNE_CHAMP);

    nomTresorier->setText(p_parametresMetiers.nomTresorier);

    //Elements impression
    QGridLayout* impressionLayout = new QGridLayout();
    QWidget* impressionWidget = new QWidget(this);
    impressionWidget->setLayout(impressionLayout);
    onglets->addTab(impressionWidget, QIcon("./ressources/printer-pos-cog.svg"), "Impression");

    ligneActuelle = impressionLayout->rowCount();
    imprimante = new QLineEdit(this);
    imprimante->setEnabled(false);
    imprimante->setToolTip(tr("Imprimante par défaut qui sera séléctionnée par le logiciel lors des demandes d'impression."));
    impressionLayout->addWidget(new QLabel(tr("Imprimante : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(imprimante, ligneActuelle, K_COLONNE_CHAMP);
    QPushButton* boutonSelectionImprimante = new QPushButton("Sélectionner l'imprimante", this);
    impressionLayout->addWidget(boutonSelectionImprimante, ligneActuelle, K_COLONNE_BOUTON);
    connect(boutonSelectionImprimante, SIGNAL(clicked()), this, SLOT(selectionnerImprimante()));

    ligneActuelle = impressionLayout->rowCount();
    resolutionImpression = new QComboBox(this);
    resolutionImpression->setToolTip(tr("Force la résolution de génération des impressions.\nRéduire la résolution si des erreurs d'impression se produisent ou si certaines impressions sont partielles."));
    impressionLayout->addWidget(new QLabel(tr("Résolution : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(resolutionImpression, ligneActuelle, K_COLONNE_CHAMP);

    imprimante->setText(p_parametresSysteme.parametresImpression.imprimante);
    QPrinter printer;
    printer.setPrinterName(p_parametresSysteme.parametresImpression.imprimante);
    if (p_parametresSysteme.parametresImpression.imprimante != ""
        && printer.isValid())
    {
        const QList<int> resolutions = printer.supportedResolutions();
        const int resolutionMax = *std::max_element(resolutions.begin(), resolutions.end());
        peuplerResolutionImpression(resolutionMax);
    }
    else
    {
        peuplerResolutionImpression(600);
    }
    if (resolutionImpression->findData(p_parametresSysteme.parametresImpression.resolutionImpression) != -1)
    {
        resolutionImpression->setCurrentIndex(resolutionImpression->findData(p_parametresSysteme.parametresImpression.resolutionImpression));
    }

    ligneActuelle = impressionLayout->rowCount();
    impressionCouleur = new QComboBox(this);
    impressionLayout->addWidget(new QLabel(tr("Couleur : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(impressionCouleur, ligneActuelle, K_COLONNE_CHAMP);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors-off.svg"), tr("Noir et blanc"), QPrinter::GrayScale);
    impressionCouleur->addItem(QIcon("./ressources/invert-colors.svg"), tr("Couleur"), QPrinter::Color);

    impressionCouleur->setCurrentIndex(p_parametresSysteme.parametresImpression.modeCouleurImpression);

    ligneActuelle = impressionLayout->rowCount();
    forcageImpressionRectoSimple = new QCheckBox(this);
    forcageImpressionRectoSimple->setToolTip(tr("Force l'imprimante à imprimer en recto simple même en cas de configuration recto-verso par défaut.\n\nAttention : sur une imprimante non compatible recto-verso automatique ou si l'option recto-verso\nautomatique n'est pas activée sur une imprimante compatible, l'activation de la présente fonction\nde forcage de recto simple provoquera l'insertion d'un page blanche entre chaque page."));
    impressionLayout->addWidget(new QLabel(tr("Forcage de l'impression recto simple : "), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(forcageImpressionRectoSimple, ligneActuelle, K_COLONNE_CHAMP);

    forcageImpressionRectoSimple->setChecked(p_parametresSysteme.parametresImpression.forcageImpressionRecto);

    ligneActuelle = impressionLayout->rowCount();
    margesHautBas = new QSpinBox(this);
    margesHautBas->setSuffix(" px");
    margesHautBas->setMaximum(100);
    margesHautBas->setToolTip(tr("Marges verticales des tableaux de récapitulatif des heures de vol et des formulaires de demandes de subventions."));
    impressionLayout->addWidget(new QLabel(tr("Marges haut et bas"), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(margesHautBas, ligneActuelle, K_COLONNE_CHAMP);

    margesHautBas->setValue(p_parametresSysteme.margesHautBas);

    ligneActuelle = impressionLayout->rowCount();
    margesGaucheDroite = new QSpinBox(this);
    margesGaucheDroite->setSuffix(" px");
    margesGaucheDroite->setMaximum(100);
    margesGaucheDroite->setToolTip(tr("Marges latérale des tableaux de récapitulatif des heures de vol et des formulaires de demandes de subventions."));
    impressionLayout->addWidget(new QLabel(tr("Marges gauche et droite"), this), ligneActuelle, K_COLONNE_LABEL);
    impressionLayout->addWidget(margesGaucheDroite, ligneActuelle, K_COLONNE_CHAMP);

    margesGaucheDroite->setValue(p_parametresSysteme.margesGaucheDroite);

    //Parametres système
    const QString texteToolTipChampSecurise = "Ce champ n'est pas éditable par sécurité.\nPour le rendre éditable, passer en mode débug (menu Aide/Activer le mode debug).";

    QGridLayout* systemeLayout = new QGridLayout();
    QWidget* systemeWidget = new QWidget(this);
    systemeWidget->setLayout(systemeLayout);
    onglets->addTab(systemeWidget, QIcon("./ressources/database-cog.svg"), "Système");

    cheminBdd = new QLineEdit(this);
    cheminBdd->setToolTip(tr("Localisation de la base de données SQLite utilisée par le logiciel.\n\n") + texteToolTipChampSecurise);
    systemeLayout->addWidget(new QLabel(tr("Dossier BDD : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(cheminBdd, ligneActuelle, K_COLONNE_CHAMP);
    QPushButton* boutonSelectionBdd = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionBdd, ligneActuelle, K_COLONNE_BOUTON, 1, 1);
    cheminBdd->setEnabled(p_editionParametresCritiques);
    boutonSelectionBdd->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionBdd, SIGNAL(clicked()), this, SLOT(selectionnerBdd()));

    cheminBdd->setText(p_parametresSysteme.cheminStockageBdd + "/" + p_parametresSysteme.nomBdd);

    ligneActuelle = systemeLayout->rowCount();
    delaisGardeBdd = new QSpinBox(this);
    delaisGardeBdd->setSuffix(" ms");
    delaisGardeBdd->setToolTip(tr("Délais entre 2 insertions en BDD, en millisecondes : ce délais permet de régler des soucis de double génération PDF liés au fait\nque le fichier SQLite est stocké sur un lecteur réseau.\nUne valeur de 50 ms semble suffisante. En cas d'occurence de mauvaise insertion, augmenter cette valeur.\n\n") + texteToolTipChampSecurise);
    QLabel* delaisGardeBddLabel = new QLabel(tr("Délais de garde BDD : "), this);
    systemeLayout->addWidget(delaisGardeBddLabel, ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(delaisGardeBdd, ligneActuelle, K_COLONNE_CHAMP);
    delaisGardeBdd->setEnabled(p_editionParametresCritiques);

    delaisGardeBdd->setValue(p_parametresMetiers.delaisDeGardeBdd);

    ligneActuelle = systemeLayout->rowCount();
    factureATraiter = new QLineEdit(this);
    factureATraiter->setToolTip("Dossier par défaut qui sera ouvert par le logiciel lorsque l'on demande à charger une nouvelle facture.");
    systemeLayout->addWidget(new QLabel(tr("Dossier d'entrée des factures : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(factureATraiter, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionFactureATraiter = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionFactureATraiter, ligneActuelle, K_COLONNE_BOUTON);
    connect(boutonSelectionFactureATraiter, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    factureATraiter->setText(p_parametresSysteme.cheminStockageFacturesATraiter);

    ligneActuelle = systemeLayout->rowCount();
    facturesSaisies = new QLineEdit(this);
    facturesSaisies->setToolTip(tr("Dossier où seront stockées les factures saisies dans le logiciel.\nAttention : en cas de déplacement de ce dossier, le logiciel n'assurera pas la copie des anciennes factures vers la\nnouvelle destination. C'est à l'utilisateur d'assurer le déplacement des factures existantes vers le nouveau dossier.\n\n") + texteToolTipChampSecurise);
    systemeLayout->addWidget(new QLabel(tr("Dossier d'enregistrement des factures : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(facturesSaisies, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionFacturesSaisies = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionFacturesSaisies, ligneActuelle, K_COLONNE_BOUTON);
    facturesSaisies->setEnabled(p_editionParametresCritiques);
    boutonSelectionFacturesSaisies->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionFacturesSaisies, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    facturesSaisies->setText(p_parametresSysteme.cheminStockageFacturesTraitees);

    ligneActuelle = systemeLayout->rowCount();
    sortieFichiersGeneres = new QLineEdit(this);
    sortieFichiersGeneres->setToolTip(tr("Dossier où seront stockées les formulaires de demandes de subventions génerées.\nDans le cadre d'une installation utilisée par plusieurs utilisateurs, ce repertoire doit être accessible par tous les utilisateurs.\nAttention : en cas de déplacement de ce dossier, le logiciel n'assurera pas la copie des anciennes demandes vers la nouvelle\ndestination. C'est à l'utilisateur d'assurer le déplacement des demandes existantes vers le nouveau dossier, si nécessaire.\n\n") + texteToolTipChampSecurise);
    systemeLayout->addWidget(new QLabel(tr("Dossier de sortie des fichiers générés : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(sortieFichiersGeneres, ligneActuelle, K_COLONNE_CHAMP);
    boutonSelectionSortieFichiersGeneres = new QPushButton("Sélectionner", this);
    systemeLayout->addWidget(boutonSelectionSortieFichiersGeneres, ligneActuelle, K_COLONNE_BOUTON);
    sortieFichiersGeneres->setEnabled(p_editionParametresCritiques);
    boutonSelectionSortieFichiersGeneres->setEnabled(p_editionParametresCritiques);
    connect(boutonSelectionSortieFichiersGeneres, SIGNAL(clicked()), this, SLOT(selectionnerRepertoire()));

    sortieFichiersGeneres->setText(p_parametresSysteme.cheminSortieFichiersGeneres);

    ligneActuelle = systemeLayout->rowCount();
    loginDaca = new QLineEdit(this);
    loginDaca->setToolTip(tr("Identifiant de connexion au site DACA pour la récupération des factures"));
    systemeLayout->addWidget(new QLabel(tr("Login site DACA : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(loginDaca, ligneActuelle, K_COLONNE_CHAMP);

    loginDaca->setText(p_parametresSysteme.loginSiteDaca);

    ligneActuelle = systemeLayout->rowCount();
    motDePasseDaca = new QLineEdit(this);
    motDePasseDaca->setToolTip(tr("Mot de passe de connexion au site DACA pour la récupération des factures"));
    motDePasseDaca->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    systemeLayout->addWidget(new QLabel(tr("Mot de passe site DACA : "), this), ligneActuelle, K_COLONNE_LABEL);
    systemeLayout->addWidget(motDePasseDaca, ligneActuelle, K_COLONNE_CHAMP);

    motDePasseDaca->setText(p_parametresSysteme.motDePasseSiteDaca);

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

        const QList<int> resolutions = printer.supportedResolutions();
        const int resolutionMax = *std::max_element(resolutions.begin(), resolutions.end());
        peuplerResolutionImpression(resolutionMax);
    }
}

void DialogueEditionParametres::peuplerResolutionImpression(const int p_resolutionMax)
{
    resolutionImpression->clear();
    
    for (int resolution = 100; resolution <= p_resolutionMax; resolution = resolution + 100)
    {
        resolutionImpression->addItem(QString::number(resolution) + " DPI", resolution);
    }

    //On gère le cas ou la plus haute résolution n'est pas un multiple de 100 => on ajoute la plus haute résolution
    if (resolutionImpression->itemData(resolutionImpression->count()-1).toInt() < p_resolutionMax)
    {
        resolutionImpression->addItem(QString::number(p_resolutionMax) + " DPI", p_resolutionMax);
    }
}

void DialogueEditionParametres::selectionnerBdd()
{
    QFileDialog dialog(this , QApplication::applicationName() + " - " + tr("Sélectionner le fichier base de données"), cheminBdd->text(), tr("Base de données SQLite(*.sqlite)"));
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
        dialog.setWindowTitle(QApplication::applicationName() + " - " + tr("Sélection du répertoire par défaut des factures à traiter"));
    }
    else if (sender() == boutonSelectionFacturesSaisies)
    {
        dialog.setDirectory(facturesSaisies->text());
        dialog.setWindowTitle(QApplication::applicationName() + " - " + tr("Sélection du répertoire de stockage des factures traitées"));
    }
    else if (sender() == boutonSelectionSortieFichiersGeneres)
    {
        dialog.setDirectory(sortieFichiersGeneres->text());
        dialog.setWindowTitle(QApplication::applicationName() + " - " + tr("Sélection du répertoire de sortie des fichiers générés"));
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
    parametresMetiers.objetMailDispoCheques = objetChequeDispo->text();
    parametresMetiers.texteMailDispoCheques = texteChequeDispo->toPlainText();
    parametresMetiers.objetMailSubventionRestante = objetSubventionRestantes->text();
    parametresMetiers.texteMailSubventionRestante = texteSubventionRestantes->toPlainText();
    parametresMetiers.objetMailAutresMailings = objetAutresMailings->text();
    
    AeroDmsTypes::ParametresSysteme parametresSysteme;

    QFileInfo fichier(cheminBdd->text());
    parametresSysteme.cheminStockageBdd = fichier.absolutePath();
    parametresSysteme.nomBdd = fichier.fileName();

    parametresSysteme.cheminStockageFacturesTraitees = facturesSaisies->text();
    parametresSysteme.cheminStockageFacturesATraiter = factureATraiter->text();
    parametresSysteme.cheminSortieFichiersGeneres = sortieFichiersGeneres->text();
    parametresSysteme.loginSiteDaca = loginDaca->text();
    parametresSysteme.motDePasseSiteDaca = motDePasseDaca->text();

    parametresSysteme.parametresImpression.imprimante = imprimante->text();
    parametresSysteme.parametresImpression.resolutionImpression = resolutionImpression->currentData().toInt();
    parametresSysteme.parametresImpression.modeCouleurImpression = static_cast<QPrinter::ColorMode>(impressionCouleur->currentData().toInt());
    parametresSysteme.parametresImpression.forcageImpressionRecto = (forcageImpressionRectoSimple->checkState() == Qt::Checked);

    parametresSysteme.margesHautBas = margesHautBas->value();
    parametresSysteme.margesGaucheDroite = margesGaucheDroite->value();

    emit envoyerParametres(parametresMetiers, parametresSysteme);
    accept();
}

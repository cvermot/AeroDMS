#include "AeroDms.h"
#include "AeroDmsTypes.h"

#include <QtWidgets>
#include <QToolBar>
#include <QPdfDocument>
#include <QPdfPageSelector>
#include <QPdfView>

AeroDms::AeroDms(QWidget* parent):QMainWindow(parent)
{

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationDirPath());
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,"AeroDMS", "AeroDMS");

    qDebug() << "DirPath : " << QCoreApplication::applicationDirPath();
    
    if (settings.value("baseDeDonnees/chemin", "") == "")
    {
        settings.beginGroup("baseDeDonnees");
        settings.setValue("chemin", QCoreApplication::applicationDirPath());
        settings.setValue("nom", "AeroDMS.sqlite");
        settings.endGroup();
    }

    if (settings.value("dossiers/facturesATraiter", "") == "")
    {
        settings.beginGroup("dossiers");
        settings.setValue("facturesATraiter", QCoreApplication::applicationDirPath());
        settings.setValue("facturesSaisies", QCoreApplication::applicationDirPath());
        //settings.setValue("fichiersAImprimer", QCoreApplication::applicationDirPath());
        settings.endGroup();
    }

    if (settings.value("parametresMetier/montantSubventionEntrainement", "") == "")
    {
        settings.beginGroup("parametresMetier");
        settings.setValue("montantSubventionEntrainement", "750");
        settings.setValue("montantCotisationPilote", "15");
        settings.endGroup();
    }

    const QString database = settings.value("baseDeDonnees/chemin", "").toString() + QString("/") + settings.value("baseDeDonnees/nom", "").toString();

    db = new ManageDb(database);
    pdf = new PdfRenderer(db);

    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);

    //==========Onglet Pilotes
    QTableWidget* vuePilotes = new QTableWidget(0, 4, this);
    vuePilotes->setHorizontalHeaderItem(0, new QTableWidgetItem("Nom"));
    vuePilotes->setHorizontalHeaderItem(1, new QTableWidgetItem("Prénom"));
    vuePilotes->setHorizontalHeaderItem(2, new QTableWidgetItem("Heures subventionnées"));
    vuePilotes->setHorizontalHeaderItem(3, new QTableWidgetItem("Montant subventionné"));
    vuePilotes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    vuePilotes->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainTabWidget->addTab(vuePilotes, "Pilotes");

    vuePilotes->setRowCount(10);
    for (int i = 0; i < 10; i++)
    {
        vuePilotes->setItem(i, 0, new QTableWidgetItem("Test"));
    }

    //==========Onglet Vols
    QTableWidget* vueVols = new QTableWidget(0, 6, this);;
    vueVols->setHorizontalHeaderItem(0, new QTableWidgetItem("Pilote"));
    vueVols->setHorizontalHeaderItem(1, new QTableWidgetItem("Date"));
    vueVols->setHorizontalHeaderItem(2, new QTableWidgetItem("Durée"));
    vueVols->setHorizontalHeaderItem(3, new QTableWidgetItem("Coût"));
    vueVols->setHorizontalHeaderItem(4, new QTableWidgetItem("Subvention"));
    vueVols->setHorizontalHeaderItem(5, new QTableWidgetItem("Soumis CE"));
    vueVols->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainTabWidget->addTab(vueVols, "Vols");

    //==========Onglet Ajout dépense
    QHBoxLayout* ajoutVol = new QHBoxLayout(this);
    QWidget* widgetAjoutVol = new QWidget(this);
    widgetAjoutVol->setLayout(ajoutVol);
    mainTabWidget->addTab(widgetAjoutVol, "Ajout dépense");

    pdfDocument = new QPdfDocument(this);
    QPdfView* pdfView = new QPdfView(this);
    pdfView->setDocument(pdfDocument);
    pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    ajoutVol->addWidget(pdfView, 3);

    QTabWidget *depenseTabWidget = new QTabWidget(this);
    ajoutVol->addWidget(depenseTabWidget, 1);
    

    //==========Sous onglet vol de l'onglet "Ajout dépense"
    typeDeVol = new QComboBox(this);
    typeDeVol->addItems(db->recupererTypesDesVol());
    typeDeVol->setCurrentIndex(2);
    QLabel* typeDeVolLabel = new QLabel(tr("Type de vol : "), this);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::changerInfosVolSurSelectionTypeVol);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    choixPilote = new QComboBox(this);
    QLabel* choixPiloteLabel = new QLabel(tr("Pilote : "), this);
    connect(choixPilote, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    dateDuVol = new QDateTimeEdit(this);
    dateDuVol->setDisplayFormat("dd/MM/yyyy");
    dateDuVol->setCalendarPopup(true);
    dateDuVol->setDate(QDate::currentDate());
    QLabel* dateDuVolLabel = new QLabel(tr("Date du vol : "), this);

    dureeDuVol = new QTimeEdit(this);
    QLabel* dureeDuVolLabel = new QLabel(tr("Durée du vol : "), this);
    connect(dureeDuVol, &QTimeEdit::timeChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    prixDuVol = new QDoubleSpinBox(this);
    prixDuVol->setRange(0.0, 2000.0);
    prixDuVol->setSingleStep(1.0);
    prixDuVol->setDecimals(2);
    prixDuVol->setSuffix("€");
    QLabel* prixDuVolLabel = new QLabel(tr("Prix du vol : "), this);
    connect(prixDuVol, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    choixBalade = new QComboBox(this);
    QLabel* choixBaladeLabel = new QLabel(tr("Sortie : "), this);
    connect(choixBalade, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    remarqueVol = new QLineEdit(this);
    QLabel* remarqueVolLabel = new QLabel(tr("Remarque : "), this);

    validerLeVol = new QPushButton("Valider le vol", this);
    connect(validerLeVol, &QPushButton::clicked, this, &AeroDms::enregistrerUnVol);

    QGridLayout* infosVol = new QGridLayout(this);
    QWidget* widgetDepenseVol = new QWidget(this);
    widgetDepenseVol->setLayout(infosVol);
    depenseTabWidget->addTab(widgetDepenseVol, "Vol");
    infosVol->addWidget(typeDeVolLabel,   0, 0);
    infosVol->addWidget(typeDeVol,        0, 1);
    infosVol->addWidget(choixPiloteLabel, 1, 0);
    infosVol->addWidget(choixPilote,      1, 1);
    infosVol->addWidget(dateDuVolLabel,   2, 0);
    infosVol->addWidget(dateDuVol,        2, 1);
    infosVol->addWidget(dureeDuVolLabel,  3, 0);
    infosVol->addWidget(dureeDuVol,       3, 1);
    infosVol->addWidget(prixDuVolLabel,   4, 0);
    infosVol->addWidget(prixDuVol,        4, 1);
    infosVol->addWidget(choixBaladeLabel, 5, 0);
    infosVol->addWidget(choixBalade,      5, 1);
    infosVol->addWidget(remarqueVolLabel, 6, 0);
    infosVol->addWidget(remarqueVol,      6, 1);
    infosVol->addWidget(validerLeVol,     7, 0, 2, 0);

    //==========Sous onglet facture de l'onglet "Ajout dépense"
    choixPayeur = new QComboBox(this);
    QLabel* choixPayeurLabel = new QLabel(tr("Payeur : "), this);
    connect(choixPayeur, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);
    /*typeDeVol = new QComboBox(this);
    typeDeVol->addItems(db->recupererTypesDesVol());
    typeDeVol->setCurrentIndex(2);
    QLabel* typeDeVolLabel = new QLabel(tr("Type de vol : "), this);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::changerInfosVolSurSelectionTypeVol);
    connect(typeDeVol, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);*/

    dateDeFacture = new QDateEdit(this);
    dateDeFacture->setDisplayFormat("dd/MM/yyyy");
    dateDeFacture->setCalendarPopup(true);
    dateDeFacture->setDate(QDate::currentDate());
    QLabel* dateDeFactureLabel = new QLabel(tr("Date de la facture : "), this);

    choixBaladeFacture = new QComboBox(this);
    QLabel* choixBaladeFactureLabel = new QLabel(tr("Sortie : "), this);
    connect(choixBaladeFacture, &QComboBox::currentIndexChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    montantFacture = new QDoubleSpinBox(this);
    montantFacture->setRange(0.0, 2000.0);
    montantFacture->setSingleStep(1.0);
    montantFacture->setDecimals(2);
    montantFacture->setSuffix("€");
    QLabel* montantFactureLabel = new QLabel(tr("Montant facture : "), this);
    connect(montantFacture, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnnesSaisies);

    remarqueFacture = new QLineEdit(this);
    QLabel* remarqueFactureLabel = new QLabel(tr("Remarque : "), this);

    validerLaFacture = new QPushButton("Valider la facture", this);
    connect(validerLaFacture, &QPushButton::clicked, this, &AeroDms::enregistrerUneFacture);

    QGridLayout* infosFacture = new QGridLayout(this);
    QWidget* widgetDepenseFacture = new QWidget(this);
    widgetDepenseFacture->setLayout(infosFacture);
    depenseTabWidget->addTab(widgetDepenseFacture, "Facture");
    infosFacture->addWidget(choixPayeurLabel, 1, 0);
    infosFacture->addWidget(choixPayeur, 1, 1);
    infosFacture->addWidget(dateDeFactureLabel, 2, 0);
    infosFacture->addWidget(dateDeFacture, 2, 1);
    infosFacture->addWidget(montantFactureLabel, 4, 0);
    infosFacture->addWidget(montantFacture, 4, 1);
    infosFacture->addWidget(choixBaladeFactureLabel, 5, 0);
    infosFacture->addWidget(choixBaladeFacture, 5, 1);
    infosFacture->addWidget(remarqueFactureLabel, 6, 0);
    infosFacture->addWidget(remarqueFacture, 6, 1);
    infosFacture->addWidget(validerLaFacture, 7, 0, 2, 0);

    //==========Onglet Ajout recette
    QHBoxLayout* ajoutRecette = new QHBoxLayout(this);
    QWidget* widgetAjoutRecette = new QWidget(this);
    widgetAjoutRecette->setLayout(ajoutRecette);
    mainTabWidget->addTab(widgetAjoutRecette, "Ajout recette");

    listeBaladesEtSorties = new QListWidget(this);
    ajoutRecette->addWidget(listeBaladesEtSorties);

    typeDeRecette = new QComboBox(this);
    qDebug() << "ComboBox typeRecetteVol";
    typeDeRecette->addItems(db->recupererTypesDesVol(true));
    connect(typeDeRecette, &QComboBox::currentIndexChanged, this, &AeroDms::chargerBaladesSorties);
    //typeDeRecette->setCurrentIndex(2);
    QLabel* typeDeRecetteLabel = new QLabel(tr("Type de vol : "), this);

    intituleRecette = new QLineEdit(this);
    intituleRecette->setToolTip(tr("Format recommandé : 'NOM EMETTEUR CHEQUE / Banque numéro de chèque'"));
    QLabel* intituleRecetteLabel = new QLabel(tr("Intitulé : "), this); 
    connect(intituleRecette, &QLineEdit::textChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    montantRecette = new QDoubleSpinBox(this);
    montantRecette->setRange(0.0, 2000.0);
    montantRecette->setSingleStep(1.0);
    montantRecette->setDecimals(2);
    montantRecette->setSuffix("€");
    QLabel* montantRecetteLabel = new QLabel(tr("Montant : "), this);
    connect(montantRecette, &QDoubleSpinBox::valueChanged, this, &AeroDms::prevaliderDonnneesSaisiesRecette);

    validerLaRecette = new QPushButton("Valider la recette", this);
    connect(validerLaRecette, &QPushButton::clicked, this, &AeroDms::enregistrerUneRecette);

    QGridLayout* infosRecette = new QGridLayout(this);
    ajoutRecette->addLayout(infosRecette);
    infosRecette->addWidget(typeDeRecetteLabel, 0, 0);
    infosRecette->addWidget(typeDeRecette, 0, 1);
    infosRecette->addWidget(intituleRecetteLabel, 1, 0);
    infosRecette->addWidget(intituleRecette, 1, 1);
    infosRecette->addWidget(montantRecetteLabel, 2, 0);
    infosRecette->addWidget(montantRecette, 2, 1);
    infosRecette->addWidget(validerLaRecette, 5, 0, 2, 0);

    //=============General
    statusBar()->showMessage("Pret");

    setWindowTitle(tr("AeroDMS"));
    setMinimumSize(160, 160);
    showMaximized();
    //resize(480, 320);

    QToolBar* toolBar = addToolBar(tr(""));
    const QIcon iconeAjouterUnVol = QIcon("./ressources/airplane-plus.svg");
    QAction* bouttonAjouterUnVol = new QAction(iconeAjouterUnVol, tr("&Ajouter un vol"), this);
    bouttonAjouterUnVol->setStatusTip(tr("Ajouter un vol"));
    connect(bouttonAjouterUnVol, &QAction::triggered, this, &AeroDms::selectionnerUneFacture);
    toolBar->addAction(bouttonAjouterUnVol);

    const QIcon iconeGenerePdf = QIcon("./ressources/file-pdf-box.svg");
    QAction* bouttonGenerePdf = new QAction(iconeGenerePdf, tr("&PDF"), this);
    bouttonGenerePdf->setStatusTip(tr("PDF"));
    connect(bouttonGenerePdf, &QAction::triggered, this, &AeroDms::genererPdf);
    toolBar->addAction(bouttonGenerePdf);

    const QIcon iconeAjouterPilote = QIcon("./ressources/account-tie-hat.svg");
    QAction* bouttonAjouterPilote = new QAction(iconeAjouterPilote, tr("&Ajouter un pilote"), this);
    bouttonAjouterPilote->setStatusTip(tr("Ajouter un pilote"));
    connect(bouttonAjouterPilote, &QAction::triggered, this, &AeroDms::ajouterUnPilote);
    toolBar->addAction(bouttonAjouterPilote);

    const QIcon iconeAjouterCotisation = QIcon("./ressources/ticket.svg");
    QAction* bouttonAjouterCotisation = new QAction(iconeAjouterCotisation, tr("&Ajouter une cotisation pour un pilote"), this);
    bouttonAjouterCotisation->setStatusTip(tr("Ajouter une cotisation pour un pilote"));
    connect(bouttonAjouterCotisation, &QAction::triggered, this, &AeroDms::ajouterUneCotisation);
    toolBar->addAction(bouttonAjouterCotisation);

    const QIcon iconeAjouterSortie = QIcon("./ressources/transit-connection-variant.svg");
    QAction* bouttonAjouterSortie = new QAction(iconeAjouterSortie, tr("&Ajouter une sortie"), this);
    bouttonAjouterSortie->setStatusTip(tr("Ajouter une sortie"));
    connect(bouttonAjouterSortie, &QAction::triggered, this, &AeroDms::ajouterUneSortie);
    toolBar->addAction(bouttonAjouterSortie);

    QToolBar* SelectionToolBar = addToolBar(tr(""));

    listeDeroulanteAnnee = new QComboBox;
    listeDeroulanteAnnee->addItem("Toutes");
    listeDeroulanteAnnee->addItem("2023");
    listeDeroulanteAnnee->addItem("2024");
    SelectionToolBar->addWidget(listeDeroulanteAnnee);

    listeDeroulantePilote = new QComboBox;
    SelectionToolBar->addWidget(listeDeroulantePilote);

    //Fenêtres
    dialogueGestionPilote = new DialogueGestionPilote(db, this);
    connect(dialogueGestionPilote, SIGNAL(accepted()), this, SLOT(ajouterUnPiloteEnBdd()));

    dialogueAjouterCotisation = new DialogueAjouterCotisation(db, this);
    connect(dialogueAjouterCotisation, SIGNAL(accepted()), this, SLOT(ajouterUneCotisationEnBdd()));

    dialogueAjouterSortie = new DialogueAjouterSortie(this);
    connect(dialogueAjouterSortie, SIGNAL(accepted()), this, SLOT(ajouterUneSortieEnBdd()));

    peuplerListesPilotes();
    peuplerListeSorties();
    peuplerListeBaladesEtSorties();
    prevaliderDonnnesSaisies();
    prevaliderDonnneesSaisiesRecette();
    changerInfosVolSurSelectionTypeVol();
}

AeroDms::~AeroDms()
{
}

void AeroDms::ajouterUneCotisationEnBdd()
{
    //On contrôle que le pilote n'a pas déjà une cotisation pour cette année
    AeroDmsTypes::CotisationAnnuelle infosCotisation = dialogueAjouterCotisation->recupererInfosCotisationAAjouter();
    if (db->piloteEstAJourDeCotisation(infosCotisation.idPilote, infosCotisation.annee))
    {
        QMessageBox::critical(this, "Cotisation déja reglée", "Le pilote indiqué est déjà à jour de\nsa cotisation pour l'année saisie.");
    }
    //Si le pilote n'a pas deja une cotisation pour cette année la, on ajoute la cotisation en BDD
    else
    {
        db->ajouterCotisation(infosCotisation);
    }
}

void AeroDms::chargerBaladesSorties()
{
    peuplerListeBaladesEtSorties();
}

void AeroDms::ajouterUnPiloteEnBdd()
{
    AeroDmsTypes::Pilote pilote = dialogueGestionPilote->recupererInfosPilote();
    AeroDmsTypes::ResultatCreationPilote resultat = db->creerPilote(pilote);

    switch (resultat)
    {
        case AeroDmsTypes::ResultatCreationPilote_SUCCES:
        {
            statusBar()->showMessage("Pilote ajouté avec succès");
            break;
        } 
        case AeroDmsTypes::ResultatCreationPilote_PILOTE_EXISTE:
        {
            statusBar()->showMessage("Echec ajout pilote : le pilote existe déjà");
            QMessageBox::critical(this, "Echec ajoute pilote", "Un pilote existe avec ce nom\nexiste déjà. Ajout impossible.");
            break;
        }
        case AeroDmsTypes::ResultatCreationPilote_AUTRE:
        {
            statusBar()->showMessage("Echec ajout pilote : erreur indéterminée");
            QMessageBox::critical(this, "Echec ajoute pilote", "Une erreur indéterminée s'est\nproduite. Ajout du pilote impossible.");
            break;
        }
    }

    //On met à jour les listes de pilotes
    peuplerListesPilotes();
    dialogueAjouterCotisation->mettreAJourLeContenuDeLaFenetre();
}

void AeroDms::ajouterUneSortieEnBdd()
{
    AeroDmsTypes::Sortie sortie = dialogueAjouterSortie->recupererInfosSortieAAjouter();

    db->creerSortie(sortie);
    statusBar()->showMessage("Sortie ajoutée");
    peuplerListeBaladesEtSorties();
    peuplerListeSorties();
}

void AeroDms::selectionnerUneFacture()
{
    QString fichier = QFileDialog::getOpenFileName(
        this,
        "Ouvrir une facture",
        cheminStockageFacturesATraiter,
        tr("Fichier PDF (*.pdf)"));

    if (!fichier.isNull())
    {
        chargerUneFacture(fichier);
    }
}

void AeroDms::chargerUneFacture(QString p_fichier)
{
    //qDebug() << "fichier validé";
    qDebug() << p_fichier;
    cheminDeLaFactureCourante = p_fichier;
    factureIdEnBdd = -1;
    //qDebug() << "deplacement " << cheminDeLaFactureCourante <<  QFile::rename(cheminDeLaFactureCourante, cheminStockageFacturesTraitees.append("test.pdf"));
    pdfDocument->load(p_fichier);

    choixPilote->setEnabled(true);
    choixPilote->setCurrentIndex(0);
    //On affiche le widget qui contient la fonction d'ajout de vol
    mainTabWidget->setCurrentIndex(2);
}

void AeroDms::genererPdf()
{
    pdf->imprimerLesDemandesDeSubvention();
}

void AeroDms::enregistrerUneFacture()
{
    bool estEnEchec = false;
    const int anneeRenseignee = dateDeFacture->date().year();
    const QString idPilote = choixPayeur->currentData().toString();
    const int idSortie = choixBaladeFacture->currentData().toInt();

    //On verifie si le pilote est a jour de sa cotisation pour l'année de la facture
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        estEnEchec = true;
        QMessageBox::critical(this, tr("AeroDMS"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
               "Impossible d'enregistrer une facture à rembourser à son profit."), QMessageBox::Cancel);
    }

    //On effectue d'abord quelques contrôles pour savoir si la facture est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en echec sur une des étapes précédentes
    if (pdfDocument->status() == QPdfDocument::Status::Ready
        && !estEnEchec)
    {
        if (choixPayeur->isEnabled())
        {
            QString nomDeLaFacture = "";
            //Le comboBox choixPayeur est activé : la facture n'est pas encore soumise
            //1) on sauvegarde la facture avec les autres, en le renommant
            //2) on la référence en BDD
            pdfDocument->close();
            //On forme le nom du fichier sous forme annee.nomSortie.intitule.idFactureBdd.pdf
            int idFactureBdd = db->recupererProchainNumeroFacture();
            nomDeLaFacture.append(QString::number(anneeRenseignee));
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(idPilote);
            nomDeLaFacture.append(".facture.");
            nomDeLaFacture.append(QString::number(idFactureBdd));
            nomDeLaFacture.append(".pdf");
            qDebug() << "nom fichier" << nomDeLaFacture << " / chemin " << cheminStockageFacturesTraitees;
            QString cheminComplet = cheminStockageFacturesTraitees;
            cheminComplet.append(nomDeLaFacture);
            QFile gestionnaireDeFichier;
            if (gestionnaireDeFichier.copy(cheminDeLaFactureCourante, cheminComplet))
            {
                qDebug() << gestionnaireDeFichier.errorString();
                factureIdEnBdd = db->ajouterFacture(nomDeLaFacture);
                pdfDocument->load(cheminComplet);
                gestionnaireDeFichier.remove(cheminDeLaFactureCourante);
                cheminDeLaFactureCourante = cheminComplet;
            }
            else
            {
                statusBar()->showMessage("Impossible de déplacer la facture : arrêt.");
                estEnEchec = true;
            }
        }
        //On desactive le choix du payeur (la facture ne concerne qu'un seul et unique payeur)
        choixPayeur->setEnabled(false);

        //Ensuite :
        //3) on enregistre la paiement en BDD, si on est pas en echec
        if (!estEnEchec)
        {
            db->enregistrerUneFacture(idPilote,
                factureIdEnBdd,
                dateDeFacture->date(),
                montantFacture->value(),
                choixBaladeFacture->currentData().toInt(),
                remarqueFacture->text());

            statusBar()->showMessage(QString("Facture ")
                + remarqueFacture->text()
                + " du "
                + dateDeFacture->date().toString("dd/MM/yyyy")
                + " ("
                + QString::number(montantFacture->value())
                + "€) ajoutée.");

            montantFacture->setValue(0);
            remarqueFacture->clear();
        }
        else
        {
            statusBar()->showMessage("Erreur ajout");
        }
    } 
}


void AeroDms::enregistrerUnVol()
{
    bool estEnEchec = false;
    const int anneeRenseignee = dateDuVol->date().year();
    const QString idPilote = choixPilote->currentData().toString();

    //On verifie si le pilote est a jour de sa cotisation pour l'année du vol
    if (!db->piloteEstAJourDeCotisation(idPilote, anneeRenseignee))
    {
        estEnEchec = true;
        QMessageBox::critical(this, tr("AeroDMS"),
            tr("Le pilote n'est pas a jour de sa cotisation pour l'année du vol.\n"
                "Impossible d'enregistrer le vol."), QMessageBox::Cancel);
    }

    //On effectue d'abord quelques contrôles pour savoir si le vol est enregistrable :
    //1) on a une facture chargée
    //2) les données (pilote, date du vol, durée, cout) sont renseignées
    //3) on est pas en echec sur une des étapes précédentes
    if (pdfDocument->status() == QPdfDocument::Status::Ready
        && !estEnEchec )
    {
        QString nomDeLaFacture = "";
        if (choixPilote->isEnabled())
        {
            //Le comboBox choixPilote est activé : on est sur le premier vol de cette facture :
            //1) on sauvegarde la facture avec les autres, en le renommant
            //2) on la référence en BDD
            pdfDocument->close();
            //On forme le nom du fichier sous forme annee.idPilote.idFactureBdd.pdf
            int idFactureBdd = db->recupererProchainNumeroFacture();
            nomDeLaFacture.append(QString::number(anneeRenseignee));
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(idPilote);
            nomDeLaFacture.append(".");
            nomDeLaFacture.append(QString::number(idFactureBdd));
            nomDeLaFacture.append(".pdf");
            qDebug() << "nom fichier" << nomDeLaFacture << " / chemin " << cheminStockageFacturesTraitees;
            QString cheminComplet = cheminStockageFacturesTraitees;
            cheminComplet.append(nomDeLaFacture);
            QFile gestionnaireDeFichier;
            if (gestionnaireDeFichier.copy(cheminDeLaFactureCourante, cheminComplet))
            {
                qDebug() << gestionnaireDeFichier.errorString();
                factureIdEnBdd = db->ajouterFacture(nomDeLaFacture);
                pdfDocument->load(cheminComplet);
                gestionnaireDeFichier.remove(cheminDeLaFactureCourante);
                cheminDeLaFactureCourante = cheminComplet;
            }
            else
            {
                statusBar()->showMessage("Impossible de déplacer la facture : arrêt.");
                estEnEchec = true;
            }
        }
        //On desactive le choix du pilote (la facture ne concerne qu'un seul et unique pilote)
        choixPilote->setEnabled(false);

        //Ensuite, si on est pas en echec :
        //1) on récupère la subvention restante pour le pilote
        //2) on calcul la subvention allouable pour le vol
        //3) on enregistre le vol en BDD    
		if (!estEnEchec)
		{
			float subventionRestante = db->recupererSubventionRestante(idPilote, anneeRenseignee);
			//On initialise le montant subventionné sur la base du prix du vol (cas vols type sortie)
			float montantSubventionne = prixDuVol->value();
			if (typeDeVol->currentText() == "Balade")
			{
				qDebug() << "Balade";
				float proportionPriseEnCharge = 1.0;
				montantSubventionne = prixDuVol->value() * proportionPriseEnCharge;
			}
			//Si on est en vol d'entrainement, calculs spécifiques et enregistrement spécifique
			if (typeDeVol->currentText() == "Entrainement")
			{
				qDebug() << "Entrainement";
				float coutHoraire = calculerCoutHoraire();
				if (coutHoraire > 150)
				{
					coutHoraire = 150;
				}
				montantSubventionne = (coutHoraire * (dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute()) / 60.0) * 0.50;
				if (montantSubventionne > subventionRestante)
				{
					montantSubventionne = subventionRestante;
				}
				qDebug() << "montant subvention : " << montantSubventionne << "Subvention restante : " << subventionRestante;
				db->enregistrerUnVolDEntrainement(idPilote,
					typeDeVol->currentText(),
					dateDuVol->date(),
					dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute(),
					prixDuVol->value(),
					montantSubventionne,
					factureIdEnBdd,
					remarqueVol->text());

				subventionRestante = subventionRestante - montantSubventionne;
			}
			//Sinon on est balade ou sortie, on enregistre le vol avec la référence de balade/sortie
			else
			{
				qDebug() << "Id de la balade/sortie enregistrée" << choixBalade->currentData().toInt();
				db->enregistrerUnVolSortieOuBalade(idPilote,
					typeDeVol->currentText(),
					dateDuVol->date(),
					dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute(),
					prixDuVol->value(),
					montantSubventionne,
					factureIdEnBdd,
					choixBalade->currentData().toInt(),
					remarqueVol->text());

				//On met a jour la liste des vols balades/sorties dans l'onglet des recettes
				peuplerListeBaladesEtSorties();
			}

            statusBar()->showMessage(QString("Vol ")
                + typeDeVol->currentText() 
                + " du " 
                + dateDuVol->date().toString("dd/MM/yyyy") 
                + " (" 
                + dureeDuVol->time().toString("hh:mm")
                + "/"
                + QString::number(prixDuVol->value())
                + "€) ajouté. Montant subvention : "
                + QString::number(montantSubventionne)
                + "€ / Subvention entrainement restante : "
                + QString::number(subventionRestante)
                + "€");

            dureeDuVol->setTime(QTime::QTime(0, 0));
            prixDuVol->setValue(0);
            remarqueVol->clear();
        }
        else
        {
            statusBar()->showMessage("Erreur ajout");
        }
    }
}

void AeroDms::enregistrerUneRecette()
{
    QStringList volsCoches;
    //On verifie si au moins un vol est coché
    for (int i = 0; i < listeBaladesEtSorties->count(); i++)
    {
        if (listeBaladesEtSorties->item(i)->checkState() == Qt::Checked)
        {
            volsCoches.append(listeBaladesEtSorties->item(i)->data(Qt::DisplayRole).toString());
            qDebug() << listeBaladesEtSorties->item(i)->data(Qt::DisplayRole).toString();
        }
    }

    if (volsCoches.size() == 0)
    {
        QMessageBox::critical(this, "Vol non sélectionné", "La dépense doit être associée à au moins un vol. \nSélectionnez au moins un vol dans la partie gauche de la fenêtre.\n\nSaisie non prise en compte.");
    }
    else
    {
        db->ajouterUneRecetteAssocieeAVol( volsCoches,
                                           typeDeRecette->currentText(),
                                           intituleRecette->text(),
                                           montantRecette->value());
        statusBar()->showMessage("Recette ajoutee");
    }

}

float AeroDms::calculerCoutHoraire()
{
    return (prixDuVol->value() / (dureeDuVol->time().hour() * 60.0 + dureeDuVol->time().minute())) * 60.0;
}

void AeroDms::peuplerListesPilotes()
{
    AeroDmsTypes::ListePilotes pilotes = db->recupererPilotes();

    listeDeroulantePilote->clear();
    choixPilote->clear();

    listeDeroulantePilote->addItem("Tous", "*");
    choixPilote->addItem("", "aucun");
    choixPayeur->addItem("", "aucun");
    for (int i = 0; i < pilotes.size(); i++)
    {
        const AeroDmsTypes::Pilote pilote = pilotes.at(i);
        const QString nomPrenom = QString(pilote.prenom).append(" ").append(pilote.nom);
        listeDeroulantePilote->addItem(nomPrenom, pilote.idPilote);
        choixPilote->addItem(nomPrenom, pilote.idPilote);
        choixPayeur->addItem(nomPrenom, pilote.idPilote);
    }
}

void AeroDms::peuplerListeSorties()
{
    choixBalade->clear();
    choixBaladeFacture->clear();
    choixBalade->addItem("", -1);
    choixBaladeFacture->addItem("", -1);

    AeroDmsTypes::ListeSortie sorties;
    if (typeDeVol->currentText() == "Balade")
    {
        sorties = db->recupererListeBalade();
    }
    else if (typeDeVol->currentText() == "Sortie")
    {
        sorties = db->recupererListeSorties();
    }
    
    for (int i = 0; i < sorties.size(); i++)
    {
        AeroDmsTypes::Sortie sortie = sorties.at(i);
        sortie.nom.append(" ").append(sortie.date.toString("dd/MM/yyyy"));
        choixBalade->addItem(sortie.nom, sortie.id);
    }

    sorties = db->recupererListeDepensesPouvantAvoirUneFacture();
    for (int i = 0; i < sorties.size(); i++)
    {
        AeroDmsTypes::Sortie sortie = sorties.at(i);
        sortie.nom.append(" ").append(sortie.date.toString("dd/MM/yyyy"));
        choixBaladeFacture->addItem(sortie.nom, sortie.id);
    }

}

void AeroDms::peuplerListeBaladesEtSorties()
{
    listeBaladesEtSorties->clear();

    QStringList itemLabels = db->recupererBaladesEtSorties(typeDeRecette->currentText());
    QStringListIterator it(itemLabels);
    while (it.hasNext())
    {
        QListWidgetItem* itemBaladesEtSorties = new QListWidgetItem(it.next(), listeBaladesEtSorties);
        itemBaladesEtSorties->setCheckState(Qt::Unchecked);
        listeBaladesEtSorties->addItem(itemBaladesEtSorties);
    }
}

void AeroDms::prevaliderDonnnesSaisies()
{
    validerLeVol->setEnabled(true);

    qDebug() << choixPilote->currentIndex();

    if ( prixDuVol->value() == 0
         || dureeDuVol->time() == QTime::QTime(0,0)
         || choixPilote->currentIndex() == 0
         || pdfDocument->status() != QPdfDocument::Status::Ready
         || ( typeDeVol->currentText() != "Entrainement" && choixBalade->currentIndex() == 0))
    {
        validerLeVol->setEnabled(false);
    } 
}

void AeroDms::prevaliderDonnneesSaisiesRecette()
{
    validerLaRecette->setEnabled(true);

    if ( montantRecette->value() == 0 
         || intituleRecette->text() == "")
    {
        validerLaRecette->setEnabled(false);
    }
}

void AeroDms::changerInfosVolSurSelectionTypeVol()
{
    peuplerListeSorties();
    if (typeDeVol->currentText() == "Entrainement")
    {
        choixBalade->setEnabled(false);
        choixBalade->setCurrentIndex(0);
    }
    else if (typeDeVol->currentText() == "Balade")
    {
        choixBalade->setEnabled(false);
        choixBalade->setCurrentIndex(1);
    }
    else if (typeDeVol->currentText() == "Sortie")
    {
        choixBalade->setEnabled(true);
        choixBalade->setCurrentIndex(0);
    }
}

void AeroDms::ajouterUnPilote()
{
    dialogueGestionPilote->exec();
}

void AeroDms::ajouterUneSortie()
{
    dialogueAjouterSortie->exec();
}

void AeroDms::ajouterUneCotisation()
{
    dialogueAjouterCotisation->exec();
}


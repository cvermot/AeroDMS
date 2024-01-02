#include "PdfRenderer.h"

#include <QFile>
#include <QPrinter>

PdfRenderer::PdfRenderer()
{
    db = new ManageDb();
    view = new QWebEnginePage(this);
}

PdfRenderer::PdfRenderer(ManageDb *p_db, QWidget* parent)
{
    db = p_db;
	view = new QWebEnginePage(this);

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(chargementTermine(bool)));
    connect(view, SIGNAL(renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus, bool)), this, SLOT(statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus, bool)));
    connect(view, SIGNAL(pdfPrintingFinished(const QString&, bool)), this, SLOT(impressionTerminee(const QString&, bool)));

    //QUrl url = QUrl::fromLocalFile("C:/Users/cleme/OneDrive/Projets/HTML/COMPTA_2023.htm");
    //view->load(url);
}

void PdfRenderer::chargementTermine(bool retour)
{
    //qDebug() << "Chargement termine QWEV" << view->url() << retour;
    //view->save("C:/Users/cleme/source/AeroDms/AeroDms/testWV.html", QWebEngineDownloadRequest::SingleHtmlSaveFormat);
    QString nomFichier = QString("C:/Users/cleme/source/AeroDms/AeroDms/fomulaire_").append(QString::number(nombreFacturesTraitees + nombreCotisationsTraitees + nombreRecettesBaladeSortieTraitees + nombreRemboursementFacturesTraitees)).append(".pdf");
    view->printToPdf(nomFichier);
    listeDesFichiers.append(nomFichier);
}

void PdfRenderer::impressionTerminee(const QString& filePath, bool success)
{
    //qDebug() << "Impression termine QWEV" << filePath << success;
    db->ajouterDemandeCeEnBdd(demandeEnCours);
    emit mettreAJourNombreFactureTraitees(nombreFacturesATraiter, nombreFacturesTraitees);
    imprimerLaProchaineDemandeDeSubvention();
}

void PdfRenderer::statusDeChargementAVarie(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
{
    //qDebug() << "QWEV erreur" << terminationStatus << exitCode;
}

int PdfRenderer::imprimerLesDemandesDeSubvention()
{
    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();
    nombreFacturesATraiter = listeDesRemboursements.size();
    nombreFacturesTraitees = 0 ;
    nombreCotisationsTraitees = 0;
    nombreRecettesBaladeSortieTraitees = 0;
    nombreRemboursementFacturesTraitees = 0;
    listeDesFichiers.clear();
    emit mettreAJourNombreFactureTraitees(nombreFacturesATraiter, nombreFacturesTraitees);

    imprimerLaProchaineDemandeDeSubvention();
    return listeDesRemboursements.size();
}

void PdfRenderer::imprimerLaProchaineDemandeDeSubvention()
{
    //On ouvre le template et on met à jour les informations communes à toutes les demandes
    QFile f("./ressources/HTML/COMPTA_2023.htm");
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        templateCe = in.readAll();
    }
    else
    {
        qDebug() << "Erreur ouverture fichier";
    }
    QString templateCeTmp = templateCe;
    //Date de la demande
    templateCeTmp.replace("xxDate", QDate::currentDate().toString("dd/MM/yyyy"));
    //Investissement => toujours décoché, pour le moment non géré par le logiciel
    templateCeTmp.replace("xxI", "");
    //Signataire => toujours celui qui exécute le logiciel
    templateCeTmp.replace("xxSignataire", "Cl&eacute;ment VERMOT-DESROCHES");


    AeroDmsTypes::ListeDemandeRemboursement listeDesRemboursements = db->recupererLesSubventionsAEmettre();
    AeroDmsTypes::ListeRecette listeDesCotisations = db->recupererLesCotisationsAEmettre();
    AeroDmsTypes::ListeRecette listeDesRecettesBaladesSorties = db->recupererLesRecettesBaladesEtSortiesAEmettre();
    AeroDmsTypes::ListeDemandeRemboursementFacture listeDesRemboursementsFactures = db->recupererLesDemandesDeRembousementAEmettre();


    qDebug() << "Nb remboursement" << listeDesRemboursements.size() << "Nb cotisations" << listeDesCotisations.size() << "Nb balade sortie" << listeDesRecettesBaladesSorties.size() << "Nb factures" << listeDesRemboursementsFactures.size();

    //if (nombreFacturesTraitees < listeDesRemboursements.size())
    if (listeDesRemboursements.size() > 0)
    {    
        const AeroDmsTypes::DemandeRemboursement demande = listeDesRemboursements.at(0);
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");

        //Montant
        remplirLeChampMontant(templateCeTmp, demande.montantARembourser);

        //Cheque a retirer au CE par le demandeur => a cocher
        templateCeTmp.replace("zzC", "X");

        //Bénéficaire
        //L'aéroclub du pilote :
        templateCeTmp.replace("xxBeneficiaire", db->recupererAeroclub(demande.piloteId));

        //Observation
        QString observation = demande.typeDeVol;
        observation.append(" / ");
        observation.append(db->recupererNomPrenomPilote(demande.piloteId));
        templateCeTmp.replace("xxObservation", observation);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(demande.typeDeVol)) ;
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(demande.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        //On envoie le HTML en génération
        view->setHtml(templateCeTmp);

        QStringList facturesAssociees = db->recupererListeFacturesAssocieeASubvention(demande);
        listeDesFichiers.append(facturesAssociees);
        //qDebug() << "liste fichiers" << listeDesFichiers;

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_HEURE_DE_VOL;
        demandeEnCours.idPilote = demande.piloteId;
        demandeEnCours.annee = demande.annee;
        demandeEnCours.typeDeVol = demande.typeDeVol;
        demandeEnCours.nomBeneficiaire = db->recupererAeroclub(demande.piloteId);
        demandeEnCours.montant = demande.montantARembourser;

        nombreFacturesTraitees++;
    }
    //Recettes "Cotisations"
    else if (listeDesCotisations.size() > 0)
    //else if (nombreCotisationsTraitees < listeDesCotisations.size())
    {
        //AeroDmsTypes::Recette recette = listeDesCotisations.at(nombreCotisationsTraitees);
        const AeroDmsTypes::Recette recette = listeDesCotisations.at(0);
        //Depense
        templateCeTmp.replace("xxD", "");
        //Recette
        templateCeTmp.replace("xxR", "X");
        //Cheque a retirer au CE par le demandeur => non coché
        templateCeTmp.replace("zzC", "");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", "CSE Thales");
        //Montant
        remplirLeChampMontant(templateCeTmp, recette.montant);
        //Observation
        templateCeTmp.replace("xxObservation", recette.intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta("Cotisation"));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml(templateCeTmp);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_COTISATION;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = "Cotisation";
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;

        qDebug() << "Ajout cotisation" << nombreCotisationsTraitees << recette.annee << recette.intitule << listeDesCotisations.size();

        nombreCotisationsTraitees++;
    }
    //Recettes "passagers" des sorties et balades
    else if (nombreRecettesBaladeSortieTraitees < listeDesRecettesBaladesSorties.size())
    //else if (listeDesRecettesBaladesSorties.size() > 0)
    {
        //const AeroDmsTypes::Recette recette = listeDesRecettesBaladesSorties.at(0);
        const AeroDmsTypes::Recette recette = listeDesRecettesBaladesSorties.at(nombreRecettesBaladeSortieTraitees);
        //Depense
        templateCeTmp.replace("xxD", "");
        //Recette
        templateCeTmp.replace("xxR", "X");
        //Cheque a retirer au CE par le demandeur => non coché
        templateCeTmp.replace("zzC", "");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", "CSE Thales");

        //Montant
        remplirLeChampMontant(templateCeTmp, recette.montant);
        //Observation
        templateCeTmp.replace("xxObservation", QString("Participations ").append(recette.intitule));

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(recette.typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml(templateCeTmp);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = recette.intitule;
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;

        nombreRecettesBaladeSortieTraitees++;
    }
    //Factures payées par les pilotes à rembourser
    else if (nombreRemboursementFacturesTraitees < listeDesRemboursementsFactures.size())
    {
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");
        //Cheque a retirer au CE par le demandeur => coché
        templateCeTmp.replace("zzC", "X");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).payeur);

        //Montant
        remplirLeChampMontant(templateCeTmp, listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).montant);
        //Observation
        templateCeTmp.replace("xxObservation", listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml(templateCeTmp);
        listeDesFichiers.append(QString("C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesTraitees/").append(listeDesRemboursementsFactures.at(nombreRemboursementFacturesTraitees).nomFacture));

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_FACTURE;
        
        nombreRemboursementFacturesTraitees++;
    }
    else
    {
        //produireFichierPdfGlobal();
    }


}

void PdfRenderer::produireFichierPdfGlobal()
{
    QByteArray pdf;
    QPrinter printer;
    QFile *fichierSortie = new QFile("C:/Users/cleme/source/AeroDms/AeroDms/sortie.pdf");
    fichierSortie->open(QIODevice::WriteOnly);
    QPdfWriter *pdfWriter = new QPdfWriter(fichierSortie);
    pdfWriter->setPageSize(QPageSize(QPageSize::A4));
    pdfWriter->setResolution(300);
    pdfWriter->setPageOrientation(QPageLayout::Orientation::Portrait);
    //printer.setOrientation(QPrinter::);
    printer.setOutputFormat(QPrinter::PdfFormat);
    //printer.setPaperSize(QPrinter::A4);

    for (int i = 0; i < listeDesFichiers.size(); i++)
    {
        QFile fichier(listeDesFichiers.at(i));
        fichier.open(QIODevice::ReadOnly);
        QByteArray donneesFichier = fichier.readAll();
        QImage image = QImage::fromData(donneesFichier);
        if (image.isNull())
        {
            qDebug() << "Erreur chargement image";
        }
        else
        {
            pdfWriter->newPage();
            QPainter painter(pdfWriter);
            painter.drawImage(QPointF(0, 0), image);
            qDebug() << "Ajout";
        }
        
        //printer.
    }

    fichierSortie->close();

    /*QTemporaryFile temp;
    if (temp.open()) // causes file creation
        temp.close();
    printer.setOutputFileName(temp.fileName());
    // paint
    if (temp.open()) {
        pdf = temp.readAll();
    }*/
}

void PdfRenderer::remplirLeChampMontant(QString &p_html, const float p_montant)
{
    QString montantARembourser = QString::number(p_montant);
    QString partieEntiere;
    QString partieDecimale;
    if (montantARembourser.contains("."))
    {
        QStringList entier = montantARembourser.split(".");
        partieEntiere = entier.at(0);
        std::reverse(partieEntiere.begin(), partieEntiere.end());
        partieDecimale = entier.at(1);
        if (partieDecimale.size() == 1)
        {
            partieDecimale.append("0");
        }
    }
    else
    {
        partieEntiere = montantARembourser;
        std::reverse(partieEntiere.begin(), partieEntiere.end());
        partieDecimale = "00";
    }
    if (partieEntiere.size() >= 4)
    {
        p_html.replace("yyM", QString(partieEntiere.at(3)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyM", "");
    }
    if (partieEntiere.size() >= 3)
    {
        p_html.replace("yyC", QString(partieEntiere.at(2)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyC", "");
    }
    if (partieEntiere.size() >= 2)
    {
        p_html.replace("yyD", QString(partieEntiere.at(1)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyD", "");
    }
    if (partieEntiere.size() >= 1)
    {
        p_html.replace("yyU", QString(partieEntiere.at(0)).append("&nbsp;&nbsp;"));
    }
    else
    {
        p_html.replace("yyU", "0");
    }
    if (partieDecimale.size() >= 1)
    {
        p_html.replace("yyd", QString("&nbsp;&nbsp;").append(partieDecimale.at(0)));
    }
    else
    {
        p_html.replace("yyd", "0");
    }
    if (partieDecimale.size() >= 2)
    {
        p_html.replace("yyc", QString("&nbsp;&nbsp;").append(partieDecimale.at(1)));
    }
    else
    {
        p_html.replace("yyc", "0");
    }
}

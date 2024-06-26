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
#include "PdfRenderer.h"
#include <podofo/podofo.h>

#include <QFile>
#include <QPrinter>

PdfRenderer::PdfRenderer()
{
    db = new ManageDb();
    view = new QWebEnginePage(this);
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;
}

PdfRenderer::PdfRenderer( ManageDb *p_db, 
                          QString p_cheminTemplatesHtml, 
                          QWidget* parent)
{
    db = p_db;
	view = new QWebEnginePage(this);
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;

    ressourcesHtml = QUrl(QString("file:///%1/").arg(p_cheminTemplatesHtml));

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(chargementTermine(bool)));
    connect(view, SIGNAL(pdfPrintingFinished(const QString&, bool)), this, SLOT(impressionTerminee(const QString&, bool)));
}

void PdfRenderer::mergerPdf()
{
    PoDoFo::PdfMemDocument document;

    const QString nomFichier = cheminSortieFichiersGeneres
        + demandeEnCours.nomFichier;
    document.Load(nomFichier.toStdString());
    
    for (int i = 0; i < demandeEnCours.listeFactures.size(); i++)
    {
        const QString nomFacture = repertoireDesFactures
            + demandeEnCours.listeFactures.at(i);
        PoDoFo::PdfMemDocument facture;
        facture.Load(nomFacture.toStdString());
        document.GetPages().AppendDocumentPages(facture);
    }

    const QString appVersion = "AeroDMS v" + QApplication::applicationVersion();
    document.GetMetadata().SetCreator(PoDoFo::PdfString(appVersion.toStdString()));
    document.GetMetadata().SetAuthor(PoDoFo::PdfString(demandeEnCours.nomTresorier.toStdString()));
    document.GetMetadata().SetTitle(PoDoFo::PdfString(demandeEnCours.nomFichier.toStdString()));
    document.GetMetadata().SetSubject(PoDoFo::PdfString("Formulaire de demande de subvention"));

    document.SaveUpdate(nomFichier.toStdString());
}

QString PdfRenderer::numeroFichierSur3Digits()
{
    QString numero = QString::number(indiceFichier);
    if (numero.size() == 1)
    {
        numero = QString("00").append(numero);
    }
    else if (numero.size() == 2)
    {
        numero = QString("0").append(numero);
    }
    indiceFichier++;

    return numero;
}

void PdfRenderer::chargementTermine(bool retour)
{
    demandeEnCours.nomFichier = numeroFichierSur3Digits()
        +demandeEnCours.nomFichier
        +(".pdf");
    QString nomFichier = cheminSortieFichiersGeneres
        + demandeEnCours.nomFichier;

    //De base le format est portrait
    QPageLayout pageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()));
    if (demandeEnCours.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_RECAP_ANNUEL)
    {
        pageLayout.setOrientation(QPageLayout::Landscape);
    }

    view->printToPdf(nomFichier, pageLayout);
    listeDesFichiers.append(demandeEnCours.nomFichier);
    nombreEtapesEffectuees++;
    emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
}

void PdfRenderer::impressionTerminee( const QString& filePath, 
                                      bool success)
{
    mergerPdf();

    nombreEtapesEffectuees++;
    emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);

    db->ajouterDemandeCeEnBdd(demandeEnCours);

    //Si la demande ne concerne pas un document unique, on fait les demandes suivantes
    if (!laDemandeEstPourUnDocumentUnique)
    {
        imprimerLaProchaineDemandeDeSubvention();
    }
    //Sinon on emet directement la fin d'impression
    else
    {
        emit generationTerminee(cheminSortieFichiersGeneres);
    }
   
}

void PdfRenderer::imprimerLeRecapitulatifDesHeuresDeVol( const int p_annee,
                                                         const QString p_cheminSortieFichiersGeneres,
                                                         const QString p_cheminStockageFactures,
                                                         const AeroDmsTypes::Signature p_signature)
{
    nombreEtapesEffectuees = 0;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = true;

    demandeEnCours.typeDeSignatureDemandee = p_signature;
    demandeEnCours.mergerTousLesPdf = true;

    cheminSortieFichiersGeneres = QString(p_cheminSortieFichiersGeneres).append(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm"));
    repertoireDesFactures = p_cheminStockageFactures;
    QDir().mkdir(cheminSortieFichiersGeneres);
    if (QDir(cheminSortieFichiersGeneres).exists())
    {
        cheminSortieFichiersGeneres.append("/");
        listeDesFichiers.clear();

        emit mettreAJourNombreFacturesATraiter(3);
        emit mettreAJourNombreFacturesTraitees(0);

        const AeroDmsTypes::ListeSubventionsParPilotes listePilotesDeCetteAnnee = db->recupererSubventionsPilotes( p_annee,
                                                                                                                   "*",
                                                                                                                   false);
        const AeroDmsTypes::SubventionsParPilote totaux = db->recupererTotauxAnnuel(p_annee, false);
        imprimerLeFichierPdfDeRecapAnnuel(p_annee, listePilotesDeCetteAnnee, totaux, true);

        nombreEtapesEffectuees++;
        emit mettreAJourNombreFacturesTraitees(nombreEtapesEffectuees);
    }
    else
    {
        qDebug() << "Erreur création repertoire";
    }
}

void PdfRenderer::imprimerLesDemandesDeSubvention( const QString p_nomTresorier,
                                                   const QString p_cheminSortieFichiersGeneres,
                                                   const QString p_cheminStockageFactures,
                                                   const AeroDmsTypes::TypeGenerationPdf p_typeGenerationPdf,
                                                   const AeroDmsTypes::Signature p_signature,
                                                   const bool p_mergerTousLesPdf)
{
    demandeEnCours.listeFactures = QStringList();
    demandeEnCours.nomTresorier = p_nomTresorier;
    demandeEnCours.typeDeGenerationDemandee = p_typeGenerationPdf;
    demandeEnCours.typeDeSignatureDemandee = p_signature;
    demandeEnCours.mergerTousLesPdf = p_mergerTousLesPdf;

    nombreEtapesEffectuees = 0 ;
    indiceFichier = 0;
    laDemandeEstPourUnDocumentUnique = false;

    cheminSortieFichiersGeneres = QString(p_cheminSortieFichiersGeneres).append(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmm"));
    repertoireDesFactures = p_cheminStockageFactures;

    QDir().mkdir(cheminSortieFichiersGeneres);
    if(QDir(cheminSortieFichiersGeneres).exists())
    { 
        cheminSortieFichiersGeneres.append("/");
        listeDesFichiers.clear();
        int nombreEtapesAEffectuer = 0 ;

        if ( demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_TOUTES
             || demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT)
        {
            nombreEtapesAEffectuer = db->recupererLesCotisationsAEmettre().size() +
                                     db->recupererLesRecettesBaladesEtSortiesAEmettre().size();
        }
        if ( demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_TOUTES
             || demandeEnCours.typeDeGenerationDemandee == AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT)
        {
            listeAnnees = db->recupererAnneesAvecVolNonSoumis();
            nombreEtapesAEffectuer = nombreEtapesAEffectuer +
                                     db->recupererLesSubventionsAEmettre().size() +
                                     db->recupererLesDemandesDeRembousementAEmettre().size() +
                                     listeAnnees.size();
        }

        emit mettreAJourNombreFacturesATraiter(3*nombreEtapesAEffectuer);
        emit mettreAJourNombreFacturesTraitees(0);

        db->sauvegarderLaBdd(cheminSortieFichiersGeneres);

        imprimerLaProchaineDemandeDeSubvention();
    }
    else
    {
        qDebug() << "Erreur création repertoire";
    }
}

void PdfRenderer::imprimerLaProchaineDemandeDeSubvention()
{
    //On ouvre le template et on met à jour les informations communes à toutes les demandes
    QFile f(QString(ressourcesHtml.toLocalFile()).append("COMPTA_2023.htm"));
    QString templateCeTmp = "";
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        templateCeTmp = in.readAll();
    }
    else
    {
        qDebug() << "Erreur ouverture fichier";
    }
    //QString templateCeTmp = templateCe;
    //Date de la demande
    templateCeTmp.replace("xxDate", QDate::currentDate().toString("dd/MM/yyyy"));
    //Investissement => toujours décoché, pour le moment non géré par le logiciel
    templateCeTmp.replace("xxI", "");
    //Signataire => toujours celui qui exécute le logiciel
    templateCeTmp.replace("xxSignataire", demandeEnCours.nomTresorier);

    //On genere un fichier de recap de l'état des subventions déjà allouées avant les demandes que l'on va générer ensuite
    if (listeAnnees.size() > 0)
    {
        const int annee = listeAnnees.takeFirst();

        const AeroDmsTypes::ListeSubventionsParPilotes listePilotesDeCetteAnnee = db->recupererLesSubventionesDejaAllouees(annee);
        const AeroDmsTypes::SubventionsParPilote totaux = db->recupererTotauxAnnuel( annee, 
                                                                                     true );
        imprimerLeFichierPdfDeRecapAnnuel( annee, 
                                           listePilotesDeCetteAnnee, 
                                           totaux);
    }
    else if (db->recupererLesSubventionsAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT)
    {    
        const AeroDmsTypes::DemandeRemboursement demande = db->recupererLesSubventionsAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");

        //Montant
        remplirLeChampMontant(templateCeTmp, demande.montantARembourser);
        //Signature
        remplirLeChampSignature(templateCeTmp);

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
        view->setHtml( templateCeTmp, 
                       ressourcesHtml);

        QStringList facturesAssociees = db->recupererListeFacturesAssocieeASubvention(demande);
        listeDesFichiers.append(facturesAssociees);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_HEURE_DE_VOL;
        demandeEnCours.idPilote = demande.piloteId;
        demandeEnCours.annee = demande.annee;
        demandeEnCours.typeDeVol = demande.typeDeVol;
        demandeEnCours.nomBeneficiaire = db->recupererAeroclub(demande.piloteId);
        demandeEnCours.montant = demande.montantARembourser;
        demandeEnCours.nomFichier = ".HdV_"
            + (demande.typeDeVol) 
            + "_" 
            + (demande.piloteId) 
            + ("_") 
            + (QString::number(demande.annee));
        demandeEnCours.listeFactures = facturesAssociees;

    }
    //Recettes "Cotisations"
    else if (db->recupererLesCotisationsAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT )
    {
        const AeroDmsTypes::Recette recette = db->recupererLesCotisationsAEmettre().at(0);
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
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        templateCeTmp.replace("xxObservation", recette.intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta("Cotisation"));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_COTISATION;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = "Cotisation";
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;
        demandeEnCours.nomFichier = ".Cotisations_"
            +(QString::number(demandeEnCours.annee));
        demandeEnCours.listeFactures = QStringList();
    }
    //Recettes "passagers" des sorties et balades
    else if (db->recupererLesRecettesBaladesEtSortiesAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_DEPENSES_SEULEMENT )
    {
        const AeroDmsTypes::Recette recette = db->recupererLesRecettesBaladesEtSortiesAEmettre().at(0);
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
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        templateCeTmp.replace("xxObservation", QString("Participations ").append(recette.intitule));

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(recette.typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(recette.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE;
        demandeEnCours.annee = recette.annee;
        demandeEnCours.typeDeVol = recette.intitule;
        demandeEnCours.nomBeneficiaire = "CSE Thales";
        demandeEnCours.montant = recette.montant;
        demandeEnCours.nomFichier = ".RecetteBalades_"
            +(QString::number(demandeEnCours.annee));
        demandeEnCours.listeFactures = QStringList();
    }
    //Factures payées par les pilotes à rembourser
    else if ( db->recupererLesDemandesDeRembousementAEmettre().size() > 0
              && demandeEnCours.typeDeGenerationDemandee != AeroDmsTypes::TypeGenerationPdf_RECETTES_SEULEMENT )
    {
        const AeroDmsTypes::DemandeRemboursementFacture demandeRembousement = db->recupererLesDemandesDeRembousementAEmettre().at(0);
        //Depense
        templateCeTmp.replace("xxD", "X");
        //Recette
        templateCeTmp.replace("xxR", "");
        //Cheque a retirer au CE par le demandeur => coché
        templateCeTmp.replace("zzC", "X");
        //Bénéficaire : le CSE
        templateCeTmp.replace("xxBeneficiaire", demandeRembousement.payeur);

        //Montant
        remplirLeChampMontant(templateCeTmp, demandeRembousement.montant);
        //Signature
        remplirLeChampSignature(templateCeTmp);
        //Observation
        templateCeTmp.replace("xxObservation", demandeRembousement.intitule);

        //Année / Budget
        QString ligneBudget = QString::number(db->recupererLigneCompta(demandeRembousement.typeDeSortie));
        ligneBudget.append(" / ");
        ligneBudget.append(QString::number(demandeRembousement.annee));
        templateCeTmp.replace("xxLigneBudgetAnneeBudget", ligneBudget);

        view->setHtml( templateCeTmp,
                       ressourcesHtml);
        //recopierFacture(demandeRembousement.nomFacture);
        listeDesFichiers.append(QString(repertoireDesFactures).append(demandeRembousement.nomFacture));

        //On met à jour l'info de demande en cours, pour mettre à jour la base de données une fois le PDF généré
        demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_FACTURE;
        //L'annee n'est pas utile pour la facture => on fourni directement l'ID facture
        demandeEnCours.annee = demandeRembousement.id;
        demandeEnCours.typeDeVol = demandeRembousement.intitule;
        demandeEnCours.nomBeneficiaire = demandeRembousement.payeur;
        demandeEnCours.montant = demandeRembousement.montant;
        demandeEnCours.nomFichier = QString(".DemandeRemboursementFacturePayeeParPilote");
        demandeEnCours.listeFactures = QStringList(demandeRembousement.nomFacture);
    }
    else
    {
        if (demandeEnCours.mergerTousLesPdf)
        {
            produireFichierPdfGlobal();
        }
        
        emit generationTerminee(cheminSortieFichiersGeneres);
    }
    nombreEtapesEffectuees++;
}

void PdfRenderer::imprimerLeFichierPdfDeRecapAnnuel( const int p_annee, 
                                                     const AeroDmsTypes::ListeSubventionsParPilotes p_listePilotesDeCetteAnnee,
                                                     const AeroDmsTypes::SubventionsParPilote p_totaux,
                                                     const bool p_ajouterLesRecettes)
{
    QFile table(QString(ressourcesHtml.toLocalFile()).append("TableauRecap.html"));
    QFile tableItem(QString(ressourcesHtml.toLocalFile()).append("TableauRecapItem.html"));
    QFile tableRecettes(QString(ressourcesHtml.toLocalFile()).append("TableauRecapRecettes.html"));
    QString templateTable = "";
    QString templateTableItem = "";
    QString templateTableRecettes = "";
    if (table.open(QFile::ReadOnly | QFile::Text) 
        && tableItem.open(QFile::ReadOnly | QFile::Text)
        && tableRecettes.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream inTable(&table);
        QTextStream inTableItem(&tableItem);
        QTextStream inTableRecettes(&tableRecettes);
        templateTable = inTable.readAll();
        templateTableItem = inTableItem.readAll();
        templateTableRecettes = inTableRecettes.readAll();
    }
    else
    {
        qDebug() << "Erreur ouverture fichier";
    }
    templateTable.replace("__date__", QDate::currentDate().toString("dd/MM/yyyy"));
    templateTable.replace("__exercice__", QString::number(p_annee));

    for (int i = 0; i < p_listePilotesDeCetteAnnee.size(); i++)
    {
        QString item = templateTableItem;
        item.replace("__pilote__", QString(p_listePilotesDeCetteAnnee.at(i).nom).append(" ").append(p_listePilotesDeCetteAnnee.at(i).prenom));
        item.replace("__HdvEnt__", p_listePilotesDeCetteAnnee.at(i).entrainement.heuresDeVol);
        item.replace("__CouEnt__", QString::number(p_listePilotesDeCetteAnnee.at(i).entrainement.coutTotal, 'f', 2));
        item.replace("__SubEnt__", QString::number(p_listePilotesDeCetteAnnee.at(i).entrainement.montantRembourse, 'f', 2));
        item.replace("__HdVBal__", p_listePilotesDeCetteAnnee.at(i).balade.heuresDeVol);
        item.replace("__CouBal__", QString::number(p_listePilotesDeCetteAnnee.at(i).balade.coutTotal, 'f', 2));
        item.replace("__SubBal__", QString::number(p_listePilotesDeCetteAnnee.at(i).balade.montantRembourse, 'f', 2));
        item.replace("__HdVSor__", p_listePilotesDeCetteAnnee.at(i).sortie.heuresDeVol);
        item.replace("__CouSor__", QString::number(p_listePilotesDeCetteAnnee.at(i).sortie.coutTotal, 'f', 2));
        item.replace("__SubSor__", QString::number(p_listePilotesDeCetteAnnee.at(i).sortie.montantRembourse, 'f', 2));
        item.replace("__HdvTot__", p_listePilotesDeCetteAnnee.at(i).totaux.heuresDeVol);
        item.replace("__CouTot__", QString::number(p_listePilotesDeCetteAnnee.at(i).totaux.coutTotal, 'f', 2));
        item.replace("__SubTot__", QString::number(p_listePilotesDeCetteAnnee.at(i).totaux.montantRembourse, 'f', 2));

        templateTable.replace("<!--Accroche-->", item);
    }

    templateTable.replace("__TotHdvEnt__", p_totaux.entrainement.heuresDeVol);
    templateTable.replace("__TotCouEnt__", QString::number(p_totaux.entrainement.coutTotal, 'f', 2));
    templateTable.replace("__TotSubEnt__", QString::number(p_totaux.entrainement.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdVBal__", p_totaux.balade.heuresDeVol);
    templateTable.replace("__TotCouBal__", QString::number(p_totaux.balade.coutTotal, 'f', 2));
    templateTable.replace("__TotSubBal__", QString::number(p_totaux.balade.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdVSor__", p_totaux.sortie.heuresDeVol);
    templateTable.replace("__TotCouSor__", QString::number(p_totaux.sortie.coutTotal, 'f', 2));
    templateTable.replace("__TotSubSor__", QString::number(p_totaux.sortie.montantRembourse, 'f', 2));
    templateTable.replace("__TotHdvTot__", p_totaux.totaux.heuresDeVol);
    templateTable.replace("__TotCouTot__", QString::number(p_totaux.totaux.coutTotal, 'f', 2));
    templateTable.replace("__TotSubTot__", QString::number(p_totaux.totaux.montantRembourse, 'f', 2));

    if (p_ajouterLesRecettes)
    {
        templateTable.replace("<!--AccrocheRecette-->", templateTableRecettes);

        const AeroDmsTypes::TotauxRecettes totaux = db->recupererTotauxRecettes(p_annee);
        templateTable.replace("_RecetteCotisation_", QString::number(totaux.cotisations, 'f', 2) + " €");
        templateTable.replace("_RecetteBalade_", QString::number(totaux.balades, 'f', 2) + " €");
        templateTable.replace("_RecetteSortie_", QString::number(totaux.sorties, 'f', 2) + " €");
    }

    if ( demandeEnCours.mergerTousLesPdf
         && demandeEnCours.typeDeSignatureDemandee == AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY)
    {
        templateTable.replace("<!--Signature-->", "<p>[SignatureField#1]</p>");
    }

    view->setHtml(templateTable,
        ressourcesHtml);

    demandeEnCours.typeDeDemande = AeroDmsTypes::PdfTypeDeDemande_RECAP_ANNUEL;
    demandeEnCours.nomFichier = ".Recap_pilote_"
        +(QString::number(p_annee));
}

void PdfRenderer::produireFichierPdfGlobal()
{
    PoDoFo::PdfMemDocument document;

    const QString nomFichier = cheminSortieFichiersGeneres
        + numeroFichierSur3Digits()
        + ".FichiersAssembles.pdf";

    QDir dir(cheminSortieFichiersGeneres);
    const QStringList fichiers = dir.entryList(QStringList("*.pdf"), QDir::Files);
   
    for (int i = 0; i < fichiers.size(); i++)
    {
        const QString nomFacture = cheminSortieFichiersGeneres
            + fichiers.at(i);
        PoDoFo::PdfMemDocument facture;
        facture.Load(nomFacture.toStdString());
        document.GetPages().AppendDocumentPages(facture);
    }

    const QString appVersion = "AeroDMS v" + QApplication::applicationVersion();
    document.GetMetadata().SetCreator(PoDoFo::PdfString(appVersion.toStdString()));
    document.GetMetadata().SetAuthor(PoDoFo::PdfString(demandeEnCours.nomTresorier.toStdString()));
    document.GetMetadata().SetTitle(PoDoFo::PdfString(demandeEnCours.nomFichier.toStdString()));
    document.GetMetadata().SetSubject(PoDoFo::PdfString("Formulaire de demande de subvention"));
    qDebug() << "Fin merge";
    document.Save(nomFichier.toStdString());
}

void PdfRenderer::remplirLeChampMontant(QString &p_html, const float p_montant)
{
    QString montantARembourser = QString::number(p_montant, 'f', 2);
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

void PdfRenderer::remplirLeChampSignature(QString& p_html)
{
    //Signature => 
    switch (demandeEnCours.typeDeSignatureDemandee)
    {
        case AeroDmsTypes::Signature_MANUSCRITE_IMAGE:
        {
            //Le chemin vers le fichier
            QString cheminSignature = QCoreApplication::applicationDirPath() + "/ressources/signature.jpg";
            p_html.replace("xxSignature", "<img src=\""+ cheminSignature +"\" width=\"140\" />");
        }
        break;

        case AeroDmsTypes::Signature_NUMERIQUE_LEX_COMMUNITY:
        {
            if (demandeEnCours.mergerTousLesPdf)
            {
                //Si on est sur un PDF mergé, la signature "visible" réalisée par Lex Community
                //se trouvera sur la première page   
                if (QFile("./ressources/signature.jpg").exists())
                {
                    QString cheminSignature = QCoreApplication::applicationDirPath() + "/ressources/signature.jpg";
                    p_html.replace("xxSignature", "<font size=\"1\">Cachet de signature numérique<br/>en première page</font><br /><img src=\"" + cheminSignature + "\" width=\"140\" />");
                }
                else
                {
                    p_html.replace("xxSignature", "<font size=\"1\">Cachet de signature numérique<br/>en première page</font>");

                }
                
            }
            else
            {
                //Le tag réservé pour Lex Community
                p_html.replace("xxSignature", "[SignatureField#1]");
            }
            
        }
        break;

        case AeroDmsTypes::Signature_SANS:
        default:
        {
            //Vide si on signe à la main
            p_html.replace("xxSignature", "");
        }
        break;
    }
    
}

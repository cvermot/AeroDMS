/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2025 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#ifndef HEADER_MANAGEDB
#define HEADER_MANAGEDB

#include <QtGui>
#include <QtSql>

#include "AeroDmsTypes.h"

class ManageDb : public QWidget {
    Q_OBJECT

public:
    ManageDb(const QString& database, 
        const int p_delaisDeGardeBdd);

    void sauvegarderLaBdd( const QString p_repertoireDeSauvegarde);
    const bool laBddEstALaVersionAttendue() const;

    const AeroDmsTypes::ListePilotes recupererPilotes();
    const AeroDmsTypes::Pilote recupererPilote( const QString p_idPilote);
    const AeroDmsTypes::ListeAeroclubs recupererAeroclubs();
    const AeroDmsTypes::Club recupererAeroclub( int p_aeroclubId);
    const AeroDmsTypes::Club recupererInfosAeroclubDuPilote( const QString p_piloteId);
    const AeroDmsTypes::ListeAerodromes recupererAerodromes();

    const int recupererProchainNumeroFacture();
    const int ajouterFacture( QString& nomFichier);

    const float recupererSubventionRestante( const QString& piloteId,
        const int annee);

    void enregistrerUnVol( const QString& p_piloteId,
        const QString& p_typeDeVol,
        const QDate& p_date,
        const int p_dureeEnMinutes,
        const float p_cout,
        const float p_montantSubventionne,
        const int p_facture,
        const int p_idSortie,
        const QString& p_remarque,
        const QString& p_immat,
        const QString& p_activite,
        const int p_idVolAEditer);
    const bool supprimerUnVol(const int p_volAEditer);
    void enregistrerUneFacture( const QString& p_payeur,
        const int factureId,
        const QDate& p_date,
        const float p_montantFacture,
        const int p_idSortie,
        const QString& p_remarqueFacture);

    void ajouterUneRecetteAssocieeAVol( const QStringList& p_listeVols,
        const QString& p_typeDeRecette,
        const QString& p_intitule,
        const float p_montant);

    const bool piloteEstAJourDeCotisation( const QString& p_piloteId,
        const int annee);

    void ajouterCotisation (const AeroDmsTypes::CotisationAnnuelle &p_infosCotisation );
    const float recupererSubventionEntrainement( const QString p_pilote,
        const int p_annee);

    const AeroDmsTypes::ListeDemandeRemboursement recupererLesSubventionsAEmettre(const int p_annee);
    const AeroDmsTypes::ListeRecette recupererLesCotisationsAEmettre(const int p_annee);
    const AeroDmsTypes::ListeRecette recupererLesRecettesBaladesEtSortiesAEmettre(const int p_annee);
    const QList<QString> recupererListeRecettesNonSoumisesCse( const int p_annee,
        const QString p_typeRecette);
    const AeroDmsTypes::ListeDemandeRemboursementFacture recupererLesDemandesDeRembousementAEmettre(const int p_annee);
    const AeroDmsTypes::ListeDemandeRemboursementFacture recupererToutesLesDemandesDeRemboursement(const int p_annee = -1);
    const AeroDmsTypes::ListeStatsHeuresDeVolParActivite recupererHeuresParActivite(const int p_annee = -1, const int p_option = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS);
    const AeroDmsTypes::ListeRecetteDetail recupererRecettesCotisations(const int p_annee = -1);
    const AeroDmsTypes::ListeRecetteDetail recupererRecettesHorsCotisation(const int p_annee = -1);
    const AeroDmsTypes::TotauxRecettes recupererTotauxRecettes(const int p_annee);
    const AeroDmsTypes::ListeSubventionsParPilotes recupererSubventionsPilotes( const int p_annee,
        const QString p_piloteId,
        const int p_options,
        const bool p_volsSoumisUniquement);
    const AeroDmsTypes::SubventionsParPilote recupererTotauxAnnuel( const int p_annee,
        const bool p_volsSoumisUniquement = false);
    const AeroDmsTypes::ListeSubventionsParPilotes recupererLesSubventionesDejaAllouees(const int annee);
    const AeroDmsTypes::ListeVols recupererVols( const int p_annee = -1, 
        const QString p_piloteId = "*");
    const AeroDmsTypes::Vol recupererVol(const int p_idVol);
    
    const AeroDmsTypes::ListeDemandesRemboursementSoumises recupererDemandesRemboursementSoumises( const int p_annee,
        const QString p_piloteId);

    const QStringList recupererListeFacturesAssocieeASubvention(const AeroDmsTypes::DemandeRemboursement p_demande);
    const QStringList recupererListeActivites();

    const QStringList recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette = false);

    const AeroDmsTypes::ListeVolSortieOuBalade recupererBaladesEtSorties( const QString p_typeDeVol, 
        const float p_proportionRemboursement,
        const int p_annee = -1 );

    const AeroDmsTypes::ListeAeronefs recupererListeAeronefs();
    const QList<int> recupererAnnees();
    const QList<int> recupererAnneesAvecVolNonSoumis(const int p_annee);
    const AeroDmsTypes::ListeSortie recupererListeSorties();
    const AeroDmsTypes::ListeSortie recupererListeBalade();
    const AeroDmsTypes::ListeSortie recupererListeDepensesPouvantAvoirUneFacture();
    const QString recupererAeroclub(const QString p_piloteId);
    const QString recupererNomPrenomPilote(const QString p_piloteId);
    const QString recupererActivitePrincipale(const QString p_piloteId);
    const QString recupererNomFacture(const int p_volId);

    const QString piloteExiste(const QString p_nom, const QString p_prenom);

    const int recupererLigneCompta(const QString p_typeDeRecetteDepenseId);
    
    const AeroDmsTypes::ResultatCreationBdd creerPilote( const AeroDmsTypes::Pilote p_pilote);
    const AeroDmsTypes::ResultatCreationBdd creerAeroclub(const AeroDmsTypes::Club p_aeroclub);
    const AeroDmsTypes::Status mettreAJourAerodrome(const QString p_indicatifOaci, 
        const QString p_nom);
    void creerSortie(const AeroDmsTypes::Sortie p_sortie);

    void ajouterDemandeCeEnBdd(const AeroDmsTypes::DemandeEnCoursDeTraitement p_demande);
    void ajouterNoteSubvention(const int p_idSubventionAAnoter, const QString p_note);

    const AeroDmsTypes::ListeStatsHeuresDeVol recupererHeuresMensuelles(const int p_annee = -1,
        const int p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS);
    const QString recupererMailPilotes( const int p_annee,
        const AeroDmsTypes::MailPilotes p_mailingDemande = AeroDmsTypes::MailPilotes_AYANT_COTISE);
    const AeroDmsTypes::StatsAeronefs recupererStatsAeronefs(const int p_annee,
        const int p_options);
    const QList<QDate> recupererDatesDesDemandesDeSubventions();
    const AeroDmsTypes::ListeAerodromes recupererAerodromesAvecPilotesActifs(const AeroDmsTypes::DonnesMailingType p_demande);
    const QString recupererMailDerniereDemandeDeSubvention(const QString p_date = "");
    const QString recupererMailPilotesDUnAerodrome(const QString p_codeOaci, 
        const AeroDmsTypes::DonnesMailingType p_demande);

    const AeroDmsTypes::ListeDetailsBaladesEtSorties recupererListeDetailsBaladesEtSorties(const int p_annee);

    const AeroDmsTypes::StatsPilotes recupererStatsPilotes();

    void mettreAJourDonneesAeronefs( const QString p_immatAeronefAMettreAJour,
        const QString p_nouvelleValeur,
        const AeroDmsTypes::AeronefTableElement p_donneeAMettreAJour);

    const bool volSembleExistantEnBdd(const QString p_idPilote,
        const int p_duree, 
        const QString p_date, 
        const float p_coutDuVol);

public slots:

private:
    QSqlDatabase db;
    int delaisDeGardeBdd = 0;
    const float versionBddAttendue = 1.11;

    const AeroDmsTypes::Club depilerRequeteAeroclub(const QSqlQuery p_query);
    const AeroDmsTypes::Vol depilerRequeteVol(const QSqlQuery p_query,
        const bool p_avecFactureEtSortie = true);
    static const QString genererClauseFiltrageActivite(const int p_options);
    void executerRequeteAvecControle(QSqlQuery& p_query,
        const QString p_nomRequete,
        const QString p_texteDetailErreur);
};

#endif

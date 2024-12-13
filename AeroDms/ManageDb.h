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
#ifndef HEADER_MANAGEDB
#define HEADER_MANAGEDB

#include <QtGui>
#include <QtSql>

#include "AeroDmsTypes.h"

class ManageDb : public QWidget {
    Q_OBJECT

public:
    ManageDb(const QString& database, const int p_delaisDeGardeBdd);

    void sauvegarderLaBdd(const QString p_repertoireDeSauvegarde);
    const bool laBddEstALaVersionAttendue() const;

    QString getLastError();
    AeroDmsTypes::ListePilotes recupererPilotes();
    AeroDmsTypes::Pilote recupererPilote(const QString p_idPilote);
    AeroDmsTypes::ListeAeroclubs recupererAeroclubs();
    AeroDmsTypes::Club recupererAeroclub(int p_aeroclubId);

    int recupererProchainNumeroFacture();
    int ajouterFacture(QString& nomFichier);

    float recupererSubventionRestante( const QString& piloteId, 
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
    bool supprimerUnVol(const int p_volAEditer);
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

    bool piloteEstAJourDeCotisation( const QString& p_piloteId, 
                                     const int annee);

    void ajouterCotisation (const AeroDmsTypes::CotisationAnnuelle &p_infosCotisation );
    float recupererSubventionEntrainement( const QString p_pilote, 
                                           const int p_annee);

    AeroDmsTypes::ListeDemandeRemboursement recupererLesSubventionsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesCotisationsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesRecettesBaladesEtSortiesAEmettre();
    QList<QString> recupererListeRecettesNonSoumisesCse( const int p_annee,
                                                         const QString p_typeRecette);
    AeroDmsTypes::ListeDemandeRemboursementFacture recupererLesDemandesDeRembousementAEmettre();
    AeroDmsTypes::ListeDemandeRemboursementFacture recupererToutesLesDemandesDeRemboursement(const int p_annee = -1);
    AeroDmsTypes::ListeStatsHeuresDeVolParActivite recupererHeuresParActivite(const int p_annee = -1, const int p_option = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS);
    AeroDmsTypes::ListeRecetteDetail recupererRecettesCotisations(const int p_annee = -1);
    AeroDmsTypes::ListeRecetteDetail recupererRecettesHorsCotisation(const int p_annee = -1);
    AeroDmsTypes::TotauxRecettes recupererTotauxRecettes(const int p_annee);
    AeroDmsTypes::ListeSubventionsParPilotes recupererSubventionsPilotes( const int p_annee = -1, 
        const QString p_piloteId = "*",
        const int p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
        const bool p_volsSoumisUniquement = false);
    AeroDmsTypes::SubventionsParPilote recupererTotauxAnnuel( const int p_annee,
                                                              const bool p_volsSoumisUniquement = false);
    AeroDmsTypes::ListeSubventionsParPilotes recupererLesSubventionesDejaAllouees(const int annee);
    AeroDmsTypes::ListeVols recupererVols( const int p_annee = -1, 
                                           const QString p_piloteId = "*");
    AeroDmsTypes::Vol recupererVol(const int p_idVol);
    AeroDmsTypes::Vol depilerRequeteVol( const QSqlQuery p_query,
                                         const bool p_avecFactureEtSortie = true);
    AeroDmsTypes::ListeDemandesRemboursementSoumises recupererDemandesRemboursementSoumises( const int p_annee,
                                                                                             const QString p_piloteId);

    QStringList recupererListeFacturesAssocieeASubvention(const AeroDmsTypes::DemandeRemboursement p_demande);
    QStringList recupererListeActivites();

    QStringList recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette = false);

    AeroDmsTypes::ListeVolSortieOuBalade recupererBaladesEtSorties( const QString p_typeDeVol, 
                                                                    const float p_proportionRemboursement,
                                                                    const int p_annee = -1 );

    AeroDmsTypes::ListeAeronefs recupererListeAeronefs();
    QList<int> recupererAnnees();
    QList<int> recupererAnneesAvecVolNonSoumis();
    AeroDmsTypes::ListeSortie recupererListeSorties();
    AeroDmsTypes::ListeSortie recupererListeBalade();
    AeroDmsTypes::ListeSortie recupererListeDepensesPouvantAvoirUneFacture();
    QString recupererAeroclub(const QString p_piloteId);
    QString recupererNomPrenomPilote(const QString p_piloteId);
    QString recupererActivitePrincipale(const QString p_piloteId);
    QString recupererNomFacture(const int p_volId);

    const QString piloteExiste(const QString p_nom, const QString p_prenom);

    int recupererLigneCompta(const QString p_typeDeRecetteDepenseId);
    
    AeroDmsTypes::ResultatCreationBdd creerPilote( const AeroDmsTypes::Pilote p_pilote);
    AeroDmsTypes::ResultatCreationBdd creerAeroclub(const AeroDmsTypes::Club p_aeroclub);
    void creerSortie(const AeroDmsTypes::Sortie p_sortie);

    void ajouterDemandeCeEnBdd(const AeroDmsTypes::DemandeEnCoursDeTraitement p_demande) const;

    AeroDmsTypes::ListeStatsHeuresDeVol recupererHeuresMensuelles(const int p_annee = -1,
        const int p_options = AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS);
    QString recupererMailPilotes( const int p_annee,
                                  const AeroDmsTypes::MailPilotes p_mailingDemande = AeroDmsTypes::MailPilotes_AYANT_COTISE);
    const AeroDmsTypes::StatsAeronefs recupererStatsAeronefs(const int p_annee,
        const int p_options);
    QList<QDate> recupererDatesDesDemandesDeSubventions();
    QString recupererMailDerniereDemandeDeSubvention(const QString p_date = "");

    AeroDmsTypes::ListeDetailsBaladesEtSorties recupererListeDetailsBaladesEtSorties(const int p_annee);

    const AeroDmsTypes::StatsPilotes recupererStatsPilotes();

    void mettreAJourDonneesAeronefs( const QString p_immatAeronefAMettreAJour,
                                     const QString p_nouvelleValeur,
                                     const AeroDmsTypes::AeronefTableElement p_donneeAMettreAJour);

    bool volSembleExistantEnBdd(const QString p_idPilote, const int p_duree, const QString p_date, const float p_coutDuVol);

    static QString genererClauseFiltrageActivite(const int p_options);

public slots:

private:
    QSqlDatabase db;
    int delaisDeGardeBdd = 0;
    const float versionBddAttendue = 1.9;
};

#endif

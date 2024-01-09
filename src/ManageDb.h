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
    ManageDb(const QString& database);
    ManageDb();

    QString getLastError();
    AeroDmsTypes::ListePilotes recupererPilotes();
    AeroDmsTypes::Pilote recupererPilote(const QString p_idPilote);

    int recupererProchainNumeroFacture();
    int ajouterFacture(QString& nomFichier);

    float recupererSubventionRestante( const QString& piloteId, 
                                       const int annee);

    void enregistrerUnVolDEntrainement( const QString &p_piloteId, 
                                        const QString &typeDeVol,
                                        const QDate &p_date, 
                                        const int p_dureeEnMinutes,
                                        const float p_cout, 
                                        const float p_montantSubventionne, 
                                        const int p_facture,
                                        const QString& p_remarque);
    void enregistrerUnVolSortieOuBalade( const QString& p_piloteId,
                                         const QString& p_typeDeVol,
                                         const QDate& p_date,
                                         const int p_dureeEnMinutes,
                                         const float p_cout,
                                         const float p_montantSubventionne,
                                         const int p_facture,
                                         const int p_idSortie,
                                         const QString& p_remarque);
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

    AeroDmsTypes::ListeDemandeRemboursement recupererLesSubventionsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesCotisationsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesRecettesBaladesEtSortiesAEmettre();
    AeroDmsTypes::ListeDemandeRemboursementFacture recupererLesDemandesDeRembousementAEmettre();
    AeroDmsTypes::ListeSubventionsParPilotes recupererSubventionsPilotes( const int p_annee = -1, 
                                                                          const QString p_piloteId = "*",
                                                                          const bool p_volsSoumisUniquement = false);
    AeroDmsTypes::SubventionsParPilote recupererTotauxAnnuel( const int p_annee,
                                                              const bool p_volsSoumisUniquement = false);
    AeroDmsTypes::ListeSubventionsParPilotes recupererLesSubventionesDejaAllouees(const int annee);
    AeroDmsTypes::ListeVols recupererVols( const int p_annee = -1, 
                                           const QString p_piloteId = "*");

    QStringList recupererListeFacturesAssocieeASubvention(const AeroDmsTypes::DemandeRemboursement p_demande);

    QStringList recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette = false);

    QStringList recupererBaladesEtSorties(const QString p_typeDeVol);
    QList<int> recupererAnnees();
    QList<int> recupererAnneesAvecVolNonSoumis();
    AeroDmsTypes::ListeSortie recupererListeSorties();
    AeroDmsTypes::ListeSortie recupererListeBalade();
    AeroDmsTypes::ListeSortie recupererListeDepensesPouvantAvoirUneFacture();
    QString recupererAeroclub(const QString p_piloteId);
    QString recupererNomPrenomPilote(const QString p_piloteId);

    int recupererLigneCompta(const QString p_typeDeRecetteDepenseId);
    
    AeroDmsTypes::ResultatCreationPilote creerPilote(const AeroDmsTypes::Pilote p_pilote);
    void creerSortie(const AeroDmsTypes::Sortie p_sortie);

    void ajouterDemandeCeEnBdd(const AeroDmsTypes::DemandeEnCoursDeTraitement p_demande);

    QString convertirMinutesEnHeuresMinutes(const int minutes);
    

public slots:

private:
    QSqlDatabase db;

};

#endif

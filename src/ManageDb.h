/******************************************************************************\
<QUas : a Free Software logbook for UAS operators>
Copyright (C) 2023 Clément VERMOT-DESROCHES (clement@vermot.net)

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

    void ajouterCotisation ( AeroDmsTypes::CotisationAnnuelle &p_infosCotisation );

    AeroDmsTypes::ListeDemandeRemboursement recupererLesSubventionsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesCotisationsAEmettre();
    AeroDmsTypes::ListeRecette recupererLesRecettesBaladesEtSortiesAEmettre();
    AeroDmsTypes::ListeDemandeRemboursementFacture recupererLesDemandesDeRembousementAEmettre();

    QStringList recupererListeFacturesAssocieeASubvention(const  AeroDmsTypes::DemandeRemboursement p_demande);

    QStringList recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette = false);

    QStringList recupererBaladesEtSorties(QString p_typeDeVol);
    AeroDmsTypes::ListeSortie recupererListeSorties();
    AeroDmsTypes::ListeSortie recupererListeBalade();
    AeroDmsTypes::ListeSortie recupererListeDepensesPouvantAvoirUneFacture();
    QString recupererAeroclub(QString p_piloteId);
    QString recupererNomPrenomPilote(QString p_piloteId);

    int recupererLigneCompta(QString p_typeDeRecetteDepenseId);
    
    AeroDmsTypes::ResultatCreationPilote creerPilote(AeroDmsTypes::Pilote p_pilote);
    void creerSortie(AeroDmsTypes::Sortie p_sortie);

    void ajouterDemandeCeEnBdd(AeroDmsTypes::DemandeEnCoursDeTraitement p_demande);
    

public slots:

private:
    QSqlDatabase db;

};

#endif

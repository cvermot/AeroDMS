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

#ifndef AERODMSTYPES_H
#define AERODMSTYPES_H

#include <QtWidgets>

class AeroDmsTypes
{
public:

    enum PiloteTableElement {
        PiloteTableElement_NOM = 0x0,
        PiloteTableElement_PRENOM = 0x1,
        PiloteTableElement_ANNEE = 0x2,
        PiloteTableElement_HEURES_ENTRAINEMENT_SUBVENTIONNEES = 0x3,
        PiloteTableElement_MONTANT_ENTRAINEMENT_SUBVENTIONNE = 0x4,
        PiloteTableElement_HEURES_BALADES_SUBVENTIONNEES = 0x5,
        PiloteTableElement_MONTANT_BALADES_SUBVENTIONNE = 0x6,
        PiloteTableElement_HEURES_SORTIES_SUBVENTIONNEES = 0x7,
        PiloteTableElement_MONTANT_SORTIES_SUBVENTIONNE = 0x8,
        PiloteTableElement_NB_COLONNES = 0x9,
    };

    enum VolTableElement {
        VolTableElement_PILOTE = 0x0,
        VolTableElement_DATE = 0x1,
        VolTableElement_TYPE_DE_VOL = 0x2,
        VolTableElement_DUREE = 0x3,
        VolTableElement_COUT = 0x4,
        VolTableElement_SUBVENTION = 0x5,
        VolTableElement_SOUMIS_CE = 0x6,
        VolTableElement_REMARQUE = 0x7,
        VolTableElement_NB_COLONNES = 0x8,
    };

    enum ResultatCreationPilote {
        ResultatCreationPilote_SUCCES = 0x0,
        ResultatCreationPilote_PILOTE_EXISTE = 0x1,
        ResultatCreationPilote_AUTRE = 0x2
    };

    enum PdfTypeDeDemande {
        PdfTypeDeDemande_HEURE_DE_VOL,
        PdfTypeDeDemande_COTISATION,
        PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE,
        PdfTypeDeDemande_FACTURE
    };

    struct DemandeEnCoursDeTraitement {
        PdfTypeDeDemande typeDeDemande;
        QString idPilote;
        QString typeDeVol;
        QString nomBeneficiaire;
        float montant;
        int annee;
        QString nomTresorier;
        //int idFacture;
    };

    struct Pilote {
        QString idPilote;
        QString nom;
        QString prenom;
        QString aeroclub;
        bool estAyantDroit;
        QString mail;
        QString telephone;
        QString remarque;
    };
    typedef QList<Pilote> ListePilotes;

    struct HeureDeVolRemboursement
    {
        float montantRembourse;
        float coutTotal;
        QString heuresDeVol;
    };
    struct SubventionsParPilote {
        QString idPilote;
        int annee;
        QString nom;
        QString prenom;
        QString aeroclub;
        HeureDeVolRemboursement entrainement;
        HeureDeVolRemboursement balade;
        HeureDeVolRemboursement sortie;
        HeureDeVolRemboursement totaux;
    };
    typedef QList<SubventionsParPilote> ListeSubventionsParPilotes;

    struct Vol {
        QString idPilote;
        QDate date;
        QString nomPilote;
        QString prenomPilote;
        QString typeDeVol;
        QString duree;
        QString remarque;
        QString estSoumisCe;
        float coutVol;
        float montantRembourse;
    };
    typedef QList<Vol> ListeVols;

    struct DemandeRemboursement {
        QString typeDeVol;
        QString piloteId;
        float montantARembourser;
        int annee;
        QString nomFichierFacture;
    };
    typedef QList<DemandeRemboursement> ListeDemandeRemboursement;

    struct Recette {
        QString intitule;
        QString typeDeSortie;
        float montant;
        int annee;
    };
    typedef QList<Recette> ListeRecette;

    struct Sortie {
        int id;
        QString nom;
        QDate date;
    };
    typedef QList<Sortie> ListeSortie;

    struct CotisationAnnuelle {
        QString idPilote;
        int annee;
        float montantSubvention;
        float montant;
    };
    
    struct DemandeRemboursementFacture {
        int id;
        QString intitule;
        float montant;
        QString payeur;
        QString nomSortie;
        QString typeDeSortie;
        int annee;
        QString nomFacture;
    };
    typedef QList<DemandeRemboursementFacture> ListeDemandeRemboursementFacture;

    struct ParametresMetier {
        float montantSubventionEntrainement;
        float montantCotisationPilote;
        float proportionRemboursementEntrainement;
        float proportionRemboursementBalade;
        float plafondHoraireRemboursementEntrainement;
        QString nomTresorier;
    };

};

#endif // AERODMSTYPES_H

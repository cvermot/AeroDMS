#ifndef AERODMSTYPES_H
#define AERODMSTYPES_H

#include <QtWidgets>

class AeroDmsTypes
{
public:

    enum FlightTableElement {
        FlightTableElement_DATE = 0x0,
        FlightTableElement_DRONE = 0x1,
        FlightTableElement_IMMAT = 0x2,
        FlightTableElement_DUREE = 0x3,
        FlightTableElement_SCENARIO = 0x4,
        FlightTableElement_OBSERVATION = 0x5,
        FlightTableElement_PILOTE = 0x6,
        FlightTableElement_NIGHT_FLIGHT = 0x7,
        FlightTableElement_MAX = 0x8
    };

    enum ResultatCreationPilote {
        ResultatCreationPilote_SUCCES = 0x0,
        ResultatCreationPilote_PILOTE_EXISTE = 0x1,
        ResultatCreationPilote_AUTRE = 0x2
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

    struct DemandeRemboursement {
        QString typeDeVol;
        QString piloteId;
        float montantARembourser;
        int annee;
        QString nomFichierFacture;
    };
    typedef QList<DemandeRemboursement> ListeDemandeRemboursement;

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

};

#endif // AERODMSTYPES_H

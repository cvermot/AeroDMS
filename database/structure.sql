--
-- File generated with SQLiteStudio v3.4.4 on ven. nov. 22 13:46:11 2024
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: activite
CREATE TABLE IF NOT EXISTS activite (nom TEXT PRIMARY KEY UNIQUE NOT NULL);
INSERT INTO activite (nom) VALUES ('Avion');
INSERT INTO activite (nom) VALUES ('ULM');
INSERT INTO activite (nom) VALUES ('Planeur');
INSERT INTO activite (nom) VALUES ('Helicoptère');
INSERT INTO activite (nom) VALUES ('Avion électrique');

-- Table: aeronef
CREATE TABLE IF NOT EXISTS aeronef (
    immatriculation TEXT PRIMARY KEY
                         UNIQUE,
    type            TEXT DEFAULT Inconnu
                         NOT NULL
);
INSERT INTO aeronef (immatriculation, type) VALUES (NULL, 'Inconnu');

-- Table: cotisation
CREATE TABLE IF NOT EXISTS cotisation (cotisationId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, pilote TEXT REFERENCES pilote (piloteId) NOT NULL, annee INTEGER NOT NULL, montantSubventionAnnuelleEntrainement REAL, idRecette REFERENCES recettes (recetteId) UNIQUE NOT NULL);

-- Table: demandeRemboursementSoumises
CREATE TABLE IF NOT EXISTS demandeRemboursementSoumises (demandeId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, dateDemande TEXT NOT NULL, montant REAL NOT NULL, nomBeneficiaire TEXT NOT NULL, typeDeDemande TEXT REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId) NOT NULL);

-- Table: facturesSorties
CREATE TABLE IF NOT EXISTS facturesSorties (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, sortie INTEGER REFERENCES sortie (sortieId) NOT NULL, facture INTEGER REFERENCES fichiersFacture (factureId) NOT NULL, date TEXT, montant REAL NOT NULL, intitule TEXT, payeur TEXT NOT NULL REFERENCES pilote (piloteId), demandeRemboursement NUMERIC REFERENCES demandeRemboursementSoumises (demandeId));

-- Table: fichiersFacture
CREATE TABLE IF NOT EXISTS fichiersFacture (factureId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, nomFichier TEXT UNIQUE NOT NULL);
INSERT INTO fichiersFacture (factureId, nomFichier) VALUES (0, 'FactureFictivePourInit');

-- Table: parametres
CREATE TABLE IF NOT EXISTS parametres (nom TEXT PRIMARY KEY NOT NULL UNIQUE, info1 TEXT, info2 TEXT, info3 TEXT);
INSERT INTO parametres (nom, info1, info2, info3) VALUES ('versionBdd', '1.7', NULL, NULL);

-- Table: pilote
CREATE TABLE IF NOT EXISTS pilote (piloteId TEXT PRIMARY KEY UNIQUE NOT NULL, nom TEXT NOT NULL, prenom TEXT NOT NULL, aeroclub TEXT NOT NULL, estAyantDroit INTEGER NOT NULL, mail TEXT, telephone TEXT, remarque TEXT, activitePrincipale TEXT REFERENCES activite (nom) NOT NULL, estActif NUMERIC NOT NULL DEFAULT (1), estBrevete NUMERIC NOT NULL DEFAULT (1));

-- Table: recettes
CREATE TABLE IF NOT EXISTS recettes (recetteId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, typeDeRecette TEXT NOT NULL REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId), Intitule TEXT NOT NULL, montant REAL NOT NULL, identifiantFormulaireSoumissionCe INTEGER REFERENCES demandeRemboursementSoumises (demandeId));

-- Table: sortie
CREATE TABLE IF NOT EXISTS sortie (sortieId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, nom TEXT NOT NULL, date TEXT, typeDeDepense TEXT REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId) NOT NULL DEFAULT Sortie);
INSERT INTO sortie (sortieId, nom, date, typeDeDepense) VALUES (1, 'Balade', NULL, 'Balade');
INSERT INTO sortie (sortieId, nom, date, typeDeDepense) VALUES (2, 'Fonctionnement', NULL, 'Fonctionnement');

-- Table: typeDeRecetteDepense
CREATE TABLE IF NOT EXISTS typeDeRecetteDepense (typeDeRecetteDepenseId TEXT PRIMARY KEY UNIQUE NOT NULL, identifiantCompta INTEGER NOT NULL, estRecette INTEGER, estDepense INTEGER, estVol NUMERIC NOT NULL);
INSERT INTO typeDeRecetteDepense (typeDeRecetteDepenseId, identifiantCompta, estRecette, estDepense, estVol) VALUES ('Balade', 1, 1, 1, 1);
INSERT INTO typeDeRecetteDepense (typeDeRecetteDepenseId, identifiantCompta, estRecette, estDepense, estVol) VALUES ('Sortie', 2, 1, 1, 1);
INSERT INTO typeDeRecetteDepense (typeDeRecetteDepenseId, identifiantCompta, estRecette, estDepense, estVol) VALUES ('Entrainement', 3, 0, 1, 1);
INSERT INTO typeDeRecetteDepense (typeDeRecetteDepenseId, identifiantCompta, estRecette, estDepense, estVol) VALUES ('Cotisation', 4, 1, 0, 0);
INSERT INTO typeDeRecetteDepense (typeDeRecetteDepenseId, identifiantCompta, estRecette, estDepense, estVol) VALUES ('Fonctionnement', 6, 0, 1, 0);

-- Table: vol
CREATE TABLE IF NOT EXISTS vol (volId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, typeDeVol TEXT REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId) NOT NULL, pilote TEXT REFERENCES pilote (piloteId) NOT NULL, date TEXT NOT NULL, duree INTEGER NOT NULL, cout REAL NOT NULL, montantRembourse REAL NOT NULL, facture INTEGER NOT NULL REFERENCES fichiersFacture (factureId), activite TEXT REFERENCES activite (nom) NOT NULL, sortie INTEGER REFERENCES sortie (sortieId), demandeRemboursement INTEGER REFERENCES demandeRemboursementSoumises (demandeId), remarque TEXT, immatriculation TEXT REFERENCES aeronef (immatriculation));

-- Table: xAssociationRecette-Vol
CREATE TABLE IF NOT EXISTS "xAssociationRecette-Vol" (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE, recetteId INTEGER REFERENCES recettes (recetteId) NOT NULL, volId INTEGER REFERENCES vol (volId) NOT NULL);

-- View: cotisationsASoumettreCe
CREATE VIEW IF NOT EXISTS cotisationsASoumettreCe AS SELECT cotisation.pilote,
       cotisation.annee,
       cotisation.cotisationId,
       cotisation.idRecette,
       pilote.nom,
       recettes.montant,
       recettes.Intitule
FROM cotisation
     INNER JOIN pilote ON cotisation.pilote = pilote.piloteId
     INNER JOIN recettes ON recettes.recetteId = cotisation.idRecette
WHERE 
      recettes.identifiantFormulaireSoumissionCe IS NULL
ORDER BY cotisation.annee;

-- View: demandesRembousementVolsSoumises
CREATE VIEW IF NOT EXISTS demandesRembousementVolsSoumises AS SELECT demandeId, 
dateDemande, 
nomBeneficiaire, 
montant, 
typeDeDemande, 
strftime('%Y', vol.date) AS anneeVol,
SUM(vol.montantRembourse) AS totalRembourse, 
SUM(vol.cout) AS totalCoutVol, 
vol.pilote, 
pilote.nom, pilote.prenom 
FROM demandeRemboursementSoumises
LEFT JOIN vol ON demandeRemboursementSoumises.demandeId = vol.demandeRemboursement
INNER JOIN pilote ON vol.pilote = pilote.piloteId
WHERE volId NOT NULL
GROUP BY demandeRemboursementSoumises.demandeId;

-- View: detailsBaladesEtSorties
CREATE VIEW IF NOT EXISTS detailsBaladesEtSorties AS SELECT vol.volId AS volId,
sortie.sortieId AS idSortie,
recettes.recetteId As idRecette,
vol.typeDeVol AS typeVol,
vol.date AS dateVol,
vol.duree AS dureeVol,
vol.cout AS coutVol,
vol.montantRembourse AS montantVolRembourse,
vol.remarque AS nomPassagers,
strftime('%Y', vol.date) AS annee,
recettes.Intitule AS intituleRecette,
recettes.montant AS montantRecette,
sortie.nom AS nomSortie
FROM vol
LEFT JOIN "xAssociationRecette-Vol" ON "xAssociationRecette-Vol".volId = vol.volId
LEFT JOIN recettes ON "xAssociationRecette-Vol".recetteId = recettes.recetteId
LEFT JOIN demandeRemboursementSoumises ON recettes.identifiantFormulaireSoumissionCe = demandeRemboursementSoumises.demandeId
LEFT JOIN sortie ON vol.sortie = sortie.sortieId
WHERE vol.typeDeVol != "Entrainement"
ORDER BY idSortie, volId, idRecette;

-- View: factures
CREATE VIEW IF NOT EXISTS factures AS SELECT 
facturesSorties.id, 
facturesSorties.intitule, 
facturesSorties.montant, 
pilote.nom, 
pilote.prenom, 
sortie.nom AS nomSortie, 
sortie.typeDeDepense ,
facturesSorties.date,
strftime('%Y', facturesSorties.date) AS annee,
fichiersFacture.nomFichier AS nomFacture,
facturesSorties.demandeRemboursement
FROM facturesSorties
INNER JOIN pilote ON facturesSorties.payeur = pilote.piloteId
INNER JOIN sortie ON facturesSorties.sortie = sortie.sortieId
INNER JOIN fichiersFacture ON facturesSorties.facture = fichiersFacture.factureId;

-- View: facturesARembourser
CREATE VIEW IF NOT EXISTS facturesARembourser AS SELECT 
facturesSorties.id, 
facturesSorties.intitule, 
facturesSorties.montant, 
pilote.nom, 
pilote.prenom, 
sortie.nom AS nomSortie, 
sortie.typeDeDepense ,
strftime('%Y', facturesSorties.date) AS annee,
fichiersFacture.nomFichier AS nomFacture
FROM facturesSorties
INNER JOIN pilote ON facturesSorties.payeur = pilote.piloteId
INNER JOIN sortie ON facturesSorties.sortie = sortie.sortieId
INNER JOIN fichiersFacture ON facturesSorties.facture = fichiersFacture.factureId
WHERE facturesSorties.demandeRemboursement IS NULL;

-- View: mailParDateDeDemandeDeSubvention
CREATE VIEW IF NOT EXISTS mailParDateDeDemandeDeSubvention AS SELECT dateDemande, mail, pilote
FROM demandeRemboursementSoumises 
INNER JOIN vol ON demandeRemboursementSoumises.demandeId = vol.demandeRemboursement
INNER JOIN pilote on vol.pilote = pilote.piloteId
GROUP BY mail, dateDemande
ORDER BY date DESC;

-- View: recettesASoumettreCe
CREATE VIEW IF NOT EXISTS recettesASoumettreCe AS SELECT recettes.typeDeRecette,
       recettes.montant,
       vol.date,
       vol.volId,
       strftime('%Y', vol.date) AS annee,
       vol.duree,
       vol.montantRembourse,
       vol.cout,
       recettes.recetteId,
       recettes.Intitule,
       sortie.nom,
       sortie.date
FROM recettes
INNER JOIN "xAssociationRecette-Vol" ON "xAssociationRecette-Vol".recetteId = recettes.recetteId
INNER JOIN vol ON "xAssociationRecette-Vol".volId = vol.volId
INNER JOIN sortie ON vol.sortie = sortie.sortieId
WHERE recettes.identifiantFormulaireSoumissionCe IS NULL
GROUP BY recettes.recetteId;

-- View: recettesASoumettreCeParTypeEtParAnnee
CREATE VIEW IF NOT EXISTS recettesASoumettreCeParTypeEtParAnnee AS SELECT typeDeRecette, 
annee, 
SUM(montant),
nom
FROM recettesASoumettreCe
GROUP BY annee, typeDeRecette, nom;

-- View: recettesCotisations
CREATE VIEW IF NOT EXISTS recettesCotisations AS SELECT cotisation.pilote,
       cotisation.annee,
       cotisation.cotisationId,
       cotisation.idRecette,
       pilote.nom,
       recettes.recetteId,
       recettes.montant,
       recettes.Intitule,
       recettes.identifiantFormulaireSoumissionCe
FROM cotisation
     INNER JOIN pilote ON cotisation.pilote = pilote.piloteId
     INNER JOIN recettes ON recettes.recetteId = cotisation.idRecette
ORDER BY cotisation.annee;

-- View: recettesHorsCotisations
CREATE VIEW IF NOT EXISTS recettesHorsCotisations AS SELECT
    recettes.recetteId, 
    typeDeRecette, 
    Intitule, 
    montant, 
    strftime('%Y', vol.date) AS annee,
    vol.date,
    recettes.identifiantFormulaireSoumissionCe
FROM recettes
INNER JOIN "xAssociationRecette-Vol" ON "xAssociationRecette-Vol".recetteId = recettes.recetteId
INNER JOIN vol ON "xAssociationRecette-Vol".volId = vol.volId
GROUP BY recettes.recetteId;

-- View: stats_aeronefs
CREATE VIEW IF NOT EXISTS stats_aeronefs AS SELECT 
    vol.immatriculation,
    aeronef.type,
    vol.activite,
    strftime('%Y', vol.date) AS annee,
    SUM(vol.duree) AS tempsDeVol
FROM vol
INNER JOIN aeronef ON vol.immatriculation = aeronef.immatriculation
GROUP BY vol.immatriculation, annee
ORDER BY annee, activite, type, vol.immatriculation;

-- View: stats_heuresDeVolParMois
CREATE VIEW IF NOT EXISTS stats_heuresDeVolParMois AS SELECT 
strftime('%m', vol.date) AS mois,
strftime('%Y', vol.date) AS annee,
SUM(vol.duree) as tempsDeVol,
typeDeVol
FROM vol
GROUP BY annee, mois, typeDeVol
ORDER BY annee, mois;

-- View: stats_heuresDeVolParPilote
CREATE VIEW IF NOT EXISTS stats_heuresDeVolParPilote AS SELECT 
vol.pilote,
pilote.nom,
pilote.prenom,
strftime('%Y', vol.date) AS annee,
SUM(vol.duree) as tempsDeVol
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
GROUP BY pilote, annee
ORDER BY pilote, annee;

-- View: stats_heuresDeVolParPiloteEtParActivite
CREATE VIEW IF NOT EXISTS stats_heuresDeVolParPiloteEtParActivite AS SELECT 
vol.pilote,
pilote.nom,
pilote.prenom,
vol.activite,
strftime('%Y', vol.date) AS annee,
SUM(vol.duree) AS tempsDeVol,
SUM(vol.cout) AS coutVol,
SUM(vol.montantRembourse) AS subventionVol
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
GROUP BY activite, pilote, annee
ORDER BY nom, prenom, annee;

-- View: subventionEntrainementAlloueeParPiloteEtParAnnee
CREATE VIEW IF NOT EXISTS subventionEntrainementAlloueeParPiloteEtParAnnee AS SELECT strftime('%Y', date) AS annee, 
pilote, 
SUM(montantRembourse) AS subventionAllouee 
FROM vol 
WHERE typeDeVol = 'Entrainement' 
GROUP BY strftime('%Y', date), 
pilote;

-- View: volARembourserParFacture
CREATE VIEW IF NOT EXISTS volARembourserParFacture AS SELECT 
vol.typeDeVol, 
vol.pilote, 
SUM(vol.montantRembourse) AS montantARembourser, 
strftime('%Y', vol.date) AS annee, 
vol.facture, 
fichiersFacture.nomFichier 
FROM vol 
INNER JOIN fichiersFacture ON vol.facture = fichiersFacture.factureId 
WHERE vol.demandeRemboursement IS NULL
GROUP BY 
typeDeVol, 
pilote, 
strftime('%Y', date), facture;

-- View: volARembourserParTypeParPiloteEtParAnnee
CREATE VIEW IF NOT EXISTS volARembourserParTypeParPiloteEtParAnnee AS SELECT 
volARembourserParFacture.typeDeVol, 
volARembourserParFacture.pilote, 
volARembourserParFacture.annee,
SUM(volARembourserParFacture.montantARembourser) AS montantARembourser
FROM volARembourserParFacture
GROUP BY volARembourserParFacture.typeDeVol, 
volARembourserParFacture.pilote,
volARembourserParFacture.annee;

-- View: volParTypeParAnEtParPilote
CREATE VIEW IF NOT EXISTS volParTypeParAnEtParPilote AS SELECT 
    vol.pilote,
    vol.typeDeVol,
    pilote.nom,
    pilote.prenom,
    pilote.aeroclub,
    strftime('%Y', vol.date) AS annee,
    SUM(vol.montantRembourse) AS montantRembourse,
    SUM(vol.cout) AS cout,
    SUM(vol.duree) AS tempsDeVol
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
GROUP BY vol.pilote, vol.typeDeVol, annee
ORDER BY annee, vol.pilote, typeDeVol;

-- View: volParTypeParAnEtParPiloteSoumis
CREATE VIEW IF NOT EXISTS volParTypeParAnEtParPiloteSoumis AS SELECT 
    vol.pilote,
    vol.typeDeVol,
    pilote.nom,
    pilote.prenom,
    pilote.aeroclub,
    strftime('%Y', vol.date) AS annee,
    SUM(vol.montantRembourse) AS montantRembourse,
    SUM(vol.cout) AS cout,
    SUM(vol.duree) AS tempsDeVol
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
WHERE vol.demandeRemboursement IS NOT NULL
GROUP BY vol.pilote, vol.typeDeVol, annee
ORDER BY annee, vol.pilote, typeDeVol;

-- View: vols
CREATE VIEW IF NOT EXISTS vols AS SELECT 
    vol.volId,
    vol.date,
    vol.pilote,
    pilote.nom,
    pilote.prenom,
    vol.typeDeVol,
    vol.montantRembourse,
    vol.cout,
    vol.duree,
    vol.remarque,
    vol.immatriculation,
    vol.demandeRemboursement,
    vol.activite
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId;

-- View: volsBaladesEtSorties
CREATE VIEW IF NOT EXISTS volsBaladesEtSorties AS SELECT 
    vol.volId,
    vol.typeDeVol,
    sortie.nom || ' - ' 
    || pilote.nom || ' ' || pilote.prenom  || ' - ' 
    || strftime('%d/%m/%Y', vol.date) 
    || ' (Durée : ' || CAST(vol.duree/60 AS text) || 'h' || printf("%02d",vol.duree%60) || ') -' 
    || ifnull(vol.remarque,'Sans remarque') || '-' AS NomVol,
    vol.cout,
    strftime('%Y', vol.date) AS annee
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
INNER JOIN sortie ON vol.sortie = sortie.sortieId
WHERE vol.typeDeVol != 'Entrainement';

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;

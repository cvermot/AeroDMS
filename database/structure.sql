--
-- File generated with SQLiteStudio v3.4.4 on ven. déc. 29 22:32:16 2023
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: cotisation
CREATE TABLE IF NOT EXISTS cotisation (cotisationId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, pilote TEXT REFERENCES pilote (piloteId) NOT NULL, annee INTEGER NOT NULL, montantSubventionAnnuelleEntrainement REAL, idRecette REFERENCES recettes (recetteId) UNIQUE NOT NULL);

-- Table: demandeRemboursementSoumises
CREATE TABLE IF NOT EXISTS demandeRemboursementSoumises (demandeId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, piloteId TEXT REFERENCES pilote (piloteId), dateDemande TEXT NOT NULL, montant REAL NOT NULL, nomBeneficiaire TEXT NOT NULL, typeDeDemande TEXT REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId) NOT NULL);

-- Table: facturesSorties
CREATE TABLE IF NOT EXISTS facturesSorties (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, sortie INTEGER REFERENCES sortie (sortieId) NOT NULL, facture INTEGER REFERENCES fichiersFacture (factureId) NOT NULL, date TEXT, montant REAL NOT NULL, intitule TEXT, payeur TEXT NOT NULL REFERENCES pilote (piloteId), demandeRemboursement NUMERIC REFERENCES demandeRemboursementSoumises (demandeId));

-- Table: fichiersFacture
CREATE TABLE IF NOT EXISTS fichiersFacture (factureId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, nomFichier TEXT UNIQUE NOT NULL);

-- Table: pilote
CREATE TABLE IF NOT EXISTS pilote (piloteId TEXT PRIMARY KEY UNIQUE NOT NULL, nom TEXT NOT NULL, prenom TEXT NOT NULL, aeroclub TEXT NOT NULL, estAyantDroit INTEGER NOT NULL, mail TEXT, telephone TEXT, remarque TEXT);

-- Table: recettes
CREATE TABLE IF NOT EXISTS recettes (recetteId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, typeDeRecette TEXT NOT NULL REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId), Intitule TEXT NOT NULL, montant REAL NOT NULL, identifiantFormulaireSoumissionCe INTEGER REFERENCES demandeRemboursementSoumises (demandeId));

-- Table: sortie
CREATE TABLE IF NOT EXISTS sortie (sortieId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, nom TEXT NOT NULL, date TEXT);

-- Table: typeDeRecetteDepense
CREATE TABLE IF NOT EXISTS typeDeRecetteDepense (typeDeRecetteDepenseId TEXT PRIMARY KEY UNIQUE NOT NULL, identifiantCompta INTEGER NOT NULL, estRecette INTEGER, estDepense INTEGER, estVol NUMERIC NOT NULL);

-- Table: vol
CREATE TABLE IF NOT EXISTS vol (volId INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, typeDeVol TEXT REFERENCES typeDeRecetteDepense (typeDeRecetteDepenseId) NOT NULL, pilote TEXT REFERENCES pilote (piloteId) NOT NULL, date TEXT NOT NULL, duree INTEGER NOT NULL, cout REAL NOT NULL, montantRembourse REAL NOT NULL, facture INTEGER NOT NULL REFERENCES fichiersFacture (factureId), sortie INTEGER REFERENCES sortie (sortieId), demandeRemboursement INTEGER REFERENCES demandeRemboursementSoumises (demandeId), remarque TEXT);

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

-- View: recettesASoumettreCe
CREATE VIEW IF NOT EXISTS recettesASoumettreCe AS SELECT recettes.typeDeRecette,
       recettes.montant,
       vol.date,
       vol.volId,
       strftime('%Y', vol.date) AS annee,
       vol.duree,
       vol.montantRembourse,
       vol.cout,
       recettes.recetteId
FROM recettes
INNER JOIN "xAssociationRecette-Vol" ON "xAssociationRecette-Vol".recetteId = recettes.recetteId
INNER JOIN vol ON "xAssociationRecette-Vol".volId = vol.volId
WHERE recettes.identifiantFormulaireSoumissionCe IS NULL
GROUP BY recettes.recetteId;

-- View: recettesASoumettreCeParTypeEtParAnnee
CREATE VIEW IF NOT EXISTS recettesASoumettreCeParTypeEtParAnnee AS SELECT typeDeRecette, annee, SUM(montant) FROM recettesASoumettreCe
GROUP BY annee, typeDeRecette;

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

-- View: volsBaladesEtSorties
CREATE VIEW IF NOT EXISTS volsBaladesEtSorties AS SELECT 
    vol.volId,
    vol.typeDeVol,
    sortie.nom || ' - ' 
    || pilote.nom || ' ' || pilote.prenom  || ' - ' 
    || strftime('%d/%m/%Y', vol.date) 
    || ' (Durée : ' || CAST(vol.duree/60 AS text) || 'h' || printf("%02d",vol.duree%60) || ') -' 
    || ifnull(vol.remarque,'Sans remarque') || '-' AS NomVol
FROM vol
INNER JOIN pilote ON vol.pilote = pilote.piloteId
INNER JOIN sortie ON vol.sortie = sortie.sortieId
WHERE vol.typeDeVol != 'Entrainement';

COMMIT TRANSACTION;
PRAGMA foreign_keys = on;

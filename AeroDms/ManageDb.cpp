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

#include "ManageDb.h"
#include "QMessageBox"
#include "AeroDmsServices.h"

ManageDb::ManageDb(const int p_delaisDeGardeBdd, 
    const QString p_nomDuVerrou,
    GestionnaireDonneesEnLigne *p_gestionnaireDonneesEnLigne)
{
    delaisDeGardeBdd = p_delaisDeGardeBdd;
    gestionnaireDonneesEnLigne = p_gestionnaireDonneesEnLigne;
    nomDuVerrou = p_nomDuVerrou;

    connect(gestionnaireDonneesEnLigne, SIGNAL(receptionSha256Bdd(QString)), this, SLOT(comparerSha256BddLocale(QString)));
    connect(gestionnaireDonneesEnLigne, SIGNAL(baseDeDonneesTelechargeeEtDisponible()), this, SLOT(prendreEnCompteBddTelechargee()));
    connect(gestionnaireDonneesEnLigne, SIGNAL(finDEnvoiBdd()), this, SLOT(rechargerBddSuiteEnvoi()));
    
}

void ManageDb::comparerSha256BddLocale(QString p_sha256BaseEnLigne)
{
    if (p_sha256BaseEnLigne != recupererShaSumBdd())
    {
        emit signalerDebutTelechargementBdd();
        gestionnaireDonneesEnLigne->telechargerBdd();
        //On demande le téléchargement de la base distante
        //On laisse le logiciel en lecture seule pendant la mise à jour de la base
        //et à l'issue on vérifiera le lock
    }
    else
    {
        //base distante et locale identique => on ne retélécharge pas
        //mais on emet le signal signalerChargementBaseSuiteTelechargement()
        //pour repasse l'IHM en lecture-ecriture
        emit signalerChargementBaseSuiteTelechargement();
        gererVerrouBdd();
    }
}

void ManageDb::gererVerrouBdd()
{
    //Le verrou est la ligne portant le nom "lock" dans la table parametres
    // Ses options sont :
    //      -info1 : nom du propriétaire du lock, issu du champ trésorier
    //      -info2 : date du lock initial
    //      -info3 : date de dernière mise à jour du lock
    
    //Si pas de verrou dans la base de données (ou si le verrou est le notre), 
    //on positionne (met à jour) le verrou
    //Si verrou dans la base de données et que le le verrou n'est pas le notre, 
    //on positionne le logiciel en lecture seule et on demande l'affichage 
    //d'un message explicatif

    emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_PRISE_VERROU);

    QSqlQuery query;
    query.prepare("SELECT * FROM parametres WHERE nom = 'lock'");
    query.exec();
    query.next();

    if (query.value("info1").toString() == "")
    {
        //Non verrouillé : on prend
        query.prepare("UPDATE 'parametres' SET 'info1' = :nomVerrou,'info2' = :dateVerrouInitial,'info3' = :derniereDateVerrou WHERE nom = 'lock'");
        query.bindValue(":nomVerrou", nomDuVerrou);
        query.bindValue(":dateVerrouInitial", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"));
        query.bindValue(":derniereDateVerrou", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"));
        query.exec();
        //On demande l'envoi de la BDD en ligne avec le verrou
        demanderEnvoiBdd();
    }
    else
    {
        //Un verrou existe : on vérifie si c'est le notre :
        if (query.value("info1").toString() == nomDuVerrou)
        {
            //Il faut le mettre à jour puis envoyer la BDD
            // => mise à jour assurée par demanderEnvoiBdd
            demanderEnvoiBdd();
        }
        else
        {
            // Ce n'est pas le notre : on notifie à l'utilisateur et on passe en lecture seule
            const QDateTime heureVerrouInitial = query.value("info2").toDateTime();
            const QDateTime heureDerniereVerrou = query.value("info3").toDateTime();
            emit signalerBddBloqueeParUnAutreUtilisateur(query.value("info1").toString(), 
                heureVerrouInitial, 
                heureDerniereVerrou);
        }
    }
}

void ManageDb::libererVerrouBdd()
{
    emit notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_PRISE_VERROU);

    //On ne libère le verrou que si c'est le notre

    QSqlQuery query;
    query.prepare("SELECT * FROM parametres WHERE nom = 'lock'");
    query.exec();
    query.next();

    //On verifie si c'est notre verrou
    if (query.value("info1").toString() == nomDuVerrou)
    {
        emit passerLogicielEnLectureSeuleDurantEnvoiBdd();

        //C'est le notre : on le libère
        query.prepare("UPDATE 'parametres' SET 'info1' = NULL, 'info2' = NULL, 'info3' = NULL WHERE nom = 'lock'");
        query.exec();
        //On demande l'envoi de la BDD en ligne avec le verrou à jour
        QThread::msleep(delaisDeGardeBdd);
        db.close();

        QThread::msleep(delaisDeGardeBdd);
        envoiTerminalAvantFermetureLogiciel = true;
        gestionnaireDonneesEnLigne->envoyerBdd(db.databaseName());

        QTimer::singleShot(10000, this, &ManageDb::reitererEnvoiBddSurFermeture);
    }
}

void ManageDb::reitererEnvoiBddSurFermeture()
{
    gestionnaireDonneesEnLigne->rincerListeDAttente();
    gestionnaireDonneesEnLigne->envoyerBdd(db.databaseName());

    QTimer::singleShot(10000, this, &ManageDb::reitererEnvoiBddSurFermeture);
}

void ManageDb::demanderEnvoiBdd()
{
    if (gestionnaireDonneesEnLigne->estActif())
    {
        emit passerLogicielEnLectureSeuleDurantEnvoiBdd();

        QSqlQuery query;
        query.prepare("UPDATE 'parametres' SET 'info3' = :derniereDateVerrou WHERE nom = 'lock'");
        query.bindValue(":derniereDateVerrou", QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"));
        query.exec();

        QThread::msleep(delaisDeGardeBdd);
        db.close();

        gestionnaireDonneesEnLigne->envoyerBdd(db.databaseName());
    }  
}

void ManageDb::rechargerBddSuiteEnvoi()
{
    if (!envoiTerminalAvantFermetureLogiciel)
    {
        ouvrirBdd();
    }
    emit sortirDuModeLectureSeule();
}

const QStringList ManageDb::recupererListeFichiersPdfFactures()
{
    QSqlQuery query;
    query.prepare("SELECT nomFichier FROM fichiersFacture WHERE nomFichier != 'FactureFictivePourInit'");
    query.exec();

    QStringList listeFactures;
    
    while (query.next())
    {
        listeFactures.append(query.value("nomFichier").toString());
    }

    return listeFactures;
}

void ManageDb::prendreEnCompteBddTelechargee()
{
    notifierEtapeChargementBdd(AeroDmsTypes::EtapeChargementBdd_DEMANDE_TELECHARGEMENT_PRISE_EN_COMPTE_BDD_TELECHARGEE);

    db.close();

    const QString nomSauvegardeBdd = db.databaseName().replace("/AeroDMS.sqlite", "/AeroDMS"+QDateTime::currentDateTime().toString("_yyyy-MM-dd_hhmm") + ".sqlite");

    QFile gestionnaireDeFichier;
    //On sauvegarde l'ancienne base locale
    gestionnaireDeFichier.copy(db.databaseName(),
        nomSauvegardeBdd);
    //On remet la nouvelle base fraichement téléchargée
    const QString nomBddTelechargee = db.databaseName().replace("/AeroDMS.sqlite", "/AeroDMS_new.sqlite");
    gestionnaireDeFichier.remove(db.databaseName());
    gestionnaireDeFichier.rename(nomBddTelechargee,
        db.databaseName());

    if (ouvrirBdd())
    {
        //On signale le chargement d'une nouvelle BDD à destination de l'IHM notamment
        emit signalerChargementBaseSuiteTelechargement();

        gererVerrouBdd();
    }

    //On fait un peu de ménage parmis les bases de données sauvegardées. On va garder les 25 dernières
    QDir repertoireBdd(db.databaseName().replace("/AeroDMS.sqlite", ""));
    QStringList filtreSqlite;
    filtreSqlite << "*.sqlite";
    repertoireBdd.setNameFilters(filtreSqlite);
    QStringList listeBddSqlite = repertoireBdd.entryList(QDir::Files);
    if (listeBddSqlite.indexOf("AeroDMS.sqlite") != -1)
    {
        listeBddSqlite.remove(listeBddSqlite.indexOf("AeroDMS.sqlite"));
    }

    while (listeBddSqlite.size() > 25)
    {
        const QString nomBddASupprimer = db.databaseName().replace("AeroDMS.sqlite", listeBddSqlite.at(0));
        gestionnaireDeFichier.remove(nomBddASupprimer);

        listeBddSqlite.removeAt(0);
    }
}

bool ManageDb::ouvrirLaBdd(const QString& p_database)
{
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        emit erreurOuvertureBdd();
        QMessageBox::critical(
            this,
            QApplication::applicationName() + " - " + tr("Impossible de charger la base de données"),
            tr("Ce logiciel nécessite le driver Qt SQLite. Celui ci n'est pas disponible.")
        );
        return false;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(p_database);

    return ouvrirBdd();
}

const bool ManageDb::ouvrirBdd()
{
    bool bddEstOuverte = false;
    int nbTentatives = 0;

    while (!bddEstOuverte
        && nbTentatives < 3)
    {
        if (nbTentatives != 0)
            QThread::msleep(delaisDeGardeBdd);
        nbTentatives++;
        bddEstOuverte = db.open();
    }

    if (!bddEstOuverte)
    {
        emit erreurOuvertureBdd();
        QMessageBox::critical(this,
            QApplication::applicationName() + " - " + tr("Impossible d'ouvrir la base de données"),
            tr("Je ne parvient pas à ouvrir la base de données car l'erreur suivante s'est produite : ") + "\n"
            + db.lastError().text()
            + "\n" + tr("Cliquez Annuler pour quitter"), QMessageBox::Cancel);
    }

    return bddEstOuverte;
}

void ManageDb::sauvegarderLaBdd(const QString p_repertoireDeSauvegarde)
{
    db.close();

    QFile gestionnaireDeFichier;
    gestionnaireDeFichier.copy(db.databaseName(), p_repertoireDeSauvegarde + "AeroDms.sqlite");

    ouvrirBdd();
}

const QString ManageDb::recupererShaSumBdd()
{
    QFile file(db.databaseName());
    if (!file.open(QIODevice::ReadOnly)) {
        return QString(); // Retourne une chaîne vide en cas d'erreur d'ouverture
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    // Lecture du fichier par morceaux pour éviter les problèmes de mémoire avec les gros fichiers
    while (!file.atEnd()) {
        hash.addData(file.read(8192)); // Lecture de 8 KB à la fois
    }
    file.close();

    // Convertir le hash en QString hexadécimal
    return QString(hash.result().toHex());
}

void ManageDb::lireParametres(AeroDmsTypes::ParametresMetier &p_parametresMetiers, 
    AeroDmsTypes::ParametresSysteme &p_parametresSysteme)
{
    QSqlQuery query;

    query.prepare("SELECT * FROM parametres WHERE nom = 'autoriserReglementParVirement'");
    query.exec();
    if (query.next())
    {
        p_parametresSysteme.autoriserReglementParVirement = query.value("info1").toBool();
    }
    else
    {
        p_parametresSysteme.autoriserReglementParVirement = false;
    }

    p_parametresMetiers.proportionParticipationBalade = lireParametreUnitaire("proportionParticipationBalade");
    p_parametresMetiers.proportionRemboursementBalade = lireParametreUnitaire("proportionRemboursementBalade");
    p_parametresMetiers.plafondHoraireRemboursementEntrainement = lireParametreUnitaire("plafondHoraireRemboursementEntrainement");
    p_parametresMetiers.proportionRemboursementEntrainement = lireParametreUnitaire("proportionRemboursementEntrainement");
    p_parametresMetiers.montantCotisationPilote = lireParametreUnitaire("montantCotisationPilote");
    p_parametresMetiers.montantSubventionEntrainement = lireParametreUnitaire("montantSubventionEntrainement");   
}

const double ManageDb::lireParametreUnitaire(const QString &p_nomParametre)
{
    QSqlQuery query;

    query.prepare("SELECT * FROM parametres WHERE nom = :nomParam");
    query.bindValue(":nomParam", p_nomParametre);
    query.exec();
    if (query.next())
    {
        return query.value("info1").toDouble();
    }
    return 0.0;
}

void ManageDb::enregistrerParametres(const AeroDmsTypes::ParametresMetier& p_parametresMetiers,
    const AeroDmsTypes::ParametresSysteme& p_parametresSysteme)
{
    QSqlQuery query;
    query.prepare("UPDATE parametres SET info1 = :valeur WHERE nom = :nomParametre");
    query.bindValue(":nomParametre", "autoriserReglementParVirement");
    query.bindValue(":valeur", p_parametresSysteme.autoriserReglementParVirement);
    query.exec();

    enregistrerParametreUnitaire("proportionParticipationBalade", p_parametresMetiers.proportionParticipationBalade);
    enregistrerParametreUnitaire("proportionRemboursementBalade", p_parametresMetiers.proportionRemboursementBalade);
    enregistrerParametreUnitaire("plafondHoraireRemboursementEntrainement", p_parametresMetiers.plafondHoraireRemboursementEntrainement);
    enregistrerParametreUnitaire("proportionRemboursementEntrainement", p_parametresMetiers.proportionRemboursementEntrainement);
    enregistrerParametreUnitaire("montantCotisationPilote", p_parametresMetiers.montantCotisationPilote);
    enregistrerParametreUnitaire("montantSubventionEntrainement", p_parametresMetiers.montantSubventionEntrainement);
}

void ManageDb::enregistrerParametreUnitaire(const QString & p_nomParametre,
    const double p_valeur)
{
    QSqlQuery query;
    query.prepare("UPDATE parametres SET info1 = :valeur WHERE nom = :nomParametre");
    query.bindValue(":nomParametre", p_nomParametre);
    query.bindValue(":valeur", p_valeur);
    query.exec();
}

const QVersionNumber ManageDb::recupererVersionLogicielleMinimale()
{
    QVersionNumber version(0);

    QSqlQuery query;

    query.prepare("SELECT * FROM parametres WHERE nom = :nomParam");
    query.bindValue(":nomParam", "miseAJour");
    query.exec();
    if (query.next())
    {
        if (!(QVersionNumber::fromString(query.value("info1").toString()).isNull()))
            version = QVersionNumber::fromString(query.value("info1").toString());
    }
    return version;
}

void ManageDb::enregistrerNouvelleVersionLogicielleMinimale(const QVersionNumber &p_version, 
    const QString &p_nomFichier)
{
    QSqlQuery query;

    query.prepare("UPDATE parametres SET info1 = :version, info2 = :fichier WHERE nom = :nomParametre");
    query.bindValue(":nomParametre", "miseAJour");
    query.bindValue(":version", p_version.toString());
    query.bindValue(":fichier", p_nomFichier);
    query.exec();
}

const QString ManageDb::recupererNomFichierMiseAJour()
{
    QSqlQuery query;

    query.prepare("SELECT * FROM parametres WHERE nom = :nomParam");
    query.bindValue(":nomParam", "miseAJour");
    query.exec();
    if (query.next())
    {
        //On retourne la valeur de info2 en supprimant les éventuels "/" du nom du fichier (sécurité 
        //empéchant de télécharger un zip provenant d'un autre dépot que le dépot légitime)
        return query.value("info2").toString().replace("/", "");
    }
    return "";
}

const AeroDmsTypes::ListeAeroclubs ManageDb::recupererAeroclubs()
{
    AeroDmsTypes::ListeAeroclubs listeDesClubs;

    const QString sql = "SELECT * FROM aeroclub ORDER BY aeroclub";

    QSqlQuery query;
    query.exec(sql);

    while (query.next()) 
    {
        listeDesClubs.append(depilerRequeteAeroclub(query));
    }

    return listeDesClubs;
}

const AeroDmsTypes::ListeAerodromes ManageDb::recupererAerodromes()
{
    AeroDmsTypes::ListeAerodromes listeDesAerodromes;

    const QString sql = "SELECT * FROM aerodrome ORDER BY nomAerodrome";

    QSqlQuery query;
    query.exec(sql);

    while (query.next())
    {
        AeroDmsTypes::Aerodrome aerodrome;
		aerodrome.indicatifOaci = query.value("identifiantOaci").toString();
		aerodrome.nom = query.value("nomAerodrome").toString();
        listeDesAerodromes.append(aerodrome);
    }

    return listeDesAerodromes;
}

const AeroDmsTypes::Club ManageDb::depilerRequeteAeroclub(const QSqlQuery p_query)
{
    AeroDmsTypes::Club aeroclub = AeroDmsTypes::K_INIT_CLUB;
    aeroclub.idAeroclub = p_query.value("aeroclubId").toInt();
    aeroclub.aeroclub = p_query.value("aeroclub").toString();
    aeroclub.aerodrome = p_query.value("aerodrome").toString();
    aeroclub.raisonSociale = p_query.value("raisonSociale").toString();
    aeroclub.iban = p_query.value("iban").toString();
    aeroclub.bic = p_query.value("bic").toString();

    return aeroclub;
}

const AeroDmsTypes::Club ManageDb::recupererAeroclub(int p_aeroclubId)
{
    AeroDmsTypes::Club aeroclub = AeroDmsTypes::K_INIT_CLUB;

    QSqlQuery query;
    query.prepare("SELECT * FROM aeroclub WHERE aeroclubId = :aeroclubId");
    query.bindValue(":aeroclubId", p_aeroclubId);
    query.exec();

    while (query.next()) 
    {
        aeroclub = depilerRequeteAeroclub(query);
    }

    return aeroclub;
}

const AeroDmsTypes::ListePilotes ManageDb::recupererPilotes()
{
    AeroDmsTypes::ListePilotes listeDesPilotes;

    const QString sql = "SELECT * FROM infosPilotes ORDER BY piloteId";

    QSqlQuery query;
    query.exec(sql);

    while (query.next()) 
    {
        AeroDmsTypes::Pilote pilote = AeroDmsTypes::K_INIT_PILOTE;
        pilote.idPilote = query.value("piloteId").toString();
        pilote.nom = query.value("nom").toString();
        pilote.prenom = query.value("prenom").toString();
        pilote.aeroclub = query.value("aeroclub").toString();
        pilote.idAeroclub = query.value("aeroclubId").toInt();
        pilote.estAyantDroit = query.value("estAyantDroit").toBool();
        pilote.mail = query.value("mail").toString();
        pilote.telephone = query.value("telephone").toString();
        pilote.remarque = query.value("remarque").toString();
        pilote.activitePrincipale = query.value("activitePrincipale").toString();
        pilote.estActif = query.value("estActif").toBool();
        listeDesPilotes.append(pilote);
    }

    return listeDesPilotes;
}

const AeroDmsTypes::Pilote ManageDb::recupererPilote(const QString p_idPilote)
{
    AeroDmsTypes::Pilote pilote = AeroDmsTypes::K_INIT_PILOTE;

    QSqlQuery query;
    query.prepare("SELECT * FROM infosPilotes WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_idPilote); 
    query.exec();

    while (query.next()) 
    {
        pilote.idPilote = query.value("piloteId").toString();
        pilote.nom = query.value("nom").toString();
        pilote.prenom = query.value("prenom").toString();
        pilote.aeroclub = query.value("aeroclub").toString();
        pilote.idAeroclub = query.value("aeroclubId").toInt();
        pilote.estAyantDroit = query.value("estAyantDroit").toBool();
        pilote.mail = query.value("mail").toString();
        pilote.telephone = query.value("telephone").toString();
        pilote.remarque = query.value("remarque").toString();
        pilote.activitePrincipale = query.value("activitePrincipale").toString();
        pilote.estActif = query.value("estActif").toBool();
        pilote.estBrevete = query.value("estBrevete").toBool();
    }

    return pilote;
}

//Cette methode retourne le prochain ID de facture disponible
const int ManageDb::recupererProchainNumeroFacture()
{
    const QString sql = "SELECT seq FROM sqlite_sequence WHERE name = 'fichiersFacture'";

    QSqlQuery query;
    query.exec(sql);
    query.next();
    return query.value(0).toInt() + 1 ;
}

const AeroDmsTypes::ListeRecetteDetail ManageDb::recupererRecettesCotisations(const int p_annee)
{
    AeroDmsTypes::ListeRecetteDetail liste;

    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM recettesCotisations WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    else
    {
        query.prepare("SELECT * FROM recettesCotisations");
    } 
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::RecetteDetail recette;
        recette.typeDeRecette = "Cotisation";
        recette.id = query.value("cotisationId").toInt();
        recette.annee = query.value("annee").toInt();
        recette.intitule = query.value("Intitule").toString();
        recette.estSoumisCe = true;
        recette.montant = query.value("montant").toDouble();
        if (query.value("identifiantFormulaireSoumissionCe").isNull())
        {
            recette.estSoumisCe = false;
        }
        if (!query.value("remarque").isNull())
        {
            recette.remarque = query.value("remarque").toString();
        }
        liste.append(recette);
    }

    return liste;
}

const AeroDmsTypes::ListeRecetteDetail ManageDb::recupererRecettesHorsCotisation(const int p_annee)
{
    AeroDmsTypes::ListeRecetteDetail liste;

    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM recettesHorsCotisations WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    else
    {
        query.prepare("SELECT * FROM recettesHorsCotisations");
    }
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::RecetteDetail recette;
        recette.typeDeRecette = query.value("typeDeRecette").toString();
        recette.id = query.value("recetteId").toInt();
        recette.annee = query.value("annee").toInt();
        recette.intitule = query.value("Intitule").toString();
        recette.estSoumisCe = true;
        recette.montant = query.value("montant").toDouble();
        if (query.value("identifiantFormulaireSoumissionCe").isNull())
        {
            recette.estSoumisCe = false;
        }
        liste.append(recette);
    }

    return liste;
}

const AeroDmsTypes::TotauxRecettes ManageDb::recupererTotauxRecettes(const int p_annee)
{
    AeroDmsTypes::TotauxRecettes totaux = AeroDmsTypes::K_INIT_TOTAUX_RECETTE;

    QSqlQuery query;
    query.prepare("SELECT SUM(montant) AS totalCotisations FROM recettesCotisations WHERE annee = :annee");
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    if (query.next())
    {
        totaux.cotisations = query.value("totalCotisations").toDouble();
    }

    query.prepare("SELECT typeDeRecette, SUM(montant) as total FROM recettesHorsCotisations WHERE annee = :annee GROUP BY typeDeRecette");
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    while (query.next())
    {
        if (query.value("typeDeRecette").toString() == "Balade")
        {
            totaux.balades = query.value("total").toDouble();
        }
        else if (query.value("typeDeRecette").toString() == "Sortie")
        {
            totaux.sorties = query.value("total").toDouble();
        }
    }

    return totaux;
}

const AeroDmsTypes::ListeStatsHeuresDeVolParActivite ManageDb::recupererHeuresParActivite(const int p_annee,
    const int p_option)
{
    AeroDmsTypes::ListeStatsHeuresDeVolParActivite liste;

    QString requete = "";
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        requete = "SELECT * FROM stats_heuresDeVolParPiloteEtParActivite WHERE annee = :annee";
    }
    else
    {
        requete = "SELECT * FROM stats_heuresDeVolParPiloteEtParActivite";
    }  

    if ((p_option & AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT) == AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT)
    {
        requete = requete.replace("stats_heuresDeVolParPiloteEtParActivite","stats_heuresDeVolParPiloteEtParActivite_VolsAvecSubventionUniquement");
    }

    QSqlQuery query;
    query.prepare(requete);
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    AeroDmsTypes::StatsHeuresDeVolParActivite item;
    item.piloteId = "";

    while (query.next())
    {
        if (item.piloteId != query.value("pilote").toString())
        {
            //Nouveau pilote, on enregistre, si on est pas sur le premier tour
            if (item.piloteId != "")
            {
                liste.append(item);
            }
            //On rince les donnéee
            item = AeroDmsTypes::K_INIT_STATS_HEURES_DE_VOL_PAR_ACTIVITES;
            item.piloteId = query.value("pilote").toString();
            item.nomPrenomPilote = query.value("prenom").toString() + " " + query.value("nom").toString();
        }

        //On complete les données
        if (query.value("activite").toString() == "Avion")
        {
            if((p_option & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION) != AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION)
            {
                item.minutesVolAvion = item.minutesVolAvion + query.value("tempsDeVol").toInt();
                item.coutVolAvion = item.coutVolAvion + query.value("coutVol").toDouble();
                item.subventionVolAvion = item.subventionVolAvion + query.value("subventionVol").toDouble();
            }
        }
        else if (query.value("activite").toString() == "Avion électrique")
        {
            if ((p_option & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION) != AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION)
            {
                item.minutesVolAvionElectrique = item.minutesVolAvionElectrique + query.value("tempsDeVol").toInt();
                item.coutVolAvionElectrique = item.coutVolAvionElectrique + query.value("coutVol").toDouble();
                item.subventionVolAvionElectrique = item.subventionVolAvionElectrique + query.value("subventionVol").toDouble();
            }
        }
        else if (query.value("activite").toString() == "ULM")
        {
            if ((p_option & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_ULM) != AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_ULM)
            {
                item.minutesVolUlm = item.minutesVolUlm + query.value("tempsDeVol").toInt();
                item.coutVolUlm = item.coutVolUlm + query.value("coutVol").toDouble();
                item.subventionVolUlm = item.subventionVolUlm + query.value("subventionVol").toDouble();
            }
        }
        else if (query.value("activite").toString() == "Planeur")
        {
            if ((p_option & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_PLANEUR) != AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_PLANEUR)
            {
                item.minutesVolPlaneur = item.minutesVolPlaneur + query.value("tempsDeVol").toInt();
                item.coutVolPlaneur = item.coutVolPlaneur + query.value("coutVol").toDouble();
                item.subventionVolPlaneur = item.subventionVolPlaneur + query.value("subventionVol").toDouble();
            }
        }
        else if (query.value("activite").toString() == "Hélicoptère")
        {
            if ((p_option & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_HELICOPTERE) != AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_HELICOPTERE)
            {
                item.minutesVolHelicoptere = item.minutesVolHelicoptere + query.value("tempsDeVol").toInt();
                item.coutVolHelicoptere = item.coutVolHelicoptere + query.value("coutVol").toDouble();
                item.subventionVolHelicoptere = item.subventionVolHelicoptere + query.value("subventionVol").toDouble();
            }
        }
    }

    //On ajoute si on a pas eu une requete vide
    if (item.piloteId != "")
    {
        liste.append(item);
    }

    return liste;
}

const AeroDmsTypes::ListeSubventionsParPilotes ManageDb::recupererSubventionsPilotes( const int p_annee,
    const QString p_piloteId,
    const int p_options,
    const bool p_volsSoumisUniquement)
{
    QSqlQuery query;
    AeroDmsTypes::ListeSubventionsParPilotes liste;

    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE
        && p_piloteId != "*")
    {
        query.prepare("SELECT * FROM cotisation "
            "INNER JOIN pilote ON cotisation.pilote = pilote.piloteId "
            "WHERE annee = :annee AND pilote = :piloteId " 
            "ORDER BY annee, pilote.nom");    
    }
    else if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM cotisation "
            "INNER JOIN pilote ON cotisation.pilote = pilote.piloteId "
            "WHERE annee = :annee " 
            "ORDER BY annee, pilote.nom");
    }
    else if (p_piloteId != "*")
    {
        query.prepare("SELECT * FROM cotisation "
            "INNER JOIN pilote ON cotisation.pilote = pilote.piloteId "
            "WHERE pilote = :piloteId " 
            "ORDER BY annee, pilote.nom");
    }
    else
    {
        query.prepare("SELECT * FROM cotisation "
            "INNER JOIN pilote ON cotisation.pilote = pilote.piloteId "
            "ORDER BY annee, pilote.nom");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":piloteId", p_piloteId);

    query.exec();

    const QString filtre = genererClauseFiltrageActivite(p_options);

    //pour chaque pilote et chaque année de cotisation, on recupère les éventuelles heures de vol effectuées
    while (query.next())
    {
        int heuresDeVolEnMinutes = 0;
        AeroDmsTypes::SubventionsParPilote subvention = AeroDmsTypes::K_INIT_SUBVENTION_PAR_PILOTE;

        subvention.idPilote = query.value("pilote").toString();
        subvention.annee = query.value("annee").toInt();
        subvention.nom = query.value("nom").toString();
        subvention.prenom = query.value("prenom").toString();
        subvention.mail = query.value("mail").toString();
        subvention.montantSubventionEntrainement = query.value("montantSubventionAnnuelleEntrainement").toDouble();

        QSqlQuery queryVolAnneePilote;
        QString requete = "SELECT *  FROM volParTypeParAnEtParPilote "
            "WHERE annee = :annee AND pilote = :piloteId";
        requete = requete  
            + (filtre != "" ? " AND " : "") 
            + filtre;
        //Si on veut uniquement les totaux des vols déjà soumis au CSE, on remplace la vue volParTypeParAnEtParPilote par volParTypeParAnEtParPiloteSoumis
        if (p_volsSoumisUniquement)
        {
            requete = requete.replace("volParTypeParAnEtParPilote", "volParTypeParAnEtParPiloteSoumis");
        }
        //Sinon (condition exclusive), si on veut uniquement les vols pour lesquels on a eu de la subvention
        else if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT) == AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT)
        {
            requete = requete.replace("volParTypeParAnEtParPilote", "volParTypeParAnEtParPilote_VolsAvecSubventionUniquement");
        }
        queryVolAnneePilote.prepare(requete);
        queryVolAnneePilote.bindValue(":annee", QString::number(subvention.annee));
        queryVolAnneePilote.bindValue(":piloteId", subvention.idPilote);
        queryVolAnneePilote.exec();

        while (queryVolAnneePilote.next())
        {
            if (queryVolAnneePilote.value("typeDeVol").toString() == "Entrainement")
            {
                subvention.entrainement.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.entrainement.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.entrainement.montantRembourse = queryVolAnneePilote.value("montantRembourse").toDouble();
                subvention.entrainement.coutTotal = queryVolAnneePilote.value("cout").toDouble();
            }
            else if (queryVolAnneePilote.value("typeDeVol").toString() == "Sortie")
            {
                subvention.sortie.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.sortie.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.sortie.montantRembourse = queryVolAnneePilote.value("montantRembourse").toDouble();
                subvention.sortie.coutTotal = queryVolAnneePilote.value("cout").toDouble();
            }
            else if (queryVolAnneePilote.value("typeDeVol").toString() == "Balade")
            {
                subvention.balade.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.balade.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.balade.montantRembourse = queryVolAnneePilote.value("montantRembourse").toDouble();
                subvention.balade.coutTotal = queryVolAnneePilote.value("cout").toDouble();
            }
            heuresDeVolEnMinutes = heuresDeVolEnMinutes + queryVolAnneePilote.value("tempsDeVol").toInt();
            subvention.totaux.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(heuresDeVolEnMinutes);
            subvention.totaux.montantRembourse = subvention.totaux.montantRembourse + queryVolAnneePilote.value("montantRembourse").toDouble();
            subvention.totaux.coutTotal = subvention.totaux.coutTotal + queryVolAnneePilote.value("cout").toDouble();
        }
        liste.append(subvention);
    }
    return liste;
}

const AeroDmsTypes::SubventionsParPilote ManageDb::recupererTotauxAnnuel( const int p_annee,
    const bool p_volsSoumisUniquement)
{
    AeroDmsTypes::SubventionsParPilote totaux = AeroDmsTypes::K_INIT_SUBVENTION_PAR_PILOTE;
    totaux.idPilote = "";
    totaux.annee = p_annee;
    totaux.nom = "Totaux";
    totaux.prenom = "";

    int heuresDeVolEnMinutes = 0;

    QSqlQuery query;
    query.prepare("SELECT typeDeVol, annee, SUM(montantRembourse) AS subventionTotale, SUM(cout) AS coutTotal, SUM(tempsDeVol) AS tempsDeVolTotal FROM volParTypeParAnEtParPilote WHERE annee = :annee GROUP BY annee, typeDeVol");
    //Si on veut uniquement les totaux des vols déjà soumis au CSE, on remplace la vue volParTypeParAnEtParPilote par volParTypeParAnEtParPiloteSoumis
    if (p_volsSoumisUniquement)
    {
        query.prepare("SELECT typeDeVol, annee, SUM(montantRembourse) AS subventionTotale, SUM(cout) AS coutTotal, SUM(tempsDeVol) AS tempsDeVolTotal FROM volParTypeParAnEtParPiloteSoumis WHERE annee = :annee GROUP BY annee, typeDeVol");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    while (query.next())
    {
        if (query.value("typeDeVol").toString() == "Entrainement")
        {
            totaux.entrainement.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("tempsDeVolTotal").toInt());
            totaux.entrainement.montantRembourse = query.value("subventionTotale").toDouble();
            totaux.entrainement.coutTotal = query.value("coutTotal").toDouble();
        }
        else if (query.value("typeDeVol").toString() == "Sortie")
        {
            totaux.sortie.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("tempsDeVolTotal").toInt());
            totaux.sortie.montantRembourse = query.value("subventionTotale").toDouble();
            totaux.sortie.coutTotal = query.value("coutTotal").toDouble();
        }
        else if (query.value("typeDeVol").toString() == "Balade")
        {
            totaux.balade.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("tempsDeVolTotal").toInt());
            totaux.balade.montantRembourse = query.value("subventionTotale").toDouble();
            totaux.balade.coutTotal = query.value("coutTotal").toDouble();
        }
        heuresDeVolEnMinutes = heuresDeVolEnMinutes + query.value("tempsDeVolTotal").toInt();
        totaux.totaux.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(heuresDeVolEnMinutes);
        totaux.totaux.montantRembourse = totaux.totaux.montantRembourse + query.value("subventionTotale").toDouble();
        totaux.totaux.coutTotal = totaux.totaux.coutTotal + query.value("coutTotal").toDouble();
    }

    return totaux;
}

const AeroDmsTypes::Vol ManageDb::recupererVol(const int p_idVol)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM vol WHERE volId = :volId");
    query.bindValue(":volId", p_idVol);

    query.exec();
    query.next();

    return depilerRequeteVol(query);
}

const AeroDmsTypes::ListeVols ManageDb::recupererVols(const int p_annee,
    const QString p_piloteId)
{
    AeroDmsTypes::ListeVols liste;

    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE && p_piloteId != "*")
    {
        query.prepare("SELECT * FROM vols WHERE strftime('%Y', vols.date) = :annee AND pilote = :piloteId ORDER BY date");
    }
    else if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM vols WHERE strftime('%Y', vols.date) = :annee ORDER BY date");
    }
    else if (p_piloteId != "*")
    {
        query.prepare("SELECT * FROM vols WHERE pilote = :piloteId ORDER BY date");
    }
    else
    {
        query.prepare("SELECT * FROM vols ORDER BY date");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":piloteId", p_piloteId);


    query.exec();

    while (query.next())
    {
        //Récupération sans les champs facture et sortie
        liste.append(depilerRequeteVol(query, false));
    }
    return liste;
}

const AeroDmsTypes::ListeVols ManageDb::recupererVolsParDemandeDeRemboursement(const int p_idDemandeDeSubvention)
{
    AeroDmsTypes::ListeVols liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM vols WHERE demandeRemboursement = :demandeRemboursement ORDER BY date");
    query.bindValue(":demandeRemboursement", p_idDemandeDeSubvention);
    query.exec();

    while (query.next())
    {
        liste.append(depilerRequeteVol(query, false));
    }
    return liste;
}

const AeroDmsTypes::Vol ManageDb::depilerRequeteVol(const QSqlQuery p_query,
    const bool p_avecFactureEtSortie)
{
    AeroDmsTypes::Vol vol = AeroDmsTypes::K_INIT_VOL;

    vol.coutVol = p_query.value("cout").toDouble();
    vol.date = p_query.value("date").toDate();
    vol.dureeEnMinutes = p_query.value("duree").toInt();
    vol.duree = AeroDmsServices::convertirMinutesEnHeuresMinutes(vol.dureeEnMinutes);
    vol.estSoumisCe = "Oui";
    vol.estSoumis = true;
    if (p_query.value("demandeRemboursement").isNull()
        || p_query.value("demandeRemboursement").toInt() == -1)
    {
        vol.estSoumisCe = "Non";
        vol.estSoumis = false;
        if (p_query.value("demandeRemboursement").toInt() == -1)
        {
            vol.soumissionEstDelayee = true;
        }
    }
    vol.idPilote = p_query.value("pilote").toString();
    vol.montantRembourse = p_query.value("montantRembourse").toDouble();
    if (!p_query.isNull("nom")
        && !p_query.isNull("prenom") )
    {
        vol.nomPilote = p_query.value("nom").toString();
        vol.prenomPilote = p_query.value("prenom").toString();
    }
    vol.remarque = p_query.value("remarque").toString();
    vol.typeDeVol = p_query.value("typeDeVol").toString();
    vol.activite = p_query.value("activite").toString();
    vol.volId = p_query.value("volId").toInt();
    vol.immat = p_query.value("immatriculation").toString();
    if (p_avecFactureEtSortie)
    {
        vol.facture = p_query.value("facture").toInt();
        if (!p_query.value("sortie").isNull())
        {
            vol.baladeId = p_query.value("sortie").toInt();
        }
    }

    return vol;
}

const AeroDmsTypes::ListeDemandesRemboursementSoumises ManageDb::recupererDemandesRemboursementSoumises( const int p_annee,
                                                                                                   const QString p_piloteId)
{
    AeroDmsTypes::ListeDemandesRemboursementSoumises liste;

    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE&& p_piloteId != "*")
    {
        query.prepare("SELECT * FROM demandesRembousementVolsSoumises WHERE anneeVol = :annee AND pilote = :piloteId ORDER BY dateDemande");
    }
    else if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM demandesRembousementVolsSoumises WHERE anneeVol = :annee ORDER BY dateDemande");
    }
    else if (p_piloteId != "*")
    {
        query.prepare("SELECT * FROM demandesRembousementVolsSoumises WHERE pilote = :piloteId ORDER BY dateDemande");
    }
    else
    {
        query.prepare("SELECT * FROM demandesRembousementVolsSoumises ORDER BY dateDemande");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":piloteId", p_piloteId);

    query.exec();

    while (query.next())
    {
        AeroDmsTypes::DemandeRemboursementSoumise demande;
        demande.id = query.value("demandeId").toInt();
        demande.dateDemande = query.value("dateDemande").toDate();
        demande.nomBeneficiaire = query.value("nomBeneficiaire").toString();
        demande.montant = query.value("montant").toDouble();
        demande.typeDeVol = query.value("typeDeDemande").toString();
        demande.anneeVol = query.value("anneeVol").toInt();
        demande.coutTotalVolAssocies = query.value("totalCoutVol").toDouble();
        demande.piloteId = query.value("vol.pilote").toString();
        demande.nomPilote = query.value("prenom").toString() + " " + query.value("nom").toString();
        demande.mail = query.value("mail").toString();

        demande.note = "";
        if (!query.value("note").isNull())
        {
            demande.note = query.value("note").toString();
        }

        demande.modeDeReglement = AeroDmsTypes::ModeDeReglement_CHEQUE;
        if (query.value("modeDeReglement").toString() == "Virement")
        {
            demande.modeDeReglement = AeroDmsTypes::ModeDeReglement_VIREMENT;
        }

        liste.append(demande);
    }
    return liste;
}

const int ManageDb::ajouterFacture(QString& p_nomFichier)
{
    QString sql = "INSERT INTO 'fichiersFacture' ('nomFichier') VALUES('";
    sql.append(p_nomFichier);
    sql.append("')");

    QSqlQuery query;
    query.exec(sql);

    QThread::msleep(delaisDeGardeBdd);

    //Recuperation de l'ID de facture
    sql = "SELECT * FROM 'fichiersFacture' WHERE nomFichier='";
    sql.append(p_nomFichier);
    sql.append("'");

    query.exec(sql);
    query.next();
    return query.value(0).toInt();
}

//Recupère le subvention restante pour l'année p_annne et pour le pilote p_piloteId. Ne concerne que les vols de
//type Entrainement (heures perso), les autres types de vols ne sont pas plafonnés
const double ManageDb::recupererSubventionRestante( const QString& p_piloteId,
                                             const int p_annee)
{
    QSqlQuery query;

    QString sql = QString("SELECT montantSubventionAnnuelleEntrainement FROM cotisation WHERE pilote = ':piloteId' AND annee = ':annee'").replace(":piloteId", p_piloteId).replace(":annee", QString::number(p_annee));
    query.exec(sql);
    query.next();
    double montantAlloue = query.value(0).toDouble();

    sql = QString("SELECT subventionAllouee FROM subventionEntrainementAlloueeParPiloteEtParAnnee WHERE pilote = ':piloteId' AND annee = ':annee'").replace(":piloteId", p_piloteId).replace(":annee", QString::number(p_annee));
    query.exec(sql);
    query.next();
    
    return montantAlloue - query.value(0).toDouble();
}

void ManageDb::enregistrerUnVol(const QString& p_piloteId,
    const QString& p_typeDeVol,
    const QDate& p_date,
    const int p_dureeEnMinutes,
    const double p_cout,
    const double p_montantSubventionne,
    const int p_facture,
    const int p_idSortie,
    const QString& p_remarque,
    const QString& p_immat,
    const QString& p_activite,
    const int p_idVolAEditer)
{
    QSqlQuery query;
    //On verifie si l'immat existe en BDD
    query.prepare("SELECT * FROM aeronef WHERE immatriculation = :immatriculation");
    query.bindValue(":immatriculation", p_immat);

    query.exec();

    //Si non trouvé : l'immat n'existe pas. On la créé :
    if (!query.next())
    {
        query.prepare("INSERT INTO aeronef (immatriculation) VALUES(:immatriculation)");
        query.bindValue(":immatriculation", p_immat);
        //Le type est par défaut à inconnu
        query.exec();

        QThread::msleep(delaisDeGardeBdd);
    }

    //Si on est sur un ajout
    if (p_idVolAEditer == AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','remarque','immatriculation','activite') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:remarque,:immatriculation,:activite)");
        //Si on est sur une sortie, on la renseigne
        if (p_idSortie != AeroDmsTypes::K_INIT_INT_INVALIDE)
            query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','sortie','remarque','immatriculation','activite') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:sortie,:remarque,:immatriculation,:activite)");
    }
    //Sinon, un numéro de vol est renseigné : on est en édition
    else
    {
        //Dans le cas de l'édition d'un vol déjà soumis au CSE, le montant de la subvention arrivera
        //ici à 0, on le sort donc de la mise à jour :
        if (p_montantSubventionne != 0)
        {
            query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'montantRembourse' = :montantRembourse,'remarque' = :remarque, 'immatriculation' = :immatriculation, 'activite' = :activite WHERE volId = :volId");
            if (p_idSortie != AeroDmsTypes::K_INIT_INT_INVALIDE)
                query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'montantRembourse' = :montantRembourse,'remarque' = :remarque, 'immatriculation' = :immatriculation,'sortie' = :sortie, 'activite' = :activite WHERE volId = :volId");
        }
        else
        {
            //Les mêmes requetes, sans montantRembourse
            query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'remarque' = :remarque, 'immatriculation' = :immatriculation, 'activite' = :activite WHERE volId = :volId");
            if (p_idSortie != AeroDmsTypes::K_INIT_INT_INVALIDE)
                query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'remarque' = :remarque, 'immatriculation' = :immatriculation, 'sortie' = :sortie, 'activite' = :activite WHERE volId = :volId");
        }
        query.bindValue(":volId", p_idVolAEditer);
    }
    
    query.bindValue(":typeDeVol", p_typeDeVol);
    query.bindValue(":pilote", p_piloteId);
    query.bindValue(":date", p_date.toString("yyyy-MM-dd"));
    query.bindValue(":duree", p_dureeEnMinutes);
    query.bindValue(":cout", p_cout);
    query.bindValue(":montantRembourse", p_montantSubventionne);
    query.bindValue(":facture", p_facture);
    query.bindValue(":sortie", p_idSortie);
    query.bindValue(":remarque", p_remarque);
    query.bindValue(":immatriculation", p_immat);
    query.bindValue(":activite", p_activite);

    query.exec();
}

const bool ManageDb::supprimerUnVol(const int p_volAEditer)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM 'xAssociationRecette-Vol' WHERE volId = :volId");
    query.bindValue(":volId", QString::number(p_volAEditer));
    query.exec();
    if (query.next())
    {
        //Le vol est associé a une recette de sortie : on refuse la suppression
        return false;
    }

    //Sinon on poursuit
    query.prepare("DELETE FROM 'vol' WHERE volId = :volId");
    query.bindValue(":volId", p_volAEditer);

    return query.exec();
}

void ManageDb::switcherVolANePasSoumettreAuCse(const int p_volASwitcher)
{
    bool demandeEstAEffectuer = true;

    QSqlQuery query;
    query.prepare("SELECT demandeRemboursement FROM 'vol' WHERE volId = :volId");
    query.bindValue(":volId", QString::number(p_volASwitcher));
    query.exec();
    if (query.next())
    {
        if (query.value("demandeRemboursement").isNull())
        {
            query.prepare("UPDATE 'vol' SET 'demandeRemboursement' = :demandeRemboursement WHERE volId = :volId");
            query.bindValue(":volId", QString::number(p_volASwitcher));
            query.bindValue(":demandeRemboursement", -1);
        }
        else if (query.value("demandeRemboursement").toInt() == -1)
        {
            query.prepare("UPDATE 'vol' SET 'demandeRemboursement' = :demandeRemboursement WHERE volId = :volId");
            query.bindValue(":volId", QString::number(p_volASwitcher));
            query.bindValue(":demandeRemboursement", QVariant(QMetaType::fromType<int>()));
        }
        else
        {
            demandeEstAEffectuer = false;
        }
    }
    else
    {
        demandeEstAEffectuer = false;
    }

    if (demandeEstAEffectuer)
    {
        executerRequeteAvecControle(query,
            "selection/déselection d'un vol à soumettre au CSE",
            "Modification du statut vol à soumettre au CSE");
    }
    else
    {
        QMessageBox::critical(
            this,
            QApplication::applicationName() + " - " + tr("Impossible de changer le statut du vol"),
            tr("Une erreur s'est produite lors de la selection/déselection du vol à soumettre au CSE")
        );
    }
}

void ManageDb::enregistrerUneFacture( const QString& p_payeur,
    const int factureId,
    const QDate& p_date,
    const double p_montantFacture,
    const int p_idSortie,
    const QString& p_remarqueFacture )
{
    QSqlQuery query;
    query.prepare("INSERT INTO 'facturesSorties' ('sortie','facture','date','montant','intitule','payeur') VALUES(:sortie,:facture,:date,:montant,:intitule,:payeur)");
    query.bindValue(":sortie", p_idSortie);
    query.bindValue(":facture", factureId);
    query.bindValue(":date", p_date.toString("yyyy-MM-dd"));
    query.bindValue(":montant", p_montantFacture);
    query.bindValue(":intitule", p_remarqueFacture);
    query.bindValue(":payeur", p_payeur);

    executerRequeteAvecControle(query, 
        "insertion facture", 
        "Ajout d'une facture en base de données");
}

const QList<int> ManageDb::recupererAnneesAvecVolNonSoumis(const int p_annee)
{
    QSqlQuery query;
	if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		query.prepare("SELECT strftime('%Y', vol.date) AS annee FROM vol WHERE demandeRemboursement IS NULL AND strftime('%Y', vol.date) = :annee GROUP BY annee");
		query.bindValue(":annee", QString::number(p_annee));
	}
    else
    {
        query.prepare("SELECT strftime('%Y', vol.date) AS annee FROM vol WHERE demandeRemboursement IS NULL GROUP BY annee");
    }
    query.exec();

    QList<int> listeAnnees;

    while (query.next())
    {
        listeAnnees.append(query.value("annee").toInt());
    }

    return listeAnnees;
}

const AeroDmsTypes::ListeSubventionsParPilotes ManageDb::recupererLesSubventionesDejaAllouees(const int p_annee)
{
    return recupererSubventionsPilotes( p_annee, 
        "*",
        AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS,
        true);
}

void ManageDb::ajouterUneRecetteAssocieeAVol( const QStringList &p_listeVols,
    const QString& p_typeDeRecette,
    const QString& p_intitule,
    const double p_montant,
    const bool p_recetteEstPayeeParCb)
{
    QSqlQuery query;
    query.prepare("INSERT INTO 'recettes' ('typeDeRecette','intitule','montant') VALUES(:typeRecette, :intitule, :montant) RETURNING recetteId");
    query.bindValue(":typeRecette", p_typeDeRecette);
    query.bindValue(":intitule", p_intitule);
    query.bindValue(":montant", p_montant);

    executerRequeteAvecControle(query, 
        "insertion recette", 
        "Ajout recette associée à vol");
    query.next();
    const int numeroDeRecetteCree = query.value(0).toInt();

    QThread::msleep(delaisDeGardeBdd);

    //Si la recette est perçue par CB, pas besoin de soumettre de formulaire au CSE
    //=> on met à -1 l'identifiant de soumission au CSE
    if (p_recetteEstPayeeParCb)
    {
        query.prepare("UPDATE recettes SET identifiantFormulaireSoumissionCe = -1 WHERE recetteId = :recetteId");
        query.bindValue(":recetteId", numeroDeRecetteCree);
        executerRequeteAvecControle(query,
            "insertion recette / validation recette CB",
            "Ajout recette associée à vol");
        QThread::msleep(delaisDeGardeBdd);
    }

    //On itère sur la liste des vols pour associer la recette aux vols :
    for (int i = 0 ; i < p_listeVols.size() ; i++)
    {
        //On recupere l'id du vol associé :
        query.prepare("SELECT * FROM 'volsBaladesEtSorties' WHERE NomVol = :nomVol");
        query.bindValue(":nomVol", p_listeVols.at(i));
        query.exec();
        query.next();
        const int numeroDeVol = query.value(0).toInt();

        query.prepare("INSERT INTO 'xAssociationRecette-Vol' ('recetteId','volId') VALUES (:recetteId, :volId)");
        query.bindValue(":recetteId", numeroDeRecetteCree);
        query.bindValue(":volId", numeroDeVol);
        executerRequeteAvecControle(query, 
            "insertion recette (xAssociationRecette-Vol)", 
            "Numero vol " + QString::number(numeroDeVol) + "\nNumero recette créée " + QString::number(numeroDeRecetteCree));

        QThread::msleep(delaisDeGardeBdd);
    }
}

const bool ManageDb::piloteEstAJourDeCotisation(const QString& p_piloteId, 
    const int annee)
{
    //Recuperation de l'ID de facture
    QString sql = "SELECT * FROM 'cotisation' WHERE pilote='";
    sql.append(p_piloteId);
    sql.append("' AND annee='");
    sql.append(QString::number(annee));
    sql.append("'");

    QSqlQuery query;
    query.exec(sql);
    if (query.next())
        return true;
    return false;
}

const AeroDmsTypes::ListeDemandeRemboursement ManageDb::recupererLesSubventionsAEmettre(const int p_annee)
{
    QSqlQuery query;

    //Recuperation de l'ID de facture
	if (p_annee == AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
        query.prepare("SELECT * FROM 'volARembourserParTypeParPiloteEtParAnnee'");
	}
    else
    {
        query.prepare("SELECT * FROM 'volARembourserParTypeParPiloteEtParAnnee' WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    AeroDmsTypes::ListeDemandeRemboursement liste;
    
    query.exec();
    while (query.next()) {
        AeroDmsTypes::DemandeRemboursement demande;
        demande.typeDeVol = query.value("typeDeVol").toString();
        demande.piloteId = query.value("pilote").toString();
        demande.montantARembourser = query.value("montantARembourser").toDouble();
        demande.annee = query.value("annee").toInt();
        //demande.nomFichierFacture = query.value(5).toString();
        liste.append(demande);
    }
    
    return liste;
}

const QStringList ManageDb::recupererListeFacturesAssocieeASubvention(const  AeroDmsTypes::DemandeRemboursement p_demande)
{
    QStringList listeFactures;

    QSqlQuery query;
    QString sql = "SELECT * FROM volARembourserParFacture WHERE typeDeVol = ':typeDeVol' AND pilote = ':pilote' AND annee = ':annee'";
    sql.replace(":typeDeVol", p_demande.typeDeVol);
    sql.replace(":pilote", p_demande.piloteId);
    sql.replace(":annee", QString::number(p_demande.annee));
    
    query.exec(sql);
    while (query.next())
    {
        listeFactures.append(query.value("nomFichier").toString());
    }

    return listeFactures;
}

void ManageDb::ajouterDemandeCeEnBdd(const AeroDmsTypes::DemandeEnCoursDeTraitement p_demande) 
{
    //On créé l'entrée dans demandeRemboursementSoumises
    QSqlQuery query;
    query.prepare("INSERT INTO demandeRemboursementSoumises (dateDemande, montant, nomBeneficiaire, typeDeDemande, modeDeReglement) VALUES (:dateDemande, :montant, :nomBeneficiaire, :typeDeDemande, :modeDeReglement) RETURNING demandeRemboursementSoumises.demandeId");
    query.bindValue(":dateDemande", QDate::currentDate().toString("yyyy-MM-dd"));
    query.bindValue(":montant", p_demande.montant);
    query.bindValue(":nomBeneficiaire", p_demande.nomBeneficiaire);
    query.bindValue(":typeDeDemande", p_demande.typeDeVol);
    if (p_demande.modeDeReglement == AeroDmsTypes::ModeDeReglement_VIREMENT)
    {
        query.bindValue(":modeDeReglement", "Virement");
    }
    else
    {
        query.bindValue(":modeDeReglement", "Chèque");
    }
    executerRequeteAvecControle(query, 
        "ajout demande remboursement soumise", 
        "");
    query.next();
    const int idDemandeRemboursement = query.value(0).toInt();

    QThread::msleep(delaisDeGardeBdd);

    if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_HEURE_DE_VOL)
    {
        query.prepare("UPDATE vol SET demandeRemboursement = :idDemandeRemboursement WHERE strftime('%Y', vol.date) = :annee AND vol.pilote = :pilote AND vol.typeDeVol = :typeDeVol AND vol.demandeRemboursement IS NULL");
        query.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
        query.bindValue(":annee", QString::number(p_demande.annee));
        query.bindValue(":pilote", p_demande.idPilote);
        query.bindValue(":typeDeVol", p_demande.typeDeVol);
        executerRequeteAvecControle(query, 
            "mise à jour table vol", 
            "Demande de remboursement heure de vol");
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_COTISATION)
    {
        query.prepare(QString("SELECT idRecette FROM cotisationsASoumettreCe WHERE annee = ").append(QString::number(p_demande.annee)));
        query.exec();

        while (query.next())
        {
            QThread::msleep(delaisDeGardeBdd);
            QSqlQuery queryCotisation;
            queryCotisation.prepare("UPDATE recettes SET identifiantFormulaireSoumissionCe = :idDemandeRemboursement WHERE recetteId = :recetteId");
            queryCotisation.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
            queryCotisation.bindValue(":recetteId", query.value(0).toInt());
            executerRequeteAvecControle(queryCotisation, 
                "mise à jour table recette", 
                "Dépot cotisation");
        }
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE)
    {
        query.prepare("SELECT recetteId FROM recettesASoumettreCe WHERE annee = :annee AND nom = :nomSortie");
        query.bindValue(":annee", QString::number(p_demande.annee));
        query.bindValue(":nomSortie", p_demande.typeDeVol);
        query.exec();

        while (query.next())
        {
            QThread::msleep(delaisDeGardeBdd);
            QSqlQuery querySortie;
            querySortie.prepare("UPDATE recettes SET identifiantFormulaireSoumissionCe = :idDemandeRemboursement WHERE recetteId = :recetteId");
            querySortie.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
            querySortie.bindValue(":recetteId", query.value(0).toInt());
            executerRequeteAvecControle(querySortie, 
                "mise à jour table recette", 
                "Dépot paiement balade ou sortie");
        }
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_FACTURE)
    {
        query.prepare("UPDATE facturesSorties SET demandeRemboursement = :idDemandeRemboursement WHERE id = :id");
        query.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
        //L'année contient en fait l'ID de facture pour une facture...
        query.bindValue(":id", p_demande.annee);
        executerRequeteAvecControle(query, 
            "mise à jour table factureSortie", 
            "Remboursement facture");
    }

    QThread::msleep(delaisDeGardeBdd);
}

void ManageDb::ajouterNoteSubvention(const int p_idSubventionAAnoter, 
    const QString p_note)
{
    QSqlQuery query;
    query.prepare("UPDATE demandeRemboursementSoumises SET note = :note WHERE demandeId = :demandeId");
    query.bindValue(":note", p_note);
    query.bindValue(":demandeId", p_idSubventionAAnoter);

    executerRequeteAvecControle(query, 
        "mise à jour de la note sur la subvention d'ID " + QString::number(p_idSubventionAAnoter), 
        "Ajout note");
}

void ManageDb::executerRequeteAvecControle(QSqlQuery &p_query,
    const QString p_nomRequete,
    const QString p_texteDetailErreur)
{
    int nbEssais = 1;

    while (!p_query.exec())
    {
        QMessageBox dialogueErreurExecutionRequete;
        dialogueErreurExecutionRequete.setText(QString(tr("Erreur d'exécution de la requête ") + p_nomRequete +".\n")
            + p_query.lastError().text()
            + "\n"
            + p_texteDetailErreur
            + "\n\n"
            + tr("Nombre d'essais pour cette requête : ")
			+ QString::number(nbEssais)
            + "\n"
            + tr("Voulez vous réexécuter cette requete ?"));
        dialogueErreurExecutionRequete.setWindowTitle(QApplication::applicationName() + " - " + tr("Erreur d'exécution d'une requête"));
        dialogueErreurExecutionRequete.setIcon(QMessageBox::Critical);
        dialogueErreurExecutionRequete.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        dialogueErreurExecutionRequete.setDefaultButton(QMessageBox::Retry);

        const int ret = dialogueErreurExecutionRequete.exec();

        if (ret == QMessageBox::Cancel) {
            return;
        }
        nbEssais = nbEssais + 1;
    }
}

const AeroDmsTypes::ListeRecette ManageDb::recupererLesCotisationsAEmettre(const int p_annee)
{
    AeroDmsTypes::ListeRecette liste;
    QSqlQuery query;
	if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		query.prepare("SELECT annee, nom, montant FROM cotisationsASoumettreCe WHERE annee = :annee");
		query.bindValue(":annee", p_annee);
	}
	else
	{
        query.prepare("SELECT annee, nom, montant FROM cotisationsASoumettreCe");
	}
    const bool result = query.exec();

    AeroDmsTypes::Recette recette;
    recette.annee = 0;
    //if (query.size() > 0)
    {
        while (query.next()) {
            if (query.value(0).toInt() != recette.annee)
            {
                //On est sur l'année suivante
                //On ajoute l'année actuelle, si on est pas au premier tour (premier tour si annee = 0)
                if (recette.annee != 0)
                {
                    //On retire le dernier /, soit le dernier caractère...
                    recette.intitule.chop(1);
                    //... et on ferme la parenthèse
                    recette.intitule.append(")");
                    liste.append(recette);
                }

                //On rince l'item recette pour la suite
                recette.intitule = "Cotisation (";
                recette.montant = 0;
                recette.annee = query.value(0).toInt();
            }

            //On ajoute le nom du pilote
            //Si la longueur de la chaine après ajout du pilote dépasse 55 caractères, on split sur 2 lignes en ajoutant un <br>
            //avant d'insérer le nom du pilote :
            //-division entière de la taille de la chaine avant et après l'ajout,
            //-si l'entier est différent c'est qu'on change de ligne
            const int nbLigneAvant = recette.intitule.size() / 55;
            const int nbLigneApres = (recette.intitule.size() + query.value(1).toString().size()) / 55;
            if (nbLigneAvant != nbLigneApres)
            {
                recette.intitule.append("<br />");
            }
            recette.intitule.append(query.value(1).toString()).append("/");
            recette.montant = recette.montant+ query.value(2).toDouble();
            recette.annee = query.value(0).toInt();
        }
        //On oublie pas d'ajouter la dernière année...
        //On retire le dernier /, soit le dernier caractère...
        recette.intitule.chop(1);
        //... et on ferme la parenthèse
        recette.intitule.append(")");
        if (recette.intitule != ")")
        {
            liste.append(recette);
        }
    }
    return liste;
}

const AeroDmsTypes::ListeRecette ManageDb::recupererLesRecettesBaladesEtSortiesAEmettre(const int p_annee)
{
    //Récupération des cotisation à soumettre au CE
    AeroDmsTypes::ListeRecette liste;
    QSqlQuery query;
	if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		query.prepare("SELECT * FROM 'recettesASoumettreCeParTypeEtParAnnee' WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
	}
	else
	{
        query.prepare("SELECT * FROM 'recettesASoumettreCeParTypeEtParAnnee'");
	}
    query.exec();
    
    while (query.next()) {
        AeroDmsTypes::Recette recette;
        recette.typeDeSortie = query.value("typeDeRecette").toString();
        recette.intitule = query.value("nom").toString();
        recette.annee = query.value("annee").toInt();
        recette.montant = query.value("SUM(montant)").toDouble();
        liste.append(recette);
    }
    return liste;
}

const QList<QString> ManageDb::recupererListeRecettesNonSoumisesCse( const int p_annee,
    const QString p_typeRecette)
{
    QList<QString> liste;

    QSqlQuery query;
    query.prepare("SELECT Intitule FROM recettesASoumettreCe WHERE annee = :annee AND typeDeRecette = :typeDeRecette");
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":typeDeRecette", p_typeRecette);
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Recette recette;
        liste.append(query.value("Intitule").toString());
    }

    return liste;
}

const AeroDmsTypes::ListeDemandeRemboursementFacture ManageDb::recupererLesDemandesDeRembousementAEmettre(const int p_annee)
{
    //Récupérationd des demandes de remboursement à soumettre au CE
    AeroDmsTypes::ListeDemandeRemboursementFacture liste;
    QSqlQuery query;
	if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
	{
		query.prepare("SELECT * FROM 'facturesARembourser' WHERE annee = :annee");
		query.bindValue(":annee", QString::number(p_annee));
	}
	else
	{
        query.prepare("SELECT * FROM 'facturesARembourser'");
	}
    query.exec();

    while (query.next()) {
        AeroDmsTypes::DemandeRemboursementFacture demandeRemboursement;
        demandeRemboursement.id = query.value("id").toInt();
        demandeRemboursement.intitule = query.value("intitule").toString();
        demandeRemboursement.montant = query.value("montant").toDouble();
        demandeRemboursement.payeur = query.value("prenom").toString().append(" ").append(query.value("nom").toString());
        demandeRemboursement.nomSortie = query.value("nomSortie").toString();
        demandeRemboursement.typeDeSortie = query.value("typeDeDepense").toString();
        demandeRemboursement.annee = query.value("annee").toInt();
        demandeRemboursement.nomFacture = query.value("nomFacture").toString();
        demandeRemboursement.soumisCe = false;

        liste.append(demandeRemboursement);
    }

    return liste;
}

const AeroDmsTypes::ListeDemandeRemboursementFacture ManageDb::recupererToutesLesDemandesDeRemboursement(const int p_annee)
{
    //Récupérationd des demandes de remboursement soumises ou non au CSE
    AeroDmsTypes::ListeDemandeRemboursementFacture liste;
    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM factures WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    else
    {
        query.prepare("SELECT * FROM factures");
    }
    query.exec();

    while (query.next()) {
        AeroDmsTypes::DemandeRemboursementFacture demandeRemboursement;
        demandeRemboursement.id = query.value("id").toInt();
        demandeRemboursement.intitule = query.value("intitule").toString();
        demandeRemboursement.montant = query.value("montant").toDouble();
        demandeRemboursement.payeur = query.value("prenom").toString().append(" ").append(query.value("nom").toString());
        demandeRemboursement.nomSortie = query.value("nomSortie").toString();
        demandeRemboursement.typeDeSortie = query.value("typeDeDepense").toString();
        demandeRemboursement.annee = query.value("annee").toInt();
        demandeRemboursement.date = query.value("date").toDate();
        demandeRemboursement.soumisCe = true;
        if(query.value("demandeRemboursement").isNull())
            demandeRemboursement.soumisCe = false;
        demandeRemboursement.nomFacture = query.value("nomFacture").toString();

        liste.append(demandeRemboursement);
    }

    return liste;
}

const QString ManageDb::recupererAeroclub(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT aeroclub FROM infosPilotes WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value("aeroclub").toString();
}

const AeroDmsTypes::Club ManageDb::recupererInfosAeroclubDuPilote(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM infosPilotes WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();

    return depilerRequeteAeroclub(query);
}

const AeroDmsTypes::ListeAeronefs ManageDb::recupererListeAeronefs()
{
    QSqlQuery query;
    query.prepare("SELECT * FROM aeronef");

    query.exec();

    AeroDmsTypes::ListeAeronefs listeAeronefs;

    while (query.next())
    {
        AeroDmsTypes::Aeronef aeronef;
        if (!query.value("immatriculation").isNull())
        {
            aeronef.immatriculation = query.value("immatriculation").toString();
            aeronef.type = query.value("type").toString();
            listeAeronefs.append(aeronef);
        }  
    }

    return listeAeronefs;
}

const QList<int> ManageDb::recupererAnnees()
{
    QList<int> listeAnnees;
    QSqlQuery query;
    query.prepare("SELECT annee FROM cotisation GROUP BY annee");

    query.exec();
    while (query.next())
    {
        listeAnnees.append(query.value("annee").toInt());
    }
    return listeAnnees;
}

const QString ManageDb::recupererNomPrenomPilote(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT prenom, nom FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value("prenom").toString().append(" ").append(query.value("nom").toString());
}

const QString ManageDb::piloteExiste(const QString p_nom,
    const QString p_prenom)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM pilote WHERE prenom = :prenom AND nom = :nom");
    query.bindValue(":prenom", p_prenom);
    query.bindValue(":nom", p_nom);

    query.exec();
    if (query.next())
    {
        return query.value("piloteId").toString();
    }
    return "";
}

const QString ManageDb::recupererActivitePrincipale(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT activitePrincipale FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value("activitePrincipale").toString();
}

const QString ManageDb::recupererNomFacture(const int p_volId)
{
    QSqlQuery query;
    query.prepare("SELECT fichiersFacture.nomFichier AS nomFichier FROM vol INNER JOIN fichiersFacture ON vol.facture = fichiersFacture.factureId WHERE volId = :volId");
    query.bindValue(":volId", QString::number(p_volId));

    query.exec();
    query.next();
    return query.value("nomFichier").toString();
}

const int ManageDb::recupererLigneCompta(QString p_typeDeRecetteDepenseId)
{
    QSqlQuery query;
    query.prepare("SELECT identifiantCompta FROM typeDeRecetteDepense WHERE typeDeRecetteDepenseId = :typeDeRecetteDepenseId");
    query.bindValue(":typeDeRecetteDepenseId", p_typeDeRecetteDepenseId);

    query.exec();
    query.next();
    return query.value(0).toInt();
}

const QStringList ManageDb::recupererTypesDesVol(const bool recupererUniquementLesTypesDeVolAvecRecette)
{
    QString sql = "SELECT * FROM 'typeDeRecetteDepense' WHERE estVol = 1";
    if (recupererUniquementLesTypesDeVolAvecRecette)
    {
        sql.append(" AND estRecette = 1");
    }

    QStringList liste;
    QSqlQuery query;
    query.exec(sql);
    while (query.next()) {
        liste.append(query.value(0).toString());
    }

    return liste;
}

const AeroDmsTypes::ListeVolSortieOuBalade ManageDb::recupererBaladesEtSorties( const QString p_typeDeVol, 
    const double p_proportionRemboursement,
    const int p_annee )
{
    AeroDmsTypes::ListeVolSortieOuBalade liste;
    QSqlQuery query;
    if (p_annee == AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT * FROM 'volsBaladesEtSorties' WHERE typeDeVol = :typeDeVol");
    }
    else
    {
        query.prepare("SELECT * FROM 'volsBaladesEtSorties' WHERE typeDeVol = :typeDeVol AND annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    query.bindValue(":typeDeVol", p_typeDeVol);
    query.exec();
    while (query.next()) 
    {
        AeroDmsTypes::VolSortieOuBalade vol;
        vol.nomVol = query.value("NomVol").toString();
        if (p_typeDeVol == "Balade")
        {
            vol.montantSubventionAttendu = query.value("cout").toDouble() * p_proportionRemboursement;
        }

        QSqlQuery paiementAssocie;
        paiementAssocie.prepare("SELECT COUNT(*) AS nombreVol FROM 'xAssociationRecette-Vol' WHERE volId = :volId");
        paiementAssocie.bindValue(":volId", query.value("volId").toInt());
        paiementAssocie.exec();
        paiementAssocie.next();

        vol.volAAuMoinsUnPaiement = (paiementAssocie.value("nombreVol").toInt() != 0 );

        liste.append(vol);

    }

    return liste;
}

const AeroDmsTypes::ListeSortie ManageDb::recupererListeSorties()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE date IS NOT NULL");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value("sortieId").toInt();
        sortie.nom = query.value("nom").toString();
        sortie.date = query.value("date").toDate();
        liste.append(sortie);
    }

    return liste;
}

const AeroDmsTypes::ListeSortie ManageDb::recupererListeDepensesPouvantAvoirUneFacture()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE nom != 'Balade'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value("sortieId").toInt();
        sortie.nom = query.value("nom").toString();
        sortie.date = query.value("date").toDate();
        liste.append(sortie);
    }

    return liste;
}

const AeroDmsTypes::ListeSortie ManageDb::recupererListeBalade()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE nom = 'Balade'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value("sortieId").toInt();
        sortie.nom = query.value("nom").toString();
        sortie.date = query.value("date").toDate();
        liste.append(sortie);
    }

    return liste;
}

void ManageDb::ajouterCotisation(const AeroDmsTypes::CotisationAnnuelle& p_infosCotisation)
{
    QSqlQuery query;

    if (!p_infosCotisation.estEnEdition)
    {
        query.prepare("SELECT nom, prenom FROM 'pilote' WHERE piloteId = :piloteId");
        query.bindValue(":piloteId", p_infosCotisation.idPilote);
        query.exec();
        query.next();
        const QString intitule = QString("Cotisation ").append(query.value(0).toString()).append(" ").append(query.value(1).toString()).append(" ").append(QString::number(p_infosCotisation.annee));

        query.prepare("INSERT INTO 'recettes' ('typeDeRecette','Intitule','montant') VALUES ('Cotisation', :intitule, :montant) RETURNING recetteId");
        query.bindValue(":intitule", intitule);
        query.bindValue(":montant", p_infosCotisation.montant);
		while (!query.exec())
		{
            QMessageBox dialogueErreurExecutionRequete;
            dialogueErreurExecutionRequete.setText(QString(tr("Erreur d'exécution de la requête de création recette.\n"))
                + query.lastError().text()
				+ "\n"
                + "Intitulé " + intitule
                + "\n" + "\n"
                + tr("Voulez vous réexécuter cette requete ?"));
            dialogueErreurExecutionRequete.setWindowTitle(QApplication::applicationName() + " - " + tr("Enregistrement de la recette"));
            dialogueErreurExecutionRequete.setIcon(QMessageBox::Critical);
            dialogueErreurExecutionRequete.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
            dialogueErreurExecutionRequete.setDefaultButton(QMessageBox::Retry);

            const int ret = dialogueErreurExecutionRequete.exec();

            if (ret == QMessageBox::Cancel) {
                return ;
            }
		}
        query.next();

        const int idRecette = query.value(0).toInt();

        QThread::msleep(delaisDeGardeBdd);

        query.prepare("INSERT INTO 'cotisation' ('pilote','annee','idRecette', 'montantSubventionAnnuelleEntrainement', 'remarque') VALUES (:pilote, :annee, :idRecette, :montantSubvention, :remarque)");
        query.bindValue(":pilote", p_infosCotisation.idPilote);
        query.bindValue(":annee",   p_infosCotisation.annee);
        query.bindValue(":idRecette", idRecette);
        query.bindValue(":montantSubvention", p_infosCotisation.montantSubvention);
        query.bindValue(":remarque", p_infosCotisation.remarque);
        while (!query.exec())
        {
            QMessageBox dialogueErreurExecutionRequete;
            dialogueErreurExecutionRequete.setText(QString(tr("Erreur d'exécution de la requête de création cotisation.\n"))
                + query.lastError().text()
                + "\n"
                + "Id recette " + QString::number(idRecette)
                + "\n" + "\n"
                + tr("Voulez vous réexécuter cette requete ?"));
            dialogueErreurExecutionRequete.setWindowTitle(QApplication::applicationName() + " - " + tr("Enregistrement de la cotisation"));
            dialogueErreurExecutionRequete.setIcon(QMessageBox::Critical);
            dialogueErreurExecutionRequete.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
            dialogueErreurExecutionRequete.setDefaultButton(QMessageBox::Retry);

            const int ret = dialogueErreurExecutionRequete.exec();

            if (ret == QMessageBox::Cancel) {
                return;
            }
        }
    }
    else
    {
        query.prepare("UPDATE 'cotisation' SET 'montantSubventionAnnuelleEntrainement' = :montantSubvention, 'remarque' = :remarque WHERE pilote = :pilote AND annee = :annee");
        query.bindValue(":pilote", p_infosCotisation.idPilote);
        query.bindValue(":annee", p_infosCotisation.annee);
        query.bindValue(":montantSubvention", p_infosCotisation.montantSubvention);
        query.bindValue(":remarque", p_infosCotisation.remarque);
        query.exec();
    }

    QThread::msleep(delaisDeGardeBdd);

    //Un pilote pour lequel on vient d'ajouter/editer une cotistion est toujours un pilote actif
    query.prepare("UPDATE 'pilote' SET 'estActif' =:estActif WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_infosCotisation.idPilote);
    query.bindValue(":estActif", true);
    query.exec();
}

const double ManageDb::recupererSubventionEntrainement( const QString p_pilote,
    const int p_annee)
{
    QSqlQuery query;

    query.prepare("SELECT montantSubventionAnnuelleEntrainement FROM 'cotisation' WHERE pilote = :pilote AND annee = :annee");
    query.bindValue(":pilote", p_pilote);
    query.bindValue(":annee", p_annee);
    query.exec();
    query.next();

    return query.value("montantSubventionAnnuelleEntrainement").toDouble();
}

const QString ManageDb::recupererRemarqueSubvention(const QString p_pilote,
    const int p_annee)
{
    QSqlQuery query;

    query.prepare("SELECT remarque FROM 'cotisation' WHERE pilote = :pilote AND annee = :annee");
    query.bindValue(":pilote", p_pilote);
    query.bindValue(":annee", p_annee);
    query.exec();
    query.next();

    return query.value("remarque").toString();
}

//Cette fonction créé (p_pilote.idPilote = "") ou met à jour (p_pilote.idPilote renseigné) un pilote
//dans la base de données
const AeroDmsTypes::ResultatCreationBdd ManageDb::creerPilote(const AeroDmsTypes::Pilote p_pilote)
{
    AeroDmsTypes::ResultatCreationBdd resultat = AeroDmsTypes::ResultatCreationBdd_SUCCES;

    QSqlQuery query;
    
    //creation
    if (p_pilote.idPilote == "")
    {
        QString piloteId = p_pilote.prenom.toLower();
        piloteId.append(".");
        piloteId.append(p_pilote.nom.toLower());

        AeroDmsServices::normaliser(piloteId);

        //Verifier si le pilote existe
        query.prepare("SELECT * FROM pilote WHERE piloteId = :piloteId");
        query.bindValue(":piloteId", piloteId);
        query.exec();
        query.next();
        if (query.size() > 0)
        {
            resultat = AeroDmsTypes::ResultatCreationBdd_ELEMENT_EXISTE;
        }
        else
        {
            query.prepare("INSERT INTO 'pilote' ('piloteId','nom','prenom','aeroclubId','estAyantDroit','mail','telephone','remarque','activitePrincipale','estActif','estBrevete') VALUES(:piloteId,:nom,:prenom,:aeroclubId,:estAyantDroit,:mail,:telephone,:remarque,:activitePrincipale,:estActif,:estBrevete)");
            query.bindValue(":piloteId", piloteId);
            query.bindValue(":nom", p_pilote.nom);
            query.bindValue(":prenom", p_pilote.prenom);
            query.bindValue(":aeroclubId", p_pilote.idAeroclub);
            query.bindValue(":estAyantDroit", p_pilote.estAyantDroit);
            query.bindValue(":mail", p_pilote.mail);
            query.bindValue(":telephone", p_pilote.telephone);
            query.bindValue(":remarque", p_pilote.remarque);
            query.bindValue(":activitePrincipale", p_pilote.activitePrincipale);
            query.bindValue(":estActif", p_pilote.estActif);
            query.bindValue(":estBrevete", p_pilote.estBrevete);

            if (!query.exec())
            {
                resultat = AeroDmsTypes::ResultatCreationBdd_AUTRE;
            }
        } 
    }
    //mise à jour
    else
    {
        query.prepare("UPDATE 'pilote' SET 'nom' = :nom,'prenom' = :prenom,'aeroclubId' = :aeroclubId,'estAyantDroit' = :estAyantDroit,'mail' = :mail,'telephone' = :telephone,'remarque' = :remarque, 'activitePrincipale' =:activitePrincipale, 'estActif' =:estActif, 'estBrevete' =:estBrevete WHERE piloteId = :piloteId");
        query.bindValue(":piloteId", p_pilote.idPilote);
        query.bindValue(":nom", p_pilote.nom);
        query.bindValue(":prenom", p_pilote.prenom);
        query.bindValue(":aeroclubId", p_pilote.idAeroclub);
        query.bindValue(":estAyantDroit", p_pilote.estAyantDroit);
        query.bindValue(":mail", p_pilote.mail);
        query.bindValue(":telephone", p_pilote.telephone);
        query.bindValue(":remarque", p_pilote.remarque);
        query.bindValue(":activitePrincipale", p_pilote.activitePrincipale);
        query.bindValue(":estActif", p_pilote.estActif);
        query.bindValue(":estBrevete", p_pilote.estBrevete);

        if (!query.exec())
        {
            resultat = AeroDmsTypes::ResultatCreationBdd_AUTRE;
        }
    }

    return resultat;
}

const AeroDmsTypes::ResultatCreationBdd ManageDb::creerAeroclub(const AeroDmsTypes::Club p_aeroclub)
{
    AeroDmsTypes::ResultatCreationBdd resultat = AeroDmsTypes::ResultatCreationBdd_SUCCES;

    QSqlQuery query;

    //creation
    if (p_aeroclub.idAeroclub == AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("INSERT INTO 'aeroclub' ('aeroclub', 'aerodrome', 'raisonSociale','IBAN','BIC') VALUES(:aeroclub,:aerodrome,:raisonSociale,:IBAN,:BIC)");
        query.bindValue(":aeroclub", p_aeroclub.aeroclub);
        query.bindValue(":aerodrome", p_aeroclub.aerodrome);
        query.bindValue(":raisonSociale", p_aeroclub.raisonSociale);
        query.bindValue(":IBAN", p_aeroclub.iban);
        query.bindValue(":BIC", p_aeroclub.bic);

        if (!query.exec())
        {
            resultat = AeroDmsTypes::ResultatCreationBdd_AUTRE;
            
            query.prepare("SELECT * FROM aeroclub WHERE aeroclub = :aeroclub");
            query.bindValue(":aeroclub", p_aeroclub.aeroclub);
            query.exec();

            if (query.next())
            {
				resultat = AeroDmsTypes::ResultatCreationBdd_ELEMENT_EXISTE;
            }
        }
    }
    //mise à jour
    else
    {
        query.prepare("UPDATE 'aeroclub' SET 'aeroclub' = :aeroclub,'aerodrome' = :aerodrome,'raisonSociale' = :raisonSociale,'IBAN' = :IBAN,'BIC' = :BIC WHERE aeroclubId = :aeroclubId");
        query.bindValue(":aeroclubId", p_aeroclub.idAeroclub);
        query.bindValue(":aeroclub", p_aeroclub.aeroclub);
        query.bindValue(":aerodrome", p_aeroclub.aerodrome);
        query.bindValue(":raisonSociale", p_aeroclub.raisonSociale);
        query.bindValue(":IBAN", p_aeroclub.iban);
        query.bindValue(":BIC", p_aeroclub.bic);

        if (!query.exec())
        {
            resultat = AeroDmsTypes::ResultatCreationBdd_AUTRE;
        }
    }

    return resultat;
}

void ManageDb::creerSortie(const AeroDmsTypes::Sortie p_sortie)
{
    QSqlQuery query;
    query.prepare("INSERT INTO sortie ('nom','date') VALUES (:nom,:date)");
    query.bindValue(":nom",p_sortie.nom);
    query.bindValue(":date", p_sortie.date.toString("yyyy-MM-dd"));
    query.exec();
}

const AeroDmsTypes::ListeStatsHeuresDeVol ManageDb::recupererHeuresMensuelles(const int p_annee,
    const int p_options)
{
    AeroDmsTypes::ListeStatsHeuresDeVol liste;

    const QString filtre = genererClauseFiltrageActivite(p_options);
    QString requete = "";
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        requete = "SELECT mois, annee, SUM(tempsDeVol) AS tempsDeVol, typeDeVol, activite FROM stats_heuresDeVolParMois WHERE annee = :annee ";
        if (filtre != "")
        {
            requete = requete + " AND ";
            requete = requete + filtre;
        }
        requete = requete + " GROUP BY typeDeVol, mois ORDER BY mois";
    }
    else
    {
        requete = "SELECT mois, annee, SUM(tempsDeVol) AS tempsDeVol, typeDeVol, activite FROM stats_heuresDeVolParMois ";
        if (filtre != "")
        {
            requete = requete + " WHERE ";
            requete = requete + filtre;
        }
        requete = requete + " GROUP BY typeDeVol, mois, annee ORDER BY annee, mois";
    }
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT) == AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT)
    {
        requete = requete.replace("stats_heuresDeVolParMois", "stats_heuresDeVolParMois_volsAvecSubventionUniquement");
    }

    QSqlQuery query;
    query.prepare(requete);
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    AeroDmsTypes::StatsHeuresDeVol heuresMensuelles;
    heuresMensuelles.mois = "";
    QLocale localeFrancaise(QLocale::French);
    QDate moisPrecedent = QDate();

    while (query.next()) {
        const QDate mois = QDate( query.value("annee").toInt(), 
                                  query.value("mois").toInt(), 
                                  1);
        //Si on change de mois
        if (heuresMensuelles.mois != localeFrancaise.toString(mois, "MMMM yyyy"))
        {
            //On ajoute les heures mensuelles courantes, si on est pas sur le premier tour...
            if (heuresMensuelles.mois != "")
            {
                liste.append(heuresMensuelles);
            }
            //Au premier tour on initialise le mois precedent
            else
            {
                moisPrecedent = mois;
            }
            //On rince
            heuresMensuelles.minutesBalade = 0;
            heuresMensuelles.minutesEntrainement = 0;
            heuresMensuelles.minutesSortie = 0;

            //On complète les trous, les mois ou éventuellement il n'y aurait pas eu de vol
            if (moisPrecedent != mois)
            {
                while (moisPrecedent.addMonths(1) != mois)
                {
                    //On ajoute un mois
                    moisPrecedent = moisPrecedent.addMonths(1);

                    //Et on ajoute le mois à la liste
                    heuresMensuelles.mois = localeFrancaise.toString(moisPrecedent, "MMMM yyyy");
                    liste.append(heuresMensuelles);
                }
            }

            heuresMensuelles.mois = localeFrancaise.toString(mois, "MMMM yyyy");
            moisPrecedent = mois;
        }
       
        if (query.value("typeDeVol").toString() == "Entrainement")
        {
            heuresMensuelles.minutesEntrainement = query.value("tempsDeVol").toInt();
        }
        else if (query.value("typeDeVol").toString() == "Sortie")
        {
            heuresMensuelles.minutesSortie = query.value("tempsDeVol").toInt();
        }
        else if (query.value("typeDeVol").toString() == "Balade")
        {
            heuresMensuelles.minutesBalade = query.value("tempsDeVol").toInt();
        }
    }

    //On ajoute le dernier element à la liste, s'il existe...
    if (heuresMensuelles.mois != "")
    {
        liste.append(heuresMensuelles);
    }
    return liste;
}

const QStringList ManageDb::recupererListeActivites()
{
    QSqlQuery query;
    query.prepare("SELECT * FROM 'activite'");
    query.exec();

    QStringList listeActivite;
    while (query.next()) 
    {
        listeActivite.append(query.value("nom").toString());
    }

    return listeActivite;
}

const QString ManageDb::recupererMailPilotes( const int p_annee, 
    const AeroDmsTypes::MailPilotes p_mailingDemande)
{
    QStringList listeMail;
    QSqlQuery query;

    switch(p_mailingDemande)
    {
        case AeroDmsTypes::MailPilotes_ACTIF_AYANT_COTISE:
        {
            query.prepare("SELECT piloteId, mail, annee FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId WHERE annee = :annee AND estActif = 1");
        }
        break;
        case AeroDmsTypes::MailPilotes_ACTIFS_ET_BREVETES:
        {
            query.prepare("SELECT piloteId, mail FROM pilote WHERE estActif = 1 AND estBrevete = 1");
        }
        break;
        case AeroDmsTypes::MailPilotes_BREVETES:
        {
            query.prepare("SELECT piloteId, mail FROM pilote WHERE estBrevete = 1");
        }
        break;
        case AeroDmsTypes::MailPilotes_ACTIFS:
        {
            query.prepare("SELECT piloteId, mail FROM pilote WHERE estActif = 1");
        }
        break;
        case AeroDmsTypes::MailPilotes_SUBVENTION_NON_CONSOMMEE:
        {
            const AeroDmsTypes::ListeSubventionsParPilotes listeSubventions = recupererSubventionsPilotes(p_annee, 
                "*", 
                AeroDmsTypes::OptionsDonneesStatistiques_TOUS_LES_VOLS, 
                false);
            for (int i = 0; i < listeSubventions.size(); i++)
            {
                const AeroDmsTypes::SubventionsParPilote subvention = listeSubventions.at(i);
                if (subvention.entrainement.montantRembourse < subvention.montantSubventionEntrainement)
                {
                    listeMail.append(subvention.mail);
                }
            }
        }
        break;
        case AeroDmsTypes::MailPilotes_AYANT_COTISE:
        default:
        {
            query.prepare("SELECT piloteId, mail, annee FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId WHERE annee = :annee");
        }
        break;
    }

    if (p_mailingDemande != AeroDmsTypes::MailPilotes_SUBVENTION_NON_CONSOMMEE)
    {
        query.bindValue(":annee", p_annee);
        query.exec();

        while (query.next())
        {
            listeMail.append(query.value("mail").toString());
        }
    }  

    return listeMail.join(";");
}

const QList<QDate> ManageDb::recupererDatesDesDemandesDeSubventions()
{
    QList<QDate> listeDemandes;

    QSqlQuery query;
    query.prepare("SELECT dateDemande FROM mailParDateDeDemandeDeSubvention GROUP BY dateDemande ORDER BY dateDemande DESC LIMIT 5");
    query.exec();

    while (query.next())
    {
        QDate date = QDate::fromString(query.value("dateDemande").toString(), "yyyy-MM-dd");
        listeDemandes.append(date);
    }

    return listeDemandes;
}

const QList<QDate> ManageDb::recupererDatesDesDemandesDeSubventionsVerseesParVirement()
{
    QList<QDate> listeDemandes;

    QSqlQuery query;
    query.prepare("SELECT * FROM demandesRembousementVolsSoumises WHERE modeDeReglement = 'Virement' GROUP BY dateDemande ORDER BY dateDemande DESC LIMIT 5");
    query.exec();

    while (query.next())
    {
        QDate date = QDate::fromString(query.value("dateDemande").toString(), "yyyy-MM-dd");
        listeDemandes.append(date);
    }

    return listeDemandes;
}

const AeroDmsTypes::ListeAerodromes ManageDb::recupererAerodromesAvecPilotesActifs(const AeroDmsTypes::DonnesMailingType p_demande)
{
    AeroDmsTypes::ListeAerodromes aerodromes;

    QSqlQuery query;
    switch (p_demande)
    {
        case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME :
        {
            query.prepare("SELECT aerodrome, nomAerodrome FROM infosPilotes WHERE estActif GROUP BY aerodrome ORDER BY nomAerodrome");
        }
        break;
        case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME:
        default:
        {
            query.prepare("SELECT aerodrome, nomAerodrome FROM infosPilotes WHERE estActif AND estBrevete AND activitePrincipale != 'Planeur' GROUP BY aerodrome ORDER BY nomAerodrome");
        }
        break;
    }
    
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::Aerodrome aerodrome;
        aerodrome.indicatifOaci = query.value("aerodrome").toString();
        aerodrome.nom = query.value("nomAerodrome").toString();
        aerodromes.append(aerodrome);
    }

    return aerodromes;
}

const QString ManageDb::recupererMailDerniereDemandeDeSubvention(const QString p_date)
{
    QStringList listeMail;

    QSqlQuery query;
    QString date = "";

    if (p_date == "")
    {   
        query.prepare("SELECT dateDemande FROM mailParDateDeDemandeDeSubvention GROUP BY dateDemande ORDER BY dateDemande DESC LIMIT 1");
        query.exec();
        if (query.next())
        {
            date = query.value("dateDemande").toString();
        }
    }
    else
    {
        date = p_date;
    }

    if (date != "")
    {
        query.prepare("SELECT mail FROM mailParDateDeDemandeDeSubvention WHERE dateDemande = :dateDemande");
        query.bindValue(":dateDemande", date);
        query.exec();

        while (query.next())
        {
            listeMail.append(query.value("mail").toString());
        }
    }

    return listeMail.join(";");
}

const QString ManageDb::recupererMailPilotesDUnAerodrome(const QString p_codeOaci, 
    const AeroDmsTypes::DonnesMailingType p_demande)
{
    QStringList listeMail;

    QSqlQuery query;

	switch (p_demande)
	{
	    case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_D_UN_AERODROME:
	    {
            query.prepare("SELECT mail FROM infosPilotes WHERE estActif AND aerodrome = :aerodrome AND mail IS NOT NULL AND mail != ''");
	    }
	    break;
	    case AeroDmsTypes::DonnesMailingType_PILOTES_ACTIFS_BREVETES_VOL_MOTEUR_D_UN_AERODROME:
	    default:
	    {
		    query.prepare("SELECT mail FROM infosPilotes WHERE estActif AND estBrevete AND activitePrincipale != 'Planeur' AND aerodrome = :aerodrome AND mail IS NOT NULL AND mail != ''");
	    }
	    break;
	    }

	query.bindValue(":aerodrome", p_codeOaci);
    query.exec();

    while (query.next())
    {
        listeMail.append(query.value("mail").toString());
    }

    return listeMail.join(";");
}

const AeroDmsTypes::ListeMailsEtVirements ManageDb::recupererMailsVirements(const QString p_dateDemande)
{
    AeroDmsTypes::ListeMailsEtVirements listeVirements;

    QSqlQuery query;
    query.prepare("SELECT * FROM demandesRembousementVolsSoumises WHERE modeDeReglement = 'Virement' AND dateDemande = :dateDemande ORDER BY pilote");
    query.bindValue(":dateDemande", p_dateDemande);
    query.exec();

    QString mail = "";
    AeroDmsTypes::MailEtVirements mailEtVirements;

    while (query.next())
    {
        if (mail != query.value("mail").toString())
        {
            //Si on est pas sur le premier tour (mail différent de sa valeur d'init), on ajoute
            if (mail != "")
            {
                listeVirements.append(mailEtVirements);
            }
            mailEtVirements.mail = query.value("mail").toString();
            mailEtVirements.pilote = query.value("pilote").toString();
            mailEtVirements.listeMontantsVirements.clear();
            mailEtVirements.listeIdVirements.clear();
            mail = query.value("mail").toString();
        }
        mailEtVirements.listeMontantsVirements.append(query.value("montant").toDouble());
        mailEtVirements.listeIdVirements.append(query.value("demandeId").toInt());
    }
    listeVirements.append(mailEtVirements);

    return listeVirements;
}

//Un changement de version attendue de la BDD intervient notamment si 
//   -la structure de la BDD évolue
//   -on souhaite empecher l'utilisation d'une version du logiciel avec laquelle
//       un défaut peut avoir un impact sur la BDD (cela imposera de mette à jour
//       AeroDMS avant de pouvoir utiliser le logiciel en ecriture sur la BDD)
const bool ManageDb::laBddEstALaVersionAttendue() const
{
    QSqlQuery query;
    query.prepare("SELECT * FROM parametres WHERE nom = 'versionBdd'");
    query.exec();

    if (!query.next())
    {
        return false;
    }     
    else
    {
        //version stockée dans le champ info1
        return (query.value("info1").toDouble() == versionBddAttendue);
    }
}

const AeroDmsTypes::StatsPilotes ManageDb::recupererStatsPilotes()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(estBrevete) AS nbTotal,"
        "COUNT(*) FILTER (WHERE estBrevete) AS nbBrevete,"
        "COUNT(*) FILTER (WHERE NOT estBrevete) AS nbNonBrevete,"
        "COUNT(*) FILTER (WHERE NOT estAyantDroit) AS nbOuvrantDroit,"
        "COUNT(*) FILTER(WHERE estAyantDroit)  AS nbAyantDroit "
        "FROM pilote");
    query.exec();

    AeroDmsTypes::StatsPilotes statsPilotes = AeroDmsTypes::K_INIT_DONNEES_STATS_PILOTES;

    if (query.next())
    {
        statsPilotes.nbBrevete = query.value("nbBrevete").toInt() ;
        statsPilotes.nbNonBrevete = query.value("nbNonBrevete").toInt() ;
        statsPilotes.nbOuvranDroit = query.value("nbOuvrantDroit").toInt();
        statsPilotes.nbAyantDroit = query.value("nbAyantDroit").toInt();
    }

    return statsPilotes;
}

const AeroDmsTypes::StatsAeronefs ManageDb::recupererStatsAeronefs(const int p_annee, 
    const int p_options)
{
    AeroDmsTypes::StatsAeronefs statsAeronefs;

    QString nomVue = "stats_aeronefs";
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT) == AeroDmsTypes::OptionsDonneesStatistiques_VOLS_SUBVENTIONNES_UNIQUEMENT)
    {
        nomVue = "stats_aeronefs_volAvecSubvention";
    }

    QString filtre = genererClauseFiltrageActivite(p_options);

    QSqlQuery query;
    if (p_annee != AeroDmsTypes::K_INIT_INT_INVALIDE)
    {
        query.prepare("SELECT "
            "immatriculation,"
            "type,"
            "activite, "
            "SUM(tempsDeVol) AS tempsDeVol "
            "FROM " + nomVue + " "
            "WHERE annee = :annee "
            + (filtre != "" ? " AND " : "")
            + filtre +
            "GROUP BY immatriculation "
            "ORDER BY activite, type, immatriculation");
        query.bindValue(":annee", QString::number(p_annee));
    }
    else
    {
        query.prepare("SELECT "
            "immatriculation,"
            "type,"
            "activite, "
            "SUM(tempsDeVol) AS tempsDeVol "
            "FROM " + nomVue + " "
            + (filtre != "" ? " WHERE " : "") 
            + filtre +
            "GROUP BY immatriculation "
            "ORDER BY activite, type, immatriculation");
    }
    
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::StatsAeronef stats;

        stats.immat = query.value("immatriculation").toString();
        stats.type = query.value("type").toString();
        stats.nombreMinutesVol = query.value("tempsDeVol").toInt();

        statsAeronefs.append(stats);
    }

    return statsAeronefs;
}

void ManageDb::mettreAJourDonneesAeronefs( const QString p_immatAeronefAMettreAJour,
    const QString p_nouvelleValeur,
    const AeroDmsTypes::AeronefTableElement p_donneeAMettreAJour )
{
    QSqlQuery query;
    bool requeteAExecuter = true;

    switch (p_donneeAMettreAJour)
    {
        case AeroDmsTypes::AeronefTableElement_TYPE:
        {
            query.prepare("UPDATE aeronef SET type = :nouvelleValeur WHERE immatriculation = :immat");
            break;
        }
        default:
        {
            requeteAExecuter = false;
            break;
        }
    }

    if (requeteAExecuter)
    {
        query.bindValue(":nouvelleValeur", p_nouvelleValeur);
        query.bindValue(":immat", p_immatAeronefAMettreAJour);
        query.exec();
    }
}

const AeroDmsTypes::ListeDetailsBaladesEtSorties ManageDb::recupererListeDetailsBaladesEtSorties(const int p_annee)
{
    AeroDmsTypes::ListeDetailsBaladesEtSorties listeDetails;

    QSqlQuery query;
    query.prepare("SELECT * FROM detailsBaladesEtSorties WHERE annee = :annee");
    query.bindValue(":annee", QString::number(p_annee));
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::DetailsBaladesEtSorties details = AeroDmsTypes::K_INIT_DETAILS_BALADES_ET_SORTIES;

        details.volId = query.value("volId").toInt();
        details.idSortie = query.value("idSortie").toInt();
        details.nomSortie = query.value("nomSortie").toString();

        details.dateVol = query.value("dateVol").toDate();
        details.dureeVol = query.value("dureeVol").toInt();
        details.nomPassagers = query.value("nomPassagers").toString();
        details.coutVol = query.value("coutVol").toDouble();
        details.montantRembouse = query.value("montantVolRembourse").toDouble();
        
        if (!query.value("idRecette").isNull())
        {
            details.idRecette = query.value("idRecette").toInt();
            details.intituleRecette = query.value("intituleRecette").toString();
            details.montantRecette = query.value("montantRecette").toDouble();
        }
        
        listeDetails.append(details);
    }

    return listeDetails;
}

const bool ManageDb::volSembleExistantEnBdd(const QString p_idPilote, 
    const int p_duree, 
    const QString p_date, 
    const double p_coutDuVol)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM vol WHERE pilote = :pilote AND date = :date AND duree = :duree");
    query.bindValue(":pilote", p_idPilote);
    query.bindValue(":duree", p_duree);
    query.bindValue(":date", p_date);
    query.bindValue(":cout", p_coutDuVol);
    query.exec();

    return query.next();
}

const QString ManageDb::genererClauseFiltrageActivite(const int p_options)
{
    QString filtre = "";
 
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_PLANEUR) == AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_PLANEUR)
    {
        filtre = filtre + "activite != 'Planeur' ";
    }
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_ULM) == AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_ULM)
    {
        if (filtre != "")
        {
            filtre = filtre + " AND ";
        }
        filtre = filtre + "activite != 'ULM' ";
    }
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_HELICOPTERE) == AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_HELICOPTERE)
    {
        if (filtre != "")
        {
            filtre = filtre + " AND ";
        }
        filtre = filtre + "activite != 'Hélicoptère' ";
    }
    if ((p_options & AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION) == AeroDmsTypes::OptionsDonneesStatistiques_EXCLURE_AVION)
    {
        if (filtre != "")
        {
            filtre = filtre + " AND ";
        }
        filtre = filtre + "activite != 'Avion électrique' AND activite != 'Avion' ";
    }

    return filtre;
}

const AeroDmsTypes::Status ManageDb::mettreAJourAerodrome(const QString p_indicatifOaci, 
    const QString p_nom)
{
	QSqlQuery query;

	query.prepare("SELECT * FROM aerodrome WHERE identifiantOaci = :identifiantOaci");
    query.bindValue(":identifiantOaci", p_indicatifOaci);
	query.exec();

    //Si le terrain existe, on vérifie si besoin de le mettre à jour
    if (query.next())
    {
        if (query.value("nomAerodrome") != p_nom)
        {
            query.prepare("UPDATE aerodrome SET nomAerodrome = :nomAerodrome WHERE identifiantOaci = :identifiantOaci");
            query.bindValue(":nomAerodrome", p_nom);
            query.bindValue(":identifiantOaci", p_indicatifOaci);
            query.exec();

            QThread::msleep(delaisDeGardeBdd);

            return AeroDmsTypes::Status_MIS_A_JOUR;
        }
    }
    //Sinon on le créé
    else
	{
		query.prepare("INSERT INTO aerodrome ('identifiantOaci','nomAerodrome') VALUES(:identifiantOaci,:nomAerodrome)");
		query.bindValue(":identifiantOaci", p_indicatifOaci);
		query.bindValue(":nomAerodrome", p_nom);
		query.exec();

        QThread::msleep(delaisDeGardeBdd);

        return AeroDmsTypes::Status_CREATION;
	}

    return AeroDmsTypes::Status_AUCUNE_ACTION_EFFECTUEE;
}

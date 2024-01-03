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

#include "ManageDb.h"
#include "QMessageBox"

ManageDb::ManageDb()
{
}

ManageDb::ManageDb(const QString &database) {
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        QMessageBox::critical(
            this,
            "Unable to load database",
            "This software needs the SQLITE driver"
        );

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(database);
    if (!db.open())
    {
        QMessageBox::warning(this, "Erreur Ouverture", db.lastError().text());
    }

    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Impossible d'ouvrir la base de données"),
            tr("Je ne parvient pas à ouvrir la base de données car l'erreur suivante s'est produite : \n"
                "Cliquez Annuler pour quitter"), QMessageBox::Cancel);
    }
}

QString ManageDb::getLastError()
{
    return db.lastError().text();
}

AeroDmsTypes::ListePilotes ManageDb::recupererPilotes()
{
    AeroDmsTypes::ListePilotes listeDesPilotes;

    const QString sql = "SELECT * FROM pilote";

    QSqlQuery query;
    query.exec(sql);

    while (query.next()) {
        AeroDmsTypes::Pilote pilote;
        pilote.idPilote = query.value(0).toString();
        pilote.nom = query.value(1).toString();
        pilote.prenom = query.value(2).toString();
        pilote.aeroclub = query.value(3).toString();
        pilote.estAyantDroit = query.value(4).toBool();
        pilote.mail = query.value(5).toString();
        pilote.telephone = query.value(6).toString();
        pilote.remarque = query.value(7).toString();
        listeDesPilotes.append(pilote);
    }

    return listeDesPilotes;
}

//Cette methode retourne le prochain ID de facture disponible
int ManageDb::recupererProchainNumeroFacture()
{
    const QString sql = "SELECT seq FROM sqlite_sequence WHERE name = 'fichiersFacture'";

    QSqlQuery query;
    query.exec(sql);
    query.next();
    return query.value(0).toInt() + 1 ;
}

AeroDmsTypes::ListeSubventionsParPilotes ManageDb::recupererSubventionsPilotes(int p_annee)
{
    QSqlQuery query;
    query.prepare("SELECT *  FROM volParTypeParAnEtParPilote");
    query.exec();

    QString idPilote = "";
    int annee = 0;

    AeroDmsTypes::ListeSubventionsParPilotes liste;

    AeroDmsTypes::SubventionsParPilote subvention;

    while (query.next())
    {
        if (idPilote != query.value("pilote").toString()
            || annee != query.value("annee").toInt())
        {
            if (annee != 0)
            {
                liste.append(subvention);
            }
            annee = query.value("annee").toInt();
            idPilote = query.value("pilote").toString();
            subvention.idPilote = query.value("pilote").toString();
            subvention.annee = query.value("annee").toInt();
            subvention.nom = query.value("nom").toString();
            subvention.prenom = query.value("prenom").toString();
            subvention.aeroclub = query.value("aeroclub").toString();
            subvention.sortie.heuresDeVol = "0h00";
            subvention.sortie.montantRembourse = 0;
            subvention.entrainement.heuresDeVol = "0h00";
            subvention.entrainement.montantRembourse = 0;
            subvention.balade.heuresDeVol = "0h00";
            subvention.balade.montantRembourse = 0;
        }

        if (query.value("typeDeVol").toString() == "Entrainement")
        {
            subvention.entrainement.heuresDeVol = convertirMinutesEnHeuresMinutes(query.value("tempsDeVol").toInt());
            subvention.entrainement.montantRembourse = query.value("montantRembourse").toFloat();
        }
        else if (query.value("typeDeVol").toString() == "Sortie")
        {
            subvention.sortie.heuresDeVol = convertirMinutesEnHeuresMinutes(query.value("tempsDeVol").toInt());
            subvention.sortie.montantRembourse = query.value("montantRembourse").toFloat();
        }
        else if(query.value("typeDeVol").toString() == "Balade")
        {
            subvention.balade.heuresDeVol = convertirMinutesEnHeuresMinutes(query.value("tempsDeVol").toInt());
            subvention.balade.montantRembourse = query.value("montantRembourse").toFloat();
        }
    }

    //Si année différente de 0, c'est qu'on était sur une liste non nulle, on ajoute le dernier élement
    if (annee != 0)
    {
        liste.append(subvention);
    }

    return liste;
}

QString ManageDb::convertirMinutesEnHeuresMinutes(const int p_minutes)
{
    const int heures = p_minutes / 60;
    const int minutes = p_minutes % 60;
    QString minutesString = QString::number(minutes);
    if (minutesString.size() == 1)
    {
        minutesString = QString("0").append(minutesString);
    }
    QString heuresMinutes = QString::number(heures).append("h").append(minutesString);
    return heuresMinutes;
}

int ManageDb::ajouterFacture(QString& p_nomFichier)
{
    QString sql = "INSERT INTO 'fichiersFacture' ('nomFichier') VALUES('";
    sql.append(p_nomFichier);
    sql.append("')");

    QSqlQuery query;
    query.exec(sql);

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
float ManageDb::recupererSubventionRestante(const QString& p_piloteId, const int p_annee)
{
    QString sql = QString("SELECT subventionAllouee FROM subventionEntrainementAlloueeParPiloteEtParAnnee WHERE pilote = ':piloteId' AND annee = ':annee'").replace(":piloteId", p_piloteId).replace(":annee", QString::number(p_annee));
    QSqlQuery query;
    query.exec(sql);
    query.next();
    //TODO => chercher le montant en BDD
    return 750.0 - query.value(0).toFloat();
}

void ManageDb::enregistrerUnVolDEntrainement( const QString& p_piloteId,
                                              const QString& p_typeDeVol,
                                              const QDate& p_date,
                                              const int p_dureeEnMinutes,
                                              const float p_cout,
                                              const float p_montantSubventionne,
                                              const int p_facture,
                                              const QString& p_remarque)
{
    QSqlQuery query;
    query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','remarque') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:remarque)");
    query.bindValue(":typeDeVol", p_typeDeVol);
    query.bindValue(":pilote", p_piloteId);
    query.bindValue(":date", p_date.toString("yyyy-MM-dd"));
    query.bindValue(":duree", p_dureeEnMinutes);
    query.bindValue(":cout", p_cout);
    query.bindValue(":montantRembourse", p_montantSubventionne);
    query.bindValue(":facture", p_facture);
    query.bindValue(":remarque", p_remarque);

    query.exec();
}

void ManageDb::enregistrerUnVolSortieOuBalade(const QString& p_piloteId,
    const QString& p_typeDeVol,
    const QDate& p_date,
    const int p_dureeEnMinutes,
    const float p_cout,
    const float p_montantSubventionne,
    const int p_facture,
    const int p_idSortie,
    const QString& p_remarque)
{
    QSqlQuery query;
    query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','sortie','remarque') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:sortie,:remarque)");
    query.bindValue(":typeDeVol", p_typeDeVol);
    query.bindValue(":pilote", p_piloteId);
    query.bindValue(":date", p_date.toString("yyyy-MM-dd"));
    query.bindValue(":duree", p_dureeEnMinutes);
    query.bindValue(":cout", p_cout);
    query.bindValue(":montantRembourse", p_montantSubventionne);
    query.bindValue(":facture", p_facture);
    query.bindValue(":sortie", p_idSortie);
    query.bindValue(":remarque", p_remarque);

    query.exec();
}

void ManageDb::enregistrerUneFacture( const QString& p_payeur,
                                      const int factureId,
                                      const QDate& p_date,
                                      const float p_montantFacture,
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

    query.exec();
}

void ManageDb::ajouterUneRecetteAssocieeAVol( const QStringList &p_listeVols,
                                              const QString& p_typeDeRecette,
                                              const QString& p_intitule,
                                              const float p_montant)
{
    QSqlQuery query;
    query.prepare("INSERT INTO 'recettes' ('typeDeRecette','intitule','montant') VALUES(:typeRecette, :intitule, :montant) RETURNING recetteId");
    query.bindValue(":typeRecette", p_typeDeRecette);
    query.bindValue(":intitule", p_intitule);
    query.bindValue(":montant", p_montant);

    query.exec();
    query.next();
    const int numeroDeRecetteCree = query.value(0).toInt();

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
        query.exec();
    }

}

bool ManageDb::piloteEstAJourDeCotisation(const QString& p_piloteId, const int annee)
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

AeroDmsTypes::ListeDemandeRemboursement ManageDb::recupererLesSubventionsAEmettre()
{
    //Recuperation de l'ID de facture
    QString sql = "SELECT * FROM 'volARembourserParTypeParPiloteEtParAnnee'";

    AeroDmsTypes::ListeDemandeRemboursement liste;
    QSqlQuery query;
    query.exec(sql);
    while (query.next()) {
        AeroDmsTypes::DemandeRemboursement demande;
        demande.typeDeVol = query.value(0).toString();
        demande.piloteId = query.value(1).toString();
        demande.montantARembourser = query.value(3).toFloat();
        demande.annee = query.value(2).toInt();
        //demande.nomFichierFacture = query.value(5).toString();
        liste.append(demande);
    }
    
    return liste;
}

QStringList ManageDb::recupererListeFacturesAssocieeASubvention(const  AeroDmsTypes::DemandeRemboursement p_demande)
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
        listeFactures.append(QString("C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesTraitees/").append(query.value(5).toString()));
    }

    return listeFactures;
}

void ManageDb::ajouterDemandeCeEnBdd(AeroDmsTypes::DemandeEnCoursDeTraitement p_demande)
{
    //On créé l'entrée dans demandeRemboursementSoumises
    QSqlQuery query;
    query.prepare("INSERT INTO demandeRemboursementSoumises (dateDemande, montant, nomBeneficiaire, typeDeDemande) VALUES (:dateDemande, :montant, :nomBeneficiaire, :typeDeDemande) RETURNING demandeRemboursementSoumises.demandeId");
    query.bindValue(":dateDemande", QDate::currentDate().toString("yyyy-MM-dd"));
    query.bindValue(":montant", p_demande.montant);
    query.bindValue(":nomBeneficiaire", p_demande.nomBeneficiaire);
    query.bindValue(":typeDeDemande", p_demande.typeDeVol);
    query.exec();
    query.next();
    const int idDemandeRemboursement = query.value(0).toInt();

    if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_HEURE_DE_VOL)
    {
        query.prepare("UPDATE vol SET demandeRemboursement = :idDemandeRemboursement WHERE strftime('%Y', vol.date) = :annee AND vol.pilote = :pilote AND vol.typeDeVol = :typeDeVol");
        query.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
        query.bindValue(":annee", QString::number(p_demande.annee));
        query.bindValue(":pilote", p_demande.idPilote);
        query.bindValue(":typeDeVol", p_demande.typeDeVol);
        query.exec();    
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_COTISATION)
    {
        query.prepare(QString("SELECT idRecette FROM cotisationsASoumettreCe WHERE annee = ").append(QString::number(p_demande.annee)));
        query.exec();


        while (query.next())
        {
            QSqlQuery queryCotisation;
            queryCotisation.prepare("UPDATE recettes SET identifiantFormulaireSoumissionCe = :idDemandeRemboursement WHERE recetteId = :recetteId");
            queryCotisation.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
            queryCotisation.bindValue(":recetteId", query.value(0).toInt());
            queryCotisation.exec();
        }
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_PAIEMENT_SORTIE_OU_BALADE)
    {
        //query.prepare(QString("SELECT idRecette FROM recettesASoumettreCe WHERE annee = ").append(QString::number(p_demande.annee)));
        query.prepare("SELECT recetteId FROM recettesASoumettreCe WHERE annee = :annee AND nom = :nomSortie");
        query.bindValue(":annee", QString::number(p_demande.annee));
        query.bindValue(":nomSortie", p_demande.typeDeVol);
        query.exec();

        while (query.next())
        {
            QSqlQuery querySortie;
            querySortie.prepare("UPDATE recettes SET identifiantFormulaireSoumissionCe = :idDemandeRemboursement WHERE recetteId = :recetteId");
            querySortie.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
            querySortie.bindValue(":recetteId", query.value(0).toInt());
            querySortie.exec();
        }
    }
    else if (p_demande.typeDeDemande == AeroDmsTypes::PdfTypeDeDemande_FACTURE)
    {
        //query.prepare(QString("SELECT idRecette FROM recettesASoumettreCe WHERE annee = ").append(QString::number(p_demande.annee)));
        query.prepare("UPDATE facturesSorties SET demandeRemboursement = :idDemandeRemboursement WHERE id = :id");
        query.bindValue(":idDemandeRemboursement", idDemandeRemboursement);
        //L'année contient en fait l'ID de facture pour une facture...
        query.bindValue(":id", p_demande.annee);
        query.exec();
    }
}

AeroDmsTypes::ListeRecette ManageDb::recupererLesCotisationsAEmettre()
{
    AeroDmsTypes::ListeRecette liste;
    QSqlQuery query;
    query.prepare("SELECT annee, nom, montant FROM cotisationsASoumettreCe");
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
            recette.montant = recette.montant+ query.value(2).toFloat();
            recette.annee = query.value(0).toInt();
        }
        //On oublie pas d'ajouter la dernière année...
        //On retire le dernier /, soit le dernier caractère...
        recette.intitule.chop(1);
        //... et on ferme la parenthèse
        recette.intitule.append(")");
        if(recette.intitule != ")")
            liste.append(recette);
    }
    return liste;
}

AeroDmsTypes::ListeRecette ManageDb::recupererLesRecettesBaladesEtSortiesAEmettre()
{
    //Récupération des cotisation à soumettre au CE
    AeroDmsTypes::ListeRecette liste;
    QSqlQuery query;
    query.prepare("SELECT * FROM 'recettesASoumettreCeParTypeEtParAnnee'");
    query.exec();
    
    while (query.next()) {
        AeroDmsTypes::Recette recette;
        recette.typeDeSortie = query.value(0).toString();
        recette.intitule = query.value(3).toString();
        recette.annee = query.value(1).toInt();
        recette.montant = query.value(2).toFloat();
        liste.append(recette);
    }

    return liste;
}

AeroDmsTypes::ListeDemandeRemboursementFacture ManageDb::recupererLesDemandesDeRembousementAEmettre()
{
    //Récupérationd des demandes de remboursement à soumettre au CE
    AeroDmsTypes::ListeDemandeRemboursementFacture liste;
    QSqlQuery query;
    query.prepare("SELECT * FROM 'facturesARembourser'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::DemandeRemboursementFacture demandeRemboursement;
        /*demandeRemboursement.typeDeSortie = query.value(0).toString();
        demandeRemboursement.intitule = query.value(3).toString();
        demandeRemboursement.annee = query.value(1).toInt();
        demandeRemboursement.montant = query.value(2).toFloat();*/
        demandeRemboursement.id = query.value(0).toInt();
        demandeRemboursement.intitule = query.value(1).toString();
        demandeRemboursement.montant = query.value(2).toFloat();
        demandeRemboursement.payeur = query.value(4).toString().append(" ").append(query.value(3).toString());
        demandeRemboursement.nomSortie = query.value(5).toString();
        demandeRemboursement.typeDeSortie = query.value(6).toString();
        demandeRemboursement.annee = query.value(7).toInt();
        demandeRemboursement.nomFacture = query.value(8).toString();

        liste.append(demandeRemboursement);
    }

    return liste;
}

QString ManageDb::recupererAeroclub(QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT aeroclub FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value(0).toString();
}

QString ManageDb::recupererNomPrenomPilote(QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT prenom, nom FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value(0).toString().append(" ").append(query.value(1).toString());
}

int ManageDb::recupererLigneCompta(QString p_typeDeRecetteDepenseId)
{
    QSqlQuery query;
    query.prepare("SELECT identifiantCompta FROM typeDeRecetteDepense WHERE typeDeRecetteDepenseId = :typeDeRecetteDepenseId");
    query.bindValue(":typeDeRecetteDepenseId", p_typeDeRecetteDepenseId);

    query.exec();
    query.next();
    return query.value(0).toInt();
}

QStringList ManageDb::recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette)
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

QStringList ManageDb::recupererBaladesEtSorties(QString p_typeDeVol)
{
    QStringList liste;
    QSqlQuery query;
    query.prepare("SELECT * FROM 'volsBaladesEtSorties' WHERE typeDeVol = :typeDeVol");
    query.bindValue(":typeDeVol", p_typeDeVol);
    query.exec();
    while (query.next()) {
        liste.append(query.value(2).toString());
    }

    return liste;
}

AeroDmsTypes::ListeSortie ManageDb::recupererListeSorties()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE date IS NOT NULL");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value(0).toInt();
        sortie.nom = query.value(1).toString();
        sortie.date = query.value(2).toDate();
        liste.append(sortie);
    }

    return liste;
}

AeroDmsTypes::ListeSortie ManageDb::recupererListeDepensesPouvantAvoirUneFacture()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE nom != 'Balade'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value(0).toInt();
        sortie.nom = query.value(1).toString();
        sortie.date = query.value(2).toDate();
        liste.append(sortie);
    }

    return liste;
}

AeroDmsTypes::ListeSortie ManageDb::recupererListeBalade()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie WHERE nom = 'Balade'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::Sortie sortie;
        sortie.id = query.value(0).toInt();
        sortie.nom = query.value(1).toString();
        sortie.date = query.value(2).toDate();
        liste.append(sortie);
    }

    return liste;
}

void ManageDb::ajouterCotisation(AeroDmsTypes::CotisationAnnuelle& p_infosCotisation)
{
    QSqlQuery query;
    query.prepare("SELECT nom, prenom FROM 'pilote' WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_infosCotisation.idPilote);
    query.exec();
    query.next();
    const QString intitule = QString("Cotisation ").append(query.value(0).toString()).append(" ").append(query.value(1).toString()).append(" ").append(QString::number(p_infosCotisation.annee));

    query.prepare("INSERT INTO 'recettes' ('typeDeRecette','Intitule','montant') VALUES ('Cotisation', :intitule, :montant) RETURNING recetteId");
    query.bindValue(":intitule", intitule);
    query.bindValue(":montant", p_infosCotisation.montant);
    query.exec();
    query.next();

    const int idRecette = query.value(0).toInt();

    query.prepare("INSERT INTO 'cotisation' ('pilote','annee','idRecette', 'montantSubventionAnnuelleEntrainement') VALUES (:pilote, :annee, :idRecette, :montantSubvention)");
    query.bindValue(":pilote", p_infosCotisation.idPilote);
    query.bindValue(":annee", p_infosCotisation.annee);
    query.bindValue(":idRecette", idRecette);
    query.bindValue(":montantSubvention", p_infosCotisation.montantSubvention);
    query.exec();
}

//Cette fonction créé (p_pilote.idPilote = "") ou met à jour (p_pilote.idPilote renseigné) un pilote
//dans la base de données
AeroDmsTypes::ResultatCreationPilote ManageDb::creerPilote(AeroDmsTypes::Pilote p_pilote)
{
    AeroDmsTypes::ResultatCreationPilote resultat = AeroDmsTypes::ResultatCreationPilote_SUCCES;
    
    //creation
    if (p_pilote.idPilote == "")
    {
        QString piloteId = p_pilote.prenom.toLower();
        piloteId.append(".");
        piloteId.append(p_pilote.nom.toLower());

        piloteId.replace(" ", "");
        piloteId.replace("é", "e");
        piloteId.replace("è", "e");
        piloteId.replace("à", "a");
        piloteId.replace("â", "a");

        //Verifier si le pilote existe
        QSqlQuery query;
        query.prepare("SELECT * FROM pilote WHERE piloteId = :piloteId");
        query.bindValue(":piloteId", piloteId);
        query.exec();
        query.next();
        if (query.size() > 0)
        {
            resultat = AeroDmsTypes::ResultatCreationPilote_PILOTE_EXISTE;
        }
        else
        {
            query.prepare("INSERT INTO 'pilote' ('piloteId','nom','prenom','aeroclub','estAyantDroit','mail','telephone','remarque') VALUES(:piloteId,:nom,:prenom,:aeroclub,:estAyantDroit,:mail,:telephone,:remarque)");
            query.bindValue(":piloteId", piloteId);
            query.bindValue(":nom", p_pilote.nom);
            query.bindValue(":prenom", p_pilote.prenom);
            query.bindValue(":aeroclub", p_pilote.aeroclub);
            query.bindValue(":estAyantDroit", p_pilote.estAyantDroit);
            query.bindValue(":mail", p_pilote.mail);
            query.bindValue(":telephone", p_pilote.telephone);
            query.bindValue(":remarque", p_pilote.remarque);

            if (!query.exec())
            {
                resultat = AeroDmsTypes::ResultatCreationPilote_AUTRE;
            }
        } 
    }
    //mise à jour
    else
    {
        //TODO
    }

    return resultat;
}

void ManageDb::creerSortie(AeroDmsTypes::Sortie p_sortie)
{
    QSqlQuery query;
    query.prepare("INSERT INTO sortie ('nom','date') VALUES (:nom,:date)");
    query.bindValue(":nom",p_sortie.nom);
    query.bindValue(":date", p_sortie.date.toString("yyyy-MM-dd"));
    query.exec();
}

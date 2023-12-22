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
    //db.setDatabaseName("C:/Users/cleme/OneDrive/Documents/AeroDMS/AeroDMS.sqlite");
    //db.setDatabaseName("./AeroDMS.sqlite");
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
    qDebug() << sql;

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
    qDebug() << sql;

    QSqlQuery query;
    query.exec(sql);
    query.next();
    return query.value(0).toInt() + 1 ;
}

int ManageDb::ajouterFacture(QString& p_nomFichier)
{
    QString sql = "INSERT INTO 'fichiersFacture' ('nomFichier') VALUES('";
    sql.append(p_nomFichier);
    sql.append("')");
    qDebug() << sql;

    QSqlQuery query;
    query.exec(sql);

    //Recuperation de l'ID de facture
    sql = "SELECT * FROM 'fichiersFacture' WHERE nomFichier='";
    sql.append(p_nomFichier);
    sql.append("'");
    qDebug() << sql;

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
    //query.prepare("SELECT subventionAllouee FROM subventionEntrainementAlloueeParPiloteEtParAnnee WHERE pilote = :piloteId AND annee = :annee");
    //query.bindValue(":piloteId", p_piloteId);
    //query.bindValue(":annee", p_annee);
    qDebug() << sql;
    query.exec(sql);
    query.next();
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

        qDebug() << numeroDeVol << numeroDeRecetteCree;

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
    qDebug() << sql;

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
    qDebug() << sql;

    AeroDmsTypes::ListeDemandeRemboursement liste;
    QSqlQuery query;
    query.exec(sql);
    while (query.next()) {
        AeroDmsTypes::DemandeRemboursement demande;
        demande.typeDeVol = query.value(0).toString();
        demande.piloteId = query.value(1).toString();
        demande.montantARembourser = query.value(2).toFloat();
        demande.annee = query.value(3).toInt();
        demande.nomFichierFacture = query.value(5).toString();
        liste.append(demande);
    }
    
    return liste;
}

QStringList ManageDb::recupererTypesDesVol(bool recupererUniquementLesTypesDeVolAvecRecette)
{
    QString sql = "SELECT * FROM 'typeDeRecetteDepense' WHERE estVol = 1";
    if (recupererUniquementLesTypesDeVolAvecRecette)
    {
        sql.append(" AND estRecette = 1");
    }
    qDebug() << sql;

    QStringList liste;
    QSqlQuery query;
    query.exec(sql);
    while (query.next()) {
        liste.append(query.value(0).toString());
    }

    return liste;
}

QStringList ManageDb::recupererBaladesEtSorties()
{
    QString sql = "SELECT * FROM 'volsBaladesEtSorties'";
    qDebug() << sql;

    QStringList liste;
    QSqlQuery query;
    query.exec(sql);
    while (query.next()) {
        liste.append(query.value(2).toString());
    }

    return liste;
}

AeroDmsTypes::ListeSortie ManageDb::recupererListeSorties()
{
    AeroDmsTypes::ListeSortie liste;

    QSqlQuery query;
    query.prepare("SELECT * FROM sortie");
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
    qDebug() << query.lastQuery();
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

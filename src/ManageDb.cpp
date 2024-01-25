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
#include "AeroDmsServices.h"

ManageDb::ManageDb()
{
}

ManageDb::ManageDb(const QString &database, const int p_delaisDeGardeBdd) 
{
    delaisDeGardeBdd = p_delaisDeGardeBdd;

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

    const QString sql = "SELECT * FROM pilote ORDER BY pilote.prenom";

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

AeroDmsTypes::Pilote ManageDb::recupererPilote(const QString p_idPilote)
{
    AeroDmsTypes::Pilote pilote;

    QSqlQuery query;
    query.prepare("SELECT * FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_idPilote); 
    query.exec();

    while (query.next()) {
        pilote.idPilote = query.value("piloteId").toString();
        pilote.nom = query.value("nom").toString();
        pilote.prenom = query.value("prenom").toString();
        pilote.aeroclub = query.value("aeroclub").toString();
        pilote.estAyantDroit = query.value("esrtAyantDroit").toBool();
        pilote.mail = query.value("mail").toString();
        pilote.telephone = query.value("telephone").toString();
        pilote.remarque = query.value("remarque").toString();
    }

    return pilote;
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

AeroDmsTypes::ListeSubventionsParPilotes ManageDb::recupererSubventionsPilotes( const int p_annee, 
                                                                                const QString p_piloteId,
                                                                                const bool p_volsSoumisUniquement)
{
    QSqlQuery query;
    AeroDmsTypes::ListeSubventionsParPilotes liste;

    if (p_annee != -1 && p_piloteId != "*")
    {
        query.prepare("SELECT * FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId WHERE annee = :annee AND pilote = :piloteId ORDER BY annee, pilote.nom");    
    }
    else if (p_annee != -1)
    {
        query.prepare("SELECT * FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId WHERE annee = :annee ORDER BY annee, pilote.nom");
    }
    else if (p_piloteId != "*")
    {
        query.prepare("SELECT * FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId WHERE pilote = :piloteId ORDER BY annee, pilote.nom");
    }
    else
    {
        query.prepare("SELECT * FROM cotisation INNER JOIN pilote ON cotisation.pilote = pilote.piloteId ORDER BY annee, pilote.nom");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":piloteId", p_piloteId);

    query.exec();

    //pour chaque pilote et chaque année de cotisation, on recupère les éventuelles heures de vol effectuées
    while (query.next())
    {
        int heuresDeVolEnMinutes = 0;
        AeroDmsTypes::SubventionsParPilote subvention;

        subvention.idPilote = query.value("pilote").toString();
        subvention.annee = query.value("annee").toInt();
        subvention.nom = query.value("nom").toString();
        subvention.prenom = query.value("prenom").toString();
        subvention.aeroclub = query.value("aeroclub").toString();
        subvention.sortie.heuresDeVol = "0h00";
        subvention.sortie.tempsDeVolEnMinutes = 0;
        subvention.sortie.montantRembourse = 0;
        subvention.sortie.coutTotal = 0;
        subvention.entrainement.heuresDeVol = "0h00";
        subvention.entrainement.tempsDeVolEnMinutes = 0;
        subvention.entrainement.montantRembourse = 0;
        subvention.entrainement.coutTotal = 0;
        subvention.balade.heuresDeVol = "0h00";
        subvention.balade.tempsDeVolEnMinutes = 0;
        subvention.balade.montantRembourse = 0;
        subvention.balade.coutTotal = 0;
        subvention.totaux.heuresDeVol = "0h00";
        subvention.totaux.tempsDeVolEnMinutes = 0;
        subvention.totaux.montantRembourse = 0;
        subvention.totaux.coutTotal = 0;

        QSqlQuery queryVolAnneePilote;
        queryVolAnneePilote.prepare("SELECT *  FROM volParTypeParAnEtParPilote WHERE annee = :annee AND pilote = :piloteId");
        //Si on veut uniquement les totaux des vols déjà soumis au CSE, on remplace la vue volParTypeParAnEtParPilote par volParTypeParAnEtParPiloteSoumis
        if (p_volsSoumisUniquement)
        {
            queryVolAnneePilote.prepare("SELECT *  FROM volParTypeParAnEtParPiloteSoumis WHERE annee = :annee AND pilote = :piloteId");
        }
        queryVolAnneePilote.bindValue(":annee", QString::number(subvention.annee));
        queryVolAnneePilote.bindValue(":piloteId", subvention.idPilote);
        queryVolAnneePilote.exec();

        while (queryVolAnneePilote.next())
        {
            if (queryVolAnneePilote.value("typeDeVol").toString() == "Entrainement")
            {
                subvention.entrainement.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.entrainement.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.entrainement.montantRembourse = queryVolAnneePilote.value("montantRembourse").toFloat();
                subvention.entrainement.coutTotal = queryVolAnneePilote.value("cout").toFloat();
            }
            else if (queryVolAnneePilote.value("typeDeVol").toString() == "Sortie")
            {
                subvention.sortie.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.sortie.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.sortie.montantRembourse = queryVolAnneePilote.value("montantRembourse").toFloat();
                subvention.sortie.coutTotal = queryVolAnneePilote.value("cout").toFloat();
            }
            else if (queryVolAnneePilote.value("typeDeVol").toString() == "Balade")
            {
                subvention.balade.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(queryVolAnneePilote.value("tempsDeVol").toInt());
                subvention.balade.tempsDeVolEnMinutes = queryVolAnneePilote.value("tempsDeVol").toInt();
                subvention.balade.montantRembourse = queryVolAnneePilote.value("montantRembourse").toFloat();
                subvention.balade.coutTotal = queryVolAnneePilote.value("cout").toFloat();
            }
            heuresDeVolEnMinutes = heuresDeVolEnMinutes + queryVolAnneePilote.value("tempsDeVol").toInt();
            subvention.totaux.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(heuresDeVolEnMinutes);
            subvention.totaux.montantRembourse = subvention.totaux.montantRembourse + queryVolAnneePilote.value("montantRembourse").toFloat();
            subvention.totaux.coutTotal = subvention.totaux.coutTotal + queryVolAnneePilote.value("cout").toFloat();
        }
        liste.append(subvention);
    }
    return liste;
}

AeroDmsTypes::SubventionsParPilote ManageDb::recupererTotauxAnnuel( const int p_annee,
                                                                    const bool p_volsSoumisUniquement)
{
    AeroDmsTypes::SubventionsParPilote totaux;
    totaux.idPilote = "";
    totaux.annee = p_annee;
    totaux.nom = "Totaux";
    totaux.prenom = "";
    totaux.aeroclub = "";
    totaux.sortie.heuresDeVol = "0h00";
    totaux.sortie.montantRembourse = 0;
    totaux.sortie.coutTotal = 0;
    totaux.entrainement.heuresDeVol = "0h00";
    totaux.entrainement.montantRembourse = 0;
    totaux.entrainement.coutTotal = 0;
    totaux.balade.heuresDeVol = "0h00";
    totaux.balade.montantRembourse = 0;
    totaux.balade.coutTotal = 0;
    totaux.totaux.heuresDeVol = "0h00";
    totaux.totaux.montantRembourse = 0;
    totaux.totaux.coutTotal = 0;

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
            totaux.entrainement.montantRembourse = query.value("subventionTotale").toFloat();
            totaux.entrainement.coutTotal = query.value("coutTotal").toFloat();
        }
        else if (query.value("typeDeVol").toString() == "Sortie")
        {
            totaux.sortie.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("tempsDeVolTotal").toInt());
            totaux.sortie.montantRembourse = query.value("subventionTotale").toFloat();
            totaux.sortie.coutTotal = query.value("coutTotal").toFloat();
        }
        else if (query.value("typeDeVol").toString() == "Balade")
        {
            totaux.balade.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("tempsDeVolTotal").toInt());
            totaux.balade.montantRembourse = query.value("subventionTotale").toFloat();
            totaux.balade.coutTotal = query.value("coutTotal").toFloat();
        }
        heuresDeVolEnMinutes = heuresDeVolEnMinutes + query.value("tempsDeVolTotal").toInt();
        totaux.totaux.heuresDeVol = AeroDmsServices::convertirMinutesEnHeuresMinutes(heuresDeVolEnMinutes);
        totaux.totaux.montantRembourse = totaux.totaux.montantRembourse + query.value("subventionTotale").toFloat();
        totaux.totaux.coutTotal = totaux.totaux.coutTotal + query.value("coutTotal").toFloat();
    }

    return totaux;
}

AeroDmsTypes::Vol ManageDb::recupererVol(const int p_idVol)
{
    AeroDmsTypes::Vol vol;

    QSqlQuery query;
    query.prepare("SELECT *  FROM vol WHERE volId = :volId");
    query.bindValue(":volId", p_idVol);

    query.exec();
    query.next();

    vol.coutVol = query.value("cout").toFloat();
    vol.date = query.value("date").toDate();
    vol.duree = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("duree").toInt());
    vol.estSoumisCe = "Oui";
    if (query.value("demandeRemboursement").isNull())
        vol.estSoumisCe = "Non";
    vol.idPilote = query.value("pilote").toString();
    vol.montantRembourse = query.value("montantRembourse").toFloat();
    vol.remarque = query.value("remarque").toString();
    vol.typeDeVol = query.value("typeDeVol").toString();
    vol.volId = query.value("volId").toInt();
    vol.baladeId = -1;
    if (!query.value("sortie").isNull())
        vol.baladeId = query.value("sortie").toInt();
     vol.activite = query.value("activite").toString();

    return vol;
}

AeroDmsTypes::ListeVols ManageDb::recupererVols( const int p_annee, 
                                                 const QString p_piloteId)
{
    AeroDmsTypes::ListeVols liste;

    QSqlQuery query;
    if (p_annee != -1 && p_piloteId != "*")
    {
        query.prepare("SELECT *  FROM vols WHERE strftime('%Y', vols.date) = :annee AND pilote = :piloteId ORDER BY date");
    }
    else if (p_annee != -1)
    {
        query.prepare("SELECT *  FROM vols WHERE strftime('%Y', vols.date) = :annee ORDER BY date");
    }
    else if (p_piloteId != "*")
    {
        query.prepare("SELECT *  FROM vols WHERE pilote = :piloteId ORDER BY date");
    }
    else
    {
        query.prepare("SELECT *  FROM vols ORDER BY date");
    }
    query.bindValue(":annee", QString::number(p_annee));
    query.bindValue(":piloteId", p_piloteId);

    
    query.exec();

    while (query.next())
    {
        AeroDmsTypes::Vol vol;
        vol.coutVol = query.value("cout").toFloat();
        vol.date = query.value("date").toDate();
        vol.duree = AeroDmsServices::convertirMinutesEnHeuresMinutes(query.value("duree").toInt());
        vol.estSoumisCe = "Oui";
        if(query.value("demandeRemboursement").isNull())
            vol.estSoumisCe = "Non";
        vol.idPilote = query.value("pilote").toString();
        vol.montantRembourse = query.value("montantRembourse").toFloat();
        vol.nomPilote = query.value("nom").toString();
        vol.prenomPilote = query.value("prenom").toString();
        vol.remarque = query.value("remarque").toString();
        vol.typeDeVol = query.value("typeDeVol").toString();
        vol.activite = query.value("activite").toString();
        vol.volId = query.value("volId").toInt();
        if (!query.value("sortie").isNull())
        {
            vol.baladeId = query.value("sortie").toInt();
        }
            
 
        liste.append(vol);
    }

    return liste;
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
float ManageDb::recupererSubventionRestante( const QString& p_piloteId, 
                                             const int p_annee)
{
    QSqlQuery query;

    QString sql = QString("SELECT montantSubventionAnnuelleEntrainement FROM cotisation WHERE pilote = ':piloteId' AND annee = ':annee'").replace(":piloteId", p_piloteId).replace(":annee", QString::number(p_annee));
    query.exec(sql);
    query.next();
    float montantAlloue = query.value(0).toFloat();

    sql = QString("SELECT subventionAllouee FROM subventionEntrainementAlloueeParPiloteEtParAnnee WHERE pilote = ':piloteId' AND annee = ':annee'").replace(":piloteId", p_piloteId).replace(":annee", QString::number(p_annee));
    query.exec(sql);
    query.next();
    
    return montantAlloue - query.value(0).toFloat();
}

void ManageDb::enregistrerUnVol(const QString& p_piloteId,
    const QString& p_typeDeVol,
    const QDate& p_date,
    const int p_dureeEnMinutes,
    const float p_cout,
    const float p_montantSubventionne,
    const int p_facture,
    const int p_idSortie,
    const QString& p_remarque,
    const QString& p_activite,
    const int p_idVolAEditer)
{
    QSqlQuery query;
    //Si on est sur un ajout
    if (p_idVolAEditer == -1)
    {
        query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','remarque','activite') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:remarque,:activite)");
        //Si on est sur une sortie, on la renseigne
        if (p_idSortie != -1)
            query.prepare("INSERT INTO 'vol' ('typeDeVol','pilote','date','duree','cout','montantRembourse','facture','sortie','remarque','activite') VALUES(:typeDeVol,:pilote,:date,:duree,:cout,:montantRembourse,:facture,:sortie,:remarque,:activite)");
    }
    //Sinon, un numéro de vol est renseigné : on est en édition
    else
    {
        query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'montantRembourse' = :montantRembourse,'remarque' = :remarque, 'activite' = :activite WHERE volId = :volId");
        if (p_idSortie != -1)
            query.prepare("UPDATE 'vol' SET 'date' = :date,'duree' = :duree,'cout' = :cout,'montantRembourse' = :montantRembourse,'remarque' = :remarque,'sortie' = :sortie, 'activite' = :activite WHERE volId = :volId");
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
    query.bindValue(":activite", p_activite);

    query.exec();
}

bool ManageDb::supprimerUnVol(const int p_volAEditer)
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

QList<int> ManageDb::recupererAnneesAvecVolNonSoumis()
{
    QSqlQuery query;
    query.prepare("SELECT strftime('%Y', vol.date) AS annee FROM vol WHERE demandeRemboursement IS NULL GROUP BY annee");
    query.exec();

    QList<int> listeAnnees;

    while (query.next())
    {
        listeAnnees.append(query.value("annee").toInt());
    }

    return listeAnnees;
}

AeroDmsTypes::ListeSubventionsParPilotes ManageDb::recupererLesSubventionesDejaAllouees(const int p_annee)
{
    return recupererSubventionsPilotes( p_annee, 
                                        "*", 
                                        true);
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
        //listeFactures.append(QString("C:/Users/cleme/OneDrive/Documents/AeroDMS/FacturesTraitees/").append(query.value(5).toString()));
        listeFactures.append(query.value(5).toString());
    }

    return listeFactures;
}

void ManageDb::ajouterDemandeCeEnBdd(const AeroDmsTypes::DemandeEnCoursDeTraitement p_demande)
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

    QThread::msleep(delaisDeGardeBdd);

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
            QThread::msleep(delaisDeGardeBdd);
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
            QThread::msleep(delaisDeGardeBdd);
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

    QThread::msleep(delaisDeGardeBdd);
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
        if (recette.intitule != ")")
        {
            liste.append(recette);
        }
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
        demandeRemboursement.id = query.value(0).toInt();
        demandeRemboursement.intitule = query.value(1).toString();
        demandeRemboursement.montant = query.value(2).toFloat();
        demandeRemboursement.payeur = query.value(4).toString().append(" ").append(query.value(3).toString());
        demandeRemboursement.nomSortie = query.value(5).toString();
        demandeRemboursement.typeDeSortie = query.value(6).toString();
        demandeRemboursement.annee = query.value(7).toInt();
        demandeRemboursement.nomFacture = query.value(8).toString();
        demandeRemboursement.soumisCe = false;

        liste.append(demandeRemboursement);
    }

    return liste;
}

AeroDmsTypes::ListeDemandeRemboursementFacture ManageDb::recupererToutesLesDemandesDeRemboursement(const int p_annee)
{
    //Récupérationd des demandes de remboursement soumises ou non au CSE
    AeroDmsTypes::ListeDemandeRemboursementFacture liste;
    QSqlQuery query;
    query.prepare("SELECT * FROM 'factures'");
    query.exec();

    while (query.next()) {
        AeroDmsTypes::DemandeRemboursementFacture demandeRemboursement;
        demandeRemboursement.id = query.value("id").toInt();
        demandeRemboursement.intitule = query.value("intitule").toString();
        demandeRemboursement.montant = query.value("montant").toFloat();
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

QString ManageDb::recupererAeroclub(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT aeroclub FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value(0).toString();
}

QList<int> ManageDb::recupererAnnees()
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

QString ManageDb::recupererNomPrenomPilote(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT prenom, nom FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value(0).toString().append(" ").append(query.value(1).toString());
}

QString ManageDb::recupererActivitePrincipale(const QString p_piloteId)
{
    QSqlQuery query;
    query.prepare("SELECT activitePrincipale FROM pilote WHERE piloteId = :piloteId");
    query.bindValue(":piloteId", p_piloteId);

    query.exec();
    query.next();
    return query.value("activitePrincipale").toString();
}

QString ManageDb::recupererNomFacture(const int p_volId)
{
    QSqlQuery query;
    query.prepare("SELECT fichiersFacture.nomFichier AS nomFichier FROM vol INNER JOIN fichiersFacture ON vol.facture = fichiersFacture.factureId WHERE volId = :volId");
    query.bindValue(":volId", QString::number(p_volId));

    query.exec();
    query.next();
    return query.value("nomFichier").toString();
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

QStringList ManageDb::recupererTypesDesVol(const bool recupererUniquementLesTypesDeVolAvecRecette)
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

QStringList ManageDb::recupererBaladesEtSorties(const QString p_typeDeVol)
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

void ManageDb::ajouterCotisation(const AeroDmsTypes::CotisationAnnuelle& p_infosCotisation)
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
AeroDmsTypes::ResultatCreationPilote ManageDb::creerPilote(const AeroDmsTypes::Pilote p_pilote)
{
    AeroDmsTypes::ResultatCreationPilote resultat = AeroDmsTypes::ResultatCreationPilote_SUCCES;

    QSqlQuery query;
    
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
            query.prepare("INSERT INTO 'pilote' ('piloteId','nom','prenom','aeroclub','estAyantDroit','mail','telephone','remarque','activitePrincipale') VALUES(:piloteId,:nom,:prenom,:aeroclub,:estAyantDroit,:mail,:telephone,:remarque,:activitePrincipale)");
            query.bindValue(":piloteId", piloteId);
            query.bindValue(":nom", p_pilote.nom);
            query.bindValue(":prenom", p_pilote.prenom);
            query.bindValue(":aeroclub", p_pilote.aeroclub);
            query.bindValue(":estAyantDroit", p_pilote.estAyantDroit);
            query.bindValue(":mail", p_pilote.mail);
            query.bindValue(":telephone", p_pilote.telephone);
            query.bindValue(":remarque", p_pilote.remarque);
            query.bindValue(":activitePrincipale", p_pilote.activitePrincipale);

            if (!query.exec())
            {
                resultat = AeroDmsTypes::ResultatCreationPilote_AUTRE;
            }
        } 
    }
    //mise à jour
    else
    {
        query.prepare("UPDATE 'pilote' SET 'nom' = :nom,'prenom' = :prenom,'aeroclub' = :aeroclub,'estAyantDroit' = :estAyantDroit,'mail' = :mail,'telephone' = :telephone,'remarque' = :remarque, 'activitePrincipale' =:activitePrincipale WHERE piloteId = :piloteId");
        query.bindValue(":piloteId", p_pilote.idPilote);
        query.bindValue(":nom", p_pilote.nom);
        query.bindValue(":prenom", p_pilote.prenom);
        query.bindValue(":aeroclub", p_pilote.aeroclub);
        query.bindValue(":estAyantDroit", p_pilote.estAyantDroit);
        query.bindValue(":mail", p_pilote.mail);
        query.bindValue(":telephone", p_pilote.telephone);
        query.bindValue(":remarque", p_pilote.remarque);
        query.bindValue("activitePrincipale", p_pilote.activitePrincipale);

        if (!query.exec())
        {
            resultat = AeroDmsTypes::ResultatCreationPilote_AUTRE;
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

AeroDmsTypes::ListeStatsHeuresDeVol ManageDb::recupererHeuresMensuelles(const int p_annee)
{
    AeroDmsTypes::ListeStatsHeuresDeVol liste;

    QSqlQuery query;
    if (p_annee != -1)
    {
        query.prepare("SELECT * FROM 'stats_heuresDeVolParMois' WHERE annee = :annee");
        query.bindValue(":annee", QString::number(p_annee));
    }
    else
    {
        query.prepare("SELECT * FROM 'stats_heuresDeVolParMois'");
    }
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

QStringList ManageDb::recupererListeActivites()
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

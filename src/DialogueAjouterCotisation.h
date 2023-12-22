#ifndef DIALOGUEAJOUTERCOTISATION_H
#define DIALOGUEAJOUTERCOTISATION_H
#include <QDialog>
#include "ManageDb.h"

class DialogueAjouterCotisation : public QDialog
{
    Q_OBJECT

public:
    DialogueAjouterCotisation();
    DialogueAjouterCotisation(ManageDb* db, QWidget* parent = nullptr);

    void mettreAJourLeContenuDeLaFenetre();
    AeroDmsTypes::CotisationAnnuelle recupererInfosCotisationAAjouter();

    //QUasShared::UasMaintenanceOperationToDb getMaintenanceData();
    //void updateWindowContentFromDb();

private:
    QComboBox* listePilote;
    QComboBox* annee;
    QDoubleSpinBox* montant;
    QDoubleSpinBox* montantSubventionAnnuelle;

    ManageDb* database;

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;
    QWidget* extension;

    void peuplerListePilote();

    /*void populateMecanicianComboBox();
    void populateEquipementTypeComboBox(const int droneTypeId = -1);
    void populateDroneImmatComboBox(const int eqptTypeId = -1);
    void populateOperationComboBox(const int eqptTypeId = -1);

private slots:
    void manageEquipementTypeChanged();
    void manageEquipementChanged();
    void updateScenarioList();
    void updateOperationDescription();*/
};

#endif // DIALOGUEAJOUTERCOTISATION_H


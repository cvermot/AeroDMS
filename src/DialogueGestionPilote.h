#ifndef DIALOGUEGESTIONPILOTE_H
#define DIALOGUEGESTIONPILOTE_H
#include <QDialog>
#include "ManageDb.h"

class DialogueGestionPilote : public QDialog
{
    Q_OBJECT

public:
    DialogueGestionPilote();
    DialogueGestionPilote(ManageDb* db, QWidget* parent = nullptr);

    AeroDmsTypes::Pilote recupererInfosPilote();

    //QUasShared::UasMaintenanceOperationToDb getMaintenanceData();
    //void updateWindowContentFromDb();

private:
    QLineEdit *nom;
    QLineEdit* prenom;
    QLineEdit* aeroclub;
    QCheckBox *estAyantDroit;
    QLineEdit* mail;
    QLineEdit* telephone;
    QLineEdit* remarque;

    QString idPilote;

    ManageDb* database;

    QDialogButtonBox* buttonBox;
    QPushButton* cancelButton;
    QPushButton* okButton;
    QWidget* extension;

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

#endif // DIALOGUEGESTIONPILOTE_H


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
    //QWidget* extension;

private slots:
    void prevaliderDonneesSaisies();
};

#endif // DIALOGUEGESTIONPILOTE_H


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
#include "DialogueGestionAeroclub.h"
#include "AeroDmsServices.h"

#include <QtWidgets>

class UppercaseValidator : public QValidator {
public:
    explicit UppercaseValidator(QObject* parent = nullptr) : QValidator(parent) {}

    QValidator::State validate(QString& input, int& pos) const override {
        input = input.toUpper(); // Convertir en majuscules
        return QValidator::Acceptable;
    }
};

DialogueGestionAeroclub::DialogueGestionAeroclub(ManageDb* db, QWidget* parent) : QDialog(parent)
{
    database = db;

    cancelButton = new QPushButton(tr("&Annuler"), this);
    cancelButton->setDefault(false);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(annulationOuFinSaisie()));
    connect(this, SIGNAL(rejected()), this, SLOT(annulationOuFinSaisie()));

    okButton = new QPushButton(tr("&Ajouter"), this);
    okButton->setDisabled(true);
    okButton->setDefault(true);
    okButton->setToolTip("Remplir à minima le champ nom de l'aéroclub");
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    selectionAeroclub = new QComboBox(this);
    selectionAeroclubLabel = new QLabel(tr("Aéroclub à éditer : "), this);
    connect(selectionAeroclub, &QComboBox::currentIndexChanged, this, &DialogueGestionAeroclub::chargerDonneesAeroclub);

	selectionAerodrome = new QComboBox(this);
    QLabel *selectionAerodromeLabel = new QLabel(tr("Aérodrome : "), this);

    nomAeroclub = new QLineEdit(this);
    QLabel* nomAeroclubLabel = new QLabel(tr("Nom de l'aéroclub : "), this);
    connect(nomAeroclub, &QLineEdit::textChanged, this, &DialogueGestionAeroclub::prevaliderDonneesSaisies);

    raisonSociale = new QLineEdit(this);
    QLabel* raisonSocialeLabel = new QLabel(tr("Raison sociale : "), this);

    iban = new QLineEdit();
    iban->setInputMask("xxxx xxxx xxxx xxxx xxxx xxxx xxxx xx");
    iban->setValidator(new UppercaseValidator(iban));
    QLabel* ibanLabel = new QLabel(tr("IBAN : "), this);
    connect(iban, &QLineEdit::textChanged, this, &DialogueGestionAeroclub::validerIban);

    bic = new QLineEdit();
    bic->setValidator(new UppercaseValidator(bic));
    QLabel* bicLabel = new QLabel(tr("BIC : "), this);

    idAeroclub = -1;

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->setColumnMinimumWidth(1, 250);

    mainLayout->addWidget(selectionAeroclubLabel, 0, 0);
    mainLayout->addWidget(selectionAeroclub, 0, 1);

    mainLayout->addWidget(nomAeroclubLabel, 1, 0);
    mainLayout->addWidget(nomAeroclub, 1, 1);

    mainLayout->addWidget(selectionAerodromeLabel, 2, 0);
    mainLayout->addWidget(selectionAerodrome, 2, 1);

    mainLayout->addWidget(raisonSocialeLabel, 3, 0);
    mainLayout->addWidget(raisonSociale, 3, 1);

    mainLayout->addWidget(ibanLabel, 4, 0);
    mainLayout->addWidget(iban, 4, 1);

    mainLayout->addWidget(bicLabel, 5, 0);
    mainLayout->addWidget(bic, 5, 1);

    mainLayout->addWidget(buttonBox, 10, 0, 1, 2);

    setLayout(mainLayout);

    peuplerListeAeroclub();
    peuplerListeAerodrome();
}

const AeroDmsTypes::Club DialogueGestionAeroclub::recupererInfosClub()
{
    AeroDmsTypes::Club club = AeroDmsTypes::K_INIT_CLUB;

    club.idAeroclub = idAeroclub;
    club.aeroclub = nomAeroclub->text();
	club.aerodrome = selectionAerodrome->currentData().toString();
    club.raisonSociale = raisonSociale->text();
    club.iban = iban->text().replace(" ", "");
    club.bic = bic->text();

    //On rince l'affichage en vue d'une éventuelle autre saisie
    annulationOuFinSaisie();

    return club;
}

void DialogueGestionAeroclub::peuplerListeAeroclub()
{
    AeroDmsTypes::ListeAeroclubs aeroclubs = database->recupererAeroclubs();

    selectionAeroclub->clear();

    selectionAeroclub->addItem("", -1);
    for (AeroDmsTypes::Club club : aeroclubs)
    {
        selectionAeroclub->addItem(club.aeroclub, club.idAeroclub);
    }
}

void DialogueGestionAeroclub::peuplerListeAerodrome()
{
    AeroDmsTypes::ListeAerodromes aerodromes = database->recupererAerodromes();

    selectionAerodrome->clear();

    for (AeroDmsTypes::Aerodrome aerodrome : aerodromes)
    {
        selectionAerodrome->addItem(aerodrome.nom + " - " + aerodrome.indicatifOaci, aerodrome.indicatifOaci);
    }
}

void DialogueGestionAeroclub::prevaliderDonneesSaisies()
{
    okButton->setDisabled(false);

    if (nomAeroclub->text() == ""
        || ( selectionAeroclub->isVisible() 
             && selectionAeroclub->currentIndex() == 0) )
    {
        okButton->setDisabled(true);
    }
}

void DialogueGestionAeroclub::chargerDonneesAeroclub()
{
    if (selectionAeroclub->currentData().toInt() != -1)
    {
        const AeroDmsTypes::Club aeroclub = database->recupererAeroclub(selectionAeroclub->currentData().toInt());

        qDebug() << aeroclub.aeroclub;
        qDebug() << aeroclub.aerodrome;

        idAeroclub = selectionAeroclub->currentData().toInt();
        nomAeroclub->setText(aeroclub.aeroclub);
        selectionAerodrome->setCurrentIndex(selectionAerodrome->findData(aeroclub.aerodrome));
        raisonSociale->setText(aeroclub.raisonSociale);
        iban->setText(aeroclub.iban);
        bic-> setText(aeroclub.bic);

        nomAeroclub->setEnabled(true);
        selectionAerodrome->setEnabled(true);
        raisonSociale->setEnabled(true);
        iban->setEnabled(true);
        bic->setEnabled(true);
    }
    else
    {
        annulationOuFinSaisie();
        nomAeroclub->setEnabled(false);
        selectionAerodrome->setEnabled(false);
        raisonSociale->setEnabled(false);
        iban->setEnabled(false);
        bic->setEnabled(false);
    }
}

void DialogueGestionAeroclub::annulationOuFinSaisie()
{
    //On rince en vue de l'éventuel ajout du pilote suivant
    idAeroclub = -1;
    nomAeroclub->clear();
    raisonSociale->clear();
    iban->clear();
    bic->clear();

    validerIban();
}

void DialogueGestionAeroclub::ouvrirFenetre(const bool p_modeEdition)
{
    selectionAeroclub->setHidden(!p_modeEdition);
    selectionAeroclubLabel->setHidden(!p_modeEdition);

    nomAeroclub->setEnabled(!p_modeEdition);
    selectionAerodrome->setEnabled(!p_modeEdition);
    raisonSociale->setEnabled(!p_modeEdition);
    iban->setEnabled(!p_modeEdition);
    bic->setEnabled(!p_modeEdition);

    if (p_modeEdition)
    {
        okButton->setText(tr("&Modifier"));
        setWindowTitle(QApplication::applicationName() + " - " + tr("Modifier un aéroclub"));
    }
    else
    {
        okButton->setText(tr("&Ajouter"));
        setWindowTitle(QApplication::applicationName() + " - " + tr("Ajouter un aéroclub"));
    }

    exec();
}

void DialogueGestionAeroclub::validerIban()
{
    QPalette palette;
    if (iban->text().replace(" ", "").length() == 0)
    {   
        //Blanc
        palette.setColor(QPalette::Base, QColor(255, 255, 255, 120));
        iban->setToolTip("");
    }
    else if (validerIbanFrancais(iban->text().replace(" ", "")))
    {
        //Vert
        palette.setColor(QPalette::Base, QColor(140, 255, 135, 120));
        iban->setToolTip(tr("L'IBAN saisi est valide"));
    }
    else
    {
        //Rouge
        palette.setColor(QPalette::Base, QColor(255, 140, 135, 120));
        iban->setToolTip(tr("L'IBAN saisi ne semble pas valide ou n'est pas un IBAN français"));
    }
    iban->setPalette(palette);
}

bool DialogueGestionAeroclub::validerIbanFrancais(const QString& p_iban) 
{
    // Vérification de la longueur
    if (p_iban.length() != 27) 
    {
        return false;
    }

    // Vérification du format général avec une expression régulière
    QRegularExpression regex("^FR\\d{2}[A-Z0-9]{23}$");
    if (!regex.match(p_iban).hasMatch()) 
    {
        return false;
    }

    // Réarrangement de l'IBAN pour calcul de la clé de contrôle
    QString rearrangedIban = p_iban.mid(4) + p_iban.left(4);

    // Conversion des lettres en chiffres
    QString numericIban;
    for (const QChar& c : rearrangedIban) 
    {
        if (c.isLetter()) 
        {
            numericIban.append(QString::number(c.toUpper().unicode() - 'A' + 10));
        }
        else if (c.isDigit()) 
        {
            numericIban.append(c);
        }
        else 
        {
            return false; // Caractère invalide
        }
    }

    // Calcul du modulo 97
    qint64 total = 0;
    for (int i = 0; i < numericIban.length(); ++i) 
    {
        total = (total * 10 + numericIban[i].digitValue()) % 97;
    }

    // Vérification finale
    return (total == 1);
}

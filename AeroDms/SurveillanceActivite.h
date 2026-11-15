/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2026 Clément VERMOT-DESROCHES (clement@vermot.net)

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
#ifndef HEADER_SURVEILLANCEACTIVITE
#define HEADER_SURVEILLANCEACTIVITE

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QLabel>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

class SurveillanceActivite : public QObject {
    Q_OBJECT

public:
    SurveillanceActivite(const int p_delaiDInactiviteEnMinutes,
        const int p_delaisDeGraceEnSecondes,
        QObject* parent);

    void reprendre();
    void arreter();
    void mettreAJourDelais(const int p_delaiDInactiviteEnMinutes,
        const int p_delaisDeGraceEnSecondes);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void pause();
    void resume();

private slots:
    void afficherDialogueInactivite();
    void expirationCompteurDialogueOuvert();

private:
    void mettreAJourDialogueInactivite(int p_tempsRestantEnMs);

    QTimer compteurDInactivite;
    QTimer compteurMiseAJourDialogue;
    QElapsedTimer compteurDialogueOuvert;

    QDialog* dialogue = nullptr;
    QLabel* labelDialogue = nullptr;
    QProgressBar* barreDeProgressionDialogue = nullptr;

    int delaiDeGraceEnMs;
    bool boiteDeDialogueEstOuverte = false;
    bool demandeFermeture = false;
    bool surveillanceEstEnPause = false;
};

#endif

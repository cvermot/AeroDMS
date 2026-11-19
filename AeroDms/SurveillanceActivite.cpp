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

#include "SurveillanceActivite.h"

SurveillanceActivite::SurveillanceActivite(const int p_delaiDInactiviteEnMinutes, 
    const int p_delaisDeGraceEnSecondes, 
    QObject* parent)
    : QObject(parent), delaiDeGraceEnMs(p_delaisDeGraceEnSecondes * 1000) 
{
    compteurDInactivite.setSingleShot(true);
    compteurDInactivite.setInterval(p_delaiDInactiviteEnMinutes * 60 * 1000);
    connect(&compteurDInactivite, &QTimer::timeout, this, &SurveillanceActivite::afficherDialogueInactivite);

    //mise à jour de la barre de progression toutes les 10 ms
    compteurMiseAJourDialogue.setInterval(10);
    connect(&compteurMiseAJourDialogue, &QTimer::timeout, this, &SurveillanceActivite::expirationCompteurDialogueOuvert);

    qApp->installEventFilter(this);
    if (p_delaiDInactiviteEnMinutes != 0)
    {
        compteurDInactivite.start();
    }
}

void SurveillanceActivite::pause()
{
    compteurDInactivite.stop();
    compteurMiseAJourDialogue.stop();
}

void SurveillanceActivite::arreter()
{
    compteurDInactivite.stop();
    compteurMiseAJourDialogue.stop();
    surveillanceEstEnPause = true;
}


void SurveillanceActivite::reprendre()
{
    compteurDInactivite.start();
    compteurMiseAJourDialogue.start();
    surveillanceEstEnPause = false;
}

void SurveillanceActivite::resume()
{
    if (!boiteDeDialogueEstOuverte
        && compteurDInactivite.interval() != 0
        && !surveillanceEstEnPause)
    {
        compteurDInactivite.start();
    }
}

void SurveillanceActivite::mettreAJourDelais(const int p_delaiDInactiviteEnMinutes,
    const int p_delaisDeGraceEnSecondes)
{
    pause();
    compteurDInactivite.setInterval(p_delaiDInactiviteEnMinutes * 60 * 1000);
    delaiDeGraceEnMs = p_delaisDeGraceEnSecondes * 1000;
    resume();
}

bool SurveillanceActivite::eventFilter(QObject* obj, QEvent* event)
{
    if (!boiteDeDialogueEstOuverte)
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::Wheel:
        case QEvent::TouchBegin:
            resume();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, event);
}

void SurveillanceActivite::afficherDialogueInactivite()
{
    boiteDeDialogueEstOuverte = true;
    demandeFermeture = false;

    dialogue = new QDialog(qobject_cast<QWidget*>(parent()));
    dialogue->setWindowTitle(QApplication::applicationName() + " - " + tr("Inactivité détectée - Fermeture automatique"));
    dialogue->setWindowFlags(dialogue->windowFlags() | Qt::WindowStaysOnTopHint);
    dialogue->setMinimumWidth(360);

    auto* layout = new QVBoxLayout(dialogue);

    labelDialogue = new QLabel();
    labelDialogue->setWordWrap(true);
    layout->addWidget(labelDialogue);

    barreDeProgressionDialogue = new QProgressBar();
    barreDeProgressionDialogue->setRange(0, delaiDeGraceEnMs);
    barreDeProgressionDialogue->setValue(delaiDeGraceEnMs);
    // On n'affiche pas le pourcentage
    barreDeProgressionDialogue->setTextVisible(false);
    layout->addWidget(barreDeProgressionDialogue);

    QDialogButtonBox buttonBox;
    QPushButton* boutonContinuer = buttonBox.addButton(tr("Ne pas fermer maintenant"), QDialogButtonBox::AcceptRole);
    QPushButton* boutonFermer = buttonBox.addButton(tr("Fermer ") + QApplication::applicationName() + tr(" maintenant"), QDialogButtonBox::RejectRole);
    boutonContinuer->setDefault(true);
    layout->addWidget(&buttonBox);

    connect(boutonContinuer, &QPushButton::clicked, dialogue, &QDialog::accept);
    connect(boutonFermer, &QPushButton::clicked, dialogue, [&]() {
        demandeFermeture = true;
        dialogue->reject();
        });

    compteurDialogueOuvert.start();
    mettreAJourDialogueInactivite(delaiDeGraceEnMs);
    compteurMiseAJourDialogue.start();

    int result = dialogue->exec();

    compteurMiseAJourDialogue.stop();
    dialogue = nullptr;
    labelDialogue = nullptr;
    barreDeProgressionDialogue = nullptr;
    boiteDeDialogueEstOuverte = false;

    if (demandeFermeture
        || result == QDialog::Rejected && compteurDialogueOuvert.elapsed() >= delaiDeGraceEnMs)
    {
        QApplication::closeAllWindows();
        QApplication::quit();
    }
    else
    {
        // L'utilisateur a cliqué sur le bouton pour rester actif ou sur la fermeture de la fenêtre d'avertissement
        // => on relance la surveillance
        resume();
    }
}

void SurveillanceActivite::expirationCompteurDialogueOuvert()
{
    if (delaiDeGraceEnMs - compteurDialogueOuvert.elapsed() <= 0)
    {
        compteurMiseAJourDialogue.stop();
        demandeFermeture = true;
        if (dialogue)
        {
            dialogue->reject();
        }
    }
    else
    {
        mettreAJourDialogueInactivite(static_cast<int>(compteurDialogueOuvert.elapsed()));
    }
}

void SurveillanceActivite::mettreAJourDialogueInactivite(int p_tempsRestantEnMs)
{
    if (barreDeProgressionDialogue)
    {
        barreDeProgressionDialogue->setValue(p_tempsRestantEnMs);
    }
    if (labelDialogue)
    {
        int tempsRestantEnSecondes = qCeil((delaiDeGraceEnMs - p_tempsRestantEnMs) / 1000.0);
        labelDialogue->setText(
            tr("Aucune activité détectée depuis plus de ") + QString::number(compteurDInactivite.interval() / 1000) + tr(" secondes.\n\n")
            + tr("Sans action, ") + QApplication::applicationName() + tr(" va fermer automatiquement dans ") + QString::number(tempsRestantEnSecondes) + tr(" secondes."));
    }
}

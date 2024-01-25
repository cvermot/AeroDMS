// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculaire.h"
#include "StatistiqueDiagrammeCirculairePartie.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"

#include <QChart>
#include <QLegend>
#include <QPieSeries>
#include <QRandomGenerator>

StatistiqueDiagrammeCirculaireWidget::StatistiqueDiagrammeCirculaireWidget(ManageDb* p_db, const int p_annee, const AeroDmsTypes::Statistiques p_statistique, QWidget* parent)
    : StatistiqueWidget(parent)
{
    auto* chart = new StatistiqueDiagrammeCirculaire;
    chart->setTheme(QChart::ChartThemeLight);
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    switch (p_statistique)
    {
        case AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE:
        {
            AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = p_db->recupererSubventionsPilotes(p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            donneesTypeDeVolParPilote->setName("Temps de vol par pilote (cliquez pour le détail par pilote)");

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                auto detailParPilote = new QPieSeries(this);
                detailParPilote->setName("Vol pour " + subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.tempsDeVolEnMinutes, "Entrainement", donneesTypeDeVolParPilote);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.tempsDeVolEnMinutes, "Sorties", donneesTypeDeVolParPilote);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.tempsDeVolEnMinutes, "Balades", donneesTypeDeVolParPilote);

                QObject::connect(detailParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailParPilote->sum(), subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, detailParPilote);
            }

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE:
        {
            AeroDmsTypes::ListeStatsHeuresDeVolParActivite subventionParActivite = p_db->recupererHeuresParActivite(p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            donneesTypeDeVolParPilote->setName("Temps de vol par activité (cliquez pour le détail des heures par pilote dans l'activité)");

            auto detailAvion = new QPieSeries(this);
            detailAvion->setName("Avion");
            auto detailUlm = new QPieSeries(this);
            detailUlm->setName("ULM");
            auto detailPlaneur = new QPieSeries(this);
            detailPlaneur->setName("Planeur");
            auto detailHelicoptere = new QPieSeries(this);
            detailHelicoptere->setName("Hélicoptère");

            for (int i = 0; i < subventionParActivite.size(); i++)
            {
                if (subventionParActivite.at(i).minutesVolAvion != 0)
                {
                    *detailAvion << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolAvion,
                        subventionParActivite.at(i).nomPrenomPilote,
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolUlm != 0)
                {
                    *detailUlm << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolUlm, 
                        subventionParActivite.at(i).nomPrenomPilote, 
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolPlaneur != 0)
                {
                    *detailPlaneur << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolPlaneur, 
                        subventionParActivite.at(i).nomPrenomPilote, 
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolHelicoptere != 0)
                {
                    *detailHelicoptere << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolHelicoptere, 
                        subventionParActivite.at(i).nomPrenomPilote, 
                        detailHelicoptere);
                }
            }
            if (detailAvion->sum() != 0)
            {
                QObject::connect(detailAvion, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailAvion->sum(), "Avion", detailAvion);
            }
            if (detailUlm->sum() != 0)
            {
                QObject::connect(detailUlm, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailUlm->sum(), "ULM", detailUlm);
            }
            if (detailPlaneur->sum() != 0)
            {
                QObject::connect(detailPlaneur, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailPlaneur->sum(), "Planeur", detailPlaneur);
            }
            if (detailHelicoptere->sum() != 0)
            {
                QObject::connect(detailHelicoptere, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailHelicoptere->sum(), "Hélicoptère", detailHelicoptere);
            }   

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL:
        default:
        {
            AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = p_db->recupererSubventionsPilotes(p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            donneesTypeDeVolParPilote->setName("Temps de vol par type de vol (cliquez pour le détail des heures par pilote dans la catégorie)");

            auto detailEntrainement = new QPieSeries(this);
            detailEntrainement->setName("Vols d'entrainement");
            auto detailSortie = new QPieSeries(this);
            detailSortie->setName("Sorties");
            auto detailBalade = new QPieSeries(this);
            detailBalade->setName("Balades");

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                *detailEntrainement << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, donneesTypeDeVolParPilote);
                *detailSortie << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, donneesTypeDeVolParPilote);
                *detailBalade << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, donneesTypeDeVolParPilote);
            }
            QObject::connect(detailEntrainement, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailEntrainement->sum(), "Vols d'entrainement", detailEntrainement);
            QObject::connect(detailSortie, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailSortie->sum(), "Sortie", detailSortie);
            QObject::connect(detailBalade, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailBalade->sum(), "Balades", detailBalade);

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

    }
    
    createDefaultChartView(chart);
}

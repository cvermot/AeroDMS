// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculaire.h"
#include "StatistiqueDiagrammeCirculairePartie.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"

#include <QChart>
#include <QLegend>
#include <QPieSeries>
#include <QRandomGenerator>

StatistiqueDiagrammeCirculaireWidget::StatistiqueDiagrammeCirculaireWidget(ManageDb* p_db, int p_annee, QWidget* parent)
    : StatistiqueWidget(parent)
{
    auto* chart = new StatistiqueDiagrammeCirculaire;
    chart->setTheme(QChart::ChartThemeBlueNcs);
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = p_db->recupererSubventionsPilotes(p_annee);

    auto donneesTypeDeVolParPilote = new QPieSeries(this);
    donneesTypeDeVolParPilote->setName("Temps de vol par pilote (cliquez pour le détail par pilote)");

    for (int i = 0; i < subventionParPilote.size(); i++)
    {
        auto detailParPilote = new QPieSeries(this);
        detailParPilote->setName("Vol pour " + subventionParPilote.at(i).prenom +" "+ subventionParPilote.at(i).nom);
        *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.tempsDeVolEnMinutes, "Entrainement", donneesTypeDeVolParPilote);
        *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.tempsDeVolEnMinutes, "Sorties", donneesTypeDeVolParPilote);
        *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.tempsDeVolEnMinutes, "Balades", donneesTypeDeVolParPilote);

        QObject::connect(detailParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
        *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailParPilote->sum(), subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, detailParPilote);
    }
        
    QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

    chart->changeSeries(donneesTypeDeVolParPilote);

    createDefaultChartView(chart);
}

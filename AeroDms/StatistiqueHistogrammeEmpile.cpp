// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueHistogrammeEmpile.h"
#include "AeroDmsTypes.h"

#include <QBarCategoryAxis>
#include <QBarSet>
#include <QChart>
#include <QLegend>
#include <QStackedBarSeries>
#include <QValueAxis>

StatistiqueHistogrammeEmpile::StatistiqueHistogrammeEmpile( ManageDb* p_db, 
    const int p_annee, 
    QWidget* parent,
    const int p_options,
    const QChart::AnimationOption p_animation,
    const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres)
    : StatistiqueWidget(parent)
{
    setMinimumSize(p_parametres.tailleMiniImage);
    QFont font("Arial", p_parametres.tailleDePolice);

    const AeroDmsTypes::ListeStatsHeuresDeVol heuresDeVol = p_db->recupererHeuresMensuelles(p_annee, p_options);

    auto entrainement = new QBarSet("Entrainement");
    auto sortie = new QBarSet("Sortie");
    auto balade = new QBarSet("Balade");

    QStringList mois;

    for (int i = 0; i < heuresDeVol.size() ; i++)
    {
        const AeroDmsTypes::StatsHeuresDeVol hdv = heuresDeVol.at(i);
        *entrainement << hdv.minutesEntrainement/60.0;
        *sortie << hdv.minutesSortie/60.0;
        *balade << hdv.minutesBalade/60.0;
        mois.append(hdv.mois);
    }

    auto series = new QStackedBarSeries;
    series->append(entrainement);
    series->append(sortie);
    series->append(balade);

    auto chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Nombre d'heures de vol par mois");
    chart->setTitleFont(QFont("Arial", p_parametres.tailleDePolice*1.5, QFont::Bold));
    chart->setAnimationOptions(p_animation);

    auto axisX = new QBarCategoryAxis;
    axisX->setLabelsFont(font);
    axisX->append(mois);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    auto axisY = new QValueAxis;
    axisY->setLabelsFont(font);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(font);

    createDefaultChartView(chart);
}

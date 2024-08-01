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
                                                            const QChart::AnimationOption p_animation)
    : StatistiqueWidget(parent)
{
    const AeroDmsTypes::ListeStatsHeuresDeVol heuresDeVol = p_db->recupererHeuresMensuelles(p_annee);

    auto entrainement = new QBarSet("Entrainement");
    auto sortie = new QBarSet("Sortie");
    auto balade = new QBarSet("Balade");
    QStringList mois;

    for (int i = 0; i < heuresDeVol.size() ; i++)
    {
        const AeroDmsTypes::StatsHeuresDeVol hdv = heuresDeVol.at(i);
        *entrainement << hdv.minutesEntrainement;
        *sortie << hdv.minutesSortie;
        *balade << hdv.minutesBalade;
        mois.append(hdv.mois);
    }

    auto series = new QStackedBarSeries;
    series->append(entrainement);
    series->append(sortie);
    series->append(balade);

    auto chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Nombre de minutes de vol par mois");
    chart->setAnimationOptions(p_animation);

    auto axisX = new QBarCategoryAxis;
    axisX->append(mois);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    auto axisY = new QValueAxis;
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    createDefaultChartView(chart);
}

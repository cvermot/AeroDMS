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

StatistiqueHistogrammeEmpile::StatistiqueHistogrammeEmpile(ManageDb* p_db, const int p_annee, QWidget* parent)
    : StatistiqueWidget(p_db, parent)
{
    const AeroDmsTypes::ListeStatsHeuresDeVol heuresDeVol = p_db->recupererHeuresMensuelles(p_annee);
    qDebug() << "annee" << p_annee;

    //![1]
    auto entrainement = new QBarSet("Entrainement");
    auto sortie = new QBarSet("Sortie");
    auto balade = new QBarSet("Balade");
    QStringList mois;

    for (int i = 0; i < heuresDeVol.size() ; i++)
    {
        const AeroDmsTypes::StatsHeuresDeVol hdv = heuresDeVol.at(i);
        /*entrainement->append(hdv.minutesEntrainement);
        sortie->append(hdv.minutesSortie);
        balade->append(hdv.minutesBalade);
        */
        *entrainement << hdv.minutesEntrainement;
        *sortie << hdv.minutesSortie;
        *balade << hdv.minutesBalade;
        mois.append(hdv.mois);
        qDebug() << hdv.mois << hdv.minutesEntrainement << hdv.minutesSortie << hdv.minutesBalade;
    }


    /*entrainement << 120 << 167 << 55 << 32 << 39 << 150;
    *sortie << 0 << 0 << 34 << 165 << 32 << 234;
    *balade << 34 << 65 << 75 << 123 << 156 << 24;*/
    //![1]

    //![2]
    auto series = new QStackedBarSeries;
    series->append(entrainement);
    series->append(sortie);
    series->append(balade);
    //![2]

    //![3]
    auto chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Nombre d'heures de vol par mois");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    //![3]

    //![4]
    auto axisX = new QBarCategoryAxis;
    axisX->append(mois);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    auto axisY = new QValueAxis;
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    //![4]

    //![5]
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
    createDefaultChartView(chart);
    //![6]
}

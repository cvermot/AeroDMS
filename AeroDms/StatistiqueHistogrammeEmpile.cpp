// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueHistogrammeEmpile.h"
#include "AeroDmsTypes.h"

#include <QtGraphs/QBarCategoryAxis>
#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QValueAxis>

StatistiqueHistogrammeEmpile::StatistiqueHistogrammeEmpile( ManageDb* p_db, 
    const int p_annee, 
    QWidget* parent,
    const int p_options,
    const bool p_animation,
    const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres)
    : StatistiqueWidget(p_animation, parent)
{
    Q_UNUSED(p_animation);
    setMinimumSize(p_parametres.tailleMiniImage);

    const AeroDmsTypes::ListeStatsHeuresDeVol heuresDeVol = p_db->recupererHeuresMensuelles(p_annee, p_options);

    auto entrainement = new QBarSet("Entrainement");
    auto sortie = new QBarSet("Sortie");
    auto balade = new QBarSet("Balade");

    entrainement->setColor(QColor("#2f89ca"));
    entrainement->setBorderColor(QColor("#226799"));
    sortie->setColor(QColor("#f28e2b"));
    sortie->setBorderColor(QColor("#c96e14"));
    balade->setColor(QColor("#59a14f"));
    balade->setBorderColor(QColor("#3f7a37"));

    QStringList mois;
    qreal valeurMax = 0;

    for (int i = 0; i < heuresDeVol.size() ; i++)
    {
        const AeroDmsTypes::StatsHeuresDeVol hdv = heuresDeVol.at(i);
        const qreal heuresEntrainement = hdv.minutesEntrainement / 60.0;
        const qreal heuresSortie = hdv.minutesSortie / 60.0;
        const qreal heuresBalade = hdv.minutesBalade / 60.0;

        *entrainement << heuresEntrainement;
        *sortie << heuresSortie;
        *balade << heuresBalade;
        mois.append(hdv.mois);
        valeurMax = qMax(valeurMax, heuresEntrainement + heuresSortie + heuresBalade);
    }

    auto series = new QBarSeries(this);
    series->setBarsType(QBarSeries::BarsType::Stacked);
    series->append(entrainement);
    series->append(sortie);
    series->append(balade);

    auto axisX = new QBarCategoryAxis(this);
    axisX->append(mois);
    axisX->setAlignment(Qt::AlignBottom);

    auto axisY = new QValueAxis(this);
    axisY->setAlignment(Qt::AlignLeft);
    axisY->setMin(0);
    axisY->setMax(valeurMax <= 0 ? 1 : valeurMax * 1.1);
    axisY->setLabelDecimals(1);

    createDefaultChartView("Nombre d'heures de vol par mois", true, Qt::AlignBottom, static_cast<int>(p_parametres.tailleDePolice * 1.5));
    setGraphAxisX(axisX);
    setGraphAxisY(axisY);
    addSeriesToGraph(series);
}

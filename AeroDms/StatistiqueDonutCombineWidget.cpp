// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonutCombine.h"
#include "StatistiqueDonutCombineWidget.h"

#include <QtGraphs/QPieSeries>

StatistiqueDonutCombineWidget::StatistiqueDonutCombineWidget( ManageDb* p_db,
                                                              const AeroDmsTypes::Statistiques p_statistique,
                                                              QWidget* parent,
                                                              const int p_annee,
                                                              const int p_options,
                                                              const bool p_animation,
                                                              const bool p_legende,
                                                              const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres)
    : StatistiqueWidget(parent)
{
    Q_UNUSED(p_statistique);
    Q_UNUSED(p_animation);
    const AeroDmsTypes::StatsAeronefs statsAeronefs = p_db->recupererStatsAeronefs(p_annee, p_options);
    setMinimumSize(p_parametres.tailleMiniImage);

    indiceCouleurEnCours = 0;

    auto donutBreakdown = new StatistiqueDonutCombine(this);

    QString typeCourant = "init";
    auto series = new QPieSeries(this);

    for (int i = 0; i < statsAeronefs.size(); i++)
    {
        if (typeCourant == statsAeronefs.at(i).type)
        {
            QPieSlice* pieSlice = new QPieSlice(statsAeronefs.at(i).immat, statsAeronefs.at(i).nombreMinutesVol);
            series->append(pieSlice);
        }
        else
        {
            if (typeCourant != "init")
            {
                donutBreakdown->addBreakdownSeries(series, recupererNouvelleCouleur(), p_parametres.tailleDePolice);
            }
            typeCourant = statsAeronefs.at(i).type;
            series = new QPieSeries(this);
            series->setName(statsAeronefs.at(i).type);
            series->append(statsAeronefs.at(i).immat, statsAeronefs.at(i).nombreMinutesVol);
        }
    }
    if (series->count() > 0)
        donutBreakdown->addBreakdownSeries(series, recupererNouvelleCouleur(), p_parametres.tailleDePolice);

    createDefaultChartView("Répartition des vols par aéronefs et types d'aéronefs", p_legende, Qt::AlignRight, static_cast<int>(p_parametres.tailleDePolice * 1.5));
    for (QPieSeries* currentSeries : donutBreakdown->series())
        addSeriesToGraph(currentSeries);
}

QColor StatistiqueDonutCombineWidget::recupererNouvelleCouleur()
{
    const QList<QColor> palette = {
        QColor("#2f89ca"),
        QColor("#f28e2b"),
        QColor("#59a14f"),
        QColor("#af7aa1"),
        QColor("#76b7b2"),
        QColor("#e15759"),
        QColor("#4e79a7"),
        QColor("#edc948")
    };

    const QColor couleur = palette.at(indiceCouleurEnCours % palette.size());
    indiceCouleurEnCours++;
    return couleur;
}

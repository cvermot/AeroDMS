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

const Qt::GlobalColor StatistiqueDonutCombineWidget::recupererNouvelleCouleur()
{
    int nbCouleur = 11;

    Qt::GlobalColor couleur = Qt::red;

    switch (indiceCouleurEnCours % nbCouleur)
    {
        case 0:
            couleur = Qt::red;
            break;
        case 1:
            couleur = Qt::green;
            break;
        case 2:
            couleur = Qt::blue;
            break;
        case 3:
            couleur = Qt::cyan;
            break;
        case 4:
            couleur = Qt::magenta;
            break;
        case 5:
            couleur = Qt::darkRed;
            break;
        case 6:
            couleur = Qt::darkGreen;
            break;
        case 7:
            couleur = Qt::darkBlue;
            break;
        case 8:
            couleur = Qt::darkCyan;
            break;
        case 9:
            couleur = Qt::darkMagenta;
            break;
        case 10:
            couleur = Qt::darkYellow;
            break;
    }

    indiceCouleurEnCours++;
    return couleur;
}

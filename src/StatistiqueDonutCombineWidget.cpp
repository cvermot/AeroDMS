// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonutCombine.h"
#include "StatistiqueDonutCombineWidget.h"

#include <QChart>
#include <QPieSeries>

StatistiqueDonutCombineWidget::StatistiqueDonutCombineWidget( ManageDb* p_db,
                                                              const AeroDmsTypes::Statistiques p_statistique,
                                                              QWidget* parent,
                                                              const int p_annee,
                                                              const QChart::AnimationOption p_animation,
                                                              const bool p_legende,
                                                              const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres)
    : StatistiqueWidget(parent)
{
    const AeroDmsTypes::StatsAeronefs statsAeronefs = p_db->recupererStatsAeronefs(p_annee);
    setMinimumSize(p_parametres.tailleMiniImage);

    indiceCouleurEnCours = 0;

    auto donutBreakdown = new StatistiqueDonutCombine;

    QString typeCourant = "init";
    int numeroSerie = 0;
    auto series = new QPieSeries;

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
            series = new QPieSeries;
            series->setName(statsAeronefs.at(i).type);
            series->append(statsAeronefs.at(i).immat, statsAeronefs.at(i).nombreMinutesVol);
        }
    }
    donutBreakdown->addBreakdownSeries(series, recupererNouvelleCouleur(), p_parametres.tailleDePolice);

    
    donutBreakdown->setAnimationOptions(p_animation);

    donutBreakdown->legend()->setVisible(p_legende);
    donutBreakdown->setTitle("Statistiques de répartition des vols par aéronefs et types d'aéronefs");
    donutBreakdown->legend()->setAlignment(Qt::AlignRight);
    
    createDefaultChartView(donutBreakdown);
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

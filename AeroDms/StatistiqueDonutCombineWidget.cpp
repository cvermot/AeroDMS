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
    Q_UNUSED(p_animation);
    
    setMinimumSize(p_parametres.tailleMiniImage);

    switch (p_statistique)
    {
        case AeroDmsTypes::Statistiques_AERONEFS:
        default:
        {
            const AeroDmsTypes::StatsAeronefs statsAeronefs = p_db->recupererStatsAeronefs(p_annee, 
                p_options);
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
        break;

        case AeroDmsTypes::Statistiques_CONSO_PAR_TYPE_DE_VOL:
        case AeroDmsTypes::Statistiques_CONSO_PAR_ACTIVITE:
        {
            const AeroDmsTypes::ListeStatsEmissionsCo2 statsCo2 = p_db->recupererStatsEmissionsParTypeDeVol( p_annee, 
                                                                                                             p_options, 
                                                                                                             p_statistique );
            indiceCouleurEnCours = 0;

            auto donutBreakdown = new StatistiqueDonutCombine(this);

            QString typeCourant = "init";
            auto series = new QPieSeries(this);

            for (int i = 0; i < statsCo2.liste.size(); i++)
            {
                if (typeCourant == statsCo2.liste.at(i).activiteOuTypeDeVol)
                {
                    QString unite = " L";
                    if (statsCo2.liste.at(i).uniteTypeConsommation == AeroDmsTypes::UniteTypeConsommation_KILOWATTHEURES)
                    {
                        unite = " kWh";
                    }
                    QPieSlice* pieSlice = new QPieSlice( statsCo2.liste.at(i).type + ", " + QString::number(statsCo2.liste.at(i).consommationEnLitres(), 'f', 0) + unite + ", ",
                                                         statsCo2.liste.at(i).consommationEnLitres());
                    series->append(pieSlice);
                }
                else
                {
                    if (typeCourant != "init")
                    {
                        donutBreakdown->addBreakdownSeries( series, 
                                                            recupererNouvelleCouleur(), 
                                                            p_parametres.tailleDePolice );
                    }
                    typeCourant = statsCo2.liste.at(i).activiteOuTypeDeVol;
                    series = new QPieSeries(this);
                    series->setName(typeCourant);
                    QString unite = " L";
                    if (statsCo2.liste.at(i).uniteTypeConsommation == AeroDmsTypes::UniteTypeConsommation_KILOWATTHEURES)
                    {
                        unite = " kWh";
                    }
                    series->append( statsCo2.liste.at(i).type + ", " + QString::number(statsCo2.liste.at(i).consommationEnLitres(), 'f', 0) + unite + ", ",
                                    statsCo2.liste.at(i).consommationEnLitres());
                }
            }
            if (series->count() > 0)
            {
                donutBreakdown->addBreakdownSeries( series, 
                                                    recupererNouvelleCouleur(), 
                                                    p_parametres.tailleDePolice);
            }

            createDefaultChartView( "Consommation d'énergie", 
                                    p_legende, 
                                    Qt::AlignRight, 
                                    static_cast<int>(p_parametres.tailleDePolice * 1.5));
            
            for (QPieSeries* currentSeries : donutBreakdown->series())
            {
                addSeriesToGraph(currentSeries);
            }
                
        }
        break;

        case AeroDmsTypes::Statistiques_CO2_PAR_TYPE_DE_VOL:
        case AeroDmsTypes::Statistiques_CO2_PAR_ACTIVITE:
        {
            const AeroDmsTypes::ListeStatsEmissionsCo2 statsCo2 = p_db->recupererStatsEmissionsParTypeDeVol(p_annee,
                p_options,
                p_statistique);

            const AeroDmsTypes::ParametresEmissionsCo2 parametresEmissionsCo2 = p_db->lireParametresEmissionsCo2();

            indiceCouleurEnCours = 0;

            auto donutBreakdown = new StatistiqueDonutCombine(this);

            QString typeCourant = "init";
            auto series = new QPieSeries(this);

            for (int i = 0; i < statsCo2.liste.size(); i++)
            {
                if (typeCourant == statsCo2.liste.at(i).activiteOuTypeDeVol)
                {
                    const double facteurDemission = recupererFacteurDEmissions( statsCo2.liste.at(i).uniteTypeConsommation, 
                                                                                parametresEmissionsCo2 );

                    QPieSlice* pieSlice = new QPieSlice(statsCo2.liste.at(i).type + ", " + QString::number(statsCo2.liste.at(i).consommationEnLitres()*facteurDemission, 'f', 0) + " kgCO₂, ",
                        statsCo2.liste.at(i).consommationEnLitres());
                    series->append(pieSlice);
                }
                else
                {
                    if (typeCourant != "init")
                    {
                        donutBreakdown->addBreakdownSeries(series,
                            recupererNouvelleCouleur(),
                            p_parametres.tailleDePolice);
                    }
                    typeCourant = statsCo2.liste.at(i).activiteOuTypeDeVol;
                    series = new QPieSeries(this);
                    series->setName(typeCourant);

                    const double facteurDemission = recupererFacteurDEmissions(statsCo2.liste.at(i).uniteTypeConsommation,
                                                                               parametresEmissionsCo2);

                    series->append(statsCo2.liste.at(i).type + ", " + QString::number(statsCo2.liste.at(i).consommationEnLitres() * facteurDemission, 'f', 0) + " kgCO₂, ",
                        statsCo2.liste.at(i).consommationEnLitres());
                }
            }
            if (series->count() > 0)
            {
                donutBreakdown->addBreakdownSeries(series,
                    recupererNouvelleCouleur(),
                    p_parametres.tailleDePolice);
            }

            createDefaultChartView("Répartition des émissions de CO₂",
                p_legende,
                Qt::AlignRight,
                static_cast<int>(p_parametres.tailleDePolice * 1.5));

            for (QPieSeries* currentSeries : donutBreakdown->series())
            {
                addSeriesToGraph(currentSeries);
            }

        }
        break;
    }  
}

const double StatistiqueDonutCombineWidget::recupererFacteurDEmissions(const AeroDmsTypes::UniteTypeConsommation p_uniteTypeConsommation, 
    const AeroDmsTypes::ParametresEmissionsCo2 p_parametresEmissionsCo2)
{
    switch (p_uniteTypeConsommation)
    {
    case AeroDmsTypes::UniteTypeConsommation_LITRES_ESSENCE:
    {
        return p_parametresEmissionsCo2.kgCo2ParLitreEssence;
    }
    break;
    case AeroDmsTypes::UniteTypeConsommation_LITRES_GASOIL_KEROSENE:
    {
        return p_parametresEmissionsCo2.kgCo2ParLitreGasoil;
    }
    break;
    case AeroDmsTypes::UniteTypeConsommation_KILOWATTHEURES:
    {
        return p_parametresEmissionsCo2.kgCo2ParKwh;
    }
    break;
    }
    return 0;
}

QColor StatistiqueDonutCombineWidget::recupererNouvelleCouleur()
{
    /*const QList<QColor> palette = {
        QColor("#2f89ca"),
        QColor("#f28e2b"),
        QColor("#59a14f"),
        QColor("#af7aa1"),
        QColor("#76b7b2"),
        QColor("#e15759"),
        QColor("#4e79a7"),
        QColor("#edc948")
    };*/
    const QList<QColor> palette = {
    QColor("#2f89ca"),
    QColor("#f28e2b"),
    QColor("#59a14f"),
    QColor("#af7aa1"),
    QColor("#76b7b2"),
    QColor("#e15759"),
    QColor("#4e79a7"),
    QColor("#edc948"),
    QColor("#b07aa1"),
    QColor("#ff9da7"),
    QColor("#9c755f"),
    QColor("#bab0ac"),
    QColor("#86bcb6"),
    QColor("#d37295"),
    QColor("#fabfd2"),
    QColor("#b6992d"),
    QColor("#499894"),
    QColor("#79706e"),
    QColor("#d4a6c8"),
    QColor("#8cd17d"),
    QColor("#f1ce63"),
    QColor("#a0cbe8"),
    QColor("#ffbe7d"),
    QColor("#d7b5a6"),
    QColor("#698553"),
    QColor("#c9b45c"),
    QColor("#6b6ecf"),
    QColor("#e07b39"),
    QColor("#3d85c6"),
    QColor("#a64d79")
    };

    const QColor couleur = palette.at(indiceCouleurEnCours % palette.size());
    indiceCouleurEnCours++;
    return couleur;
}

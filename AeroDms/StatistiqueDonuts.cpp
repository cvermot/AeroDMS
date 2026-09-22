// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonuts.h"

#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>

StatistiqueDonuts::StatistiqueDonuts( ManageDb* p_db, 
                                      const AeroDmsTypes::Statistiques p_statistique, 
                                      QWidget* parent,
                                      int p_annee,
                                      const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres,
                                      const AeroDmsTypes::OptionsDonneesStatistiques p_options,
                                      const bool p_legende)
    : StatistiqueWidget(parent)
{
    setMinimumSize(p_parametres.tailleMiniImage);

    bool animation = true;
    if (p_parametres.tailleMiniImage != AeroDmsTypes::K_INIT_RESOLUTION_ET_PARAMETRES_STATISTIQUES.tailleMiniImage)
    {
        animation = false;
    }

    switch (p_statistique)
    {
        case AeroDmsTypes::Statistiques_STATUTS_PILOTES:
        {
            afficherStatsPilotes(p_db, p_parametres, animation, p_legende);
            break;
        }
        case AeroDmsTypes::Statistiques_AERONEFS:
        default:
        {
            afficherStatsAeronefs(p_db, p_annee, p_options, p_parametres, animation, p_legende);
            break;
        }
    }
}

void StatistiqueDonuts::afficherStatsPilotes( ManageDb* p_db,
                                              const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres, 
                                              bool p_animation,
                                              bool p_legende)
{
    Q_UNUSED(p_animation);
    QFont font("Arial", p_parametres.tailleDePolice);

    const AeroDmsTypes::StatsPilotes statsPilotes = p_db->recupererStatsPilotes();
  
    qreal minSize = 0.5;
    qreal maxSize = 0.9;
    int donutCount = 2;
    
    int niveauDuDonut = 0;
    auto donutBrevete = new QPieSeries;

    auto sliceBrevete = new QPieSlice(QString("Breveté"), statsPilotes.nbBrevete);
    sliceBrevete->setLabelFont(font);
    sliceBrevete->setLabelVisible(true);
    sliceBrevete->setLabelColor(Qt::white);
    sliceBrevete->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
    //connect(slice, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutBrevete->append(sliceBrevete);
    donutBrevete->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutBrevete->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);

    auto sliceNonBrevete = new QPieSlice(QString("Non Breveté"), statsPilotes.nbNonBrevete);
    sliceNonBrevete->setLabelFont(font);
    sliceNonBrevete->setLabelVisible(true);
    sliceNonBrevete->setLabelColor(Qt::white);
    sliceNonBrevete->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
    //connect(sliceNonBrevete, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutBrevete->append(sliceNonBrevete);
    donutBrevete->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutBrevete->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutBrevete);

    niveauDuDonut++;
    auto donutAyantDroit = new QPieSeries;
    auto sliceOuvrantDroit = new QPieSlice(QString("Ouvrant droit"), statsPilotes.nbOuvranDroit);
    sliceOuvrantDroit->setLabelFont(font);
    sliceOuvrantDroit->setLabelVisible(true);
    sliceOuvrantDroit->setLabelColor(Qt::white);
    sliceOuvrantDroit->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
    //connect(sliceOuvrantDroit, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutAyantDroit->append(sliceOuvrantDroit);
    donutAyantDroit->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutAyantDroit->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);

    auto sliceAyantDroit = new QPieSlice(QString("Ayant droit"), statsPilotes.nbAyantDroit);
    sliceAyantDroit->setLabelFont(font);
    sliceAyantDroit->setLabelVisible(true);
    sliceAyantDroit->setLabelColor(Qt::white);
    sliceAyantDroit->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
    //connect(sliceAyantDroit, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutAyantDroit->append(sliceAyantDroit);
    donutAyantDroit->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutAyantDroit->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutAyantDroit);

    createDefaultChartView("Statistiques sur les pilotes", p_legende, Qt::AlignRight, static_cast<int>(p_parametres.tailleDePolice * 1.5));
    addSeriesToGraph(donutBrevete);
    addSeriesToGraph(donutAyantDroit);
}

void StatistiqueDonuts::afficherStatsAeronefs( ManageDb* p_db, 
                                               int p_annee, 
                                               const AeroDmsTypes::OptionsDonneesStatistiques p_options,
                                               const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres,
                                               bool p_animation,
                                               bool p_legende)
{
    Q_UNUSED(p_animation);
    const AeroDmsTypes::StatsAeronefs statsAeronefs = p_db->recupererStatsAeronefs(p_annee, p_options);

    qreal minSize = 0.5;
    qreal maxSize = 0.9;
    int donutCount = 2;
    
    int niveauDonutImmat = 0;
    int niveauDonutType = 1;

    auto donutImmat = new QPieSeries;
    auto donutType = new QPieSeries;

    int nbMinutesType = 0;
    QString typeCourant = statsAeronefs.at(0).type;
    
    for (int i = 0; i < statsAeronefs.size(); i++)
    {
        auto sliceImmat = new QPieSlice(statsAeronefs.at(i).immat, statsAeronefs.at(i).nombreMinutesVol);
        sliceImmat->setLabelVisible(true);
        sliceImmat->setLabelColor(Qt::white);
        sliceImmat->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
        //connect(slice, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
        donutImmat->append(sliceImmat);

        if (typeCourant != statsAeronefs.at(i).type)
        {
            auto sliceType = new QPieSlice(typeCourant, nbMinutesType);
            sliceType->setLabelVisible(true);
            sliceType->setLabelColor(Qt::white);
            sliceType->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
            //connect(slice, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
            donutType->append(sliceType);
            nbMinutesType = 0;
            typeCourant = statsAeronefs.at(i).type;
        }
        nbMinutesType = nbMinutesType + statsAeronefs.at(i).nombreMinutesVol;

        if (i == statsAeronefs.size() - 1)
        {
            auto sliceType = new QPieSlice(typeCourant, nbMinutesType);
            sliceType->setLabelVisible(true);
            sliceType->setLabelColor(Qt::white);
            sliceType->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
            //connect(slice, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
            donutType->append(sliceType);
        }

    }
    donutImmat->setHoleSize(minSize + niveauDonutImmat * (maxSize - minSize) / donutCount);
    donutImmat->setPieSize(minSize + (niveauDonutImmat + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutImmat);

    donutType->setHoleSize(minSize + niveauDonutType * (maxSize - minSize) / donutCount);
    donutType->setPieSize(minSize + (niveauDonutType + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutType);

    createDefaultChartView("Statistiques sur les aéronefs", p_legende, Qt::AlignRight, static_cast<int>(p_parametres.tailleDePolice * 1.5));
    addSeriesToGraph(donutImmat);
    addSeriesToGraph(donutType);
}

void StatistiqueDonuts::explodeSlice(bool exploded)
{
    auto slice = qobject_cast<QPieSlice*>(sender());
    if (exploded) {
        qreal sliceStartAngle = slice->startAngle();
        qreal sliceEndAngle = slice->startAngle() + slice->angleSpan();

        QPieSeries* donut = slice->series();
        qreal seriesIndex = m_donuts.indexOf(donut);
        for (int i = seriesIndex + 1; i < m_donuts.count(); i++) {
            m_donuts.at(i)->setStartAngle(sliceEndAngle);
            m_donuts.at(i)->setEndAngle(360 + sliceStartAngle);
        }
    }
    else {
        for (int i = 0; i < m_donuts.count(); i++) {
            m_donuts.at(i)->setStartAngle(0);
            m_donuts.at(i)->setEndAngle(360);
        }
    }
    slice->setExploded(exploded);
}

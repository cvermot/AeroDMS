// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonuts.h"

#include <QChart>
#include <QChartView>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QLegend>
#include <QPieSeries>
#include <QPieSlice>
#include <QRandomGenerator>

StatistiqueDonuts::StatistiqueDonuts(ManageDb* p_db, QWidget* parent)
    : StatistiqueWidget(parent)
{
    const AeroDmsTypes::StatsPilotes statsPilotes =  p_db->recupererStatsPilotes();
    //! [1]
    auto chartView = new QChartView(this);
    chartView->setRenderHint(QPainter::Antialiasing);
    QChart* chart = chartView->chart();
    chart->legend()->setVisible(false);
    chart->setTitle("Statistiques sur les pilotes");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    //! [1]

    //! [2]
    qreal minSize = 0.5;
    qreal maxSize = 0.9;
    int donutCount = 2;
    //! [2]

    //! [3]
    //for (int i = 0; i < donutCount; i++) {
    int niveauDuDonut = 0;
    auto donutBrevete = new QPieSeries;

    auto sliceBrevete = new QPieSlice(QString("Breveté"), statsPilotes.nbBrevete);
    sliceBrevete->setLabelVisible(true);
    sliceBrevete->setLabelColor(Qt::white);
    sliceBrevete->setLabelPosition(QPieSlice::LabelInsideTangential);
    //connect(slice, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutBrevete->append(sliceBrevete);
    donutBrevete->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutBrevete->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);

    auto sliceNonBrevete = new QPieSlice(QString("Non Breveté"), statsPilotes.nbNonBrevete);
    sliceNonBrevete->setLabelVisible(true);
    sliceNonBrevete->setLabelColor(Qt::white);
    sliceNonBrevete->setLabelPosition(QPieSlice::LabelInsideTangential);
    //connect(sliceNonBrevete, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutBrevete->append(sliceNonBrevete);
    donutBrevete->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutBrevete->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutBrevete);
    chartView->chart()->addSeries(donutBrevete);


    niveauDuDonut++;
    auto donutAyantDroit = new QPieSeries;
    qDebug() << "stats pilote" << statsPilotes.nbBrevete << statsPilotes.nbNonBrevete;
    auto sliceOuvrantDroit = new QPieSlice(QString("Ouvrant droit"), statsPilotes.nbOuvranDroit);
    sliceOuvrantDroit->setLabelVisible(true);
    sliceOuvrantDroit->setLabelColor(Qt::white);
    sliceOuvrantDroit->setLabelPosition(QPieSlice::LabelInsideTangential);
    //connect(sliceOuvrantDroit, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutAyantDroit->append(sliceOuvrantDroit);
    donutAyantDroit->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutAyantDroit->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);

    auto sliceAyantDroit = new QPieSlice(QString("Ayant droit"), statsPilotes.nbAyantDroit);
    sliceAyantDroit->setLabelVisible(true);
    sliceAyantDroit->setLabelColor(Qt::white);
    sliceAyantDroit->setLabelPosition(QPieSlice::LabelInsideTangential);
    //connect(sliceAyantDroit, &QPieSlice::hovered, this, &StatistiqueDonuts::explodeSlice);
    donutAyantDroit->append(sliceAyantDroit);
    donutAyantDroit->setHoleSize(minSize + niveauDuDonut * (maxSize - minSize) / donutCount);
    donutAyantDroit->setPieSize(minSize + (niveauDuDonut + 1) * (maxSize - minSize) / donutCount);
    m_donuts.append(donutAyantDroit);
    chartView->chart()->addSeries(donutAyantDroit);
    //}
    //! [3]

    // create main layout
    //! [4]
    auto mainLayout = new QGridLayout;
    mainLayout->addWidget(chartView, 1, 1);
    setLayout(mainLayout);
    //! [4]

}

//! [7]
void StatistiqueDonuts::explodeSlice(bool exploded)
{
    auto slice = qobject_cast<QPieSlice*>(sender());
    if (exploded) {
        qreal sliceStartAngle = slice->startAngle();
        qreal sliceEndAngle = slice->startAngle() + slice->angleSpan();

        QPieSeries* donut = slice->series();
        qreal seriesIndex = m_donuts.indexOf(donut);
        for (int i = seriesIndex + 1; i < m_donuts.count(); i++) {
            m_donuts.at(i)->setPieStartAngle(sliceEndAngle);
            m_donuts.at(i)->setPieEndAngle(360 + sliceStartAngle);
        }
    }
    else {
        for (int i = 0; i < m_donuts.count(); i++) {
            m_donuts.at(i)->setPieStartAngle(0);
            m_donuts.at(i)->setPieEndAngle(360);
        }
    }
    slice->setExploded(exploded);
}
//! [7]
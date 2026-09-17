// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonutCombine.h"
#include "StatistiqueDonutCombinePartie.h"

#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>

StatistiqueDonutCombine::StatistiqueDonutCombine(QObject* parent)
    : QObject(parent)
{
    // create the series for main center pie
    m_mainSeries = new QPieSeries(this);
    m_mainSeries->setProperty("excludeFromLegend", true);
    m_mainSeries->setPieSize(0.7);
    m_mainSeries->setHoleSize(0.25);
    m_series.append(m_mainSeries);
}

void StatistiqueDonutCombine::addBreakdownSeries(QPieSeries* breakdownSeries, QColor color, int tailleDePolice)
{
    QFont font("Arial", tailleDePolice);

    // add breakdown series as a slice to center pie
    auto mainSlice = new StatistiqueDonutCombinePartie(breakdownSeries);
    mainSlice->setName(breakdownSeries->name());
    mainSlice->setValue(breakdownSeries->sum());
    m_mainSeries->append(mainSlice);

    // customize the slice
    mainSlice->setColor(color);
    mainSlice->setLabelVisible();
    const double darkness = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
    if(darkness > 0.5)
        mainSlice->setLabelColor(Qt::black);
    else
        mainSlice->setLabelColor(Qt::white);
    mainSlice->setLabelPosition(QPieSlice::LabelPosition::InsideNormal);
    mainSlice->setLabelFont(font);

    // position and customize the breakdown series
    breakdownSeries->setPieSize(0.8);
    breakdownSeries->setHoleSize(0.7);
    breakdownSeries->setLabelsVisible();
    const auto slices = breakdownSeries->slices();
    for (QPieSlice* slice : slices) {
        color = color.lighter(115);
        slice->setColor(color);
        slice->setLabelFont(font);
        slice->setProperty("baseLabel", slice->label());
    }

    m_series.append(breakdownSeries);

    // recalculate breakdown donut segments
    recalculateAngles();

    for (QPieSlice* slice : slices) {
        const QString baseLabel = slice->property("baseLabel").toString();
        slice->setLabel(QString("%1 %2%").arg(baseLabel).arg(slice->percentage() * 100, 0, 'f', 2));
    }
}

void StatistiqueDonutCombine::recalculateAngles()
{
    qreal angle = 0;
    const auto slices = m_mainSeries->slices();
    for (QPieSlice* slice : slices) {
        QPieSeries* breakdownSeries = qobject_cast<StatistiqueDonutCombinePartie*>(slice)->breakdownSeries();
        breakdownSeries->setStartAngle(angle);
        angle += slice->percentage() * 360.0; // full pie is 360.0
        breakdownSeries->setEndAngle(angle);
    }
}

QList<QPieSeries*> StatistiqueDonutCombine::series() const
{
    return m_series;
}

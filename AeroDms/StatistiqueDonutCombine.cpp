// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonutCombine.h"
#include "StatistiqueDonutCombinePartie.h"

#include <QPieLegendMarker>
#include <QPieSeries>
#include <QPieSlice>

QT_USE_NAMESPACE

StatistiqueDonutCombine::StatistiqueDonutCombine(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QChart(QChart::ChartTypeCartesian, parent, wFlags)
{
    // create the series for main center pie
    m_mainSeries = new QPieSeries;
    m_mainSeries->setPieSize(0.7);
    m_mainSeries->setHoleSize(0.25);
    QChart::addSeries(m_mainSeries);
}

void StatistiqueDonutCombine::addBreakdownSeries(QPieSeries* breakdownSeries, QColor color, int tailleDePolice)
{
    QSizeF widgetSize = size();
    QFont font("Arial", tailleDePolice);

    setTitleFont(font);

    // add breakdown series as a slice to center pie
    auto mainSlice = new StatistiqueDonutCombinePartie(breakdownSeries);
    mainSlice->setName(breakdownSeries->name());
    mainSlice->setValue(breakdownSeries->sum());
    m_mainSeries->append(mainSlice);

    // customize the slice
    mainSlice->setBrush(color);
    mainSlice->setLabelVisible();
    const double darkness = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
    if(darkness > 0.5)
        mainSlice->setLabelColor(Qt::black);
    else
        mainSlice->setLabelColor(Qt::white);
    mainSlice->setLabelPosition(QPieSlice::LabelInsideNormal);
    mainSlice->setLabelFont(font);

    // position and customize the breakdown series
    breakdownSeries->setPieSize(0.8);
    breakdownSeries->setHoleSize(0.7);
    breakdownSeries->setLabelsVisible();
    const auto slices = breakdownSeries->slices();
    for (QPieSlice* slice : slices) {
        color = color.lighter(115);
        slice->setBrush(color);
        slice->setLabelFont(font);
    }

    // add the series to the chart
    QChart::addSeries(breakdownSeries);

    // recalculate breakdown donut segments
    recalculateAngles();

    // update customize legend markers
    updateLegendMarkers(font);
}

void StatistiqueDonutCombine::recalculateAngles()
{
    qreal angle = 0;
    const auto slices = m_mainSeries->slices();
    for (QPieSlice* slice : slices) {
        QPieSeries* breakdownSeries = qobject_cast<StatistiqueDonutCombinePartie*>(slice)->breakdownSeries();
        breakdownSeries->setPieStartAngle(angle);
        angle += slice->percentage() * 360.0; // full pie is 360.0
        breakdownSeries->setPieEndAngle(angle);
    }
}

void StatistiqueDonutCombine::updateLegendMarkers(QFont p_font)
{
    // go through all markers
    const auto allseries = series();
    for (QAbstractSeries* series : allseries) {
        const auto markers = legend()->markers(series);
        for (QLegendMarker* marker : markers) {
            auto pieMarker = qobject_cast<QPieLegendMarker*>(marker);
            if (series == m_mainSeries) {
                // hide markers from main series
                pieMarker->setVisible(false);
            }
            else {
                // modify markers from breakdown series
                pieMarker->setLabel(QString("%1 %2%")
                    .arg(pieMarker->slice()->label())
                    .arg(pieMarker->slice()->percentage() * 100, 0, 'f', 2));
                pieMarker->setFont(p_font);
            }
        }
    }
}

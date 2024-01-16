// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculaire.h"
#include "StatistiqueDiagrammeCirculairePartie.h"

StatistiqueDiagrammeCirculaire::StatistiqueDiagrammeCirculaire(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QChart(QChart::ChartTypeCartesian, parent, wFlags)
{
}

void StatistiqueDiagrammeCirculaire::changeSeries(QAbstractSeries* series)
{
    if (m_currentSeries)
        removeSeries(m_currentSeries);
    m_currentSeries = series;
    addSeries(series);
    setTitle(series->name());
}

void StatistiqueDiagrammeCirculaire::handleSliceClicked(QPieSlice* slice)
{
    auto* drilldownSlice = static_cast<StatistiqueDiagrammeCirculairePartie*>(slice);
    changeSeries(drilldownSlice->drilldownSeries());
}

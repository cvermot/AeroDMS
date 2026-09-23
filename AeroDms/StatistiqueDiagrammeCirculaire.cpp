// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculaire.h"
#include "StatistiqueDiagrammeCirculairePartie.h"

#include <QMetaObject>

StatistiqueDiagrammeCirculaire::StatistiqueDiagrammeCirculaire(QObject* parent)
    : QObject(parent)
{
}

void StatistiqueDiagrammeCirculaire::changeSeries(QAbstractSeries* series)
{
    m_currentSeries = series;
    QMetaObject::invokeMethod(this, [this, series]() {
        emit seriesChanged(series);
    }, Qt::QueuedConnection);
}

QAbstractSeries* StatistiqueDiagrammeCirculaire::currentSeries() const
{
    return m_currentSeries;
}

void StatistiqueDiagrammeCirculaire::handleSliceClicked(QPieSlice* slice)
{
    auto* drilldownSlice = static_cast<StatistiqueDiagrammeCirculairePartie*>(slice);
    changeSeries(drilldownSlice->drilldownSeries());
}

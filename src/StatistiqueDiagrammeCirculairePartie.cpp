// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculairePartie.h"
#include "AeroDmsTypes.h"

StatistiqueDiagrammeCirculairePartie::StatistiqueDiagrammeCirculairePartie(qreal value, const QString& prefix, QAbstractSeries* drilldownSeries)
    : m_drilldownSeries(drilldownSeries),
    m_prefix(prefix)
{
    setValue(value);
    if (value != 0)
    {
        setLabelVisible(true);
    }  
    updateLabel();
    setLabelFont(QFont("Arial", 8));
    connect(this, &StatistiqueDiagrammeCirculairePartie::percentageChanged, this, &StatistiqueDiagrammeCirculairePartie::updateLabel);
    connect(this, &StatistiqueDiagrammeCirculairePartie::hovered, this, &StatistiqueDiagrammeCirculairePartie::showHighlight);
}

QAbstractSeries* StatistiqueDiagrammeCirculairePartie::drilldownSeries() const
{
    return m_drilldownSeries;
}

void StatistiqueDiagrammeCirculairePartie::updateLabel()
{
    setLabel(QStringLiteral("%1 : %2, %3%").arg(m_prefix,
        AeroDmsTypes::convertirMinutesEnHeuresMinutes(value()),
        QString::number(percentage() * 100, 'f', 1)));
}

void StatistiqueDiagrammeCirculairePartie::showHighlight(bool show)
{
    //setLabelVisible(show);
    setExploded(show);
}

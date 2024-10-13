// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDonutCombinePartie.h"

StatistiqueDonutCombinePartie::StatistiqueDonutCombinePartie(QPieSeries* breakdownSeries, QObject* parent)
    : QPieSlice(parent),
    m_breakdownSeries(breakdownSeries)
{
    connect(this, &StatistiqueDonutCombinePartie::percentageChanged,
        this, &StatistiqueDonutCombinePartie::updateLabel);
}

QPieSeries* StatistiqueDonutCombinePartie::breakdownSeries() const
{
    return m_breakdownSeries;
}

void StatistiqueDonutCombinePartie::setName(const QString& name)
{
    m_name = name;
}

QString StatistiqueDonutCombinePartie::name() const
{
    return m_name;
}

void StatistiqueDonutCombinePartie::updateLabel()
{
    setLabel(QString("%1 %2%").arg(m_name).arg(percentage() * 100, 0, 'f', 2));
    QFont font;
    font.setPointSize(30);
    //setLabelFont(font);
}

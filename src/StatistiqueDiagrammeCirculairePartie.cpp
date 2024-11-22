// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculairePartie.h"
#include "AeroDmsServices.h"

StatistiqueDiagrammeCirculairePartie::StatistiqueDiagrammeCirculairePartie(qreal value, const QString& prefix, const int p_taillePolice, QAbstractSeries* drilldownSeries, const AeroDmsTypes::Unites p_unites)
    : m_drilldownSeries(drilldownSeries),
    m_prefix(prefix)
{
    unites = p_unites;
    setValue(value);
    if (value != 0)
    {
        setLabelVisible(true);
    }  
    updateLabel();
    setLabelFont(QFont("Arial", p_taillePolice));
    connect(this, &StatistiqueDiagrammeCirculairePartie::percentageChanged, this, &StatistiqueDiagrammeCirculairePartie::updateLabel);
    connect(this, &StatistiqueDiagrammeCirculairePartie::hovered, this, &StatistiqueDiagrammeCirculairePartie::showHighlight);
}

QAbstractSeries* StatistiqueDiagrammeCirculairePartie::drilldownSeries() const
{
    return m_drilldownSeries;
}

void StatistiqueDiagrammeCirculairePartie::updateLabel()
{
    QString valeur = "";
    if (unites == AeroDmsTypes::Unites_HEURES)
    {
        valeur = AeroDmsServices::convertirMinutesEnHeuresMinutes(value());
    }
    else
    {
        valeur = QString::number(value(), 'f', 2) + " €";
    }

    setLabel(QStringLiteral("%1 : %2, %3%").arg(m_prefix,
        valeur,
        QString::number(percentage() * 100, 'f', 1)));
}

void StatistiqueDiagrammeCirculairePartie::showHighlight(bool show)
{
    //setLabelVisible(show);
    setExploded(show);
}

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEDIAGRAMMECIRCULAIREPARTIE_H
#define STATISTIQUEDIAGRAMMECIRCULAIREPARTIE_H

#include "AeroDmsTypes.h"

#include <QPieSlice>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries);

class StatistiqueDiagrammeCirculairePartie : public QPieSlice
{
    Q_OBJECT
public:
    StatistiqueDiagrammeCirculairePartie(qreal value, const QString& prefix, const int p_taillePolice, QAbstractSeries* drilldownSeries, const AeroDmsTypes::Unites p_unites = AeroDmsTypes::Unites_HEURES);
    QAbstractSeries* drilldownSeries() const;

public slots:
    void updateLabel();
    void showHighlight(bool show);

private:
    QAbstractSeries* m_drilldownSeries = nullptr;
    AeroDmsTypes::Unites unites;
    QString m_prefix;
};

#endif //STATISTIQUEDIAGRAMMECIRCULAIREPARTIE_H

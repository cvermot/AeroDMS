// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIEDRILLDOWNSLICE_H
#define PIEDRILLDOWNSLICE_H

#include <QPieSlice>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries);

class StatistiqueDiagrammeCirculairePartie : public QPieSlice
{
    Q_OBJECT
public:
    StatistiqueDiagrammeCirculairePartie(qreal value, const QString& prefix, const int p_taillePolice, QAbstractSeries* drilldownSeries);
    QAbstractSeries* drilldownSeries() const;

public slots:
    void updateLabel();
    void showHighlight(bool show);

private:
    QAbstractSeries* m_drilldownSeries = nullptr;
    QString m_prefix;
};

#endif

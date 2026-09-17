// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATISTIQUEWIDGET_H
#define STATISTIQUEWIDGET_H

#include <QList>
#include <QWidget>

#include "ManageDb.h"

QT_FORWARD_DECLARE_CLASS(QAbstractAxis)
QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_CLASS(QObject)
QT_FORWARD_DECLARE_CLASS(QQuickWidget)

class StatistiqueWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatistiqueWidget(QWidget* parent = nullptr);

    void load();

protected:
    virtual bool doLoad();
    void resizeEvent(QResizeEvent*) override;
    void createDefaultChartView(const QString& p_titre,
                                bool p_legendeVisible = true,
                                Qt::Alignment p_alignementLegende = Qt::AlignRight,
                                int p_tailleTitre = 20);
    QObject* defaultChartView() const { return m_defaultChartView; }
    void setGraphTitle(const QString& p_titre);
    void addSeriesToGraph(QAbstractSeries* p_series);
    void clearGraphSeries();
    void setGraphAxisX(QAbstractAxis* p_axis);
    void setGraphAxisY(QAbstractAxis* p_axis);
    void refreshLegend();

    QString m_loadError;

private:
    QQuickWidget* m_quickWidget = nullptr;
    QObject* m_defaultChartView = nullptr;
    QObject* m_rootObject = nullptr;
    QList<QAbstractSeries*> m_series;
    bool m_loaded = false;
    ManageDb* db;
};

#endif //STATISTIQUEWIDGET_H

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueWidget.h"

#include <QFont>
#include <QLabel>
#include <QMetaObject>
#include <QQuickWidget>
#include <QResizeEvent>
#include <QUrl>
#include <QVariantMap>
#include <QVBoxLayout>

#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QAbstractAxis>
#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>

StatistiqueWidget::StatistiqueWidget(QWidget* parent)
    : QWidget(parent)
{
}

void StatistiqueWidget::load()
{
    if (m_loaded || layout())
        return;

    if (!doLoad()) {
        auto errorLabel = new QLabel(this);
        auto errorLayout = new QVBoxLayout(this);
        errorLabel->setText(tr("Erreur de chargement :\n%1").arg(m_loadError));
        QFont font = errorLabel->font();
        font.setPointSize(20);
        errorLabel->setFont(font);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLayout->addWidget(errorLabel);
        setLayout(errorLayout);
    }

    m_loaded = true;
}

bool StatistiqueWidget::doLoad()
{
    // Most examples do their initialization in constructor.
    // Only those that can fail and show error message need to reimplement this method.
    return true;
}

void StatistiqueWidget::resizeEvent(QResizeEvent*)
{
    if (m_quickWidget)
        m_quickWidget->resize(size());
}

// Most examples are simple and need only basic chart view widget, so provide it in this base class
// to avoid duplicating code
void StatistiqueWidget::createDefaultChartView(const QString& p_titre,
                                               bool p_legendeVisible,
                                               Qt::Alignment p_alignementLegende,
                                               int p_tailleTitre)
{
    if (m_quickWidget)
        return;

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/AeroDms/ressources/GraphsViewContainer.qml"));

    m_rootObject = m_quickWidget->rootObject();
    if (!m_rootObject) {
        m_loadError = tr("Impossible de charger la vue Qt Graphs");
        delete m_quickWidget;
        m_quickWidget = nullptr;
        return;
    }

    m_defaultChartView = m_rootObject->findChild<QObject*>("graphView");
    if (!m_defaultChartView) {
        m_loadError = tr("Impossible d'initialiser la vue de graphique");
        delete m_quickWidget;
        m_quickWidget = nullptr;
        m_rootObject = nullptr;
        return;
    }

    m_rootObject->setProperty("chartTitle", p_titre);
    m_rootObject->setProperty("legendVisible", p_legendeVisible);
    m_rootObject->setProperty("titlePointSize", p_tailleTitre);
    m_rootObject->setProperty("legendPosition", p_alignementLegende == Qt::AlignBottom ? "bottom" : "right");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_quickWidget);
    setLayout(layout);
}

void StatistiqueWidget::setGraphTitle(const QString& p_titre)
{
    if (m_rootObject)
        m_rootObject->setProperty("chartTitle", p_titre);
}

void StatistiqueWidget::addSeriesToGraph(QAbstractSeries* p_series)
{
    if (!m_defaultChartView || !p_series)
        return;

    QMetaObject::invokeMethod(m_defaultChartView, "addSeries", Q_ARG(QAbstractSeries*, p_series));
    m_series.append(p_series);
    refreshLegend();
}

void StatistiqueWidget::showGraphSeries(QAbstractSeries* p_series)
{
    if (!m_defaultChartView || !p_series)
        return;

    if (m_activeSeries == p_series && !m_series.isEmpty())
        return;

    clearGraphSeries();
    m_activeSeries = p_series;
    addSeriesToGraph(p_series);
}

void StatistiqueWidget::clearGraphSeries()
{
    if (!m_defaultChartView)
        return;

    const QList<QAbstractSeries*> series = m_series;
    for (QAbstractSeries* serie : series) {
        QMetaObject::invokeMethod(m_defaultChartView, "removeSeries", Q_ARG(QAbstractSeries*, serie));
    }
    m_series.clear();
    m_activeSeries = nullptr;
    refreshLegend();
}

void StatistiqueWidget::setGraphAxisX(QAbstractAxis* p_axis)
{
    if (m_rootObject)
        m_rootObject->setProperty("axisX", QVariant::fromValue(p_axis));
}

void StatistiqueWidget::setGraphAxisY(QAbstractAxis* p_axis)
{
    if (m_rootObject)
        m_rootObject->setProperty("axisY", QVariant::fromValue(p_axis));
}

void StatistiqueWidget::refreshLegend()
{
    if (!m_rootObject)
        return;

    const QList<QColor> palette = {
        QColor("#1f77b4"),
        QColor("#ff7f0e"),
        QColor("#2ca02c"),
        QColor("#d62728"),
        QColor("#9467bd"),
        QColor("#8c564b"),
        QColor("#e377c2"),
        QColor("#7f7f7f"),
        QColor("#bcbd22"),
        QColor("#17becf")
    };

    QVariantList legendEntries;
    for (QAbstractSeries* serie : m_series) {
        if (!serie || serie->property("excludeFromLegend").toBool())
            continue;

        if (auto* pieSeries = qobject_cast<QPieSeries*>(serie)) {
            const auto slices = pieSeries->slices();
            for (int i = 0; i < slices.size(); ++i) {
                QPieSlice* slice = slices.at(i);
                QColor color = slice->color();
                if (!color.isValid()) {
                    color = palette.at(i % palette.size());
                    slice->setColor(color);
                }

                QVariantMap entry;
                entry.insert("legendColor", color);
                entry.insert("legendBorderColor", slice->borderColor());
                entry.insert("legendLabel", slice->label());
                legendEntries.append(entry);
            }
            continue;
        }

        if (auto* barSeries = qobject_cast<QBarSeries*>(serie)) {
            QList<QColor> seriesColors = barSeries->seriesColors();
            QList<QColor> borderColors = barSeries->borderColors();
            const auto barSets = barSeries->barSets();

            if (seriesColors.size() < barSets.size()) {
                for (int i = seriesColors.size(); i < barSets.size(); ++i)
                    seriesColors.append(palette.at(i % palette.size()));
                barSeries->setSeriesColors(seriesColors);
            }

            if (borderColors.size() < barSets.size()) {
                for (int i = borderColors.size(); i < barSets.size(); ++i)
                    borderColors.append(seriesColors.at(i % seriesColors.size()).darker(125));
                barSeries->setBorderColors(borderColors);
            }

            for (int i = 0; i < barSets.size(); ++i) {
                QVariantMap entry;
                entry.insert("legendColor", seriesColors.at(i));
                entry.insert("legendBorderColor", borderColors.at(i));
                entry.insert("legendLabel", barSets.at(i)->label());
                legendEntries.append(entry);
            }
        }
    }

    m_rootObject->setProperty("legendEntries", legendEntries);
    QMetaObject::invokeMethod(m_rootObject, "rebuildLegend");
}

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueWidget.h"

#include <QFont>
#include <QLabel>
#include <QMetaObject>
#include <QMetaMethod>
#include <QPropertyAnimation>
#include <QQuickItem>
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

namespace {

bool invokeSeriesMethod(QObject* target, const char* signature, QObject* series)
{
    if (!target || !series)
        return false;

    const QMetaObject* metaObject = target->metaObject();
    const int methodIndex = metaObject->indexOfMethod(signature);
    if (methodIndex < 0)
        return false;

    return metaObject->method(methodIndex).invoke(target, Q_ARG(QObject*, series));
}

}

StatistiqueWidget::StatistiqueWidget(QWidget* parent)
    : QWidget(parent)
{
}

void StatistiqueWidget::load()
{
    if (m_loaded)
        return;

    if (!m_loadError.isEmpty() || !doLoad()) {
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

    if (!invokeSeriesMethod(m_defaultChartView, "addSeries(QObject*)", p_series))
        return;

    p_series->setOpacity(0.0);
    m_series.append(p_series);
    refreshLegend();

    auto* animation = new QPropertyAnimation(p_series, "opacity", this);
    animation->setDuration(250);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatistiqueWidget::showGraphSeries(QAbstractSeries* p_series)
{
    if (!m_defaultChartView || !p_series)
        return;

    if (m_series.contains(p_series)) {
        if (m_activeSeries == p_series)
            return;

        const QList<QAbstractSeries*> trackedSeries = m_series;
        for (QAbstractSeries* serie : trackedSeries)
            serie->setVisible(serie == p_series);

        p_series->setOpacity(0.0);
        auto* animation = new QPropertyAnimation(p_series, "opacity", this);
        animation->setDuration(250);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        m_activeSeries = p_series;
        refreshLegend();
        return;
    }

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
        invokeSeriesMethod(m_defaultChartView, "removeSeries(QObject*)", serie);
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
        QColor("#d7eefb"),
        QColor("#bfdff6"),
        QColor("#a3d0ef"),
        QColor("#84bfe8"),
        QColor("#66addf"),
        QColor("#499bd5"),
        QColor("#2f89ca"),
        QColor("#266fa7"),
        QColor("#1b567f"),
        QColor("#113b59")
    };

    QVariantList legendEntries;
    for (QAbstractSeries* serie : m_series) {
        if (!serie || !serie->isVisible() || serie->property("excludeFromLegend").toBool())
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
            }

            if (borderColors.size() < barSets.size()) {
                for (int i = borderColors.size(); i < barSets.size(); ++i)
                    borderColors.append(seriesColors.at(i % seriesColors.size()).darker(125));
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

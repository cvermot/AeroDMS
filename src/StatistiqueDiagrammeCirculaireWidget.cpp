// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "StatistiqueDiagrammeCirculaire.h"
#include "StatistiqueDiagrammeCirculairePartie.h"
#include "StatistiqueDiagrammeCirculaireWidget.h"

#include <QChart>
#include <QLegend>
#include <QPieSeries>
#include <QRandomGenerator>

StatistiqueDiagrammeCirculaireWidget::StatistiqueDiagrammeCirculaireWidget( ManageDb* p_db, 
                                                                            const int p_annee, 
                                                                            const AeroDmsTypes::Statistiques p_statistique, 
                                                                            QWidget* parent, 
                                                                            const QChart::AnimationOption p_animation, 
                                                                            const bool p_legende,
                                                                            const AeroDmsTypes::ResolutionEtParametresStatistiques p_parametres)
    : StatistiqueWidget(parent)
{
    setMinimumSize(p_parametres.tailleMiniImage);
    QFont font("Arial", p_parametres.tailleDePolice);

    auto* chart = new StatistiqueDiagrammeCirculaire;
    chart->setTheme(QChart::ChartThemeLight);
    chart->setAnimationOptions(p_animation);
    chart->legend()->setVisible(p_legende);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->setFont(font);
    chart->setFont(font);

    font.setBold(true);
    font.setPointSizeF(p_parametres.tailleDePolice * 1.5);
    chart->setTitleFont(font);

    switch (p_statistique)
    {
        case AeroDmsTypes::Statistiques_HEURES_PAR_PILOTE:
        { 
            const AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = recupererSubventionsPilotes( p_db, 
                                                                                                      p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par pilote (cliquez pour le détail par pilote)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par pilote");
            }

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                auto detailParPilote = new QPieSeries(this);
                detailParPilote->setName("Détails par type de vol pour " + subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.tempsDeVolEnMinutes, "Entrainement", p_parametres.tailleDePolice, donneesTypeDeVolParPilote);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.tempsDeVolEnMinutes, "Sorties", p_parametres.tailleDePolice, donneesTypeDeVolParPilote);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.tempsDeVolEnMinutes, "Balades", p_parametres.tailleDePolice, donneesTypeDeVolParPilote);

                QObject::connect(detailParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailParPilote->sum(), subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, detailParPilote);

            }

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_EUROS_PAR_PILOTE:
        {
            const AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = recupererSubventionsPilotes(p_db,
                p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Subvention par pilote (cliquez pour le détail par pilote)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Subvention par pilote");
            }

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                auto detailParPilote = new QPieSeries(this);
                detailParPilote->setName("Détails par type de vol pour " + subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.montantRembourse, 
                    "Entrainement", p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.montantRembourse, 
                    "Sorties", p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);
                *detailParPilote << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.montantRembourse, 
                    "Balades", p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);

                QObject::connect(detailParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailParPilote->sum(), 
                    subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, detailParPilote, AeroDmsTypes::Unites_EUROS);

            }

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_HEURES_PAR_ACTIVITE:
        {
            AeroDmsTypes::ListeStatsHeuresDeVolParActivite subventionParActivite = p_db->recupererHeuresParActivite(p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par activité (cliquez pour le détail des heures par pilote dans l'activité)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par activité");
            }        

            auto detailAvion = new QPieSeries(this);
            detailAvion->setName("Avion");
            auto detailAvionElectrique = new QPieSeries(this);
            detailAvionElectrique->setName("Avion électrique");
            auto detailUlm = new QPieSeries(this);
            detailUlm->setName("ULM");
            auto detailPlaneur = new QPieSeries(this);
            detailPlaneur->setName("Planeur");
            auto detailHelicoptere = new QPieSeries(this);
            detailHelicoptere->setName("Hélicoptère");

            for (int i = 0; i < subventionParActivite.size(); i++)
            {
                if (subventionParActivite.at(i).minutesVolAvion != 0)
                {
                    *detailAvion << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolAvion,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolAvionElectrique != 0)
                {
                    *detailAvionElectrique << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolAvionElectrique,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolUlm != 0)
                {
                    *detailUlm << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolUlm, 
                        subventionParActivite.at(i).nomPrenomPilote, 
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolPlaneur != 0)
                {
                    *detailPlaneur << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolPlaneur, 
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote);
                }
                if (subventionParActivite.at(i).minutesVolHelicoptere != 0)
                {
                    *detailHelicoptere << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).minutesVolHelicoptere, 
                        subventionParActivite.at(i).nomPrenomPilote, 
                        p_parametres.tailleDePolice,
                        detailHelicoptere);
                }
            }
            if (detailAvion->sum() != 0)
            {
                QObject::connect(detailAvion, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailAvion->sum(), 
                    "Avion", p_parametres.tailleDePolice, detailAvion);
            }
            if (detailAvionElectrique->sum() != 0)
            {
                QObject::connect(detailAvionElectrique, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailAvionElectrique->sum(), 
                    "Avion électrique", p_parametres.tailleDePolice, detailAvionElectrique);
            }
            if (detailUlm->sum() != 0)
            {
                QObject::connect(detailUlm, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailUlm->sum(), 
                    "ULM", p_parametres.tailleDePolice, detailUlm);
            }
            if (detailPlaneur->sum() != 0)
            {
                QObject::connect(detailPlaneur, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailPlaneur->sum(), 
                    "Planeur", p_parametres.tailleDePolice, detailPlaneur);
            }
            if (detailHelicoptere->sum() != 0)
            {
                QObject::connect(detailHelicoptere, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailHelicoptere->sum(), 
                    "Hélicoptère", p_parametres.tailleDePolice, detailHelicoptere);
            }   

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_EUROS_PAR_ACTIVITE:
        {
            AeroDmsTypes::ListeStatsHeuresDeVolParActivite subventionParActivite = p_db->recupererHeuresParActivite(p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Subvention par activité (cliquez pour le détail des heures par pilote dans l'activité)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Subvention par activité");
            }

            auto detailAvion = new QPieSeries(this);
            detailAvion->setName("Avion");
            auto detailAvionElectrique = new QPieSeries(this);
            detailAvionElectrique->setName("Avion électrique");
            auto detailUlm = new QPieSeries(this);
            detailUlm->setName("ULM");
            auto detailPlaneur = new QPieSeries(this);
            detailPlaneur->setName("Planeur");
            auto detailHelicoptere = new QPieSeries(this);
            detailHelicoptere->setName("Hélicoptère");

            for (int i = 0; i < subventionParActivite.size(); i++)
            {
                if (subventionParActivite.at(i).subventionVolAvion != 0)
                {
                    *detailAvion << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).subventionVolAvion,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote, 
                        AeroDmsTypes::Unites_EUROS);
                }
                if (subventionParActivite.at(i).subventionVolAvionElectrique != 0)
                {
                    *detailAvionElectrique << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).subventionVolAvionElectrique,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote, 
                        AeroDmsTypes::Unites_EUROS);
                }
                if (subventionParActivite.at(i).subventionVolUlm != 0)
                {
                    *detailUlm << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).subventionVolUlm,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote, 
                        AeroDmsTypes::Unites_EUROS);
                }
                if (subventionParActivite.at(i).subventionVolPlaneur != 0)
                {
                    *detailPlaneur << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).subventionVolPlaneur,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        donneesTypeDeVolParPilote,
                        AeroDmsTypes::Unites_EUROS);
                }
                if (subventionParActivite.at(i).subventionVolHelicoptere != 0)
                {
                    *detailHelicoptere << new StatistiqueDiagrammeCirculairePartie(subventionParActivite.at(i).subventionVolHelicoptere,
                        subventionParActivite.at(i).nomPrenomPilote,
                        p_parametres.tailleDePolice,
                        detailHelicoptere, 
                        AeroDmsTypes::Unites_EUROS);
                }
            }
            if (detailAvion->sum() != 0)
            {
                QObject::connect(detailAvion, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailAvion->sum(), 
                    "Avion", p_parametres.tailleDePolice, detailAvion, AeroDmsTypes::Unites_EUROS);
            }
            if (detailAvionElectrique->sum() != 0)
            {
                QObject::connect(detailAvionElectrique, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailAvionElectrique->sum(), 
                    "Avion électrique", p_parametres.tailleDePolice, detailAvionElectrique, AeroDmsTypes::Unites_EUROS);
            }
            if (detailUlm->sum() != 0)
            {
                QObject::connect(detailUlm, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailUlm->sum(), 
                    "ULM", p_parametres.tailleDePolice, detailUlm, AeroDmsTypes::Unites_EUROS);
            }
            if (detailPlaneur->sum() != 0)
            {
                QObject::connect(detailPlaneur, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailPlaneur->sum(), 
                    "Planeur", p_parametres.tailleDePolice, detailPlaneur, AeroDmsTypes::Unites_EUROS);
            }
            if (detailHelicoptere->sum() != 0)
            {
                QObject::connect(detailHelicoptere, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
                *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailHelicoptere->sum(), 
                    "Hélicoptère", p_parametres.tailleDePolice, detailHelicoptere, AeroDmsTypes::Unites_EUROS);
            }

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_HEURES_PAR_TYPE_DE_VOL:
        {
            const AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = recupererSubventionsPilotes( p_db,
                                                                                                              p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par type de vol (cliquez pour le détail des heures par pilote dans la catégorie)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Temps de vol par type de vol");
            }

            auto detailEntrainement = new QPieSeries(this);
            detailEntrainement->setName("Vols d'entrainement");
            auto detailSortie = new QPieSeries(this);
            detailSortie->setName("Sorties");
            auto detailBalade = new QPieSeries(this);
            detailBalade->setName("Balades");

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                *detailEntrainement << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote);
                *detailSortie << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote);
                *detailBalade << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.tempsDeVolEnMinutes, subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote);
            }
            QObject::connect(detailEntrainement, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailEntrainement->sum(), "Vols d'entrainement", p_parametres.tailleDePolice, detailEntrainement);
            QObject::connect(detailSortie, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailSortie->sum(), "Sortie", p_parametres.tailleDePolice, detailSortie);
            QObject::connect(detailBalade, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailBalade->sum(), "Balades", p_parametres.tailleDePolice, detailBalade);

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

        case AeroDmsTypes::Statistiques_EUROS_PAR_TYPE_DE_VOL:
        default:
        {
            const AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = recupererSubventionsPilotes(p_db,
                p_annee);

            auto donneesTypeDeVolParPilote = new QPieSeries(this);
            if (p_animation != QChart::NoAnimation)
            {
                donneesTypeDeVolParPilote->setName("Subventions par type de vol (cliquez pour le détail des subventions par pilote dans la catégorie)");
            }
            else
            {
                donneesTypeDeVolParPilote->setName("Subventions par type de vol");
            }

            auto detailEntrainement = new QPieSeries(this);
            detailEntrainement->setName("Vols d'entrainement");
            auto detailSortie = new QPieSeries(this);
            detailSortie->setName("Sorties");
            auto detailBalade = new QPieSeries(this);
            detailBalade->setName("Balades");

            for (int i = 0; i < subventionParPilote.size(); i++)
            {
                *detailEntrainement << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).entrainement.montantRembourse, 
                    subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);
                *detailSortie << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).sortie.montantRembourse, 
                    subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);
                *detailBalade << new StatistiqueDiagrammeCirculairePartie(subventionParPilote.at(i).balade.montantRembourse, 
                    subventionParPilote.at(i).prenom + " " + subventionParPilote.at(i).nom, p_parametres.tailleDePolice, donneesTypeDeVolParPilote, AeroDmsTypes::Unites_EUROS);
            }
            QObject::connect(detailEntrainement, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailEntrainement->sum(), "Vols d'entrainement", p_parametres.tailleDePolice, detailEntrainement, AeroDmsTypes::Unites_EUROS);
            QObject::connect(detailSortie, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailSortie->sum(), "Sortie", p_parametres.tailleDePolice, detailSortie, AeroDmsTypes::Unites_EUROS);
            QObject::connect(detailBalade, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);
            *donneesTypeDeVolParPilote << new StatistiqueDiagrammeCirculairePartie(detailBalade->sum(), "Balades", p_parametres.tailleDePolice, detailBalade, AeroDmsTypes::Unites_EUROS);

            QObject::connect(donneesTypeDeVolParPilote, &QPieSeries::clicked, chart, &StatistiqueDiagrammeCirculaire::handleSliceClicked);

            chart->changeSeries(donneesTypeDeVolParPilote);
        }
        break;

    }
    
    createDefaultChartView(chart);
}

AeroDmsTypes::ListeSubventionsParPilotes StatistiqueDiagrammeCirculaireWidget::recupererSubventionsPilotes(ManageDb* p_db, const int p_annee)
{
    AeroDmsTypes::ListeSubventionsParPilotes subventionParPilote = p_db->recupererSubventionsPilotes(p_annee);

    AeroDmsTypes::ListeSubventionsParPilotes subventionParPiloteSansDoublon = QList<AeroDmsTypes::SubventionsParPilote>();
    while (!subventionParPilote.isEmpty())
    {
        AeroDmsTypes::SubventionsParPilote subvention = subventionParPilote.takeFirst();
        bool estTrouve = false;
        int pos = 0;
        while (pos < subventionParPiloteSansDoublon.size() && !estTrouve)
        {
            if (subventionParPiloteSansDoublon.at(pos).idPilote == subvention.idPilote)
            {
                estTrouve = true;
            }
            else
            {
                pos++;
            }
        }
        if (!estTrouve)
        {
            subventionParPiloteSansDoublon.append(subvention);
        }
        else
        {
            subvention = subvention + subventionParPiloteSansDoublon.at(pos);
            subventionParPiloteSansDoublon.replace(pos, subvention);
        }
    }
    
    return subventionParPiloteSansDoublon;
}

# AeroDMS
[![GPLv3 License](https://img.shields.io/badge/License-GPL%20v3-yellow.svg)](https://opensource.org/licenses/)
![GitHub Release](https://img.shields.io/github/v/release/cvermot/AeroDMS)

Logiciel de gestion de subventions d'une section aéronautique d'un CSE.

## Technologies utilisées

- Développé en C++
- Base de données SQLite
- Librairie Qt (6.6.1)
  - GUI (QtGui)
  - Lecture PDF (QtPdf)
  - Écriture PDF à partir d’un template HTML via WebEngine (QtWebEngine)
  - Accès à la BDD SQLite (QtSql)
  - Affichage des statistiques (QtCharts)
- Librairie PoDoFo (0.10.3)

## Mise en place de l’environnement de développement
Le développement est effectué sous Visual Studio 2022 (MSVC). Cette contrainte découle du fait que QtWebEngine n’est pas disponible lorsque Qt est compilé avec MinGw, cela interdit donc la compilation de l’application sous QtCreator sous Windows.

### Installation des outils
Installer Qt, MSVC 2022 et le plugin Qt pour MSVC.

### Installation de librairies
Les librairies sont installées via [vcpkg](https://vcpkg.io/).

#### Installation de vcpkg
L’installation de vcpkg nécessite que git soit installé sur la machine.

Suivre [les étapes d’installation de vcpkg](https://vcpkg.io/en/getting-started).

#### Compilation et installation de PoDoFo
Récupérer et compiler PoDoFo :
```
vcpkg install podofo
```
Après la compilation de PoDoFo et de ses dépendances, installer PoDoFo (nécessite un accès administrateur sur la machine) :
```
vcpkg integrate install
```
Une fois installée, PoDoFo est directement disponible sous MSVC sans manipulations supplémentaires.

## Déploiement

## Configuration, utilisation
Voir le [Wiki](https://github.com/cvermot/AeroDMS/wiki).

## Elements graphiques
Les images du dossier ressources sont des icones "Material Design" issues du site [Pictogrammers.com](https://pictogrammers.com/).

## Licence
Le logiciel est diffusé sur licence GNU GPL v3. Il intègre des portions de code sous licence BSD.



 



 



 



# AeroDMS
[![GitHub License](https://img.shields.io/github/license/cvermot/AeroDMS)](https://www.gnu.org/licenses/gpl-3.0.fr.html#license-text)
[![GitHub Release](https://img.shields.io/github/v/release/cvermot/AeroDMS)](https://github.com/cvermot/AeroDMS/releases)
![GitHub top language](https://img.shields.io/github/languages/top/cvermot/AeroDMS)
![GitHub line of code](https://sloc.xyz/github/cvermot/AeroDMS?category=code)

Logiciel de gestion de subventions d'une section aéronautique d'un CSE.

![Copie d'écran d'AéroDMS, version 3.4](https://raw.githubusercontent.com/wiki/cvermot/AeroDMS/images/AeroDmsFactureOuverteExtrait.png)

## Technologies utilisées

- Développé en C++
- Base de données [SQLite](https://www.sqlite.org/)
- Librairie [Qt](https://www.qt.io/) (6.8.0)
  - GUI (Qt Gui)
  - Lecture PDF (Qt Pdf)
  - Écriture PDF à partir d’un template HTML via WebEngine (Qt Web Engine)
  - Accès à la BDD SQLite (Qt Sql)
  - Affichage des statistiques (Qt Charts)
  - Export des graphiques en SVG (Qt Svg)
  - Impression (Qt Print Support)
  - Chargement de données de sites Web (Qt Network)
  - Gestion multithread (Qt Concurrent)
- Librairie [PoDoFo](https://github.com/podofo/podofo) (0.10.3)
- Librairie Dpapi (Win32/crypt32.lib) pour le chiffrement des mots de passe stockés en local, à défaut de support d'un mécanisme générique par Qt. Si on souhaite compiler pour Mac OS ou Linux, il sera nécessaire d'implémenter les mécanismes spécifiques à ces systèmes d'exploitation.

## Mise en place de l’environnement de développement
Le développement est effectué sous Visual Studio 2022 (MSVC). Cette contrainte découle du fait que QtWebEngine n’est pas disponible lorsque Qt est compilé avec MinGw, cela interdit donc la compilation de l’application avec QtCreator sous Windows.

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
Le déploiement est réalisé sous forme d'archives ZIP.

Visual Studio déploie automatiquement les DLL nécéssaires aux librairies utilisées via vcpkg. Pour les DLL Qt, il est nécessaire de passer par [windeployqt](https://doc.qt.io/Qt-5/windows-deployment.html).

:warning: Il existe actuellement un soucis avec la libraire OpenSSL produite par vcpkg et utilisée par PoDoFo. En effet, cette DLL fait appel à une autre DLL qui est referencée par un chemin absolu ([Issue créée sur le GitHub vcpkg](https://github.com/microsoft/vcpkg/issues/36482)). Il est donc pour le moment nécessaire de déployer la DLL `legacy.dll` au chemin exacte ou se trouve celle-ci sur la machine ayant produit la DLL via vcpkg.

## Configuration, utilisation
Voir le [Wiki](https://github.com/cvermot/AeroDMS/wiki).

## Elements graphiques
Les images du dossier ressources sont des icones "Material Design" issues du site [Pictogrammers.com](https://pictogrammers.com/).

Le splashscreen est généré via IA avec [Microsoft Designer](https://designer.microsoft.com/).

![Splashscreen](https://github.com/cvermot/AeroDMS/blob/main/ressources/splash.png)

## Licence
Le logiciel est diffusé sous licence GNU GPL v3. Il intègre des portions de code sous licence BSD.



 



 



 



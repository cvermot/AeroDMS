/******************************************************************************\
<AeroDms : logiciel de gestion compta section aéronautique>
Copyright (C) 2023-2025 Clément VERMOT-DESROCHES (clement@vermot.net)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
/******************************************************************************/
#include "ZipExtractorWorker.h"

#include <bit7z/bitfileextractor.hpp>
#include <bit7z/bitabstractarchivehandler.hpp>

void ZipExtractorWorker::run()
{
    try // bit7z classes can throw BitException objects
    { 
        using namespace bit7z;

        Bit7zLibrary lib{ "7z.dll" };
        BitFileExtractor extractor{ lib, BitFormat::Zip };

        // Configureration des callback
        extractor.setProgressCallback([this](uint64_t inProgress) {
            return this->traiterProgressionDecompression(inProgress);
            });
        extractor.setTotalCallback([this](uint64_t total) {
            this->recupererTailleATraiter(total);
            });

        // On extrait pour de vrai
        extractor.extract(cheminFichierZip.toStdString(), cheminDossierDeSortie.toStdString());
    }
    catch (const bit7z::BitException& ex) 
    { 
        emit error(ex.what()); 
    }
}

bool ZipExtractorWorker::traiterProgressionDecompression(uint64_t p_nbOctetsDecompresses)
{
    emit progress(p_nbOctetsDecompresses, nbOctetsTotaux);

    if (p_nbOctetsDecompresses >= nbOctetsTotaux)
    {
        emit finished();
    }

    return true;
}

void ZipExtractorWorker::recupererTailleATraiter(uint64_t p_nbTotalOctets)
{
    nbOctetsTotaux = p_nbTotalOctets;
}
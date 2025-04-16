<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

/**
 * Calcule la somme SHA-256 d'un fichier.
 *
 * @param string $filePath Le chemin vers le fichier.
 * @return string|null La somme SHA-256 en hexadécimal, ou null si une erreur se produit.
 */
function calculateSha256Sum(string $filePath): ?string {
    if (!file_exists($filePath) || !is_readable($filePath)) {
        // Retourne null si le fichier n'existe pas ou ne peut pas être lu
        return null;
    }

    // Utilise la fonction hash_file pour calculer le SHA-256
    return hash_file('sha256', $filePath);
}

// Exemple d'utilisation
$filePath = './uploads/bdd/AeroDMS.sqlite';
$sha256Sum = calculateSha256Sum($filePath);

if ($sha256Sum === null) {
    echo "Erreur : Impossible de lire le fichier." . PHP_EOL;
} else {
    echo $sha256Sum;
}
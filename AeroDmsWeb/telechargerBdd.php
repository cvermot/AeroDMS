<?php
// Chemin du fichier SQLite sur le serveur
$filePath = 'uploads/bdd/AeroDMS.sqlite';

// Vérifiez si le fichier existe
if (!file_exists($filePath)) {
    http_response_code(404); // Code HTTP 404 : fichier non trouvé
    echo "Erreur : Fichier introuvable.";
    exit;
}

// Définissez les en-têtes HTTP pour le téléchargement du fichier
header('Content-Description: File Transfer');
header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename="' . basename($filePath) . '"');
header('Expires: 0');
header('Cache-Control: must-revalidate');
header('Pragma: public');
header('Content-Length: ' . filesize($filePath));

// Nettoyez le tampon de sortie (si nécessaire)
ob_clean();
flush();

// Envoyez le fichier au client
readfile($filePath);
exit;
?>
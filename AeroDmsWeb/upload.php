<?php
header('Content-Type: text/plain');

// Débogage : Afficher toutes les données reçues
echo "Contenu brut de la requête POST :\n";
echo file_get_contents('php://input') . "\n\n";

echo "Contenu de \$_FILES :\n";
print_r($_FILES);

echo "Contenu de \$_POST :\n";
print_r($_POST);

if (empty($_FILES)) {
    echo "Erreur : Aucun fichier reçu.";
    exit;
}

// Déplacer le fichier reçu dans le dossier de destination
$target_dir = "uploads/";
$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);

if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
    echo "Le fichier " . htmlspecialchars(basename($_FILES["fileToUpload"]["name"])) . " a été téléchargé avec succès.";
} else {
    echo "Erreur lors du téléchargement du fichier.";
}
?>
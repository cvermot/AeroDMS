<?php
header('Content-Type: text/plain');

// Activer l'affichage des erreurs
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);

// Définir le niveau de rapport d'erreurs à E_ALL (affiche toutes les erreurs)
error_reporting(E_ALL);

// Débogage : Afficher toutes les données reçues
/*echo "Contenu brut de la requête POST :\n";
echo file_get_contents('php://input') . "\n\n";

echo "Contenu de \$_FILES :\n";
print_r($_FILES);

echo "Contenu de \$_POST :\n";
print_r($_POST);

if (empty($_FILES)) {
    echo "Erreur : Aucun fichier reçu.";
    exit;
}*/

function checkMagicNumber($filepath):bool {
    if(!$handle = fopen($filepath, 'r')) 
    {
        echo "Echec fopen";
        return false;
    }
    if(!$readBytes = fread($handle, 16)) 
    {
        echo "Echec fread";
        return false;
    }

    $readBytes = mb_strtoupper(bin2hex($readBytes));

    if($readBytes === "53514C69746520666F726D6174203300") {
        return true;
    }
    return false;
}

if(pathinfo($_FILES["fileToUpload"]["name"], PATHINFO_EXTENSION) != "sqlite")
{
    echo "L'extension de la base de données n'est pas .sqlite";
}
else
{
    if(!checkMagicNumber($_FILES["fileToUpload"]["tmp_name"]))
    {
        echo "La base de données envoyée n'est pas une base SQLite";
    }
    else
    {
        // Déplacer le fichier reçu dans le dossier de destination
        $target_dir = "uploads/bdd/";
        $target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);
        if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
            echo "La base de données a été téléchargée avec succès.";
        } else {
            echo "Erreur lors du téléchargement de la base de données";
        }
    }
    
}
?>

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
    if(!$readBytes = fread($handle, 5)) 
    {
        echo "Echec fread";
        return false;
    }

    $readBytes = mb_strtoupper(bin2hex($readBytes));

    if($readBytes === "255044462D") {
        return true;
    }
    return false;
}

if(pathinfo($_FILES["fileToUpload"]["name"], PATHINFO_EXTENSION) != "pdf")
{
    echo "L'extension du fichier n'est pas .pdf";
}
else
{
    if(!checkMagicNumber($_FILES["fileToUpload"]["tmp_name"]))
    {
        echo "Le fichier envoyé n'est pas un fichier PDF";
    }
    else
    {
        // Déplacer le fichier reçu dans le dossier de destination
        $target_dir = "uploads/";
        $filename = basename($_FILES["fileToUpload"]["name"]);
        $filename = preg_replace('/^factures[_\s-]*/i', '', $filename);
        $target_file = $target_dir . $filename;
        if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
            echo "Le fichier PDF été téléchargé avec succès.";
        } else {
            echo "Erreur lors du téléchargement du ficher PDF";
        }
    }
    
}
?>

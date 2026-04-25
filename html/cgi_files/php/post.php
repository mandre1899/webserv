<?php
echo "=== RAW \$_FILES ===\n";
var_dump($_FILES);

echo "=== RAW \$_POST ===\n";
var_dump($_POST);

// Enable full error reporting for debugging
error_reporting(E_ALL);
ini_set('display_errors', 1);

// Directory where uploaded files will be stored
$uploadDir = __DIR__ . "/uploads/";

// Make sure the directory exists
if (!is_dir($uploadDir)) {
    if (!mkdir($uploadDir, 0777, true)) {
        die("ERROR: Failed to create upload directory: " . htmlspecialchars($uploadDir));
    }
}

// Helper: convert PHP upload error codes to readable messages
function upload_error_message($code) {
    $errors = [
        UPLOAD_ERR_OK         => "UPLOAD_ERR_OK: No error.",
        UPLOAD_ERR_INI_SIZE   => "UPLOAD_ERR_INI_SIZE: File exceeds upload_max_filesize.",
        UPLOAD_ERR_FORM_SIZE  => "UPLOAD_ERR_FORM_SIZE: File exceeds MAX_FILE_SIZE in HTML form.",
        UPLOAD_ERR_PARTIAL    => "UPLOAD_ERR_PARTIAL: File was only partially uploaded.",
        UPLOAD_ERR_NO_FILE    => "UPLOAD_ERR_NO_FILE: No file was uploaded.",
        UPLOAD_ERR_NO_TMP_DIR => "UPLOAD_ERR_NO_TMP_DIR: Missing temporary folder.",
        UPLOAD_ERR_CANT_WRITE => "UPLOAD_ERR_CANT_WRITE: Failed to write file to disk.",
        UPLOAD_ERR_EXTENSION  => "UPLOAD_ERR_EXTENSION: A PHP extension stopped the upload."
    ];
    return $errors[$code] ?? "Unknown upload error code: $code";
}

echo "<pre>"; // nicer formatting

echo "=== DEBUG INFO ===\n";
echo "Request Method: " . $_SERVER['REQUEST_METHOD'] . "\n";
echo "Upload Directory: $uploadDir\n";

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    echo "ERROR: This script only accepts POST requests.\n";
    exit;
}

if (!isset($_FILES['file'])) {
    echo "ERROR: \$_FILES['file'] is not set. Did you name the input field 'file'?\n";
    exit;
}

$file = $_FILES['file'];

echo "\n=== FILE INFO ===\n";
print_r($file);

if ($file['error'] !== UPLOAD_ERR_OK) {
    echo "\n=== UPLOAD ERROR ===\n";
    echo upload_error_message($file['error']) . "\n";
    exit;
}

$filename = basename($file['name']);
$target = $uploadDir . $filename;

echo "\n=== MOVING FILE ===\n";
echo "Source (tmp): " . $file['tmp_name'] . "\n";
echo "Target: $target\n";

if (!is_uploaded_file($file['tmp_name'])) {
    echo "ERROR: The file is not recognized as an uploaded file.\n";
    exit;
}

if (!move_uploaded_file($file['tmp_name'], $target)) {
    echo "ERROR: move_uploaded_file() failed.\n";
    echo "Possible causes:\n";
    echo "- Permissions on upload directory\n";
    echo "- open_basedir restrictions\n";
    echo "- Disk full\n";
    echo "- SELinux/AppArmor restrictions\n";
    exit;
}

echo "\nSUCCESS: File uploaded to: $target\n";
echo "</pre>";
?>

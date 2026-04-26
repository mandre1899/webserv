<?php
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html>
<head>
    <title>Cool Multipart CGI Handler</title>
    <style>
        body {
            background:#111;
            color:#eee;
            font-family:Arial,sans-serif;
            padding:30px;
        }
        .box {
            background:#1e1e1e;
            border-radius:12px;
            padding:15px;
            margin-bottom:15px;
        }
        code { color:#00ff99; }
    </style>
</head>
<body>

<h1>Multipart POST Received </h1>

<?php
foreach ($_POST as $key => $value) {
    echo "<div class='box'>";
    echo "<h2>" . htmlspecialchars($key) . "</h2>";
    echo "<p>Text Field: <code>" . htmlspecialchars($value) . "</code></p>";
    echo "</div>";
}

foreach ($_FILES as $key => $file) {
    echo "<div class='box'>";
    echo "<h2>" . htmlspecialchars($key) . "</h2>";
    echo "<p>Filename: <code>" . htmlspecialchars($file["name"]) . "</code></p>";
    echo "<p>Size: " . $file["size"] . " bytes</p>";
    echo "<p>Temp Path: <code>" . htmlspecialchars($file["tmp_name"]) . "</code></p>";
    echo "</div>";
}
?>

</body>
</html>

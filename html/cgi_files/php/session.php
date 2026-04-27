<?php

// Session starten (muss immer ganz oben stehen, vor HTML!)
session_start();

// Prüfen, ob ein Wert gesetzt werden soll (z. B. durch Formular oder URL)
if (isset($_GET['name'])) {
    $_SESSION['username'] = $_GET['name'];
}

// Session auslesen
$username = $_SESSION['username'] ?? null;
?>

<!DOCTYPE html>
<html>
<head>
    <title>PHP Session Beispiel</title>
</head>
<body>

<h1>PHP Session Demo</h1>

<?php if ($username): ?>
    <p>Hallo, <?php echo htmlspecialchars($username); ?>!</p>
    <p><a href="?logout=1">Logout</a></p>
<?php else: ?>
    <form method="get">
        <label>Name:</label>
        <input type="text" name="name">
        <button type="submit">Speichern</button>
    </form>
<?php endif; ?>

<?php
// Logout: Session zerstören
if (isset($_GET['logout'])) {
    session_unset();      // Variablen löschen
    session_destroy();    // Session komplett zerstören

    // Seite neu laden
    header("Location: session.php");
    exit;
}
?>

</body>
</html>
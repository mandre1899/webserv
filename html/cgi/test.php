<?php
declare(strict_types=1);

header('Content-Type: text/html; charset=UTF-8');

function h(string $value): string
{
    return htmlspecialchars($value, ENT_QUOTES | ENT_SUBSTITUTE, 'UTF-8');
}

function collectEnvironment(): array
{
    $vars = [];

    foreach ($_SERVER as $key => $value) {
        if (is_array($value)) {
            $value = json_encode($value, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES) ?: '[array]';
        } elseif (!is_string($value)) {
            $value = (string)$value;
        }

        $vars[$key] = $value;
    }

    ksort($vars, SORT_STRING);
    return $vars;
}

function classifyKey(string $key): string
{
    if (str_starts_with($key, 'HTTP_')) {
        return 'HTTP Headers';
    }

    if (in_array($key, [
        'REQUEST_METHOD',
        'REQUEST_URI',
        'QUERY_STRING',
        'CONTENT_TYPE',
        'CONTENT_LENGTH',
        'SCRIPT_NAME',
        'SCRIPT_FILENAME',
        'PATH_INFO',
        'PATH_TRANSLATED',
        'REQUEST_SCHEME',
        'DOCUMENT_ROOT',
    ], true)) {
        return 'Request';
    }

    return 'Other';
}

$env = collectEnvironment();
$grouped = [];

foreach ($env as $key => $value) {
    $group = classifyKey($key);
    $grouped[$group][$key] = $value;
}

$preferredOrder = ['Request', 'Connection / Server', 'HTTP Headers', 'Auth', 'Other'];
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Environment</title>
    <style>
        :root {
            --bg: #0b1020;
            --panel: #121a2b;
            --panel-2: #182235;
            --text: #e8eefc;
            --muted: #9fb0d1;
            --accent: #7cc7ff;
            --border: #2a3a57;
            --good: #7ee787;
            --shadow: 0 14px 40px rgba(0, 0, 0, 0.28);
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            font-family: Inter, Arial, Helvetica, sans-serif;
            background:
                radial-gradient(circle at top right, rgba(124, 199, 255, 0.10), transparent 25%),
                radial-gradient(circle at top left, rgba(126, 231, 135, 0.08), transparent 20%),
                var(--bg);
            color: var(--text);
        }

        .wrap {
            width: min(1200px, calc(100% - 32px));
            margin: 32px auto 48px;
        }

        .hero {
            background: rgba(18, 26, 43, 0.92);
            border: 1px solid var(--border);
            border-radius: 22px;
            padding: 28px;
            box-shadow: var(--shadow);
            margin-bottom: 22px;
        }

        .hero h1 {
            margin: 0 0 8px;
            font-size: 2rem;
            letter-spacing: -0.02em;
        }

        .hero p {
            margin: 0;
            color: var(--muted);
            line-height: 1.5;
        }

        .stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(170px, 1fr));
            gap: 14px;
            margin-top: 22px;
        }

        .stat {
            background: var(--panel-2);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 16px 18px;
        }

        .stat .label {
            color: var(--muted);
            font-size: 0.92rem;
            margin-bottom: 6px;
        }

        .stat .value {
            font-size: 1.7rem;
            font-weight: 700;
        }

        .section {
            margin-top: 22px;
            background: rgba(18, 26, 43, 0.94);
            border: 1px solid var(--border);
            border-radius: 20px;
            overflow: hidden;
            box-shadow: var(--shadow);
        }

        .section-header {
            padding: 18px 22px;
            border-bottom: 1px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 12px;
            background: rgba(255,255,255,0.02);
        }

        .section-header h2 {
            margin: 0;
            font-size: 1.1rem;
        }

        .badge {
            color: var(--good);
            background: rgba(126, 231, 135, 0.10);
            border: 1px solid rgba(126, 231, 135, 0.25);
            border-radius: 999px;
            padding: 6px 10px;
            font-size: 0.85rem;
            white-space: nowrap;
        }

        .table-wrap {
            overflow-x: auto;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        th, td {
            text-align: left;
            vertical-align: top;
            padding: 14px 18px;
            border-bottom: 1px solid var(--border);
        }

        th {
            font-size: 0.85rem;
            text-transform: uppercase;
            letter-spacing: 0.04em;
            color: var(--muted);
            background: rgba(255,255,255,0.015);
        }

        td.key {
            width: 280px;
            font-weight: 600;
            color: var(--accent);
            white-space: nowrap;
        }

        td.value {
            width: auto;
        }

        code {
            font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
            font-size: 0.92rem;
            white-space: pre-wrap;
            word-break: break-word;
            color: var(--text);
        }

        .footer {
            margin-top: 22px;
            color: var(--muted);
            font-size: 0.92rem;
            text-align: center;
        }

        @media (max-width: 700px) {
            .hero h1 {
                font-size: 1.6rem;
            }

            td.key {
                width: 180px;
            }

            th, td {
                padding: 12px 14px;
            }
        }
    </style>
</head>
<body>
    <div class="wrap">
        <div class="hero">
            <h1>CGI Environment Variables</h1>
            <p>
                This page shows the environment and request-related variables available to this CGI script.
                Values are HTML-escaped for safe display.
            </p>

            <div class="stats">
                <div class="stat">
                    <div class="label">Total Variables</div>
                    <div class="value"><?= count($env) ?></div>
                </div>
                <div class="stat">
                    <div class="label">Request Method</div>
                    <div class="value"><?= h($_SERVER['REQUEST_METHOD'] ?? '-') ?></div>
                </div>
                <div class="stat">
                    <div class="label">Request URI</div>
                    <div class="value" style="font-size:1rem; word-break:break-word;">
                        <?= h($_SERVER['REQUEST_URI'] ?? '-') ?>
                    </div>
                </div>
            </div>
        </div>

        <?php foreach ($preferredOrder as $groupName): ?>
            <?php if (empty($grouped[$groupName])) continue; ?>
            <section class="section">
                <div class="section-header">
                    <h2><?= h($groupName) ?></h2>
                    <div class="badge"><?= count($grouped[$groupName]) ?> entries</div>
                </div>
                <div class="table-wrap">
                    <table>
                        <thead>
                            <tr>
                                <th style="width: 280px;">Variable</th>
                                <th>Value</th>
                            </tr>
                        </thead>
                        <tbody>
                        <?php foreach ($grouped[$groupName] as $key => $value): ?>
                            <tr>
                                <td class="key"><code><?= h($key) ?></code></td>
                                <td class="value"><code><?= h($value) ?></code></td>
                            </tr>
                        <?php endforeach; ?>
                        </tbody>
                    </table>
                </div>
            </section>
        <?php endforeach; ?>

        <div class="footer">
            Generated by a standalone PHP CGI script.
        </div>
    </div>
</body>
</html>



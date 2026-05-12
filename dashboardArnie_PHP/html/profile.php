<?php
// ============================================================
// ACCOUNT DISABILITATI — decommentare quando si lavora a scuola
// ============================================================
// require_once '../auth.php';

// DATI MOCK
$utente_nome = $utente_nome ?? 'Dev User';
$ruolo = 'Amministratore';
$email_utente = 'dev@smarthive.it';
$data_iscrizione = "12 Maggio 2024";
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Profilo • Smart Hive</title>

    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://unpkg.com/lucide@latest"></script>
    <link rel="stylesheet" href="../css/profile.css">
</head>

<body>
<?php require_once '../includes/navbar.php'; ?>

<div class="container py-5">
    <div class="mb-5">
        <h1 class="hero-title mb-2">Account Center</h1>
        <p class="text-muted">Gestisci sicurezza, notifiche e accessi del tuo account</p>
    </div>

    <div class="row g-4 justify-content-center">
        <div class="col-lg-4">
            <div class="glass-panel text-center h-100">
                <div class="profile-avatar">
                    <?= strtoupper(substr($utente_nome, 0, 1)) ?>
                </div>
                <h2 class="h4 fw-bold mb-2"><?= htmlspecialchars($utente_nome) ?></h2>
                <div class="role-badge mb-4">
                    <i data-lucide="shield-check" size="16"></i> <?= $ruolo ?>
                </div>

                <div class="text-start mt-4">
                    <div class="info-box">
                        <div class="info-label">Email</div>
                        <div class="info-value"><?= $email_utente ?></div>
                    </div>
                    <div class="info-box">
                        <div class="info-label">Iscrizione</div>
                        <div class="info-value"><?= $data_iscrizione ?></div>
                    </div>

                    <div class="mt-4">
                        <label class="info-label d-block mb-2">Permessi attivi</label>
                        <ul class="permissions-list">
                            <li><i data-lucide="check-circle-2"></i> Lettura sensori</li>
                            <li><i data-lucide="check-circle-2"></i> Gestione allarmi</li>
                            <?php if($ruolo == 'Amministratore'): ?>
                                <li><i data-lucide="check-circle-2"></i> Gestione utenti</li>
                            <?php endif; ?>
                        </ul>
                    </div>
                </div>

                <button class="btn btn-logout w-100 mt-4">
                    <i data-lucide="log-out" class="me-2"></i> Disconnetti
                </button>
            </div>
        </div>

        <div class="col-lg-8">
            <div class="glass-panel mb-4">
                <div class="d-flex align-items-center gap-3 mb-4">
                    <div class="mini-badge p-2"><i data-lucide="lock"></i></div>
                    <div>
                        <h3 class="h5 mb-0 fw-bold">Sicurezza Account</h3>
                        <p class="text-muted small mb-0">Aggiorna le tue credenziali</p>
                    </div>
                </div>

                <form action="#" method="POST">
                    <div class="mb-4">
                        <label class="form-label small text-muted">Password Attuale</label>
                        <input type="password" class="form-control" placeholder="*********">
                    </div>
                    <div class="row g-3 mb-4">
                        <div class="col-md-6">
                            <label class="form-label small text-muted">Nuova Password</label>
                            <input type="password" class="form-control" placeholder="Minimo 8 caratteri">
                        </div>
                        <div class="col-md-6">
                            <label class="form-label small text-muted">Conferma Password</label>
                            <input type="password" class="form-control" placeholder="Ripeti password">
                        </div>
                    </div>
                    <button type="submit" class="btn btn-honey">
                        Aggiorna Password
                    </button>
                </form>
            </div>

            <div class="glass-panel">
                <div class="d-flex align-items-center gap-3 mb-4">
                    <div class="mini-badge p-2"><i data-lucide="bell"></i></div>
                    <div>
                        <h3 class="h5 mb-0 fw-bold">Preferenze Notifiche</h3>
                        <p class="text-muted small mb-0">Gestisci i canali di avviso</p>
                    </div>
                </div>

                <div class="settings-group">
                    <div class="setting-item">
                        <div>
                            <h6 class="mb-1">Avvisi Email Critici</h6>
                            <p class="text-muted small mb-0">Ricevi notifiche immediate via email</p>
                        </div>
                        <div class="form-check form-switch">
                            <input class="form-check-input" type="checkbox" checked>
                        </div>
                    </div>
                    <div class="setting-item">
                        <div>
                            <h6 class="mb-1">Monitoraggio Sicurezza</h6>
                            <p class="text-muted small mb-0">Avvisi login e accessi sospetti</p>
                        </div>
                        <div class="form-check form-switch">
                            <input class="form-check-input" type="checkbox" checked>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

<script>
    lucide.createIcons();
</script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
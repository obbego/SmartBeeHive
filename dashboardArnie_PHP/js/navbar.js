// ─── SIDEBAR TOGGLE ────────────────────────────────────────
const sidebar        = document.getElementById('mainSidebar');
const overlay        = document.getElementById('sidebarOverlay');
const hamburgerBtn   = document.getElementById('hamburgerBtn');
const closeBtn       = document.getElementById('sidebarClose');

function openSidebar() {
    sidebar.classList.add('open');
    overlay.classList.add('active');
    document.body.style.overflow = 'hidden';
}

function closeSidebar() {
    sidebar.classList.remove('open');
    overlay.classList.remove('active');
    document.body.style.overflow = '';
}

if (hamburgerBtn) hamburgerBtn.addEventListener('click', openSidebar);
if (closeBtn)     closeBtn.addEventListener('click', closeSidebar);
if (overlay)      overlay.addEventListener('click', closeSidebar);

// Chiudi con ESC
document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') closeSidebar();
});

// Evidenzia voce attiva
const currentFile = window.location.pathname.split('/').pop().split('?')[0];
const currentParams = new URLSearchParams(window.location.search);
const currentId = currentParams.get('id');

document.querySelectorAll('.sidebar-link[data-page]').forEach(link => {
    const linkPage = link.getAttribute('data-page');
    const linkHref = link.getAttribute('href');
    const linkParams = new URLSearchParams(linkHref.includes('?') ? linkHref.split('?')[1] : '');
    const linkId = linkParams.get('id');

    if (linkPage === currentFile) {
        // Se il link ha un id (es. arnie), controlla che combaci
        if (linkId !== null) {
            if (linkId === currentId) link.classList.add('active');
        } else {
            // Link senza id (es. Dashboard, Allarmi) — basta il nome file
            link.classList.add('active');
        }
    }
});

// ─── POLLING STATUS DOT ARNIE (ogni 30s) ───────────────────
const API_URL_NAVBAR = '../api.php';

function statusToClass(status) {
    if (status === 'green')  return 'green';
    if (status === 'yellow') return 'yellow';
    if (status === 'red')    return 'red';
    return 'offline';
}

async function updateHivesDots() {
    try {
        const res  = await fetch(API_URL_NAVBAR);
        const data = await res.json();

        // hivesData è definito in dati.js, già caricato nella pagina
        if (typeof hivesData === 'undefined') return;

        hivesData.forEach(hive => {
            const telemetry = data[hive.id];
            if (!telemetry) return;

            const temp   = telemetry.tempIn   ? telemetry.tempIn[0].value   : 0;
            const hum    = telemetry.humidity  ? telemetry.humidity[0].value  : 0;
            const weight = telemetry.weight    ? telemetry.weight[0].value    : 0;

            let status = 'offline';
            if (temp == 0 && hum == 0 && weight == 0) {
                status = 'yellow';
            } else if (temp > 40 || temp < -5) {
                status = 'red';
            } else {
                status = 'green';
            }

            const dot = document.getElementById('dot-hive-' + hive.id);
            if (dot) {
                dot.className = 'sidebar-dot ' + statusToClass(status);
            }
        });
    } catch (err) {
        // Silenzioso — se l'API non risponde i dot restano com'erano
        console.warn('Navbar: aggiornamento status dot fallito', err);
    }
}

// Avvia subito e poi ogni 30 secondi
// Solo se siamo in modalità reale (non demo)
document.addEventListener('DOMContentLoaded', () => {
    const isMockMode = localStorage.getItem('mockMode') === 'true';
    if (!isMockMode) {
        updateHivesDots();
        setInterval(updateHivesDots, 30000);
    }
});

// js/alarm_state.js
// ─────────────────────────────────────────────────────────────────
// FONTE UNICA DI VERITÀ per lo stato degli allarmi.
// Includi questo file PRIMA di index.js, allarmi.js e arnie.js.
// ─────────────────────────────────────────────────────────────────

/**
 * Mappa lo status grezzo di ThingsBoard nello stato UI.
 *  ACTIVE_UNACK  → 'system'   (Da gestire)
 *  ACTIVE_ACK    → 'open'     (Aperto / preso in carico)
 *  CLEARED_UNACK → 'closed'   (Risolto)
 *  CLEARED_ACK   → 'closed'   (Risolto)
 */
function tbStatusToUI(tbStatus) {
    if (!tbStatus) return 'system';
    switch (tbStatus) {
        case 'ACTIVE_ACK':    return 'open';
        case 'CLEARED_UNACK':
        case 'CLEARED_ACK':   return 'closed';
        default:              return 'system'; // ACTIVE_UNACK e qualunque altro
    }
}

/**
 * Restituisce lo stato effettivo di un allarme:
 * prima controlla localStorage (override manuale), poi ricade sul
 * valore derivato dallo status TB.
 *
 * @param {string} alarmId   - ID univoco dell'allarme
 * @param {string} tbStatus  - Status grezzo proveniente da ThingsBoard
 * @returns {'system'|'open'|'closed'}
 */
function getEffectiveAlarmStatus(alarmId, tbStatus) {
    try {
        const states = JSON.parse(localStorage.getItem('alarmStates') || '{}');
        if (states[alarmId] !== undefined) return states[alarmId];
    } catch (_) {}
    return tbStatusToUI(tbStatus);
}

function loadLocalAlarmStates() {
    try { return JSON.parse(localStorage.getItem('alarmStates') || '{}'); } catch (_) { return {}; }
}

function saveLocalAlarmStates(states) {
    localStorage.setItem('alarmStates', JSON.stringify(states));
}
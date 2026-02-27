#include "fragmenter.h"

// --- FragmentConstructor ---

FragmentConstructor::FragmentConstructor(str data, const int _max_size = 256) {
    _data = data;
    _cursor = 0;
    _current_index = 0;
    FRAG_SIZE = _max_size;
    
    int len = _data.length();
    if (len == 0) {
        _total_fragments = 0;
    } else {
        // Calcolo dei frammenti necessari
        _total_fragments = (len + FRAG_SIZE - 1) / FRAG_SIZE;
    }
}

int FragmentConstructor::next_fragment(str* output) {
    if (_current_index >= _total_fragments || _total_fragments == 0) {
        return -1; 
    }

    int end = _cursor + FRAG_SIZE;
    if (end > _data.length()) end = _data.length();
    
    str payload = _data.substring(_cursor, end);

    // Costruzione pacchetto: "indice|totale|payload"
    // Assumiamo che str(int) o l'operatore + gestisca la conversione
    *output = str(_current_index) + "|" + str(_total_fragments) + "|" + payload;

    _cursor = end;
    _current_index++;

    // Ritorna quanti frammenti rimangono da inviare
    return _total_fragments - _current_index;
}

// --- FragmentDestructor ---

FragmentDestructor::FragmentDestructor() {
    _expected_total = -1;
    _next_expected_index = 0;
    _buffer = "";
}

int FragmentDestructor::add_fragment(str data) {
    int firstPipe = data.indexOf('|');
    int secondPipe = data.indexOf('|', firstPipe + 1);

    // 1. Controllo integrità header (presenza dei delimitatori)
    if (firstPipe <= 0 || secondPipe <= firstPipe + 1) {
        return -2; // Errore: Header malformato
    }

    int index = data.substring(0, firstPipe).toInt();
    int total = data.substring(firstPipe + 1, secondPipe).toInt();
    str payload = data.substring(secondPipe + 1);

    // 2. Controllo errori di conversione toInt()
    if (index == -1 || total == -1) {
        return -3; // Errore: Indici non numerici
    }

    // 3. Gestione nuovo messaggio o reset forzato
    // Se arriva l'indice 0, resettiamo sempre il buffer (nuova trasmissione)
    if (index == 0) {
        _buffer = "";
        _next_expected_index = 0;
        _expected_total = total;
    } 

    // 4. Controllo Sincronizzazione
    // Se non abbiamo ancora un 'total' valido o l'indice non è quello che aspettiamo
    if (_expected_total == -1 || index != _next_expected_index) {
        return -4; // Errore: Frammento fuori sequenza o orfano
    }

    // 5. Controllo consistenza 'total'
    // Se a metà opera il mittente cambia idea sul totale, c'è un problema grave
    if (total != _expected_total) {
        _expected_total = -1; // Invalida lo stato
        return -5; 
    }

    // 6. Accettazione del payload
    _buffer += payload;
    _next_expected_index++;

    // Controllo completamento
    if (_next_expected_index == _expected_total) {
        return 0; // Messaggio pronto
    }

    // Ritorna il numero di frammenti mancanti
    return _expected_total - _next_expected_index;
}

str FragmentDestructor::get_message() {
    str result = _buffer;
    
    // Reset completo dello stato interno
    _buffer = "";
    _expected_total = -1;
    _next_expected_index = 0;
    
    return result;
}
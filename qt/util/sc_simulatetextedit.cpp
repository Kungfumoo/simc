// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "sc_simulatetextedit.hpp"

// ============================================================================
// SC_SimulateTextEdit
// ============================================================================

// protected virtual overrides

void SC_SimulateTextEdit::insertFromMimeData( const QMimeData *source )
{
    //TODO: Look for #APL_PROFILE in the string and populate the apl under it

    SC_TextEdit::insertFromMimeData( source );
}

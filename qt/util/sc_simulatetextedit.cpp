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
    //Look for the apl heading in the string and populate the apl under it
    if ( source->hasText() )
    {
        QString apl = fetchApl();
        QString text = source->text();

        if ( text.indexOf( aplHeading ) != -1 )
        {
            QMimeData *data = new QMimeData();
            text.replace( aplHeading, apl );
            data->setText( text );

            SC_TextEdit::insertFromMimeData(data);

            delete data;
            return;
        }
    }

    SC_TextEdit::insertFromMimeData(source);
}

// private
QString SC_SimulateTextEdit::fetchApl()
{
    return QString("I'm an apl");
}

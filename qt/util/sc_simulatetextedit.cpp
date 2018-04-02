// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "sc_simulatetextedit.hpp"
#include "simulationcraft.hpp"

// ============================================================================
// SC_SimulateTextEdit
// ============================================================================

// protected virtual overrides

void SC_SimulateTextEdit::insertFromMimeData( const QMimeData *source )
{
    //Look for the apl heading in the string and populate the apl under it
    if ( source->hasText() )
    {
        QString text = source->text();

        if ( text.indexOf( aplHeading ) != -1 )
        {
            QString apl = fetchApl( text );
            QMimeData *data = new QMimeData();

            text.replace( aplHeading, apl );
            data->setText( text );

            SC_TextEdit::insertFromMimeData( data );

            delete data;
            return;
        }
    }

    SC_TextEdit::insertFromMimeData( source );
}

// private

QString SC_SimulateTextEdit::fetchApl( QString &text )
{
    const module_t *module = module_t::get( "SHAMAN" );
    sim_t *sim = new sim_t();
    player_t *player = module->create_player( sim, "Player" );

    QString actionListStr = "";

    if ( player->init_actions() )
    {
        actionListStr = QString::fromStdString( player->create_apl_str() );
    }

    delete player;

    return actionListStr;
}

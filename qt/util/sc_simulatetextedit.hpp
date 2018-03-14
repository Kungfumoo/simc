// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#pragma once

#include "config.hpp"

#include "sc_textedit.hpp"

class QMimeData;

// ============================================================================
// SC_SimulateTextEdit
// ============================================================================

class SC_SimulateTextEdit : public SC_TextEdit
{
  protected:
    void insertFromMimeData( const QMimeData *source ) override;
};

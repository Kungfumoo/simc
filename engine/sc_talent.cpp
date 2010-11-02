// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

#if 0
static talent_tab_name tree_to_tab_name( const player_type c, const talent_tree_type tree )
{
  switch ( tree )
  {
  case TREE_BLOOD: return DEATH_KNIGHT_BLOOD;
  case TREE_FROST: return c == MAGE ? MAGE_FROST : DEATH_KNIGHT_FROST;
  case TREE_UNHOLY: return DEATH_KNIGHT_UNHOLY;
  case TREE_BALANCE: return DRUID_BALANCE;
  case TREE_FERAL: return DRUID_FERAL;
  case TREE_RESTORATION: return c == DRUID ? DRUID_RESTORATION : SHAMAN_RESTORATION;
  case TREE_BEAST_MASTERY: return HUNTER_BEAST_MASTERY;
  case TREE_MARKSMANSHIP: return HUNTER_MARKSMANSHIP;
  case TREE_SURVIVAL: return HUNTER_SURVIVAL;
  case TREE_ARCANE: return MAGE_ARCANE;
  case TREE_FIRE: return MAGE_FIRE;
  case TREE_HOLY: return c == PALADIN ? PALADIN_HOLY : PRIEST_HOLY;
  case TREE_PROTECTION: return c == PALADIN? PALADIN_PROTECTION : WARRIOR_PROTECTION;
  case TREE_RETRIBUTION: return PALADIN_RETRIBUTION;
  case TREE_DISCIPLINE: return PRIEST_DISCIPLINE;
  case TREE_SHADOW: return PRIEST_SHADOW;
  case TREE_ASSASSINATION: return ROGUE_ASSASSINATION;
  case TREE_COMBAT: return ROGUE_COMBAT;
  case TREE_SUBTLETY: return ROGUE_SUBTLETY;
  case TREE_ELEMENTAL: return SHAMAN_ELEMENTAL;
  case TREE_ENHANCEMENT: return SHAMAN_ENHANCEMENT;
  case TREE_AFFLICTION: return WARLOCK_AFFLICTION;
  case TREE_DEMONOLOGY: return WARLOCK_DEMONOLOGY;
  case TREE_DESTRUCTION: return WARLOCK_DESTRUCTION;
  case TREE_ARMS: return WARRIOR_ARMS;
  case TREE_FURY: return WARRIOR_FURY;
  default: return WARRIOR_ARMS;
  }
}
#endif

// ==========================================================================
// Talent
// ==========================================================================

// talent_t::talent_t =======================================================

talent_t::talent_t( player_t* player, const char* t_name, const char* name ) : 
  spell_id_t( player, t_name, name ), t_data( 0 ), t_rank( 0 ), t_overridden( false )
{
  assert( s_player && name && s_player -> sim );

  uint32_t id = find_talent_id( name );
  
  if ( id != 0 )
  {
    t_data = s_player->player_data.m_talents_index[ id ];
    t_enabled = s_player -> player_data.talent_is_enabled( t_data -> id );

    s_player -> talent_list2.push_back( const_cast<talent_t*>( this ) );
    s_player -> player_data.talent_set_used( id, true );

    check_unknown_options( OPT_TALENT_RANK_FORCED );
  }
}

talent_t::talent_t( const talent_t& copy ) :
  spell_id_t( copy ), t_data( copy.t_data ), t_rank( copy.t_rank ),
  t_enabled( copy.t_enabled ), t_overridden( copy.t_overridden )
{
// Not sure if I should push back or not yet.
/*
  s_player -> talent_list2.push_back( const_cast<talent_t *>( this ) );
*/
}

bool talent_t::ok() SC_CONST
{
  if ( ! s_player || ! t_data )
    return false;

  return ( ( t_rank > 0 ) && spell_id_t::ok() && ( t_enabled ) );
}

std::string talent_t::to_str() SC_CONST
{
  std::ostringstream s;
  
  s << spell_id_t::to_str();
  s << " talent_enabled=" << ( t_enabled ? "true" : "false" );
  if ( t_overridden ) s << " (forced)";
  s << " talent_rank=" << t_rank;
  
  return s.str();
}

// talent_t::get_spell_id ===================================================

uint32_t talent_t::spell_id( ) SC_CONST
{
  assert( s_player -> sim && ( t_rank <= 3 ) );
  
  if ( ! ok() )
    return 0;

  return s_player -> player_data.talent_rank_spell_id( t_data -> id, t_rank );
}

bool talent_t::set_rank( uint32_t value, bool overridden )
{
  if ( ! t_data || ! t_enabled )
  {
    if ( s_player -> sim -> debug ) 
      log_t::output( s_player -> sim, "Talent status: %s", to_str().c_str() );
    return false;
  }

  if ( value > s_player -> player_data.talent_max_rank( t_data -> id ) )
  {
    if ( s_player -> sim -> debug ) 
      log_t::output( s_player -> sim, "Talent status: %s", to_str().c_str() );
    return false;
  }
  
  // We cannot allow non-overridden set_rank to take effect, if 
  // we have already overridden the talent rank once
  if ( ! t_overridden || overridden )
  {
    t_overridden = overridden;
    t_rank       = value;
    s_id         = rank_spell_id( t_rank );

    if ( t_enabled && t_rank > 0 && ! initialize() )
    {
      if ( s_player -> sim -> debug ) 
        log_t::output( s_player -> sim, "Talent status: %s", to_str().c_str() );
      return false;
    }
  }
   
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Talent status: %s", to_str().c_str() );
  return true;
}

// talent_t::find_talent_id ===================================================

uint32_t talent_t::find_talent_id( const char* name )
{
  uint32_t i_tab, talent_num, talent_id;

  assert( s_player && name && name[ 0 ] );

  for ( i_tab = 0; i_tab < MAX_TALENT_TABS; i_tab++ )
  {
    talent_num = 0;
    while ( ( talent_id = s_player->player_data.talent_player_get_id_by_num( s_player -> type, i_tab, talent_num ) ) != 0 )
    {
      if ( !_stricmp( s_player -> player_data.talent_name_str( talent_id ), name ) )
      {
        return talent_id;
      }
      talent_num++;
    }
  }
  
  return 0; 
}

uint32_t talent_t::max_rank() SC_CONST
{
  if ( ! s_player || ! t_data || ! t_data->id )
    return 0;

  return s_player -> player_data.talent_max_rank( t_data -> id );
}

uint32_t talent_t::rank_spell_id( const uint32_t r ) SC_CONST
{
  if ( ! s_player || ! t_data || ! t_data->id )
    return 0;

  return s_player -> player_data.talent_rank_spell_id( t_data -> id, r );
}

uint32_t talent_t::rank() SC_CONST
{
  if ( ! ok() )
    return 0;

  return t_rank;
}

// ==========================================================================
// Spell ID
// ==========================================================================

// spell_id_t::spell_id_t =======================================================

spell_id_t::spell_id_t( player_t* player, const char* t_name ) :
    s_type( T_SPELL ), s_id( 0 ), s_data( 0 ), s_enabled( false ), s_player( player ),
    s_overridden( false ), s_required_talent( 0 ), s_single( 0 ), s_tree( -1 )
{
  if ( ! t_name )
    s_token = "";
  else
    s_token = t_name;
  
  // Dummy constructor for old-style
  memset( s_effects, 0, sizeof( s_effects ) );
}

spell_id_t::spell_id_t( player_t* player, const char* t_name, const uint32_t id, talent_t* talent ) :
    s_type( T_SPELL ), s_id( id ), s_data( 0 ), s_enabled( false ), s_player( player ),
    s_overridden( false ), s_token( t_name ), s_required_talent( talent ), s_single( 0 ), s_tree( -1 )
{
  initialize();
}

spell_id_t::spell_id_t( player_t* player, const char* t_name, const char* s_name, talent_t* talent ) :
    s_type( T_SPELL ), s_id( 0 ), s_data( 0 ), s_enabled( false ), s_player( player ), 
    s_overridden( false ), s_token( t_name ), s_required_talent( talent ), s_single( 0 ), s_tree( -1 )
{
  initialize( s_name );
}

spell_id_t::spell_id_t( const spell_id_t& copy ) :
    s_type( copy.s_type ), s_id( copy.s_id ), s_data( copy.s_data ), s_enabled( copy.s_enabled ),
    s_player( copy.s_player ), s_overridden( copy.s_overridden ),
    s_token( copy.s_token ), s_required_talent( copy.s_required_talent ), s_single( copy.s_single ),
    s_tree( copy.s_tree )
{
  memcpy( s_effects, copy.s_effects, sizeof( s_effects ) );
}

bool spell_id_t::initialize( const char* s_name )
{
  player_type player_class;
  uint32_t n_effects       = 0;

  assert( s_player && s_player -> sim );

  memset( s_effects, 0, sizeof(s_effects) );

  player_class = s_player -> type;
  
  // For pets, find stuff based on owner class, as that's how our spell lists
  // are structured
  if ( s_player -> is_pet() )
  {
    const pet_t* pet = s_player -> cast_pet();
    player_class = pet -> owner -> type;
  }

  // Search using spell name to find the spell type
  if ( ! s_id )
  {
    if ( ! s_name || ! *s_name )
      return false;

    if ( ( s_id = s_player -> player_data.find_mastery_spell( player_class, s_name ) ) )
      s_type      = T_MASTERY;
    else if ( ! s_id && ( s_id = s_player -> player_data.find_talent_spec_spell( player_class, s_name ) ) )
      s_type      = T_SPEC;
    else if ( ! s_id && ( s_id = s_player -> player_data.find_class_spell( player_class, s_name ) ) )
      s_type      = T_CLASS;
    else if ( ! s_id && ( s_id = s_player -> player_data.find_racial_spell( player_class, s_player -> race, s_name ) ) )
      s_type      = T_RACE;
    else if ( ! s_id && ( s_id = s_player -> player_data.find_glyph_spell( player_class, s_name ) ) )
      s_type      = T_GLYPH;
    else if ( ! s_id && ( s_id = s_player -> player_data.find_set_bonus_spell( player_class, s_name ) ) )
      s_type      = T_ITEM;
  }
  // Search using spell id to find the spell type
  else
  {
    if ( s_player -> player_data.is_mastery_spell( s_id ) )
      s_type      = T_MASTERY;
    else if ( s_player -> player_data.is_talent_spec_spell( s_id ) )
      s_type      = T_SPEC;
    else if ( s_player -> player_data.is_class_spell( s_id ) )
      s_type      = T_CLASS;
    else if ( s_player -> player_data.is_racial_spell( s_id ) )
      s_type      = T_RACE;
    else if ( s_player -> player_data.is_glyph_spell( s_id ) )
      s_type      = T_GLYPH;
    else if ( s_player -> player_data.is_set_bonus_spell( s_id ) )
      s_type      = T_ITEM;
  }
  
  // At this point, our spell must exist or we are in trouble
  if ( ! s_id || ! s_player -> player_data.spell_exists( s_id ) )
    return false;
  
  // Do second phase of spell initialization
  s_data = s_player -> player_data.m_spells_index[ s_id ];

  s_player -> player_data.spell_set_used( s_id, true );

  // Some spells, namely specialization and class spells
  // can specify a tree for the spell
  switch ( s_type )
  {
    case T_SPEC:
      s_tree = s_player -> player_data.find_talent_spec_spell_tree( player_class, s_id );
      break;
    case T_CLASS:
      s_tree = s_player -> player_data.find_class_spell_tree( player_class, s_id );
      break;
    default:
      s_tree = -1;
      break;
  }

  s_enabled = s_player -> player_data.spell_is_enabled( s_id ) & 
              s_player -> player_data.spell_is_level( s_id, s_player -> level );
   
  // Warn if the player is enabling a spell that the player has no level for
  /*
  if ( ! s_player -> player_data.spell_is_level( s_id, s_player -> level ) )
  {
    s_player -> sim -> errorf( "Warning: Player %s level (%d) too low for spell %s, requires level %d",
      s_player -> name_str.c_str(),
      s_player -> level,
      s_data -> name,
      s_data -> spell_level );
  }
  */
  if ( s_type == T_MASTERY )
  {
    if ( s_player -> level < 75 )
      s_enabled = false;
  }

  // Map s_effects, figure out if this is a s_single-effect spell
  for ( int i = 0; i < MAX_EFFECTS; i++ )
  {
    if ( ! s_data -> effect[ i ] )
      continue;
      
    if ( ! s_player -> player_data.m_effects_index[ s_data -> effect[ i ] ] )
      continue;
    
    s_effects[ i ] = s_player -> player_data.m_effects_index[ s_data -> effect[ i ] ];
    n_effects++;
  }
  
  if ( n_effects == 1 )
  {
    for ( int i = 0; i < MAX_EFFECTS; i++ )
    {
      if ( ! s_effects[ i ] )
        continue;
        
      s_single = s_effects[ i ];
      break;
    }
  }
  else
    s_single = 0;

  return true;  
}


bool spell_id_t::enable( bool override_value )
{
  assert( s_player && s_player -> sim );

  s_overridden = true;
  s_enabled    = override_value;
  
  return true;
}

bool spell_id_t::ok() SC_CONST
{
  bool res = s_enabled;

  if ( ! s_player || ! s_data || ! s_id )
    return false;

  if ( s_required_talent )
    res = res & s_required_talent -> ok();

  if ( s_type == T_SPEC )
    res = res & ( s_tree == s_player -> pri_tree );

  return res;
}

std::string spell_id_t::to_str() SC_CONST
{
  std::ostringstream s;
  
  s << "enabled=" << ( s_enabled ? "true" : "false" );
  s << " (ok=" << ( ok() ? "true" : "false" ) << ")";
  if ( s_overridden ) s << " (forced)";
  s << " token=" << s_token;
  s << " type=" << s_type;
  s << " tree=" << s_tree;
  s << " id=" << s_id;
  s << " player=" << s_player -> name_str;
  if ( s_required_talent )
    s << " req_talent=" << s_required_talent -> s_token;
  
  return s.str();
}


bool spell_id_t::check_unknown_options( int opt_type )
{
  nvpair_t* t = 0;

  assert( s_player && s_player -> sim );

  for ( int32_t i = s_player -> unknown_options.size() - 1; i >= 0; i-- )
  {
    t = s_player -> unknown_options[ i ];
    
    if ( t -> name == s_token )
    {
      option_t opt[ 2 ];
      std::vector<option_t> opt_vector;

      opt[ 1 ].name = 0;
      opt[ 1 ].type = OPT_UNKNOWN;
      opt[ 1 ].address = 0;
      opt[ 0 ].name = s_token.c_str();
      opt[ 0 ].type = opt_type;
      opt[ 0 ].address = this;
      option_t::copy( opt_vector, opt );

      if ( ! option_t::parse( s_player -> sim, opt_vector, t -> name, t -> value ) )
        return false;

      return true;
    }
  }

  return false;
}

const char* spell_id_t::real_name() SC_CONST
{
  if ( ! s_player || ! s_data || ! s_id )
    return 0;

  return s_player -> player_data.spell_name_str( s_id );
}

const std::string spell_id_t::token() SC_CONST
{
  if ( ! s_player || ! s_data || ! s_id )
    return 0;

  return s_token;
}

double spell_id_t::missile_speed() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_missile_speed( s_id );
}

uint32_t spell_id_t::school_mask() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_school_mask( s_id );
}

uint32_t spell_id_t::get_school_mask( const school_type s )
{
  switch ( s )
  {
  case SCHOOL_PHYSICAL      : return 0x01;
  case SCHOOL_HOLY          : return 0x02;
  case SCHOOL_FIRE          : return 0x04;
  case SCHOOL_NATURE        : return 0x08;
  case SCHOOL_FROST         : return 0x10;
  case SCHOOL_SHADOW        : return 0x20;
  case SCHOOL_ARCANE        : return 0x40;
  case SCHOOL_HOLYSTRIKE    : return 0x03;
  case SCHOOL_FLAMESTRIKE   : return 0x05;
  case SCHOOL_HOLYFIRE      : return 0x06;
  case SCHOOL_STORMSTRIKE   : return 0x09;
  case SCHOOL_HOLYSTORM     : return 0x0a;
  case SCHOOL_FIRESTORM     : return 0x0c;
  case SCHOOL_FROSTSTRIKE   : return 0x11;
  case SCHOOL_HOLYFROST     : return 0x12;
  case SCHOOL_FROSTFIRE     : return 0x14;
  case SCHOOL_FROSTSTORM    : return 0x18;
  case SCHOOL_SHADOWSTRIKE  : return 0x21;
  case SCHOOL_SHADOWLIGHT   : return 0x22;
  case SCHOOL_SHADOWFLAME   : return 0x24;
  case SCHOOL_SHADOWSTORM   : return 0x28;
  case SCHOOL_SHADOWFROST   : return 0x30;
  case SCHOOL_SPELLSTRIKE   : return 0x41;
  case SCHOOL_DIVINE        : return 0x42;
  case SCHOOL_SPELLFIRE     : return 0x44;
  case SCHOOL_SPELLSTORM    : return 0x48;
  case SCHOOL_SPELLFROST    : return 0x50;
  case SCHOOL_SPELLSHADOW   : return 0x60;
  case SCHOOL_ELEMENTAL     : return 0x1c;
  case SCHOOL_CHROMATIC     : return 0x7c;
  case SCHOOL_MAGIC         : return 0x7e;
  case SCHOOL_CHAOS         : return 0x7f;
  default:
    return SCHOOL_NONE;
  }
  return 0x00;
}

bool spell_id_t::is_school( const school_type s, const school_type s2 )
{
  return ( get_school_mask( s ) & get_school_mask( s2 ) ) != 0;
}

school_type spell_id_t::get_school_type( const uint32_t mask )
{
  switch ( mask )
  {
  case 0x01: return SCHOOL_PHYSICAL;
  case 0x02: return SCHOOL_HOLY;
  case 0x04: return SCHOOL_FIRE;
  case 0x08: return SCHOOL_NATURE;
  case 0x10: return SCHOOL_FROST;
  case 0x20: return SCHOOL_SHADOW;
  case 0x40: return SCHOOL_ARCANE;
  case 0x03: return SCHOOL_HOLYSTRIKE;
  case 0x05: return SCHOOL_FLAMESTRIKE;
  case 0x06: return SCHOOL_HOLYFIRE;
  case 0x09: return SCHOOL_STORMSTRIKE;
  case 0x0a: return SCHOOL_HOLYSTORM;
  case 0x0c: return SCHOOL_FIRESTORM;
  case 0x11: return SCHOOL_FROSTSTRIKE;
  case 0x12: return SCHOOL_HOLYFROST;
  case 0x14: return SCHOOL_FROSTFIRE;
  case 0x18: return SCHOOL_FROSTSTORM;
  case 0x21: return SCHOOL_SHADOWSTRIKE;
  case 0x22: return SCHOOL_SHADOWLIGHT;
  case 0x24: return SCHOOL_SHADOWFLAME;
  case 0x28: return SCHOOL_SHADOWSTORM;
  case 0x30: return SCHOOL_SHADOWFROST;
  case 0x41: return SCHOOL_SPELLSTRIKE;
  case 0x42: return SCHOOL_DIVINE;
  case 0x44: return SCHOOL_SPELLFIRE;
  case 0x48: return SCHOOL_SPELLSTORM;
  case 0x50: return SCHOOL_SPELLFROST;
  case 0x60: return SCHOOL_SPELLSHADOW;
  case 0x1c: return SCHOOL_ELEMENTAL;
  case 0x7c: return SCHOOL_CHROMATIC;
  case 0x7e: return SCHOOL_MAGIC;
  case 0x7f: return SCHOOL_CHAOS;
  default:
    return SCHOOL_NONE;
  }
}

school_type spell_id_t::get_school_type() SC_CONST
{
  if ( ! ok() )
    return SCHOOL_NONE;

  return get_school_type( school_mask() );
}

resource_type spell_id_t::power_type() SC_CONST
{
  if ( ! ok() )
    return RESOURCE_NONE;

  return s_player -> player_data.spell_power_type( s_id );
}

double spell_id_t::min_range() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_min_range( s_id );
}

double spell_id_t::max_range() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_max_range( s_id );
}

bool spell_id_t::in_range() SC_CONST
{
  if ( ! ok() )
    return false;

  return s_player -> player_data.spell_in_range( s_id, s_player -> distance );
}

double spell_id_t::cooldown() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  double d = s_player -> player_data.spell_cooldown( s_id );

  if ( d > ( s_player -> sim -> wheel_seconds - 2.0 ) )
    d = s_player -> sim -> wheel_seconds - 2.0;

  return d;
}

double spell_id_t::gcd() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_gcd( s_id );
}

uint32_t spell_id_t::category() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_category( s_id );
}

double spell_id_t::duration() SC_CONST
{
  if ( ! ok() )
    return 0.0;
  
  double d = s_player -> player_data.spell_duration( s_id );

  if ( d > ( s_player -> sim -> wheel_seconds - 2.0 ) )
    d = s_player -> sim -> wheel_seconds - 2.0;

  return d;
}

double spell_id_t::cost() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_cost( s_id );
}

uint32_t spell_id_t::rune_cost() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_rune_cost( s_id );
}

double spell_id_t::runic_power_gain() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_runic_power_gain( s_id );
}

uint32_t spell_id_t::max_stacks() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_max_stacks( s_id );
}

uint32_t spell_id_t::initial_stacks() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_initial_stacks( s_id );
}

double spell_id_t::proc_chance() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_proc_chance( s_id );
}

double spell_id_t::cast_time() SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.spell_cast_time( s_id, s_player -> level );
}

uint32_t spell_id_t::effect_id( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_effect_id( s_id, effect_num );
}

bool spell_id_t::flags( const spell_attribute_t f ) SC_CONST
{
  if ( ! ok() )
    return false;

  return s_player -> player_data.spell_flags( s_id, f );
}

const char* spell_id_t::desc() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_desc( s_id );
}

const char* spell_id_t::tooltip() SC_CONST
{
  if ( ! ok() )
    return 0;

  return s_player -> player_data.spell_tooltip( s_id );
}

int32_t spell_id_t::effect_type( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_type( effect_id );
}

int32_t spell_id_t::effect_subtype( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_subtype( effect_id );
}

int32_t spell_id_t::effect_base_value( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_base_value( effect_id );
}

int32_t spell_id_t::effect_misc_value1( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_misc_value1( effect_id );
}

int32_t spell_id_t::effect_misc_value2( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_misc_value2( effect_id );
}

uint32_t spell_id_t::effect_trigger_spell( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_trigger_spell_id( effect_id );
}

double spell_id_t::effect_average( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_average( effect_id, 
    s_player -> player_data.spell_scaling_class( s_id ), s_player -> level );
}

double spell_id_t::effect_delta( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_delta( effect_id, 
    s_player -> player_data.spell_scaling_class( s_id ), s_player -> level );
}

double spell_id_t::effect_unk( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_unk( effect_id, 
    s_player -> player_data.spell_scaling_class( s_id ), s_player -> level );
}

double spell_id_t::effect_min( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_min( effect_id, 
    s_player -> player_data.spell_scaling_class( s_id ), s_player -> level );
}

double spell_id_t::effect_max( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_max( effect_id, 
    s_player -> player_data.spell_scaling_class( s_id ), s_player -> level );
}

double spell_id_t::effect_min( effect_type_t type, effect_subtype_t sub_type, int misc_value ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.effect_min( s_id, s_player -> level, type, sub_type, misc_value );
}

double spell_id_t::effect_max( effect_type_t type, effect_subtype_t sub_type, int misc_value ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  return s_player -> player_data.effect_max( s_id, s_player -> level, type, sub_type, misc_value );
}

double spell_id_t::effect_coeff( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_coeff( effect_id );
}

double spell_id_t::effect_period( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_period( effect_id );
}

double spell_id_t::effect_radius( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_radius( effect_id );
}

double spell_id_t::effect_radius_max( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_radius_max( effect_id );
}

double spell_id_t::effect_pp_combo_points( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_pp_combo_points( effect_id );
}

double spell_id_t::effect_real_ppl( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_real_ppl( effect_id );
}

int spell_id_t::effect_die_sides( const uint32_t effect_num ) SC_CONST
{
  if ( ! ok() )
    return 0;

  uint32_t effect_id = s_player -> player_data.spell_effect_id( s_id, effect_num );

  return s_player -> player_data.effect_die_sides( effect_id );
}

double spell_id_t::base_value( effect_type_t type, effect_subtype_t sub_type, int misc_value, int misc_value2 ) SC_CONST
{
  if ( ! ok() )
    return 0.0;

  if ( s_single )
    return sc_data_access_t::fmt_value( s_single -> base_value, s_single -> type, s_single -> subtype );

  for ( int i = 0; i < MAX_EFFECTS; i++ )
  {
    if ( ! s_effects[ i ] )
      continue;

    if ( ( type == E_MAX || s_effects[ i ] -> type == type ) && 
         ( sub_type == A_MAX || s_effects[ i ] -> subtype == sub_type ) && 
         ( misc_value == DEFAULT_MISC_VALUE || s_effects[ i ] -> misc_value == misc_value ) &&
         ( misc_value2 == DEFAULT_MISC_VALUE || s_effects[ i ] -> misc_value_2 == misc_value2 ) )
      return sc_data_access_t::fmt_value( s_effects[ i ] -> base_value, type, sub_type );
  }
  
  return 0.0;
}

double spell_id_t::mod_additive( property_type_t p_type ) SC_CONST
{
  // Move this somewhere sane, here for now
  static double property_flat_divisor[] = {
    1.0,    // P_GENERIC
    1000.0, // P_DURATION
    1.0,    // P_THREAT
    1.0,    // P_EFFECT_1
    1.0,    // P_STACK
    1.0,    // P_RANGE
    1.0,    // P_RADIUS
    100.0,  // P_CRIT
    1.0,    // P_UNKNOWN_1
    1.0,    // P_PUSHBACK
    1000.0, // P_CAST_TIME
    1000.0, // P_COOLDOWN
    1.0,    // P_EFFECT_2
    1.0,    // Unused
    1.0,    // P_RESOURCE_COST
    1.0,    // P_CRIT_DAMAGE
    1.0,    // P_PENETRATION
    1.0,    // P_TARGET
    100.0,  // P_PROC_CHANCE
    1000.0, // P_TICK_TIME
    1.0,    // P_TARGET_BONUS
    1000.0, // P_GCD
    1.0,    // P_TICK_DAMAGE
    1.0,    // P_EFFECT_3
    100.0,  // P_SPELL_POWER
    1.0,    // Unused
    1.0,    // P_PROC_FREQUENCY
    1.0,    // P_DAMAGE_TAKEN
    100.0,  // P_DISPEL_CHANCE
  };
  
  if ( ! ok() )
    return 0.0;

  if ( s_single )
  {
    if ( ( p_type == P_MAX ) || ( s_single -> subtype == A_ADD_FLAT_MODIFIER ) || ( s_single -> subtype == A_ADD_PCT_MODIFIER ) )
    {
      if ( s_single -> subtype == (int) A_ADD_PCT_MODIFIER )
        return s_single -> base_value / 100.0;
      // Divide by property_flat_divisor for every A_ADD_FLAT_MODIFIER
      else
        return s_single -> base_value / property_flat_divisor[ s_single -> misc_value ];
    }
    else
      return 0.0;
  }

  for ( int i = 0; i < MAX_EFFECTS; i++ )
  {
    if ( ! s_effects[ i ] )
      continue;

    if ( s_effects[ i ] -> subtype != A_ADD_FLAT_MODIFIER && s_effects[ i ] -> subtype != A_ADD_PCT_MODIFIER )
      continue;

    if ( p_type == P_MAX || s_effects[ i ] -> misc_value == p_type )
    {
      // Divide by 100 for every A_ADD_PCT_MODIFIER
      if ( s_effects[ i ] -> subtype == (int) A_ADD_PCT_MODIFIER )
        return s_effects[ i ] -> base_value / 100.0;
      // Divide by property_flat_divisor for every A_ADD_FLAT_MODIFIER
      else
        return s_effects[ i ] -> base_value / property_flat_divisor[ s_effects[ i ] -> misc_value ];
    }
  }
  
  return 0.0;
}


// ==========================================================================
// Active Spell ID
// ==========================================================================

active_spell_t::active_spell_t( player_t* player, const char* t_name, const uint32_t id, talent_t* talent ) :
  spell_id_t( player, t_name, id, talent )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Active Spell status: %s", to_str().c_str() );
}

active_spell_t::active_spell_t( player_t* player, const char* t_name, const char* s_name, talent_t* talent ) :
  spell_id_t( player, t_name, s_name, talent )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Active Spell status: %s", to_str().c_str() );
}

// ==========================================================================
// Passive Spell ID
// ==========================================================================

passive_spell_t::passive_spell_t( player_t* player, const char* t_name, const uint32_t id, talent_t* talent ) :
  spell_id_t( player, t_name, id, talent )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Passive Spell status: %s", to_str().c_str() );
}

passive_spell_t::passive_spell_t( player_t* player, const char* t_name, const char* s_name, talent_t* talent ) :
  spell_id_t( player, t_name, s_name, talent )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Passive Spell status: %s", to_str().c_str() );
}

// Glyph basic object

glyph_t::glyph_t( player_t* player, const char* t_name ) :
  spell_id_t( player, t_name )
{
  std::string name = t_name;
  s_token = armory_t::format( name, FORMAT_ASCII_MASK );
  
  initialize( t_name );
  s_enabled = false;
  
  s_player -> spell_list.push_back( this );
}

bool glyph_t::enable( bool override_value ) 
{
  spell_id_t::enable( override_value );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Glyph Spell status: %s", to_str().c_str() );

  return s_enabled;
}

mastery_t::mastery_t( player_t* player, const char* t_name, const uint32_t id, talent_tree_type tree ) :
  spell_id_t( player, t_name, id ), m_tree( tree )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Mastery status: %s", to_str().c_str() );
}

mastery_t::mastery_t( player_t* player, const char* t_name, const char* s_name, talent_tree_type tree ) :
  spell_id_t( player, t_name, s_name ), m_tree( tree )
{
  s_player -> spell_list.push_back( this );
  
  if ( s_player -> sim -> debug ) 
    log_t::output( s_player -> sim, "Mastery status: %s", to_str().c_str() );
}

bool mastery_t::ok() SC_CONST
{
  return spell_id_t::ok() && ( s_player -> primary_tree() == m_tree );
}

double mastery_t::base_value( effect_type_t type, effect_subtype_t sub_type, int misc_value, int misc_value2 ) SC_CONST
{
  return spell_id_t::base_value( type, sub_type, misc_value, misc_value2 ) / 10000.0;
}

std::string mastery_t::to_str() SC_CONST
{
  std::ostringstream s;
  
  s << spell_id_t::to_str();
  s << " mastery_tree=" << m_tree;
  
  return s.str();
}


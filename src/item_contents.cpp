#include "item_contents.h"

#include <limits>
#include <algorithm>
#include <memory>

#include "character.h"
#include "enums.h"
#include "handle_liquid.h"
#include "item.h"
#include "itype.h"
#include "map.h"

struct tripoint;

item_contents::~item_contents()
{
    for( item *&it : items ) {
        it->remove_location();
        it->destroy();
    }
}

bool item_contents::empty() const
{
    return items.empty();
}

ret_val<bool> item_contents::insert_item( item &it )
{
    items.push_back( &it );
    return ret_val<bool>::make_success();
}

size_t item_contents::num_item_stacks() const
{
    return items.size();
}

bool item_contents::spill_contents( const tripoint &pos )
{
    for( item * const &it : items ) {
        get_map().add_item_or_charges( pos, *it );
    }

    items.clear();
    return true;
}

void item_contents::handle_liquid_or_spill( Character &guy )
{
    for( auto iter = items.begin(); iter != items.end(); ) {
        if( ( *iter )->made_of( LIQUID ) ) {

            item *liquid = *iter;
            iter = items.erase( iter );
            liquid_handler::handle_all_liquid( *liquid, 1 );
        } else {
            item *i_copy = *iter;
            iter = items.erase( iter );
            guy.i_add_or_drop( *i_copy );
        }
    }
}

void item_contents::casings_handle( const std::function<bool( item & )> &func )
{

    for( auto it = items.begin(); it != items.end(); ) {
        if( ( *it )->has_flag( "CASING" ) ) {
            ( *it )->unset_flag( "CASING" );
            detached_ptr<item> obj = std::move( detached_ptr::internal_create( *it ) );
            func( obj );
            if( !obj ) {
                it = items.erase( it, &obj );
                obj.release();
                continue;
            }
            // didn't handle the casing so reset the flag ready for next call
            ( *it )->set_flag( "CASING" );
        }
        ++it;
    }
}

void item_contents::clear_items()
{
    items.clear();
}

void item_contents::set_item_defaults()
{
    /* For Items with a magazine or battery in its contents */
    for( item * const &contained_item : items ) {
        /* for guns and other items defined to have a magazine but don't use "ammo" */
        if( contained_item->is_magazine() ) {
            contained_item->ammo_set(
                contained_item->ammo_default(), contained_item->ammo_capacity() / 2
            );
        } else { //Contents are batteries or food
            contained_item->charges = contained_item->typeId()->charges_default();
        }
    }
}

void item_contents::migrate_item( item &obj, const std::set<itype_id> &migrations )
{
    for( const itype_id &c : migrations ) {
        if( std::none_of( items.begin(), items.end(), [&]( const item * const & e ) {
        return e->typeId() == c;
        } ) ) {
            obj.put_in( *item_spawn( c, obj.birthday() ) );
        }
    }
}

bool item_contents::has_any_with( const std::function<bool( const item &it )> &filter ) const
{
    return std::any_of( items.begin(), items.end(), [&filter]( const item * const & it ) -> bool{ return filter( *it );} );
}

bool item_contents::stacks_with( const item_contents &rhs ) const
{
    return std::equal( items.begin(), items.end(), rhs.items.begin(), []( const item * const & a,
    const item * const & b ) {
        return a->charges == b->charges && a->stacks_with( *b );
    } );
}

item *item_contents::get_item_with( const std::function<bool( const item &it )> &filter )
{
    auto bomb_it = std::find_if( items.begin(),
                                 items.end(), [&filter]( const item * const & it ) -> bool{ return filter( *it );} );
    if( bomb_it == items.end() ) {
        return nullptr;
    } else {
        return *bomb_it;
    }
}

std::vector<item *> &item_contents::all_items_top()
{
    return items;
}

const std::vector<item *> &item_contents::all_items_top() const
{
    return items;
}


std::vector<detached_ptr<item>> item_contents::remove_all()
{
    std::vector<detached_ptr<item>> ret;
    items.clear_with( [&ret]( detached_ptr<item> &&it ) {
        ret.push_back( std::move( it ) );
    }
    return std::move( ret );
}

void item_contents::remove_top( item *it )
{
    std::vector<item *>::iterator iter = std::find_if( items.begin(),
    items.end(), [&it]( item *&against ) {
        return against == it;
    } );
    it->remove_location();//TODO!: need to make all the other functions here do locations too
    items.erase( iter );
}


std::vector<item *>::iterator item_contents::remove_top( std::vector<item *>::iterator &it )
{
    ( *it )->remove_location();
    return items.erase( it );
}

std::vector<item *>::iterator item_contents::remove_top( std::vector<item *>::iterator &it )
{
    ( *it )->remove_location();
    return items.erase( it );
}

std::vector<item *> item_contents::all_items_ptr()
{
    std::vector<item *> ret;
    for( item * const &it : items ) {
        ret.push_back( it );
        std::vector<item *> inside = it->contents.all_items_ptr();
        //TODO!:check
        ret.insert( ret.end(), inside.begin(), inside.end() );
    }
    return ret;
}

std::vector<const item *> item_contents::all_items_ptr() const
{
    std::vector<const item *> ret;
    for( const item * const &it : items ) {
        ret.push_back( it );
        std::vector<const item *> inside = it->contents.all_items_ptr();
        ret.insert( ret.end(), inside.begin(), inside.end() );
    }
    return ret;
}

std::vector<item *> item_contents::gunmods()
{
    std::vector<item *> res;
    for( item *&e : items ) {
        if( e->is_gunmod() ) {
            res.push_back( e );
        }
    }
    return res;
}

std::vector<const item *> item_contents::gunmods() const
{
    std::vector<const item *> res;
    for( const item * const &e : items ) {
        if( e->is_gunmod() ) {
            res.push_back( e );
        }
    }
    return res;
}

item &item_contents::front()
{
    return *items.front();
}

const item &item_contents::front() const
{
    return *items.front();
}

item &item_contents::back()
{
    return *items.back();
}

const item &item_contents::back() const
{
    return *items.back();
}

units::volume item_contents::item_size_modifier() const
{
    units::volume ret = 0_ml;
    for( const item * const &it : items ) {
        ret += it->volume();
    }
    return ret;
}

units::mass item_contents::item_weight_modifier() const
{
    units::mass ret = 0_gram;
    for( const item * const &it : items ) {
        ret += it->weight();
    }
    return ret;
}

int item_contents::best_quality( const quality_id &id ) const
{
    int ret = INT_MIN;
    for( const item * const &it : items ) {
        ret = std::max( ret, it->get_quality( id ) );
    }
    return ret;
}

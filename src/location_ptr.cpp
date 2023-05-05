#include "location_ptr.h"

#include "locations.h"

template<typename T, bool error_if_null>
void location_ptr<T, error_if_null>::update_location()
{
    if( ptr ) {
        ptr->set_location( &*loc );
    }
}

template<typename T, bool error_if_null>
void location_ptr<T, error_if_null>::unset_location()
{
    if( ptr ) {
        ptr->remove_location();
    }
}




template<typename T, bool error_if_null>
location_ptr<T, error_if_null>::location_ptr( location<T> *loc ) : loc( loc ) {};

template<typename T, bool error_if_null>
location_ptr<T, error_if_null> &location_ptr<T, error_if_null>::operator=
( detached_ptr<T> &&source )
{
    if( ptr ) {
        ptr->remove_location();
        ptr->destroy();
    }
    ptr = source.ptr;
    update_location();
    source.ptr = nullptr;
    return  *this;
}

template<typename T, bool error_if_null>
location_ptr<T, error_if_null> &location_ptr<T, error_if_null>::operator=
( location_ptr<T, true> &&source )
{
    if( ptr ) {
        ptr->remove_location();
        ptr->destroy();
    }
    ptr = source.ptr;
    update_location();
    source.ptr = nullptr;
    return *this;
}

template<typename T, bool error_if_null>
location_ptr<T, error_if_null> &location_ptr<T, error_if_null>::operator=
( location_ptr<T, false> &&source )
{
    if( ptr ) {
        ptr->remove_location();
        ptr->destroy();
    }
    ptr = source.ptr;
    update_location();
    source.ptr = nullptr;
    return *this;
}

template<typename T, bool error_if_null>
location_ptr<T, error_if_null>::~location_ptr()
{
    unset_location();
}

template<typename T, bool error_if_null>
detached_ptr<T> location_ptr<T, error_if_null>::release()
{
    if( !ptr ) {
        return detached_ptr<T>();
    }
    ptr->remove_location();
    T *obj = ptr;
    ptr = nullptr;
    return detached_ptr<T>( obj );
}

template<typename T, bool error_if_null>
void location_ptr<T, error_if_null>::set_location( location<T> *l )
{
    if( loc ) {
        debugmsg( "Attempted to set the location of a location_ptr that already has one" );
    }
    loc = std::unique_ptr<location<T>>( l );
}

template<typename T, bool error_if_null>
T *location_ptr<T, error_if_null>::get() const
{
    if( !*this ) {
        if( error_if_null ) {
            debugmsg( "Attempted to resolve invalid location_ptr" );
        }
        return &null_item_reference();//TODO! more than items
    }
    return ptr;
}

template<typename T, bool error_if_null>
location_ptr<T, error_if_null>::operator bool() const
{
    return !!*this;
}

template<typename T, bool error_if_null>
bool location_ptr<T, error_if_null>::operator!() const
{
    return !ptr;
}

template<typename T, bool error_if_null>
T &location_ptr<T, error_if_null>::operator*() const
{
    return *get();
}

template<typename T, bool error_if_null>
T *location_ptr<T, error_if_null>::operator->() const
{
    return get();
}

template<typename T, bool error_if_null>
bool location_ptr<T, error_if_null>::operator==( const T &against ) const
{
    return ptr == &against;
}

template<typename T, bool error_if_null>
bool location_ptr<T, error_if_null>::operator==( const T *against ) const
{
    return ptr == against;
}

template <typename T, bool error_if_null>
template <typename U>
bool location_ptr<T, error_if_null>::operator!=( const U against ) const
{
    return !( *this == against );
}


template
class location_ptr<item, true>;

template
class location_ptr<item, false>;

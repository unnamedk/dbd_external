#pragma once

#include "objects.hpp"
#include <string>

namespace sdk
{
    std::string get_object_name( uobject *object );

    std::string get_object_full_name( uobject *object );

    template <class T>
    T *find_object( const std::string &name )
    {
        for ( auto i = 0; i < gobjects->obj_objects.size(); ++i ) {
            auto object = gobjects->obj_objects.by_id( i );
            if ( !object || !object->object ) {
                continue;
            }

            auto n = get_object_full_name( object->object );
            if ( n == name ) {
                return static_cast<T *>( object->object );
            }
        }

        return nullptr;
    }
}
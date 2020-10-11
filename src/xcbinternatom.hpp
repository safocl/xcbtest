#pragma once

#include <iostream>
#include <string_view>
#include <vector>
#include <memory>
#include <cassert>
#include <xcb/xcb.h>

#include "xcbconnect.hpp"
#include "xcbwindowprop.hpp"

template < class T > class XCBInternAtom {
public:
    using Type                             = T;
    [[nodiscard]] virtual Type get() const = 0;

    virtual ~XCBInternAtom() = default;

    [[nodiscard]] static std::vector< uint32_t >
    getInternAtomValueArray( std::string_view atom );

    [[nodiscard]] static uint32_t
    getInternAtomValue( std::string_view atom );
};

template < class T >
[[nodiscard]] std::vector< uint32_t >
XCBInternAtom< T >::getInternAtomValueArray( std::string_view atom ) {
    XCBConnect  connect {};
    auto atomCookie =
    xcb_intern_atom( connect, false, atom.size(), atom.data() );
    std::unique_ptr< xcb_intern_atom_reply_t > atomRep {
        xcb_intern_atom_reply( connect, atomCookie, nullptr )
    };

    assert( atomRep != nullptr );

    size_t offset { 0 }, maxQueueLength { 32 }, remaining { 0 };
    std::vector< uint32_t > replyMessage {};
    replyMessage.reserve( maxQueueLength );

    auto screen =
    xcb_setup_roots_iterator( xcb_get_setup( connect ) ).data;

    do {
        using UniquePropRep =
        std::unique_ptr< xcb_get_property_reply_t >;

        auto          propCookie = xcb_get_property( connect,
                                            false,
                                            screen->root,
                                            atomRep->atom,
                                            XCB_GET_PROPERTY_TYPE_ANY,
                                            offset,
                                            maxQueueLength );
        UniquePropRep propRep { xcb_get_property_reply(
        connect, propCookie, nullptr ) };

        auto propValue = static_cast< uint32_t * >(
        xcb_get_property_value( propRep.get() ) );

        for ( int i = 0,
                  propRepLength =
                  xcb_get_property_value_length( propRep.get() ) /
                  ( propRep->format / 8 );
              i < propRepLength;
              ++i ) {
            replyMessage.push_back( propValue[ i ] );
        }

        remaining = propRep->bytes_after;
        offset += maxQueueLength;
        maxQueueLength *= 2;

    } while ( remaining > 0 );

    return replyMessage;
}

template < class T >
[[nodiscard]] uint32_t
XCBInternAtom< T >::getInternAtomValue( std::string_view atom ) {
    XCBConnect  connect {};
    auto atomCookie =
    xcb_intern_atom( connect, false, atom.size(), atom.data() );
    std::unique_ptr< xcb_intern_atom_reply_t > atomRep {
        xcb_intern_atom_reply( connect, atomCookie, nullptr )
    };

    assert( atomRep != nullptr );

    size_t offset { 0 }, maxQueueLength { 1 };

    auto screen =
    xcb_setup_roots_iterator( xcb_get_setup( connect ) ).data;

    using UniquePropRep = std::unique_ptr< xcb_get_property_reply_t >;

    auto          propCookie = xcb_get_property( connect,
                                        false,
                                        screen->root,
                                        atomRep->atom,
                                        XCB_GET_PROPERTY_TYPE_ANY,
                                        offset,
                                        maxQueueLength );
    UniquePropRep propRep { xcb_get_property_reply(
    connect, propCookie, nullptr ) };

    if ( propRep->bytes_after > 0 )
        throw std::runtime_error( "Data is't single" );

    return *static_cast< uint32_t * >(
    xcb_get_property_value( propRep.get() ) );
}

class AtomNetClientList final :
public XCBInternAtom< std::vector< XCBWindowProp > > {
public:
    ~AtomNetClientList() override = default;
    [[nodiscard]] Type get() const override;
};

[[nodiscard]] AtomNetClientList::Type AtomNetClientList::get() const {
    Type winPropVec {};
    for ( auto && el : getInternAtomValueArray( "_NET_CLIENT_LIST" ) )
        winPropVec.emplace_back( el );
    return winPropVec;
}

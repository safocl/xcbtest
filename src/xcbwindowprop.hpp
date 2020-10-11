#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <xcb/xproto.h>

#include "xcbconnect.hpp"

using XCBWindowClass = std::string;

struct XCBWindowGeometry final {
    int16_t  x;
    int16_t  y;
    uint16_t width;
    uint16_t height;
    uint16_t border_width;
};

class XCBWindowProp final {
public:
    using WindowIDType = u_int32_t;

private:
    WindowIDType windowID;

public:
    XCBWindowProp( uint32_t windowID );
    ~XCBWindowProp();
    XCBWindowGeometry getGeometry() const;
    XCBWindowClass    getClass() const;
    WindowIDType      getID() const;
};

XCBWindowProp::XCBWindowProp( WindowIDType windowID ) :
windowID( windowID ) {}

XCBWindowProp::~XCBWindowProp() = default;

XCBWindowGeometry XCBWindowProp::getGeometry() const {
    XCBConnect connect {};

    std::unique_ptr< xcb_get_geometry_reply_t > geometryRep {
        xcb_get_geometry_reply(
        connect, xcb_get_geometry( connect, windowID ), nullptr )
    };

    std::unique_ptr< xcb_query_tree_reply_t > tree {
        xcb_query_tree_reply(
        connect, xcb_query_tree( connect, windowID ), nullptr )
    };

    auto screen =
    xcb_setup_roots_iterator( xcb_get_setup( connect ) ).data;

    std::unique_ptr< xcb_translate_coordinates_reply_t >
    trans { xcb_translate_coordinates_reply(
    connect,
    xcb_translate_coordinates(
    connect, windowID, screen->root, geometryRep->x, geometryRep->y ),
    nullptr ) };

    return XCBWindowGeometry { trans->dst_x,
                               trans->dst_y,
                               geometryRep->width,
                               geometryRep->height,
                               geometryRep->border_width };
}

std::string XCBWindowProp::getClass() const {
    XCBConnect connect {};

    std::unique_ptr< xcb_get_property_reply_t > nameRep {
        xcb_get_property_reply( connect,
                                xcb_get_property( connect,
                                                  false,
                                                  windowID,
                                                  XCB_ATOM_WM_CLASS,
                                                  XCB_ATOM_STRING,
                                                  0,
                                                  3 ),
                                nullptr )
    };

    return std::string { static_cast< char * >(
    xcb_get_property_value( nameRep.get() ) ) };
}

XCBWindowProp::WindowIDType XCBWindowProp::getID() const {
    return windowID;
}

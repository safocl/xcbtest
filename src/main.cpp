#include <cassert>
#include <ios>
#include <iostream>
#include <xcb/xcb.h>
#include "xcbinternatom.hpp"

int main() {
    XCBWindowProp::WindowIDType watcherID { 0 };
    XCBWindowGeometry           watcherGeom {};
    for ( auto && el : AtomNetClientList {}.get() ) {
        if ( el.getClass() == "Watcher" ) {
            watcherID   = el.getID();
            watcherGeom = el.getGeometry();
            std::ios state { nullptr };
            state.copyfmt( std::cout );
            std::cout << std::hex << "0x" << el.getID() << std::endl;
            std::cout.copyfmt( state );

            std::cout << "Coord x: " << el.getGeometry().x
                      << std::endl
                      << "Coord y: " << el.getGeometry().y
                      << std::endl
                      << "Coord width: "
                      << el.getGeometry().width << std::endl
                      << "Coord height: "
                      << el.getGeometry().height << std::endl;
        }
        //std::cout << el.getClass()<<std::endl;
    }

    //    xcb_rectangle_t rectangles[] = { { 10, 50, 40, 20 },
    //                                     { 80, 50, 10, 40 } };

    auto connect = xcb_connect( nullptr, nullptr );
    assert( connect != nullptr );

    auto screen =
    xcb_setup_roots_iterator( xcb_get_setup( connect ) ).data;
    assert( screen != nullptr );

    auto window = screen->root;
    //    auto     window         = xcb_generate_id( connect );
    uint32_t winMask        = XCB_CW_EVENT_MASK;
    uint32_t winValueList[] = { XCB_EVENT_MASK_EXPOSURE |
                                XCB_EVENT_MASK_COLOR_MAP_CHANGE };
    //    xcb_create_window( connect,
    //                       window_depth,
    //                       window,
    //                       window,
    //                       0,
    //                       0,
    //                       1920,
    //                       1080,
    //                       1,
    //                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
    //                       window_visual,
    //                       winMask,
    //                       winValueList );

    uint32_t mask = /*XCB_GC_FUNCTION |*/ XCB_GC_BACKGROUND |
                    XCB_GC_GRAPHICS_EXPOSURES | XCB_GC_FOREGROUND /*|
                    XCB_GC_SUBWINDOW_MODE*/
    ;

    uint32_t valueList[] = { /*XCB_GX_COPY,*/
                             screen->black_pixel,
                             screen->black_pixel,
                             /*XCB_SUBWINDOW_MODE_CLIP_BY_CHILDREN,*/
                             1
    };

    auto gc = xcb_generate_id( connect );
    xcb_create_gc( connect, gc, window, mask, valueList );
    //xcb_configure_window( connect, watcherID, mask, valueList );

    //!!! It's important !!!
    //xcb_change_window_attributes(
    //connect, window, winMask, winValueList );

    auto watcherConn = xcb_connect( nullptr, nullptr );
    xcb_change_window_attributes(
    watcherConn, watcherID, winMask, winValueList );

    xcb_map_window( watcherConn, watcherID );

    xcb_flush( connect );
    xcb_flush( watcherConn );

    std::cout << "Before cycle" << std::endl;
    for ( auto event = xcb_wait_for_event( watcherConn );
          event != nullptr;
          event = xcb_wait_for_event( watcherConn ) ) {
        switch ( event->response_type )
        case XCB_EXPOSE: {
            xcb_copy_area( connect,
                           watcherID,
                           window,
                           gc,
                           0,
                           0,
                           watcherGeom.x,
                           watcherGeom.y,
                           watcherGeom.width,
                           watcherGeom.height );

            xcb_flush( connect );
            xcb_flush( watcherConn );
        }
            delete event;
    }

    return EXIT_SUCCESS;
}

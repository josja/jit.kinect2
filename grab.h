//
//  grab.h
//  jit.simple
//
//  Created by Josja Krosenbrink on 18/03/16.
//
//

#ifndef __jit_simple__grab__
#define __jit_simple__grab__

#include <iostream>
#include <map>

/// [headers]
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
//#include <libfreenect2/packet_pipeline.h>
//#include <libfreenect2/logger.h>
/// [headers]

enum GRABERROR {
    NoError,
    CouldNotFindDevice,
    CouldNotOpenDevice,
    CouldNotStartDevice,
    UnknownError
};

enum FRAMETYPE {
    Color = libfreenect2::Frame::Color,
    IR = libfreenect2::Frame::Ir,
    Depth = libfreenect2::Frame::Depth,
    Registration
};

const std::string GrabError[] =
{
    "Everything is fine.",
    "Couldn't find kinect2.",
    "Couldn't open kinect2.",
    "Couldn't start kinect2.",
    "Sumthin wrong."
};

class Grab {
public:
    
    bool isOpen = false;
    bool hasNewFrames = false;
    GRABERROR lastError = NoError;
    
    /// [context]
    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev;
    libfreenect2::PacketPipeline *pipeline;
    libfreenect2::Registration *registration;
    /// [context]
    
    
    int deviceId = -1;
    std::string serial = "";
    std::string firmware = "";
    size_t framecount = 0;
    size_t framemax = -1;
    
    //static bool shutdown;
    
    libfreenect2::SyncMultiFrameListener listener = libfreenect2::SyncMultiFrameListener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
    libfreenect2::FrameMap frames;
    libfreenect2::Frame undistorted = libfreenect2::Frame(512, 424, 4);
    libfreenect2::Frame registered = libfreenect2::Frame(512, 424, 4);
    
    //
    //Grab();
    //static void sigint_handler(int s);
    bool                    open();
    libfreenect2::FrameMap  getframes();
    libfreenect2::Frame*    frame(FRAMETYPE type);
    void                    releaseframes();
    void                    close();
    std::string             getlasterrorstring() {
        return GrabError[lastError] + "\nDevice Serial: " + serial + "\n" + "\nDevice FW: " + firmware + "\n";
    };
};

 


#endif /* defined(__jit_simple__grab__) */

//
//  grab.cpp
//  jit.simple
//
//  Created by Josja Krosenbrink on 18/03/16.
//
//

#include <iostream>
#include <map>

#include "grab.h"

bool Grab::open()
{
    /// [discovery]
    if(freenect2.enumerateDevices() == 0)
    {
        lastError = CouldNotFindDevice;
        isOpen = false;
        return isOpen;
    }
    
    serial = freenect2.getDefaultDeviceSerialNumber();
    /// [discovery]
    
    dev = freenect2.openDevice(serial);
    if(dev == 0)
    {
        lastError = CouldNotOpenDevice;
        isOpen = false;
        return isOpen;
    }
    
    //signal(SIGINT,sigint_handler);
    //Grab::shutdown = false;
    
    /// [listeners]
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);
    /// [listeners]
    
    /// [start]
    if (!dev->start())
    {
        lastError = CouldNotStartDevice;
        isOpen = false;
        return isOpen;
    }
    
    serial = dev->getSerialNumber();
    firmware = dev->getFirmwareVersion();
    /// [start]
    
    /// [registration setup]
    registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());

    /// [registration setup]
    
    lastError = NoError;
    isOpen = true;
    return isOpen;
}

libfreenect2::FrameMap Grab::getframes() {
    hasNewFrames = false;
    listener.waitForNewFrame(frames,5);
    hasNewFrames = listener.hasNewFrame();
    return frames;
}

libfreenect2::Frame* Grab::frame(FRAMETYPE type) {
    switch (type)
    {
        case Color:
            return frames[libfreenect2::Frame::Color];
            break;
        case IR:
            return frames[libfreenect2::Frame::Ir];
            break;
        case Depth:
            return frames[libfreenect2::Frame::Depth];
            break;
        case Registration:
            registration->apply(frames[libfreenect2::Frame::Color], frames[libfreenect2::Frame::Depth], &undistorted, &registered);
            registered.format = libfreenect2::Frame::BGRX;
            return &registered;
            break;
    }
}

void Grab::releaseframes() {
    listener.release(frames);
}

void Grab::close() {
    // TODO: restarting ir stream doesn't work!
    // TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
    
    if (isOpen)
    {
        /// [stop]
        dev->stop();
        dev->close();
        /// [stop]
    }
    
    delete registration;
}

//
//  jit.kinect2.h
//  jit.kinect2
//
//  Created by Josja Krosenbrink on 22/03/16.
//
//

#ifndef jit_kinect2_jit_simple_h
#define jit_kinect2_jit_simple_h

#include "jit.common.h"
#include "grab.h"

// Our Jitter object instance data
typedef struct _jit_kinect2 {
    t_object	ob;
    Grab        *kinect;
    bool        once = true;
} t_jit_kinect2;


#endif

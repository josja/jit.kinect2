/****************************************************************************
	jit.openni
	Copyright (C) 2011 Dale Phurrough
 
	This file is part of jit.openni.
 
 jit.openni is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 jit.openni is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with jit.openni.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

/**
 jit.openni - a Max Jitter external for OpenNI middleware
 Shell of it was imspired by the jit.simple example from the MaxSDK and
 the MaxSDK documentation
 */


//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

#include "jit.openni.h"

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------


// globals
XnVersion currentOpenNIVersion;
static void	*max_jit_openni_class = NULL;
static char *strJointNames[] = { NULL,
    "head", "neck", "torso", "waist",
    "l_collar", "l_shoulder", "l_elbow", "l_wrist", "l_hand", "l_fingertip",
    "r_collar", "r_shoulder", "r_elbow", "r_wrist", "r_hand", "r_fingertip",
    "l_hip", "l_knee", "l_ankle", "l_foot",
    "r_hip", "r_knee", "r_ankle", "r_foot"};
#define MAX_LENGTH_STR_JOINT_NAME 11	// always keep the number of characters of the longest string in this group above

// strSkeletonEvents[] must match the number/order of enum JitOpenNIEvents defined in jit.openni.h
static char *strSkeletonEvents[3][8] = {
    { "/new_user", "/lost_user", "/calib_pose",	"/calib_start", "/calib_success", "/calib_fail", "/exit_user", "/reenter_user"},
    { "new_user", "lost_user", "calib_pose", "calib_start", "calib_success", "calib_fail", "exit_user", "reenter_user"},
    { "/new_user", "/lost_user", "/pose_detected", "/calibration_started", "/new_skel", "/calibration_failed", "/exit_user", "/reenter_user"}  };


/************************************************************************************/

int main(void)
{
    void *p, *q;
    t_jit_object	*attr;
    
    LOG_DEBUG("max.jit_openni.main()");
    post("jit.openni %s, Copyright (c) 2012 Dale Phurrough. This program comes with ABSOLUTELY NO WARRANTY.", JIT_OPENNI_VERSION);
    post("jit.openni %s, Licensed under the GNU General Public License v3.0 (GPLv3) available at http://www.gnu.org/licenses/gpl-3.0.html", JIT_OPENNI_VERSION);
    post("jit.openni %s, Compiled and casually tested with OpenNI %s, NITE %s", JIT_OPENNI_VERSION, XN_BRIEF_VERSION_STRING, XNV_NITE_BRIEF_VERSION_STRING);
#ifdef _DEBUG
    post("jit.openni %s, DEBUG build %s @ %s", JIT_OPENNI_VERSION, __DATE__, __TIME__);
#endif
    xnGetVersion(&currentOpenNIVersion);
    //#ifdef _DEBUG
    //	post("Need OpenNI (%ld) currently have OpenNI (%ld)", OPENNI_REQUIRED_VERSION, (currentOpenNIVersion.nMajor*100000000 + currentOpenNIVersion.nMinor*1000000 + currentOpenNIVersion.nMaintenance*10000 + currentOpenNIVersion.nBuild) );
    //#endif
    //if ( (currentOpenNIVersion.nMajor*100000000 + currentOpenNIVersion.nMinor*1000000 + currentOpenNIVersion.nMaintenance*10000 + currentOpenNIVersion.nBuild) < OPENNI_REQUIRED_VERSION)
    //	error("jit.openni %s, Requires at least OpenNI %s. Please read the installation documentation.", JIT_OPENNI_VERSION, XN_BRIEF_VERSION_STRING);
    
    // initialize the Jitter class by calling Jitter class's registration function
    jit_openni_init();
    
    // create the Max wrapper class
    setup((t_messlist**)&max_jit_openni_class, (method)max_jit_openni_new, (method)max_jit_openni_free, sizeof(t_max_jit_openni), 0, A_GIMME, 0);
    
    // specify a byte offset to keep additional OBEX information
    p = max_jit_classex_setup(calcoffset(t_max_jit_openni, obex));
    
    // look up the Jitter class in the class registry
    q = jit_class_findbyname(gensym("jit_openni"));
    
    // add default methods and attributes for MOP max wrapper class, e.g. name, type, dim, planecount, bang, outputmatrix, etc
    max_jit_classex_mop_wrap(p, q, MAX_JIT_MOP_FLAGS_OWN_OUTPUTMATRIX|MAX_JIT_MOP_FLAGS_OWN_JIT_MATRIX|MAX_JIT_MOP_FLAGS_OWN_ADAPT);
    
    // add custom max wrapper attributes
    attr = jit_object_new(_jit_sym_jit_attr_offset, "skeleton_format", _jit_sym_char, JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW,
                          NULL, NULL, calcoffset(t_max_jit_openni, chrSkeletonOutputFormat));
    jit_attr_addfilterset_clip(attr,0,2,TRUE,TRUE);
    max_jit_classex_addattr(p, attr);
    
    // wrap the Jitter class with the standard methods for Jitter objects, e.g. getattributes, dumpout, maxjitclassaddmethods, etc
    max_jit_classex_standard_wrap(p, q, 0);
    
    // add methods to the Max wrapper class
    max_addmethod_usurp_low((method)max_jit_openni_outputmatrix, "outputmatrix");
    max_addmethod_usurp_low((method)max_jit_openni_XMLConfig_read, "read");
    max_addmethod_usurp_low((method)max_jit_openni_get_versions, "getversions");
    
    // add an inlet/outlet assistance method; in this case the default matrix-operator (mop) assist fn
    addmess((method)max_jit_openni_assist, "assist", A_CANT, 0);
    return 0;
}


/************************************************************************************/
// Object Life Cycle

void *max_jit_openni_new(t_symbol *s, long argc, t_atom *argv)
{
    t_max_jit_openni	*x;
    void				*o;
#ifdef _DEBUG
    long i = 0;
#endif
    
    x = (t_max_jit_openni*)max_jit_obex_new(max_jit_openni_class, gensym("jit_openni"));
    if (x)
    {
        o = jit_object_new(gensym("jit_openni"), x);	// instantiate jit.openni jitter object
        if (o)
        {
            // typically, max_jit_mop_setup_simple(x, o, argc, argv) is called here to handle standard MOP max wrapper setup tasks
            // however, I need to create a max only outlet between the MOP outlets and dumpout, so will use the code from MAx SDK 21.6.
            max_jit_obex_jitob_set(x,o);
            max_jit_obex_dumpout_set(x,outlet_new(x,NULL));
            x->osc_outlet = (t_object *)outlet_new(x, NULL);
            max_jit_mop_setup(x);
            max_jit_mop_inputs(x);
            max_jit_mop_outputs(x);
            x->chrSkeletonOutputFormat = 0;
            max_jit_mop_matrix_args(x,argc,argv);
            
            max_jit_attr_args(x, (short)argc, argv); // process attribute arguments, like auto handling of @attribute's
#ifdef _DEBUG
            for (i = 0; i < argc; i++)
            {
                switch (atom_gettype(&(argv[i])))
                {
                    case A_LONG:
                        LOG_COMMENT3("arg %ld: long (%ld)", i, atom_getlong(&(argv[i])));
                        break;
                    case A_FLOAT:
                        LOG_COMMENT3("arg %ld: float (%f)", i, atom_getfloat(&(argv[i])));
                        break;
                    case A_SYM:
                        LOG_COMMENT3("arg %ld: symbol (%s)", i, atom_getsym(&(argv[i]))->s_name);
                        break;
                    default:
                        LOG_WARNING2("arg %ld: forbidden argument", i);
                }
            }
#endif
            if(RegisterJitOpenNIEventCallbacks((t_jit_openni *)max_jit_obex_jitob_get(x), max_jit_openni_post_events))
            {
                LOG_ERROR("jit.openni: could not register for jit.openni event callbacks");
                max_jit_openni_free(x);
                freeobject((t_object*)x);
                x = NULL;
            }
        }
        else
        {
            LOG_ERROR("jit.openni: could not allocate object");
            max_jit_obex_free(x);
            freeobject((t_object*)x);
            x = NULL;
        }
    }
    return(x);
}


void max_jit_openni_free(t_max_jit_openni *x)
{
    if (UnregisterJitOpenNIEventCallbacks((t_jit_openni *)max_jit_obex_jitob_get(x), max_jit_openni_post_events))
    {
        LOG_ERROR("jit.openni: could not unregister for jit.openni event callbacks");
    }
    
    max_jit_mop_free(x);
    
    // lookup the internal Jitter object instance and free
    jit_object_free(max_jit_obex_jitob_get(x));
    
    // free resources associated with the obex entry
    max_jit_obex_free(x);
}


void max_jit_openni_assist(t_max_jit_openni *x, void *b, long io, long index, char *s)
{
    // I acknowledge the code below is redundant
    switch (io)
    {
        case 1:
            strncpy_zero(s, "(text) read filename.xml only", 512);
            break;
        case 2:
            switch (index)
        {
            case DEPTHMAP_OUTPUT_INDEX:
                snprintf_zero(s, 512, "%s out%d", DEPTHMAP_ASSIST_TEXT, index+1);
                break;
            case IMAGEMAP_OUTPUT_INDEX:
                snprintf_zero(s, 512, "%s out%d", IMAGEMAP_ASSIST_TEXT, index+1);
                break;
            case IRMAP_OUTPUT_INDEX:
                snprintf_zero(s, 512, "%s out%d", IRMAP_ASSIST_TEXT, index+1);
                break;
            case USERPIXELMAP_OUTPUT_INDEX:
                snprintf_zero(s, 512, "%s out%d", USERPIXELMAP_ASSIST_TEXT, index+1);
                break;
            case SKELETON_OUTPUT_INDEX:
                snprintf_zero(s, 512, "%s out%d", SKELETON_ASSIST_TEXT, index+1);
                break;
            case NUM_JITOPENNI_OUTPUTS:
                strncpy_zero(s, "dumpout", 512);
        }
    }
}

void max_jit_openni_get_versions(t_max_jit_openni *x, t_symbol *s, short argc, t_atom *argv)
{
    t_atom OutAtoms[4];
    
    atom_setlong(OutAtoms, JIT_OPENNI_VERSION_MAJOR);
    atom_setlong(OutAtoms + 1, JIT_OPENNI_VERSION_MINOR);
    atom_setlong(OutAtoms + 2, JIT_OPENNI_VERSION_INCRM);
    max_jit_obex_dumpout(x, gensym("version_jit.openni"), 3, OutAtoms);
    
    atom_setlong(OutAtoms, currentOpenNIVersion.nMajor);
    atom_setlong(OutAtoms + 1, currentOpenNIVersion.nMinor);
    atom_setlong(OutAtoms + 2, currentOpenNIVersion.nMaintenance);
    atom_setlong(OutAtoms + 3, currentOpenNIVersion.nBuild);
    max_jit_obex_dumpout(x, gensym("version_openni"), 4, OutAtoms);
}

void max_jit_openni_XMLConfig_read(t_max_jit_openni *x, t_symbol *s, short argc, t_atom *argv)
{
    t_atom OutAtoms[2];
    short filePathID;
    long fileType = 'TEXT', outType = 0;	// wacko Max API for filetypes takes 4 chars hacked into a long
    char filename[MAX_FILENAME_CHARS];
    char fullyQualifiedPathname[MAX_PATH_CHARS];
    XnStatus nRetVal = XN_STATUS_OK;
    
#ifdef _DEBUG
    char hackpath[MAX_PATH_CHARS];			// BUGBUG remove this debug variable
    char hackfilename[MAX_FILENAME_CHARS];	// BUGBUG remove this debug variable
    t_object *mypatcher;
    t_symbol *mypatcherpath = NULL;
    
    if (object_obex_lookup(x, gensym("#P"), &mypatcher) != MAX_ERR_NONE)
    {
        LOG_ERROR("error getting patcher for jit.openni");
    }
    else
    {
        mypatcher = jpatcher_get_toppatcher(mypatcher);
        mypatcherpath = object_attr_getsym(mypatcher, gensym("filepath"));
    }
    if ((mypatcherpath) && (mypatcherpath != gensym("")))
    {
        LOG_DEBUG("The top patcher path is %s", mypatcherpath->s_name);
        path_splitnames(mypatcherpath->s_name, hackpath, hackfilename);
        LOG_DEBUG("patcher@ \"%s\" \"%s\"", hackpath, hackfilename);
    }
    else
    {
        LOG_DEBUG("error getting filepath symbol for top patcher containing jit.openni");
    }
#endif
    
    if (argc == 0) // if no argument supplied, ask for file
    {
        if (open_dialog(filename, &filePathID, &outType, &fileType, 1))
        {
            // non-zero: user cancelled or error
            LOG_DEBUG("error getting XML config file from dialog box for max.jit.openni");
            atom_setsym(OutAtoms, gensym("<none>"));
            atom_setlong(OutAtoms + 1, 0);
            max_jit_obex_dumpout(x, gensym("read"), 2, OutAtoms);
            return;
        }
    }
    else if ((argc != 1) || (atom_gettype(argv) != A_SYM))
    {
        LOG_DEBUG("read must have only one symbol argument");
        atom_setsym(OutAtoms, gensym("<none>"));
        atom_setlong(OutAtoms + 1, 0);
        max_jit_obex_dumpout(x, gensym("read"), 2, OutAtoms);
        return;
    }
    else // we have exactly one symbol argument
    {
        strncpy_zero(filename, atom_getsym(argv)->s_name, MAX_FILENAME_CHARS);
        if (locatefile_extended(filename, &filePathID, &outType, &fileType, 1))
        {
            LOG_DEBUG("locatefile_extended() failed finding \"%s\"", atom_getsym(argv)->s_name);
            atom_setsym(OutAtoms, atom_getsym(argv));
            atom_setlong(OutAtoms + 1, 0);
            max_jit_obex_dumpout(x, gensym("read"), 2, OutAtoms);
            return;
        }
        LOG_DEBUG("locatefile_extended(pathid,otyp,name): %d, %.4s, \"%s\"", filePathID, (char *)&outType, filename);
    }
    
#ifdef _DEBUG
    path_topathname(filePathID, NULL, hackpath);
    LOG_DEBUG("pathIDtoName=> \"%s\"", hackpath);
#endif
    
    //Load file
    atom_setsym(OutAtoms, gensym(filename));
    if (path_topathname(filePathID, filename, fullyQualifiedPathname) == 0)
    {
        char nativeQualifiedPathname[MAX_PATH_CHARS];
        LOG_DEBUG("fullyQualPath=%s", fullyQualifiedPathname);
#ifdef WIN_VERSION
        path_nameconform(fullyQualifiedPathname, nativeQualifiedPathname, PATH_STYLE_NATIVE, PATH_TYPE_ABSOLUTE);
#else
        path_nameconform(fullyQualifiedPathname, nativeQualifiedPathname, PATH_STYLE_NATIVE, PATH_TYPE_BOOT);
#endif
        LOG_DEBUG("nativeQualPath=%s", nativeQualifiedPathname);
        jit_object_method(max_jit_obex_jitob_get(x), gensym("init_from_xml"), gensym(nativeQualifiedPathname), &nRetVal);
        if (nRetVal)
        {
            atom_setlong(OutAtoms + 1, 0);
        }
        else
        {
            atom_setlong(OutAtoms + 1, 1);
        }
        max_jit_obex_dumpout(x, gensym("read"), 2, OutAtoms);
    }
    else
    {
        LOG_DEBUG("path_topathname failed with: %d,\"%s\"", filePathID, filename);
        atom_setlong(OutAtoms + 1, 0);
        max_jit_obex_dumpout(x, gensym("read"), 2, OutAtoms);
    }
    
}

void max_jit_openni_outputmatrix(t_max_jit_openni *x)
{
    long outputmode = max_jit_mop_getoutputmode(x);
    void *mop = max_jit_obex_adornment_get(x,_jit_sym_jit_mop);
    t_jit_openni *pJit_OpenNI = (t_jit_openni *)max_jit_obex_jitob_get(x);
    t_jit_err err;
    t_atom osc_argv[16];			// max number of atoms/values after the message selector
    char osc_string[MAX_LENGTH_STR_JOINT_NAME + 10];	// max joint string + 9 for "/skel/xx/" + terminating null
    char *msg_selector_string;
    unsigned int i, j, k;
    const char strSkelFormatOutput[3][12] = { "/skel/%u/%s", "skel", "/joint" };		// switchable skeleton output format selectors
    // the [2] format string below should be an unsigned %u, however OSCeleton codebase incorrectly uses %d so I also use it here for compatibility
    const char strUserCoMFormatOutput[3][9] = { "/user/%u", "user", "/user/%d" };		// switchable user CoM output format selectors
    const char strFloorFormatOutput[3][7] = { "/floor", "floor", "/floor" };			// switchable floor output format selectors
    
    LOG_DEBUG("custom max_jit_openni_outputmatrix()");
    if (outputmode && mop)
    {
        //always output unless output mode is none
        if (outputmode==2) // pass input case, but since jit.openni has no input then this means no output
        {
            //LOG_DEBUG("bypassing matrix_calc(), bypassing outputmatrix()");
            return;
        }
        
        if (outputmode==1)
        {
            //LOG_DEBUG("about to call matrix_calc from custom outputmatrix");
            if (err = (t_jit_err)jit_object_method(max_jit_obex_jitob_get(x), _jit_sym_matrix_calc,
                                                   jit_object_method(mop,_jit_sym_getinputlist),
                                                   jit_object_method(mop,_jit_sym_getoutputlist)))
            {
                if (err != JIT_ERR_HW_UNAVAILABLE) jit_error_code(x,err);
                return;
            }
        }
        
        // output floor from scene generator
        if (pJit_OpenNI->hProductionNode[SCENE_GEN_INDEX] && pJit_OpenNI->bOutputSceneFloor)
        {
            msg_selector_string = (char *)strFloorFormatOutput[x->chrSkeletonOutputFormat];
            atom_setfloat(&(osc_argv[0]), pJit_OpenNI->planeFloor.ptPoint.X);
            atom_setfloat(&(osc_argv[1]), pJit_OpenNI->planeFloor.ptPoint.Y);
            atom_setfloat(&(osc_argv[2]), pJit_OpenNI->planeFloor.ptPoint.Z);
            atom_setfloat(&(osc_argv[3]), pJit_OpenNI->planeFloor.vNormal.X);
            atom_setfloat(&(osc_argv[4]), pJit_OpenNI->planeFloor.vNormal.Y);
            atom_setfloat(&(osc_argv[5]), pJit_OpenNI->planeFloor.vNormal.Z);
            outlet_anything(x->osc_outlet, gensym(msg_selector_string), 6, osc_argv);
        }
        
        // output seen users' centers of mass and skeletons
        for (i=0; i<pJit_OpenNI->iNumUsersSeen; i++)
        {
            short iNumAtoms = 0;
            
            // output user center of mass
            switch (x->chrSkeletonOutputFormat)
            {
                case 1: // native max route compatible format
                    msg_selector_string = (char *)strUserCoMFormatOutput[1];
                    atom_setlong(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userID);
                    break;
                case 0:		// default jit.openni skeleton OSC output format
                case 2:		// legacy OSCeleton format
                default:	// clearly defining the default case rids of a compiler caution
                    snprintf_zero(osc_string, sizeof(osc_string), strUserCoMFormatOutput[x->chrSkeletonOutputFormat], pJit_OpenNI->pUserSkeletonJoints[i].userID);
                    msg_selector_string = osc_string;
                    break;
            }
            atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userCoM.X);
            atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userCoM.Y);
            atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userCoM.Z);
            outlet_anything(x->osc_outlet, gensym(msg_selector_string), iNumAtoms, osc_argv);
            
            // output skeletons
            if (pJit_OpenNI->pUserSkeletonJoints[i].bUserSkeletonTracked)
            {
                for (j=1; j<= NUM_OF_SKELETON_JOINT_TYPES; j++)
                {
                    if (pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].position.fConfidence >= pJit_OpenNI->fPositionConfidenceFilter &&
                        ( pJit_OpenNI->bOutputSkeletonOrientation ?
                         (pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.fConfidence >= pJit_OpenNI->fOrientConfidenceFilter) : true))
                    {
                        iNumAtoms = 0;
                        
                        switch (x->chrSkeletonOutputFormat)
                        {
                            case 0:	// default jit.openni skeleton output format "/skel/%u/%s"
                                snprintf_zero(osc_string, sizeof(osc_string), strSkelFormatOutput[0], pJit_OpenNI->pUserSkeletonJoints[i].userID, strJointNames[j]);
                                msg_selector_string = osc_string;
                                break;
                            case 1: // skeleton output "skel %u %s" to easily use with standard max route object
                                msg_selector_string = (char *)strSkelFormatOutput[1];
                                atom_setlong(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userID);
                                atom_setsym(osc_argv + (iNumAtoms++), gensym(strJointNames[j]));
                                break;
                            case 2: // skeleton output "/joint %s %u" same format as OSCeleton's default output with no orientation and no OSCeleton's legacy "normalized" values
                                msg_selector_string = (char *)strSkelFormatOutput[2];
                                atom_setsym(osc_argv + (iNumAtoms++), gensym(strJointNames[j]));
                                atom_setlong(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].userID);
                                break;
                        }
                        atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].position.position.X);
                        atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].position.position.Y);
                        atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].position.position.Z);
                        
                        if (x->chrSkeletonOutputFormat != 2)
                        {						
                            atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].position.fConfidence);
                        }
                        
                        if (pJit_OpenNI->bOutputSkeletonOrientation && (x->chrSkeletonOutputFormat != 2))
                        {
                            // General belief and https://groups.google.com/forum/#!topic/openni-dev/-ib1yX-o0lk say that OpenNI stores the
                            // orientation matrix in row-major order.
                            // X axis = (elements[0], elements[3], elements[6])
                            // Y axis = (elements[1], elements[4], elements[7])
                            // Z axis = (elements[2], elements[5], elements[8])
                            // Just in case, column major order is easy to substitute with: for (k=0; k<9; k++) atom_setfloat(osc_argv + iAtomOffset + 4 + k, pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.orientation.elements[k]);
                            for (k=0; k<3; k++)
                            {
                                atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.orientation.elements[k]);
                                atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.orientation.elements[k+3]);
                                atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.orientation.elements[k+6]);
                            }
                            if (x->chrSkeletonOutputFormat != 2)
                            {
                                atom_setfloat(osc_argv + (iNumAtoms++), pJit_OpenNI->pUserSkeletonJoints[i].jointTransform[j].orientation.fConfidence);
                            }
                        }
                        outlet_anything(x->osc_outlet, gensym(msg_selector_string), iNumAtoms, osc_argv);
                    }
                }
            }
        }
        
        //LOG_DEBUG("calling standard max_jit_mop_outputmatrix()");
        max_jit_mop_outputmatrix(x);
    }	
}

void max_jit_openni_post_events(t_jit_openni *x, enum JitOpenNIEvents iEventType, XnUserID userID)
{
    t_atom osc_argv;
    
    atom_setlong(&osc_argv, userID);
    outlet_anything(((t_max_jit_openni *)(x->pParent))->osc_outlet, gensym(strSkeletonEvents[((t_max_jit_openni *)(x->pParent))->chrSkeletonOutputFormat][iEventType]), 1, &osc_argv);
}
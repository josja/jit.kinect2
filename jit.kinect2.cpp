

#include "jit.kinect2.h"
#include <math.h>

#define NUMOFOUTPUTS 4

// prototypes
BEGIN_USING_C_LINKAGE
t_jit_err		jit_kinect2_init			(void);
t_jit_kinect2	*jit_kinect2_new			(void);
void			jit_kinect2_free            (t_jit_kinect2 *x);
t_jit_err		jit_kinect2_matrix_calc		(t_jit_kinect2 *x, void *inputs, void *outputs);
void            jit_kinect2_copy            (void* frametype, long dimcount, long *dim, long planecount, t_jit_matrix_info *in_minfo, char *bip,t_jit_matrix_info *out_minfo, char *bop, t_jit_parallel_ndim_worker *para_worker);
void            jit_kinect2_deduct_info     (libfreenect2::Frame* frame, t_jit_matrix_info *info);
END_USING_C_LINKAGE


// globals
static void *s_jit_kinect2_class = NULL;

/************************************************************************************/

t_jit_err jit_kinect2_init(void)
{
	t_jit_object	*mop;
    void			*output;
    t_atom			a_arr[4];

	s_jit_kinect2_class = jit_class_new("jit_kinect2", (method)jit_kinect2_new, (method)jit_kinect2_free, sizeof(t_jit_kinect2), 0);

	// add matrix operator (mop)
	mop = (t_jit_object *)jit_object_new(_jit_sym_jit_mop, 0, NUMOFOUTPUTS); // no inputs, NUMOFOUTPUTS outputs
    
    // RGB output
    jit_mop_output_nolink(mop, 1);
    output = jit_object_method(mop, _jit_sym_getoutput, 1);
    jit_attr_setlong(output,_jit_sym_minplanecount,4);
    jit_attr_setlong(output,_jit_sym_maxplanecount,4);
    jit_attr_setlong(output,_jit_sym_mindimcount,2);
    jit_attr_setlong(output,_jit_sym_maxdimcount,2);
    jit_atom_setlong(&a_arr[0], 1920);
    jit_atom_setlong(&a_arr[1], 1080);
    jit_object_method(output,_jit_sym_maxdim,2,a_arr);
    jit_object_method(output,_jit_sym_mindim,2,a_arr);
    jit_atom_setsym(&a_arr[0], _jit_sym_char);
    jit_object_method(output,_jit_sym_types,1,a_arr);

    // IR output
    jit_mop_output_nolink(mop, 2);
    output = jit_object_method(mop, _jit_sym_getoutput, 2);
    jit_attr_setlong(output,_jit_sym_minplanecount,1);
    jit_attr_setlong(output,_jit_sym_maxplanecount,1);
    jit_attr_setlong(output,_jit_sym_mindimcount,2);
    jit_attr_setlong(output,_jit_sym_maxdimcount,2);
    jit_atom_setlong(&a_arr[0], 512);
    jit_atom_setlong(&a_arr[1], 424);
    jit_object_method(output,_jit_sym_maxdim,2,a_arr);
    jit_object_method(output,_jit_sym_mindim,2,a_arr);
    jit_atom_setsym(&a_arr[0], _jit_sym_char);
    jit_object_method(output,_jit_sym_types,1,a_arr);

    // DEPTH output
    jit_mop_output_nolink(mop, 3);
    output = jit_object_method(mop, _jit_sym_getoutput, 3);
    jit_attr_setlong(output,_jit_sym_minplanecount,4);
    jit_attr_setlong(output,_jit_sym_maxplanecount,4);
    jit_attr_setlong(output,_jit_sym_mindimcount,2);
    jit_attr_setlong(output,_jit_sym_maxdimcount,2);
    jit_atom_setlong(&a_arr[0], 512);
    jit_atom_setlong(&a_arr[1], 424);
    jit_object_method(output,_jit_sym_maxdim,2,a_arr);
    jit_object_method(output,_jit_sym_mindim,2,a_arr);
    jit_atom_setsym(&a_arr[0], _jit_sym_char);
    jit_object_method(output,_jit_sym_types,1,a_arr);

    // Registration output
    jit_mop_output_nolink(mop, 4);
    output = jit_object_method(mop, _jit_sym_getoutput, 4);
    jit_attr_setlong(output,_jit_sym_minplanecount,4);
    jit_attr_setlong(output,_jit_sym_maxplanecount,4);
    jit_attr_setlong(output,_jit_sym_mindimcount,2);
    jit_attr_setlong(output,_jit_sym_maxdimcount,2);
    jit_atom_setlong(&a_arr[0], 512);
    jit_atom_setlong(&a_arr[1], 424);
    jit_object_method(output,_jit_sym_maxdim,2,a_arr);
    jit_object_method(output,_jit_sym_mindim,2,a_arr);
    jit_atom_setsym(&a_arr[0], _jit_sym_char);
    jit_object_method(output,_jit_sym_types,1,a_arr);

    
	jit_class_addadornment(s_jit_kinect2_class, mop);

	// add method(s)
	jit_class_addmethod(s_jit_kinect2_class, (method)jit_kinect2_matrix_calc, "matrix_calc", A_CANT, 0);
    jit_class_addmethod(s_jit_kinect2_class, (method)jit_kinect2_deduct_info, "deduct_info", A_CANT, 0);
    
	// finalize class
	jit_class_register(s_jit_kinect2_class);
	return JIT_ERR_NONE;
}


/************************************************************************************/
// Object Life Cycle

t_jit_kinect2 *jit_kinect2_new(void)
{
	t_jit_kinect2	*x = NULL;

	x = (t_jit_kinect2 *)jit_object_alloc(s_jit_kinect2_class);
	if (x) {
#if DEBUG
        object_post((t_object*)x, "%s", "DEVELOPMENT VERSION FOR DEBUGGING");
#endif
        x->kinect = new Grab();
        object_post((t_object*)x, "%s", "Initialized");
        x->kinect->open();
        object_post((t_object*)x, "%s", x->kinect->getlasterrorstring().c_str());
    }
    
	return x;
}


void jit_kinect2_free(t_jit_kinect2 *x)
{
    if (x->kinect) {
        x->kinect->close();
        object_post((t_object*)x, "%s", "Closed");
        delete x->kinect;
    }
}


/************************************************************************************/
// Methods bound to input/inlets

t_jit_err jit_kinect2_matrix_calc(t_jit_kinect2 *x, void *inputs, void *outputs)
{
    t_jit_err			err = JIT_ERR_NONE;
    t_jit_matrix_info	in_minfo;
    t_jit_matrix_info	out_minfo;
    char				*out_bp;
    long				i;
    long				dimcount;
    long				planecount;
    long				dim[JIT_MATRIX_MAX_DIMCOUNT];
    long                out_savelock[NUMOFOUTPUTS];
    void *              out_matrix[NUMOFOUTPUTS];
    bool                gotMatrices = true;
    
    // Get matrices of the outputs
    for (i=0;i<NUMOFOUTPUTS;i++)
    {
        if (!(out_matrix[i] = jit_object_method(outputs,_jit_sym_getindex,i)))
        {
            gotMatrices = false;
        }
    }
    
    // If valid object and valid output
    if (x && gotMatrices) {
        // Lock outputs
        for (i = 0; i< NUMOFOUTPUTS; i++)
        {
            out_savelock[i] = (long) jit_object_method(out_matrix[i],_jit_sym_lock,1);
        }
        
        if (!x->kinect->isOpen)
        {
            x->kinect->open();
            object_post((t_object*)x, "%s", x->kinect->getlasterrorstring().c_str());
        }
        if (x->kinect->isOpen)
        {
            libfreenect2::FrameMap framemap = x->kinect->getframes();
            for (i = 0; i< NUMOFOUTPUTS; i++)
            {
                jit_object_method((out_matrix[i]), _jit_sym_getinfo, &out_minfo); // Get output format
                jit_object_method((out_matrix[i]), _jit_sym_getdata, &out_bp); // Get output bitmap char pointer
                
                if (!out_bp) { // If no pointer to output bitmap
                    err=JIT_ERR_INVALID_OUTPUT;
                    goto out;
                }
                
                //get dimensions/planecount
                dimcount   = out_minfo.dimcount;
                planecount = out_minfo.planecount;
                
                FRAMETYPE frametype;
                if(i==0)
                    frametype = Color;
                else if (i==1)
                    frametype = IR;
                else if (i==2)
                    frametype = Depth;
                else
                    frametype = Registration;
                
                libfreenect2::Frame *frame = x->kinect->frame(frametype);

                if(frame)
                {
                    jit_kinect2_deduct_info(frame, &in_minfo);
                    jit_parallel_ndim_simplecalc2((method)jit_kinect2_copy, &frametype, dimcount, dim, planecount, &in_minfo, reinterpret_cast<char*>(frame->data), &out_minfo, out_bp, 0, 0);
                }
            }
            x->kinect->releaseframes();
        } else
            return JIT_ERR_INVALID_PTR;
	}
	else
		return JIT_ERR_INVALID_PTR;

out:
    for (i = 0; i<NUMOFOUTPUTS; i++)
    {
        jit_object_method(out_matrix[i],_jit_sym_lock,out_savelock[i]);
    }
    return err;
}


void jit_kinect2_deduct_info (libfreenect2::Frame* frame, t_jit_matrix_info *info)
{
    jit_matrix_info_default(info);
    if (frame)
    {
        switch (frame->format) {
            case libfreenect2::Frame::BGRX:
                info->planecount = 4; // 4 bytes per pixel (BGRX)
                info->dimcount = 2;
                info->dim[0] = frame->width;
                info->dim[1] = frame->height;
                info->dimstride[0] = info->planecount; // move one pixel right
                info->dimstride[1] = info->planecount * frame->width; // move one pixel down
                info->type = _jit_sym_char;
                info->size = frame->width * frame->height * info->planecount;
                break;

            case libfreenect2::Frame::Float:
                info->planecount = 4; // 4 bytes per pixel (float)
                info->dimcount = 2;
                info->dim[0] = frame->width;
                info->dim[1] = frame->height;
                info->dimstride[0] = info->planecount; // move one pixel right
                info->dimstride[1] = info->planecount * frame->width; // move one pixel down
                info->type = _jit_sym_char;
                info->size = frame->width * frame->height * info->planecount;
                break;
                
            default:
                break;
        }
    }
}

void jit_kinect2_copy(void* type, long dimcount, long *dim, long planecount, t_jit_matrix_info *in_minfo, char *bip,t_jit_matrix_info *out_minfo, char *bop, t_jit_parallel_ndim_worker *para_worker)
{
    long i,j,k;
    if (dimcount<1) return; //safety
    if (!bip || ! bop) return; // safety
    
    FRAMETYPE frametype = *(FRAMETYPE *)type;
    
    // OFFSET para
    bip += para_worker->offset[1] * in_minfo->dimstride[0];
    bop += para_worker->offset[1] * out_minfo->dimstride[0];
    
    long width = in_minfo->dim[0];
    long height = in_minfo->dim[1];
    
    for(i=0; i < height; i++) // for each row
    {
        for(j=0; j < width; j++)  // go across each column
        {
            switch (frametype)
            {
                case Color:
                    if (in_minfo->type == _jit_sym_char) {
                        for(k=0; k < in_minfo->planecount; k++)  // go across each column
                        {
                            if(k==0)
                                bop[j*out_minfo->planecount+k] = 0x00;
                            else
                                bop[j*out_minfo->planecount+k] = ((unsigned char*)bip)[j*in_minfo->planecount+in_minfo->planecount-k-1];
                        }
                    }
                    break;
                case Registration:
                    if (in_minfo->type == _jit_sym_char) {
                        for(k=0; k < in_minfo->planecount; k++)  // go across each column
                        {
                            if(k==0)
                                bop[j*out_minfo->planecount+k] = 0x00;
                            else
                                bop[j*out_minfo->planecount+k] = ((unsigned char*)bip)[j*in_minfo->planecount+in_minfo->planecount-k-1];
                        }
                    }
                    break;
                case IR:
                    if (in_minfo->type == _jit_sym_char) {
                        bop[j] = ((Float32*)bip)[j]/256;
                    }
                    break;
                case Depth:
                    if (in_minfo->type == _jit_sym_char) {
                        
                        float dist = 256.0f* (512.0f/((Float32*)bip)[j]);
                        bool invalid = dist <= 0 || dist == INFINITY || dist == NAN;
                        for(k=0; k < in_minfo->planecount; k++)  // go across each column
                        {
                            if(k==0)
                                bop[j*out_minfo->planecount+k] = invalid ? 0xFF : 0x00;
                            else if (k==1)
                                bop[j*out_minfo->planecount+k] = invalid ? 0x00 : fabsf(dist);
                            else if (k==2)
                                bop[j*out_minfo->planecount+k] = invalid ? 0x00 : fabsf(dist);
                            else if (k==3)
                                bop[j*out_minfo->planecount+k] = invalid ? 0x00 : fabsf(dist);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        bop += out_minfo->dimstride[1];//*yscale;
        bip += in_minfo->dimstride[1];//*yscale;
    }
    //std::cout << "Drawn: " << width << " x " << height << std::endl;
}
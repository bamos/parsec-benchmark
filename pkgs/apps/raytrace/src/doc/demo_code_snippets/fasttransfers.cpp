/*
 *
 * Example code to demonstrate the benefits of PBOs for fast
 * transfers to the GPU.
 *
 * (c) 2006 Dominik Göddeke, University of Dortmund
 * dominik.goeddeke@math.uni-dortmund.de
 *
 * For details, please refer to the tutorial available at
 * www.mathematik.uni-dortmund.de/~goeddeke/gpgpu/tutorial3.html
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <time.h>
#include <assert.h>




/*********************************************************
 *    macros, problem size definition etc.               *
 *********************************************************/



// PBO macro (see spec for details)
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// array size N is texsize*texsize
static const int texSize = 1025;
static const int N = texSize*texSize;

// hard-coded for shader + Cg, do not change this
static const int numArraysPerChunk = 9;
// input data size, default setting results in approx 1.6 GB of phys mem.
static const int numChunks = 20;
// number of iterations per chunk to emulate kernel workload
static const int numKernelSteps = 1;
// number of iterations the test is repeated, to reduce timing noise.
static const int numIterations = 10;


// uncomment the following to check for GL and Cg errors
// Note that enabling this creates an (artificial) sync point (by glGetError())
//#define DEBUGDEBUG


//
// Enable this #define to enable PBOs for glTexSubImage() and glReadPixels().
// Otherwise, PBOs are not used.
//
#define USE_PBO


// my own memcheck macro, only works on linux. Use with care, this will
// slow down the application significantly.
#define MEMCHECK() system("top -b -n1 | grep test | awk '{print $5\" \"$6}'")


/****************************************************************************/

//
// forward declaration of some functions
//
void cgErrorCallback(void);
void checkErrors(const char *label);
void copy(double* in, float* out);
void copy(double* in, void* out);
void copy(void* in, double* out);


/*****************************************************/

//
// fragment program: out = accum + vec
//
const char* kernelSource = " float main(                    "\
        "            uniform samplerRECT accum,             " \
        "            uniform samplerRECT vec1,              " \
        "            uniform samplerRECT vec2,              " \
        "            uniform samplerRECT vec3,              " \
        "            uniform samplerRECT vec4,              " \
        "            uniform samplerRECT vec5,              " \
        "            uniform samplerRECT vec6,              " \
        "            uniform samplerRECT vec7,              " \
        "            uniform samplerRECT vec8,              " \
        "            uniform samplerRECT vec9,              " \
        "            in float4 coords:TEXCOORD0) : COLOR {  " \
    "  return texRECT(accum,coords.xy) +                " \
        "  texRECT(vec1,coords.xy)  +                       " \
        "  texRECT(vec2,coords.xy)  +                       " \
        "  texRECT(vec3,coords.xy)  +                       " \
        "  texRECT(vec4,coords.xy)  +                       " \
        "  texRECT(vec5,coords.xy)  +                       " \
        "  texRECT(vec6,coords.xy)  +                       " \
        "  texRECT(vec7,coords.xy)  +                       " \
        "  texRECT(vec8,coords.xy)  +                       " \
        "  texRECT(vec9,coords.xy); }                       ";

#ifdef DEBUGDEBUG
//
// Cg error callback
//
void cgErrorCallback(void) {
    CGerror lastError = cgGetError();
    if(lastError)
        printf(cgGetErrorString(lastError));
    exit(-1);
}

//
// GL error checking
//
void checkErrors(const char *label) {
    GLenum errCode;
    const GLubyte *errStr;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errStr = gluErrorString(errCode);
    printf("%s: OpenGL ERROR ",label);
    printf((char*)errStr);
        printf("\n");
    exit(-2);
    }
}
#endif


//
// Conversion functions from double to float, the versions
// using the void pointers are used with PBOs.
// For details, see below where PBOs are actually used.
// Note that I rely on the compiler to memalign these properly.
// Whoever knows a way to do this operation for variable N
// (not compile-time constant) in the most efficient way,
// please drop me a note.
//
void copy(double* in, float* out) {
    for (int i=0; i<N; i++)
    out[i] = (float)in[i];
}
void copy(double* in, void* out) {
    float* bla = (float*)out;
    for (int i=0; i<N; i++)
    bla[i] = (float)in[i];
}
void copy(void* in, double* out) {
    float* bla = (float*)in;
    for (int i=0; i<N; i++)
    out[i] = bla[i];
}


//
// this does all the relevant things
//
int main(int argc, char **argv) {
    assert (numArraysPerChunk == 9); // because kernel is hard coded
    //
    // set up glut to get valid GL context and
    // get extension entry points
    //
    glutInit (&argc, argv);
    glutCreateWindow("STREAMING TUTORIAL");
    glewInit();
    //
    // viewport transform for 1:1 'pixel=texel=data' mapping
    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,texSize,0.0,texSize);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0,0,texSize,texSize);
    //
    // create FBO and bind it
    //
    GLuint fb;
    glGenFramebuffersEXT(1,&fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fb);
    //
    // create a whole bunch of double precision data
    //
    printf("---------------------------------------------\n");
    printf("Creating input data, if the following amount \n");
    printf("is greater than the physically available RAM,\n");
    printf("press CTRL-C immediately to avoid HD paging. \n");
    // double precision arrays
    double memrequirements = (double)(numChunks*numArraysPerChunk + numChunks)*N*sizeof(double);
    // single precision textures
    memrequirements += (numArraysPerChunk + 2) * N * sizeof(float);
    // convert to megabytes
    memrequirements /= (1024.0 * 1024.0);
    printf("Allocating %.2f MB.\n",memrequirements);
#ifdef USE_PBO
    printf("PBO transfers are enabled.\n");
#else
    printf("PBO transfers are disabled.\n");
#endif
    // array to store intial value and final result, initially zeroed
    double* data = new double[N];
    for (int i=0; i<N; i++)
    data[i] = 0.0;
    // input arrays: arrays in chunk one contain values from 1..9,
    // in chunk two from 10..19, and so on
    double*** chunks = new double**[numChunks];
    for (int chunk=0; chunk<numChunks; chunk++) {
    chunks[chunk] = new double*[numArraysPerChunk];
    for (int array=0; array<numArraysPerChunk; array++) {
        chunks[chunk][array] = new double[N];
        for (int i=0; i<N; i++)
        chunks[chunk][array][i] = chunk*10.0 + array + 1;
//        printf("%d: %.2f\n",array,chunks[chunk][array][88]);
    }
    }
    //
    // local storage for conversion
    //
    float* tmpfloat = new float[N];
    //
    // create necessary textures:
    // we need two textures to pingpong the iteration,
    // and one set of input textures (numArraysPerChunk).
    // Pingpong textures are zeroed here already, input textures
    // are just allocated.
    // We could allocate textures for all input arrays (numChunks times
    // numArraysPerChunk), but memory is a precious resource. Allocating
    // them increase memory requirements by factor of 1.5 (in addition to
    // the double precision arrays), since glTexImage() implies a
    // malloc() in driver controlled memory.
    //
    GLuint pingtex[2];
    glGenTextures(2,pingtex);
    // set texture parameters for pingpong textures and zero them
    copy(data,tmpfloat);
    for (int i=0; i<2; i++) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB,pingtex[i]);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_FLOAT_R32_NV,texSize,texSize,0,GL_LUMINANCE,GL_FLOAT,tmpfloat);
    }
    // create numArraysPerChunk input textures, set parameters and
    // allocate space. do not populate them!
    GLuint inputTextures[numArraysPerChunk];
    glGenTextures (numArraysPerChunk, inputTextures);
    for (int array=0; array<numArraysPerChunk; array++) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB,inputTextures[array]);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_FLOAT_R32_NV,texSize,texSize,0,GL_LUMINANCE,GL_FLOAT,NULL);
    }
#ifdef DEBUGDEBUG
    checkErrors("create textures");
#endif
#ifdef USE_PBO
    //
    // create buffer objects (nine for download, one for readback)
    //
    GLuint ioBuf[10];
    glGenBuffers(10, ioBuf);
#endif
    //
    // set up Cg runtime
    //
    CGcontext cgContext = cgCreateContext();
#ifdef DEBUGDEBUG
    cgSetErrorCallback(cgErrorCallback);
#endif
    CGprofile fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(fragmentProfile);
    //
    // create kernel program and load it
    //
    CGprogram kernel = cgCreateProgram (cgContext,CG_SOURCE,kernelSource,fragmentProfile,NULL,NULL);
    cgGLLoadProgram (kernel);
    CGparameter accumParam = cgGetNamedParameter (kernel,"accum");
    CGparameter *params = new CGparameter[numArraysPerChunk];
    params[0] = cgGetNamedParameter (kernel,"vec1");
    params[1] = cgGetNamedParameter (kernel,"vec2");
    params[2] = cgGetNamedParameter (kernel,"vec3");
    params[3] = cgGetNamedParameter (kernel,"vec4");
    params[4] = cgGetNamedParameter (kernel,"vec5");
    params[5] = cgGetNamedParameter (kernel,"vec6");
    params[6] = cgGetNamedParameter (kernel,"vec7");
    params[7] = cgGetNamedParameter (kernel,"vec8");
    params[8] = cgGetNamedParameter (kernel,"vec9");
    cgGLEnableProfile(fragmentProfile);
    cgGLBindProgram(kernel);
    //
    // attach ping pong textures to FBO,
    // init pingpong management variables
    //
    GLuint readTex = 0;
    GLuint writeTex = 1;
    GLenum attachmentpoints[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[readTex], GL_TEXTURE_RECTANGLE_ARB,pingtex[readTex],0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[writeTex], GL_TEXTURE_RECTANGLE_ARB,pingtex[writeTex],0);
#ifdef DEBUGDEBUG
    checkErrors("attaching");
#endif
    //
    // perform computation by looping over all chunks of input data,
    // streaming them in as we proceed.
    //
    printf("Starting calculation\n");
    glFinish();
    double start_compute = clock();
    for (int iter=0; iter<numIterations; iter++) {
    for (int chunk=0; chunk<numChunks; chunk++) {
        //
        // populate input textures for current chunk and transfer
        // data to driver-controlled memory.
        //
        for (int array=0; array<numArraysPerChunk; array++) {
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, inputTextures[array]);
#ifdef USE_PBO
        //
        // bind buffer object
        //
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, ioBuf[array]);
        //
        // invalidate this buffer object by passing NULL as data
        //
        // Note: according to the spec at
        // http://www.opengl.org/registry/specs/ARB/vertex_buffer_object.txt
        // GL_STREAM_DRAW hints at the driver that the data store contents
        // will be specified once by the application, and used at most a
        // few times as the source of a GL (drawing) command.
        glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,texSize*texSize*sizeof(float), NULL, GL_STREAM_DRAW);
        //
        // Acquire a pointer to the first data item in this buffer object
        // by mapping it to the so-called unpack buffer target. The unpack
        // buffer refers to a location in memory that gets "unpacked"
        // from CPU (main) memory into GPU memory, hence the somewhat
        // confusing language.
        // Important: This is a pointer to a chunk of memory in what I
        // like to call "driver-controlled memory". Depending on the
        // driver, this might be PCIe/AGP memory, or ideally onboard
        // memory.
        // GL_WRITE_ONLY tells the GL that while we have control of the
        // memory, we will access it write-only. This allows for internal
        // optimisations and increases our chances to get good performance.
        // If we mapped this buffer read/write, it would almost always be
        // located in system memory, from which reading is much faster.
        //
        void* ioMem = glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
        assert(ioMem); // we are in trouble
        //
        // "memcpy" the double precision array into the driver memory,
        // doing the explicit conversion to single precision.
        //
        copy(chunks[chunk][array], ioMem);
        //
        // release memory, i.e. give control back to the driver
        //
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
        //
        // Since the buffer object is still bound, this call populates our
        // texture by sourcing the buffer object. In effect, this means
        // that ideally no actual memcpy takes place (if the driver
        // decided to map the buffer in onboard memory previously), or at
        // most one memcpy inside driver-controlled memory which is much
        // faster since the previous copy of our data into driver-
        // controlled memory already resulted in laying out the data for
        // optimal performance.
        // In other words: This call is at most a real DMA transfer,
        // without any (expensive) interference by the CPU.
        //
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, texSize, texSize, GL_LUMINANCE, GL_FLOAT, BUFFER_OFFSET(0));
        //
        // Unbind buffer object by binding it to zero.
        // This call is crucial, as doing the following computations while
        // the buffer object is still bound will result in all sorts of
        // weird side effects, eventually even delivering wrong results.
        // Refer to the specification to learn more about which
        // GL calls act differently while a buffer is bound.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
#else
        //
        // Do a conventional texture population.
        // Note that this version implies a series of non-DMA memory
        // operations:
        // - conversion from double to single precision is done
        //   explicitely by the CPU
        // - glTexSubImage() implies a memcpy into driver-controlled
        //   memory.
        // - glTexSubImage() implies rearranging the data as the driver
        //   deems necessary.
        //
        // The version using PBOs is hence faster since the CPU is only
        // involved once in the entire process, converting and
        // rearranging in one go.
        // We do definitely save on one memcpy, and eventually a second one
        // if the driver does not decide to DMA the data anyway after we
        // release the pointer / unmap the buffer.
        copy (chunks[chunk][array],tmpfloat);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, texSize, texSize, GL_LUMINANCE, GL_FLOAT, tmpfloat);
#endif
        //
        // assign texture as input to shader
        //
        cgGLSetTextureParameter(params[array], inputTextures[array]);
        cgGLEnableTextureParameter(params[array]);
        }
        for (int iter=0; iter<numKernelSteps; iter++) {
        //
        // assign accum texture (input pingpong) to kernel
        //
        cgGLSetTextureParameter(accumParam, pingtex[readTex]);
        cgGLEnableTextureParameter(accumParam);
        //
        // redirect output to write-only pingpong texture
        //
        glDrawBuffer(attachmentpoints[writeTex]);
        //
        // render viewport-sized quad to perform actual computation
        //
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
        glTexCoord2f(texSize, 0.0); glVertex2f(texSize, 0.0);
        glTexCoord2f(texSize, texSize); glVertex2f(texSize,texSize);
        glTexCoord2f(0.0, texSize); glVertex2f(0.0, texSize);
        glEnd();
        //
        // swap pingpong input and output textures
        readTex = 1-readTex;
        writeTex = 1-writeTex;
        }
    }
    }
#ifdef DEBUGDEBUG
    checkErrors("compute");
#endif
    glFinish();
    double end_compute = clock();
    //
    // Read back results (into double precision arrays).
    //
    printf("Reading back results\n");
    glFinish();
    double start_readback = clock();
#ifdef USE_PBO
    //
    // Readback using PBOs is performed in a similar way as download is.
    // First, we set one texture attached to our PBO as source for the
    // readback and bind the buffer to the pixel-pack target.
    // Note: PBOs 0--8 were used during download, hence #9
    //
    glReadBuffer(attachmentpoints[readTex]);
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, ioBuf[9]);
    //
    // invalidate this buffer by passing NULL
    //
    // Note: according to the spec at
    // http://www.opengl.org/registry/specs/ARB/vertex_buffer_object.txt
    // GL_STREAM_READ hints at the driver that the data store contents
    // will be specified once by the application, and used at most a few
    // times by the application, which is exactly what we will do.
    //
    glBufferData(GL_PIXEL_PACK_BUFFER_ARB,texSize*texSize*sizeof(float), NULL, GL_STREAM_READ);
    //
    // In contrast to conventional glReadPixels(), this call returns
    // immediately and just triggers a DMA transfer into the buffer object we
    // allocated preciously.
    //
    glReadPixels (0, 0, texSize, texSize, GL_LUMINANCE, GL_FLOAT, BUFFER_OFFSET(0));
    //
    // Readbacks are only asynchroneous on the CPU side (since the above call
    // returns immediately). To take advantage of this, a real application
    // would have to perform a lot of independent work on some other data
    // while the data is DMA'ed in the background.
    //
    // In our case, there is no work to be done, so we acquire a pointer to
    // the data by mapping the buffer to the pixel pack target. We again
    // indicate what we want to do with the pointer (accessing the data
    // read-only). Note that this call will block until the data is available.
    //
    void* mem = glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
    assert(mem);
    //
    // convert data back to double precision since the underlying "application"
    // will continue using the data in double precision.
    //
    copy(mem,data);
    //
    // Release pointer and buffer object back to driver control, and unbind the
    // buffer object
    //
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
#else
    //
    // The conventional glReadPixels() will block until all data has been
    // copied into the array we pass in as last argument.
    // This includes the transfer and the eventual rearrangement back to a
    // CPU-friendly (or better: non-GPU-optimised) layout in memory.
    //
    glReadBuffer(attachmentpoints[readTex]);
    glReadPixels(0, 0, texSize, texSize,GL_LUMINANCE,GL_FLOAT,tmpfloat);
    copy(tmpfloat, data);
#endif
    glFinish();
    double end_readback = clock();
#ifdef DEBUGDEBUG
    checkErrors("readback");
#endif
    //
    // do result comparison, exemplarily for array entry 88
    // since I am too lazy to add a random number generator.
    //
    double res = 0;
    for (int iter=0; iter<numIterations; iter++) {
    for (int i=0; i<numChunks; i++)
        for (int j=0; j<numArraysPerChunk; j++)
        for (int k=0; k<numKernelSteps; k++)
            res += i*10.0 + j + 1;
    }
    printf("%d: should be: %.2f, is: %.2f\n",88, res, data[88]);
    //
    // print out timings
    //
    double time_compute  = (end_compute - start_compute)/CLOCKS_PER_SEC;
    double time_readback = (end_readback - start_readback)/CLOCKS_PER_SEC;
    double data_download = (double)N*numChunks*numArraysPerChunk*numIterations*sizeof(double) / (1024.0*1024.0);
    double data_readback = (double)N*sizeof(double) / (1024.0*1024.0);
    printf("DOWNLOAD\n");
    printf("Total amount of doubleprec data : %.2f MB\n", data_download);
    printf("Compute time                    : %.2f sec\n", time_compute);
    printf("Throughput:                     : %.2f MB/s\n", data_download / time_compute);
    printf("READBACK\n");
    printf("Total amount of doubleprec data : %.2f MB\n", data_readback);
    printf("Compute time                    : %.2f sec\n",time_readback);
    printf("Throughput:                     : %.2f MB/s\n", data_readback / time_readback);
    //
    // clean up
    //
    for (int chunk=0; chunk<numChunks; chunk++) {
        for (int array=0; array<numArraysPerChunk; array++)
            delete [] chunks[chunk][array];
        delete [] chunks[chunk];
    }
    delete [] tmpfloat;
    delete [] data;
    delete [] chunks;
    glDeleteFramebuffersEXT (1,&fb);
#ifdef USE_PBO
    glDeleteBuffers(2, ioBuf);
#endif
    glDeleteTextures(numArraysPerChunk,inputTextures);
    glDeleteTextures(2,pingtex);

    return 0;
}



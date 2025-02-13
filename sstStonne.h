#ifndef _SST_STONNE_H
#define _SST_STONNE_H

//sst_stonne.h
#include "include/STONNEModel.h"
#include "include/types.h"
#include "include/MemInterface.h"

#include "lsQueue.h"


namespace SST_STONNE {
class sstStonne {
public:
    sstStonne();
    sstStonne();
    ~sstStonne();

    // Override SST::Component Virtual Methods
    void setup();
    void finish();
    void init( uint32_t phase );
    void handleEvent( SimpleMem::Request* ev );
    //void Status();
    bool cycle();

private:
    //SST Variables
    SimpleMem*  mem_interface_;
    Stonne* stonne_instance;
    Layer_t kernelOperation;

    //Input parameters
    /***************************************************************************/
    /*Convolution parameters (See MAERI paper to find out the taxonomy meaning)*/
    /***************************************************************************/
    std::string layer_name;
    unsigned int R;                                  // R
    unsigned int S;                                  // S
    unsigned int C;                                  // C
    unsigned int K;                                  // K
    unsigned int G;                                  // G
    unsigned int N;                                  // N
    unsigned int X;                                  // X //TODO CHECK X=1 and Y=1
    unsigned int Y;                                  // Y
    unsigned int X_;                                 // X_
    unsigned int Y_;                                 // Y_
    unsigned int strides;                            // Strides

    unsigned int matrixA_size;
    unsigned int matrixB_size;
    unsigned int matrixC_size;

    //Convolution Tile parameters (See MAERI paper to find out the taxonomy meaning)
    unsigned int T_R;                                // T_R
    unsigned int T_S;                                // T_S
    unsigned int T_C;                                // T_C
    unsigned int T_K;                                // T_K
    unsigned int T_G;                                // T_G
    unsigned int T_N;                                // T_N
    unsigned int T_X_;                               // T_X
    unsigned int T_Y_;                               // T_Y   


    /******************************************************************************/
    /*  GEMM Parameters */
    /******************************************************************************/
    //Layer parameters
    unsigned int GEMM_K;
    unsigned int GEMM_N;
    unsigned int GEMM_M;

    //Tile parameters
    unsigned int GEMM_T_K;
    unsigned int GEMM_T_N;
    unsigned int GEMM_T_M;

    /**************************************************************************/
    /* Hardware parameters */
    /**************************************************************************/
    Config stonne_cfg; 
    std::string memFileName;
    uint64_t dram_matrixA_address;
    uint64_t dram_matrixB_address;
    uint64_t dram_matrixC_address;

    std::string memMatrixCFileName;

    std::string bitmapMatrixAFileName;
    std::string bitmapMatrixBFileName;

    std::string rowpointerMatrixAFileName;
    std::string colpointerMatrixAFileName;
   
    std::string rowpointerMatrixBFileName;
    std::string colpointerMatrixBFileName;


    /**************************************************************************/
    /* Data pointers */
    /**************************************************************************/

    float* matrixA;  //This is input ifmaps in CONV operation or MK matrix in GEMM operation
    float* matrixB;  //This is filter matrix in CONV operation or KN matrix in GEMM operation
    float* matrixC;  //This is output fmap in CONV operation or resulting MN matrix in GEMM operation

    //These three structures are used to represent the bitmaps in a bitmapSpMSpM operation. The datatype
    //could be smaller to unsigned int (one single bit per element is necessary) but will use this for simplicity 
    //In terms of functionallity the simulation is not affected
    unsigned int* bitmapMatrixA; //This is the bitmap of MK matrix in bitmapSpMSpM operation
    unsigned int* bitmapMatrixB; //This is the bitmap of KN matrix in bitmapSpMSpM operation
    unsigned int* bitmapMatrixC; //This is the bitmap for the resulting matrix in bitmapSpMSpM operation

    unsigned int* rowpointerMatrixA; //This is the row pointer of MK matrix in csrSpMM operation
    unsigned int* colpointerMatrixA; //This is the col pointer of MK matrix in csrSpMM operation

    unsigned int* rowpointerMatrixB;  //This is the pointer of KN matrix in outerProduct operation
    unsigned int* colpointerMatrixB;  //This is the id pointer of KN matrix in outerProduct operation
    
    /**************************************************************************/
    /* Auxiliary variables */
    /**************************************************************************/
    float EPSILON=0.05;
    unsigned int MAX_RANDOM=10; //Variable used to generate the random values

    /**************************************************************************/
    /* Memory Hierarchy structures and variables */
    /**************************************************************************/

    LSQueue* load_queue_;    //This FIFO stores the load requests sent to the memory hirarchy component
    LSQueue* write_queue_;   //This FIFO stores the write requests sent to the memory hierarchy component

    /**************************************************************************/
    /* Private functions */
    /**************************************************************************/
    std::vector< uint32_t >* constructMemory(std::string fileName);
    unsigned int constructBitmap(std::string fileName, unsigned int * array, unsigned int size);
    void dumpMemoryToFile(std::string fileName, float* array, unsigned int size);
    unsigned int constructCSRStructure(std::string fileName, unsigned int * array);
};
}

#endif

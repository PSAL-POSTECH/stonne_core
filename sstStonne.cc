//sstStonne.cc
#include "sstStonne.h"
#include <math.h>
#include <iostream>
#include "include/utility.h"
using namespace SST_STONNE;

//Constructor
sstStonne::sstStonne()  {
    //set up memory interfaces
    //mem_interface_ = ; //FIXME

    write_queue_ = new LSQueue();
    load_queue_ = new LSQueue();
}

sstStonne::~sstStonne() {

}

void sstStonne::init( uint32_t phase )
{
    mem_interface_->init( phase );

    if( 0 == phase ) {
        std::vector< uint32_t >* initVector;

        //Check to see if there is any memory being initialized
        if( memFileName != "" ) {
            initVector = constructMemory(memFileName);
        } else {
            initVector = new std::vector< uint32_t > {16, 64, 32, 0 , 16382, 0, 0};
        }

        std::vector<uint8_t> memInit;
        constexpr auto buff_size = sizeof(uint32_t);
        uint8_t buffer[buff_size] = {};
        for( auto it = initVector->begin(); it != initVector->end(); ++it ) {
            std::memcpy(buffer, std::addressof(*it), buff_size);
            for( uint32_t i = 0; i < buff_size; ++i ){
                memInit.push_back(buffer[i]);
            }
        }

        SimpleMem::Request* initMemory = new SimpleMem::Request(SimpleMem::Request::Write, 0, memInit.size(), memInit);
        mem_interface_->sendInitData(initMemory);
    }
}


bool sstStonne::cycle() {
    stonne_instance->cycle();
    bool work_done = stonne_instance->isExecutionFinished();
    if(work_done) {
        if(this->stonne_cfg.print_stats_enabled) { //If sats printing is enable
            stonne_instance->printStats();
            std::cout << "Stats printed correctly" << std::endl;
            stonne_instance->printEnergy();
        }
        std::cout << "The execution has finished in sstStonne" << std::endl;
    }
    return work_done;
}

void sstStonne::setup() {
    //Creating arrays for this version of the integration
    if((kernelOperation==CONV) || (kernelOperation==GEMM)) { //Initializing dense operation
        switch(kernelOperation) {
            case CONV:
                matrixA_size=N*X*Y*C; //ifmap
                matrixB_size=R*S*(C/G)*K;
                matrixC_size=N*X_*Y_*K;
                break;
            case GEMM:
                matrixA_size=GEMM_M*GEMM_K;
                matrixB_size=GEMM_N*GEMM_K;
                matrixC_size=GEMM_M*GEMM_N;
                break;
            default:
        };
        matrixA = NULL; //TODO: remove when everything is done
        matrixB = NULL;
        matrixC = new float[matrixC_size];
    } //End initializing dense operation

    else { //Initializing sparse operation
        if(kernelOperation==bitmapSpMSpM) {
            if(bitmapMatrixAFileName=="") {

            }
            if(bitmapMatrixBFileName=="") {

            }

            matrixA_size=GEMM_M*GEMM_K;
            matrixB_size=GEMM_N*GEMM_K;
            matrixC_size=GEMM_M*GEMM_N;
            bitmapMatrixA=new unsigned int[matrixA_size];
            bitmapMatrixB=new unsigned int[matrixB_size];
            bitmapMatrixC=new unsigned int[matrixC_size];
            unsigned int nActiveValuesA=constructBitmap(bitmapMatrixAFileName, bitmapMatrixA, matrixA_size);
            unsigned int nActiveValuesB=constructBitmap(bitmapMatrixBFileName, bitmapMatrixB, matrixB_size);
            matrixC=new float[matrixC_size];
        } //End bitmapSpMSpM operation

        else if(kernelOperation==csrSpMM) {
            if(rowpointerMatrixAFileName=="") {
            }
            if(colpointerMatrixAFileName=="") {
            }
            matrixA_size=GEMM_M*GEMM_K;
            matrixB_size=GEMM_N*GEMM_K;
            matrixC_size=GEMM_M*GEMM_N;

            rowpointerMatrixA=new unsigned int[matrixA_size]; //Change to the minimum using vector class
            colpointerMatrixA=new unsigned int[matrixA_size];
            unsigned int nValuesRowPointer=constructCSRStructure(rowpointerMatrixAFileName,rowpointerMatrixA);
            unsigned int nValuesColPointer=constructCSRStructure(colpointerMatrixAFileName, colpointerMatrixA);
            matrixA=NULL; //TODO fix this
            matrixB=NULL;
            matrixC=new float[matrixC_size];
            // Data is not mandatory

        } //End csrSpMM operation

        else if((kernelOperation==outerProductGEMM) || (kernelOperation==gustavsonsGEMM)) {
            matrixA_size=GEMM_M*GEMM_K;
            matrixB_size=GEMM_N*GEMM_K;
            matrixC_size=GEMM_M*GEMM_N;

            rowpointerMatrixA=new unsigned int[matrixA_size+1]; //Change to the minimum using vector class
            colpointerMatrixA=new unsigned int[matrixA_size+1];

            rowpointerMatrixB = new unsigned int[matrixB_size+1];
            colpointerMatrixB = new unsigned int[matrixB_size+1];

            unsigned int nValuesRowPointerA=constructCSRStructure(rowpointerMatrixAFileName,rowpointerMatrixA);
            unsigned int nValuesColPointerA=constructCSRStructure(colpointerMatrixAFileName, colpointerMatrixA);

            unsigned int nValuesRowPointerB=constructCSRStructure(rowpointerMatrixBFileName, rowpointerMatrixB);
            unsigned int nValuesColPointerB=constructCSRStructure(colpointerMatrixBFileName, colpointerMatrixB);
            matrixA=NULL; //TODO fix this
            matrixB=NULL;
            matrixC=new float[matrixC_size];
            // Data is not mandatory
        }
    }

    //Updating hardware parameters
    stonne_instance = new Stonne(stonne_cfg, load_queue_, write_queue_, mem_interface_);

    switch(kernelOperation) {
        case CONV:
            stonne_instance->loadDNNLayer(CONV, layer_name, R, S, C, K, G, N, X, Y, strides, matrixA, matrixB, matrixC, CNN_DATAFLOW); //Loading the layer
            stonne_instance->loadTile(T_R, T_S, T_C, T_K, T_G, T_N, T_X_, T_Y_);
            break;
        case GEMM:
            stonne_instance->loadDenseGEMM(layer_name, GEMM_N, GEMM_K, GEMM_M, matrixA, matrixB, matrixC, CNN_DATAFLOW);
            stonne_instance->loadGEMMTile(GEMM_T_N, GEMM_T_K, GEMM_T_M);
                break;
        case bitmapSpMSpM:
            stonne_instance->loadGEMM(layer_name, GEMM_N, GEMM_K, GEMM_M, matrixA, matrixB, bitmapMatrixA, bitmapMatrixB, matrixC, bitmapMatrixC, MK_STA_KN_STR );
            break;
        case csrSpMM:
            stonne_instance->loadSparseDense(layer_name, GEMM_N, GEMM_K, GEMM_M, matrixA, matrixB, colpointerMatrixA, rowpointerMatrixA, matrixC, GEMM_T_N, GEMM_T_K);
            break;
        case outerProductGEMM:
            stonne_instance->loadSparseOuterProduct(layer_name, GEMM_N, GEMM_K, GEMM_M, matrixA, matrixB, colpointerMatrixA, rowpointerMatrixA, colpointerMatrixB, rowpointerMatrixB, matrixC);
            break;
        case gustavsonsGEMM:
            stonne_instance->loadSparseOuterProduct(layer_name, GEMM_N, GEMM_K, GEMM_M, matrixA, matrixB, colpointerMatrixA, rowpointerMatrixA, colpointerMatrixB, rowpointerMatrixB, matrixC);
            break;
        default:
            break;
    };
}

std::vector< uint32_t >* sstStonne::constructMemory(std::string fileName) { //In the future version this will be directly simulated memory
    std::vector< uint32_t >* tempVector = new std::vector< uint32_t >;
    std::ifstream inputStream(fileName, std::ios::in);
    if( inputStream.is_open() ) {
        std::string thisLine;
        while( std::getline( inputStream, thisLine ) )
        {
            std::string value;
            std::stringstream stringIn(thisLine);
            while( std::getline(stringIn, value, ',') ) {
                tempVector->push_back(std::stoul(value));
            }
        }
        inputStream.close();
    } else {
        exit(0);
    }
    return tempVector;
}

//Return number of active elements to build later the data array. This is necessary because we do not know the number of elements.
unsigned int sstStonne::constructBitmap(std::string fileName, unsigned int * array, unsigned int size) { //In the future version this will be directly simulated memory
    std::ifstream inputStream(fileName, std::ios::in);
    unsigned int currentIndex=0;
    unsigned int nActiveValues=0;
    if( inputStream.is_open() ) {
        std::string thisLine;
        while( std::getline( inputStream, thisLine ) )
        {
            std::string value;
            std::stringstream stringIn(thisLine);
            while( std::getline(stringIn, value, ',') ) {
                array[currentIndex]=stoi(value);
                currentIndex++;
                if(stoi(value)==1) {
                        nActiveValues++;
                }
            }
        }
        inputStream.close();
    } else {
            exit(0);
    }
    return nActiveValues;
}

//Return number of active elements to build later the data array. This is necessary because we do not know the number of elements.
unsigned int sstStonne::constructCSRStructure(std::string fileName, unsigned int * array) { //In the future version this will be directly simulated memory
    std::ifstream inputStream(fileName, std::ios::in);
    unsigned int currentIndex=0;
    if( inputStream.is_open() ) {
        std::string thisLine;
        while( std::getline( inputStream, thisLine ) )
        {
            std::string value;
            std::stringstream stringIn(thisLine);
            while( std::getline(stringIn, value, ',') ) {
                array[currentIndex]=stoi(value);
                currentIndex++;
            }
        }
        inputStream.close();
    } else {
        exit(0);
    }
    return currentIndex;
}

void sstStonne::finish() {
    //This code should have the logic to write the output memory into a certain file passed by parameter. TODO
    dumpMemoryToFile(memMatrixCFileName, matrixC, matrixC_size);
    std::cout << "The file is dumped" << std::endl;
    //delete stonne_instance;
    //delete[] matrixC;

    if(kernelOperation==bitmapSpMSpM) {
        delete[] bitmapMatrixA;
        delete[] bitmapMatrixB;
    }

    else if(kernelOperation==csrSpMM) {
        delete[] rowpointerMatrixA;
        delete[] colpointerMatrixA;
    }
    else if((kernelOperation==outerProductGEMM) || (kernelOperation==gustavsonsGEMM)) {
        delete[] rowpointerMatrixA;
        delete[] colpointerMatrixA;
        delete[] rowpointerMatrixB;
        delete[] colpointerMatrixB;
    }
    std::cout << "The finish function ends" << std::endl;
}

void sstStonne::dumpMemoryToFile(std::string fileName, float* array, unsigned int size) {
    if(fileName != "") {
        std::ofstream outputStream (fileName, std::ios::out);
        if( outputStream.is_open()) {
            for(unsigned i=0; i<size; i++) {
                float value = array[i];
                outputStream << std::fixed << std::setprecision(1) << value << ",";
            }
            outputStream.close();
        }
        else {
            exit(0);
        }
    }
}

void sstStonne::handleEvent( SimpleMem::Request* ev ) {
    if( ev->cmd == SimpleMem::Request::Command::ReadResp ) {
        // Read request needs some special handling
        uint64_t addr = ev->addr;
        data_t memValue = 0.0;

        std::memcpy( std::addressof(memValue), std::addressof(ev->data[0]), sizeof(memValue) );
        //std::cout << "Response to read addr " << addr << " has arrived with data " << memValue << std::endl;
        //output_->verbose(CALL_INFO, 8, 0, "Response to a read, payload=%" PRIu64 ", for addr: %" PRIu64
        //               " to PE %" PRIu32 "\n", memValue, addr, ls_queue_->lookupEntry( ev->id ).second );
        load_queue_->setEntryData( ev->id, memValue);
        load_queue_->setEntryReady( ev->id, 1 );
    } else {
        //output_->verbose(CALL_INFO, 8, 0, "Response to a write for addr: %" PRIu64 " to PE %" PRIu32 "\n",
        //                 ev->addr, ls_queue_->lookupEntry( ev->id ).second );
        write_queue_->setEntryReady( ev->id, 1 );
    }

    // Need to clean up the events coming back from the cache
    delete ev;
}

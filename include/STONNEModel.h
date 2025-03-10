#ifndef STONNEMODEL_H_
#define STONNEMODEL_H_

#include <string>
//#include "RSNetwork.h"
#include "MSNetwork.h"
#include "DSNetworkTop.h"
#include "ASNetwork.h"
#include "SDMemory.h"
#include "Connection.h"
#include "LookupTable.h"
#include "CollectionBus.h"
#include "Config.h"
#include "CompilerART.h"
#include "CompilerMSN.h"
#include "ReduceNetwork.h"
#include "DistributionNetwork.h"
#include "FENetwork.h"
#include "MemoryController.h"
#include "SparseSDMemory.h"
#include "SparseDenseSDMemory.h"
#include "TemporalRN.h"
#include "OSMeshSDMemory.h"
#include "OSMeshMN.h"
#include "SparseFlex_MSNetwork.h"
#include "SparseFlex_ASNetwork.h"
#include "OuterLoopSpGEMMSDMemory.h"
#include "GustavsonsSpGEMMSDMemory.h"
#include "lsQueue.h"

class Stonne {
private:
    //Hardware paramenters
    Config stonne_cfg;
    unsigned int ms_size; //Number of multipliers
    unsigned int n_adders; //Number of adders obtained from ms_size
    DistributionNetwork* dsnet; //Distribution Network
    MultiplierNetwork* msnet; //Multiplier Network
    ReduceNetwork* asnet; //ART Network
    LookupTable* lt; //Lookuptable
    MemoryController* mem; //MemoryController abstraction (e.g., SDMemory from MAERI)
    Bus* collectionBusRN = NULL; //CollectionBus
    Bus* collectionBusMN = NULL;
    Connection* outputASConnection = NULL; //The last connection of the AS and input to the lookuptable
    Connection* outputLTConnection = NULL; //Output of the lookup table connection and write port to the SDMemory
    Connection** addersBusConnections = NULL; //Array of output connections between the adders and the bus
    Connection** BusMemoryConnections = NULL; //Array of output Connections between the bus and the memory. (Write output ports)

    //Software parameters
    DNNLayer* dnn_layer=NULL;
    STONNE_Tile* current_tile=NULL;
    bool layer_loaded; //Indicates if the function loadDNN
    bool tile_loaded; 

    //Connection and cycle functions
    void connectMemoryandDSN(); 
    void connectMSNandDSN(); //Function to connect the multiplieers of the MSN to the last level switches in the DSN.
    void connectMSNandASN();
    void connectASNandBus(); //Connect the adders to the Collection bus
    void connectMSNandBus();
    void connectBusandMemory(); //Connect the bus and the memory write ports.
    void printGlobalStats(std::ofstream& out, unsigned int indent);
   
    // DEBUG PARAMETERS
    unsigned long time_ds;
    unsigned long time_ms;
    unsigned long time_as;
    unsigned long time_lt;
    unsigned long time_mem;
    //DEBUG functions
    void testDSNetwork(unsigned int num_ms);
    void testTile(unsigned int num_ms);
    void testMemory(unsigned int num_ms);

    //Statistics
    unsigned int n_cycles;   

    //SST variables and structures
    SST_STONNE::LSQueue* load_queue_;
    SST_STONNE::LSQueue* write_queue_;
    SimpleMem*  mem_interface_;

   
public:
    Stonne (Config stonne_cfg, SST_STONNE::LSQueue* load_queue_, SST_STONNE::LSQueue* write_queue_, SimpleMem*  mem_interface_);
    ~Stonne();

    void loadDNNLayer(Layer_t layer_type, std::string layer_name, unsigned int R, unsigned int S, unsigned int C, unsigned int K, unsigned int G,  unsigned int N, unsigned int X, unsigned int Y, unsigned int strides, address_t input_address, address_t filter_address, address_t output_address, Dataflow dataflow); //General constructor
  
   //Load CONV Layer. At the end this calls to the general constructor  with all the parameters
    void loadCONVLayer(std::string layer_name, unsigned int R, unsigned int S, unsigned int C, unsigned int K, unsigned int G, unsigned int N, unsigned int X, unsigned int Y, unsigned int strides, address_t input_address, address_t filter_address, address_t output_address);

    //Load FC layer just with the appropiate parameters
    //N = batch size (i.e., number of rows in input matrix); S=number of inputs per batch (i.e., column size in input matrix and weight matrix); K=number of outputs neurons (i.e, number of rows weight matrix)
    void loadFCLayer(std::string layer_name, unsigned int N, unsigned int S, unsigned int K, address_t input_address, address_t filter_address, address_t output_address); 

    //Load Sparse GEMM onto STONNE according to SIGMA parameter taxonomy. 
    void loadGEMM(std::string layer_name, unsigned int N, unsigned int K, unsigned int M, address_t MK_matrix, address_t KN_matrix, metadata_address_t MK_metadata, metadata_address_t KN_metadata, address_t output_matrix, metadata_address_t output_metadata, Dataflow dataflow);

   //Load Dense GEMM onto STONNE according to SIGMA parameter taxonomy and tiling according to T_N, T_K and T_M
   void loadDenseGEMM(std::string layer_name, unsigned int N, unsigned int K, unsigned int M, address_t MK_matrix, address_t KN_matrix, address_t output_matrix, Dataflow dataflow);

    //Load sparse-dense GEMM onto STONNE
    void loadSparseDense(std::string layer_name, unsigned int N, unsigned int K, unsigned int M, address_t MK_matrix, address_t KN_matrix, metadata_address_t MK_metadata_id, metadata_address_t MK_metadata_pointer, address_t output_matrix, unsigned int T_N, unsigned int T_K);

    void loadSparseOuterProduct(std::string layer_name, unsigned int N, unsigned int K, unsigned int M, address_t MK_matrix, address_t KN_matrix, metadata_address_t MK_metadata_id, metadata_address_t MK_metadata_pointer, metadata_address_t KN_metadata_id, metadata_address_t KN_metadata_pointer, address_t output_matrix);

    //Load a Dense GEMM tile to run it using the loadDenseGEMM function
    void loadGEMMTile(unsigned int T_N, unsigned int T_K, unsigned int T_M);
    void loadAddress(uint64_t input_loc, uint64_t filter_loc, uint64_t output_loc) { mem->loadAddress(input_loc, filter_loc, output_loc); }
    void loadTile(unsigned int T_R, unsigned int T_S, unsigned int T_C, unsigned int T_K, unsigned int T_G, unsigned int T_N, unsigned int T_X_, unsigned int T_Y_); //Load general and CONV tile
    void loadFCTile(unsigned int T_S, unsigned int T_N, unsigned int T_K); //VNSize = T_S, NumVNs= T_N*T_K
    void run();
    void cycle();
    void printStats();
    void printEnergy();
    bool isExecutionFinished();
    int getMCFSMStatus() { return mem->getFSMStatus(); }
    MSwitchStats getMSstat() { return msnet->getStats(); }
};

#endif
//TO DO add enumerate.

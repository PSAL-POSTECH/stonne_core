//Created by Francisco Munoz Martinez on 02/07/2019
#ifndef __MEMORYCONTROLLER__H__
#define __MEMORYCONTROLLER__H__

#include <list>
#include <cstring>
#include "Tile.h"
#include "Connection.h"
#include "Fifo.h"
#include "types.h"
#include "DNNLayer.h"
#include "Unit.h"
#include "Config.h"
#include "DataPackage.h"
#include "Stats.h"
#include <assert.h>
#include "ReduceNetwork.h"
#include "MultiplierNetwork.h"
#include "lsQueue.h"

class MemoryController : Unit {        
public:
    MemoryController(id_t id, std::string name) : Unit(id, name){}
    virtual void setLayer(DNNLayer* dnn_layer,  address_t input_address, address_t filter_address, address_t output_address, Dataflow dataflow) {assert(false);}
    virtual void setTile(Tile* current_tile) {assert(false);}
    virtual void setReadConnections(std::vector<Connection*> read_connections) {assert(false);}
    virtual void setWriteConnections(std::vector<Connection*> write_port_connections) {assert(false);} //All the write connections must be set at a time
    virtual void setSparseMetadata(metadata_address_t MK_metadata, metadata_address_t KN_metadata, metadata_address_t output_metadata) {assert(false);}
    //Used to configure the ReduceNetwork according to the controller if needed
    virtual void setReduceNetwork(ReduceNetwork* reduce_network) {assert(false);}
    //Used to configure the MultiplierNetwork according to the controller if needed
    virtual void setMultiplierNetwork(MultiplierNetwork* multiplier_network) {assert(false);} 
    virtual void cycle() {assert(false);}
    virtual bool isExecutionFinished() {assert(false);}
    virtual void setDenseSpatialData(unsigned int T_N, unsigned int T_K){assert(false);}
    virtual void setSparseMatrixMetadata(metadata_address_t MK_metadata_id, metadata_address_t MK_metadata_pointer){assert(false);}
    virtual void setSparseMatrixMetadata(metadata_address_t MK_metadata_id, metadata_address_t MK_metadata_pointer, metadata_address_t KN_metadata_id, metadata_address_t KN_metadata_pointer) {assert(false);}
    bool doLoad(uint64_t addr, DataPackage* data_package)
    {
        SimpleMem::Request* req = new SimpleMem::Request(SimpleMem::Request::Read, addr, this->data_width);
        SST_STONNE::LSEntry* tempEntry = new SST_STONNE::LSEntry( req->id, data_package, 0 );
        load_queue_->addEntry( tempEntry );
        if (mem_interface_)
            mem_interface_->sendRequest( req );
        return 1;
    }
    bool doStore(uint64_t addr, DataPackage* data_package)
    {
        SimpleMem::Request* req = new SimpleMem::Request(SimpleMem::Request::Write, addr, 4);
        const auto newValue = data_package->get_data();
        constexpr auto size = sizeof(uint32_t);
        uint8_t buffer[size] = {};
        std::memcpy(buffer, std::addressof(newValue), size);
    
        std::vector< uint8_t > payload(4);
        memcpy( std::addressof(payload[0]), std::addressof(newValue), size );
        req->setPayload( payload );
    
        SST_STONNE::LSEntry* tempEntry = new SST_STONNE::LSEntry( req->id, data_package, 1);
        write_queue_->addEntry( tempEntry );
        if (mem_interface_)
            mem_interface_->sendRequest( req );
        return 1;
    }

    //SST Memory hierarchy component structures and variables
    SST_STONNE::LSQueue* load_queue_;
    SST_STONNE::LSQueue* write_queue_;
    SimpleMem*  mem_interface_ = NULL;

    virtual void printStats(std::ofstream& out, unsigned int indent) {assert(false);}
    virtual void printEnergy(std::ofstream& out, unsigned int indent) {assert(false);}
protected:
    uint32_t data_width;
};


#endif //SDMEMORY_H_

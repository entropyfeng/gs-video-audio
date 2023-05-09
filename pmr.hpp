#pragma once
#include "blockingconcurrentqueue.h"
#include "iostream"
#include <memory_resource>
#include "spdlog/spdlog.h"
class PMRQueue{
public:
    std::pmr::synchronized_pool_resource pool;
    std::pmr::polymorphic_allocator<std::vector<uint8_t>> allocator{&pool};
    moodycamel::BlockingConcurrentQueue<std::shared_ptr<std::vector<uint8_t>>> queue;

    PMRQueue(): queue(moodycamel::BlockingConcurrentQueue<std::shared_ptr<std::vector<uint8_t>>>()){

    };
    std::shared_ptr<std::vector<uint8_t>>prepareBuffer(long size){
        spdlog::info("call prepareBuffer");
        auto obj= allocator.new_object<std::vector<uint8_t>>(size);

        auto ptr=  std::shared_ptr<std::vector<uint8_t>>(obj,[this](std::vector<uint8_t>* ptr){;
            this->allocator.delete_object(ptr);
            std::cout<<"deleter"<<std::endl;
        });
        queue.enqueue(ptr);
        return ptr;
    }

    std::shared_ptr<std::vector<uint8_t>> next(){
        std::shared_ptr<std::vector<uint8_t>> sharedPtr;
        auto x=std::make_shared<std::vector<uint8_t>>();
        queue.wait_dequeue(x);

        return x;
    }
    ~PMRQueue(){
        //pool.release();
    }
};
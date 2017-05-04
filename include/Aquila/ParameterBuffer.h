#pragma once
#include "IParameterBuffer.hpp"
#include "Aquila/types/SyncedMemory.hpp"
#include "Aquila/types/Stamped.hpp"

#include <boost/circular_buffer.hpp>
#include <mutex>
#include <map>

namespace aq
{
    class ParameterBuffer : public IParameterBuffer
    {
        std::map<mo::TypeInfo, std::map<std::string, boost::circular_buffer<FN<boost::any>>>> _parameter_map;
        std::mutex mtx;
        int _initial_size;
    public:
        ParameterBuffer(int size);
        void SetBufferSize(int size);
        virtual boost::any& GetParameter(mo::TypeInfo type, const std::string& name, int frameNumber);
    };

}

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include "MetaObject/params/buffers/StreamBuffer.hpp"
#include "Aquila/nodes/Node.hpp"


#include "Aquila/Logging.h"
#include "Aquila/nodes/NodeInfo.hpp"

#include "MetaObject/params/ParameterMacros.hpp"
#include "MetaObject/params/TInputParam.hpp"
#include "MetaObject/object/MetaObjectFactory.hpp"
#include "MetaObject/Detail/MetaObjectMacros.hpp"
#include "MetaObject/object/MetaObjectFactory.hpp"


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "AquilaFrameGrabbers"
#include <boost/thread.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace aq;
using namespace aq::Nodes;

struct test_node: public Node
{
    static std::vector<std::string> GetNodeCategory()
    {
        return {"test1", "test2"};
    }

    bool ProcessImpl()
    {
        return true;
    }

    MO_DERIVE(test_node, Node)
        MO_SLOT(void, node_slot, int);
        MO_SIGNAL(void, node_signal, int);
        PARAM(int, test_param, 5);
    MO_END;
};
void test_node::node_slot(int val)
{
    
}

struct test_output_node : public Node
{
	MO_DERIVE(test_output_node, Node)
		OUTPUT(int, value, 0)
	MO_END;
	bool ProcessImpl()
	{
		++timestamp;
        value_param.UpdateData(timestamp * 10, mo::ms * timestamp, _ctx);
		++process_count;
		_modified = true;
		return true;
	}
	int timestamp = 0;
	int process_count = 0;
};


struct test_input_node : public Node
{
	MO_DERIVE(test_input_node, Node)
		INPUT(int, value, nullptr)
	MO_END;
	bool ProcessImpl()
	{
        auto ts = (*value_param.GetTimestamp()).value();
        BOOST_REQUIRE_EQUAL((*value), ts * 10);
		++process_count;
		return true;
	}
	int process_count = 0;
};

struct test_multi_input_node : public Node
{
	MO_DERIVE(test_multi_input_node, Node)
		INPUT(int, value1, nullptr)
		INPUT(int, value2, nullptr)
	MO_END;
	bool ProcessImpl()
	{
		BOOST_REQUIRE_EQUAL((*value1), (*value2));
        BOOST_REQUIRE_EQUAL((*value1) * 10, (*value1_param.GetTimestamp()).value());
        BOOST_REQUIRE_EQUAL(value1_param.GetTimestamp(), value2_param.GetTimestamp());
		return true;
	}
};

MO_REGISTER_CLASS(test_node)
MO_REGISTER_CLASS(test_output_node)
MO_REGISTER_CLASS(test_input_node)
MO_REGISTER_CLASS(test_multi_input_node)

BOOST_AUTO_TEST_CASE(test_node_reflection)
{
    mo::MetaObjectFactory::instance()->registerTranslationUnit();
    auto info = mo::MetaObjectFactory::instance()->GetObjectInfo("test_node");
    auto node_info = dynamic_cast<NodeInfo*>(info);
    BOOST_REQUIRE(node_info);
    BOOST_REQUIRE_EQUAL(node_info->GetNodeCategory().size(), 2);
    BOOST_REQUIRE_EQUAL(node_info->GetParameterInfo().size(), 1);
    BOOST_REQUIRE_EQUAL(node_info->GetSignalInfo().size(), 2);
    BOOST_REQUIRE_EQUAL(node_info->GetSlotInfo().size(), 2);
}

BOOST_AUTO_TEST_CASE(test_node_single_input_output_direct)
{
	auto ds = aq::IDataStream::create();
	auto output_node = test_output_node::create();
	auto input_node = test_input_node::create();	

	BOOST_REQUIRE(input_node->connectInput(output_node, "value", "value"));
	output_node->setDataStream(ds.get());
	for (int i = 0; i < 10; ++i)
	{
		output_node->process();
	}
	BOOST_REQUIRE_EQUAL(output_node->process_count, 10);
	BOOST_REQUIRE_EQUAL(input_node->process_count, 10);
}

BOOST_AUTO_TEST_CASE(test_node_single_input_output_buffered)
{
	auto ds = aq::IDataStream::create();
	auto output_node = test_output_node::create();
	auto input_node = test_input_node::create();
	output_node->setDataStream(ds.get());
	input_node->setDataStream(ds.get());
    static const mo::ParamType test_cases[] = { mo::CircularBuffer_e, mo::ConstMap_e, mo::Map_e, mo::StreamBuffer_e, mo::BlockingStreamBuffer_e, mo::NNStreamBuffer_e };
	for (int i = 0; i < sizeof(test_cases); ++i)
	{
		std::cout << "Buffer type: " << mo::ParameterTypeFlagsToString(test_cases[i]) << std::endl;
		output_node->process_count = 0;
		input_node->process_count = 0;
		output_node->timestamp = 0;
		BOOST_REQUIRE(input_node->connectInput(output_node, "value", "value", test_cases[i]));
		
		for (int i = 0; i < 10; ++i)
		{
			output_node->process();
		}
		BOOST_REQUIRE_EQUAL(output_node->process_count, 10);
		BOOST_REQUIRE_EQUAL(input_node->process_count, 10);
	}
}

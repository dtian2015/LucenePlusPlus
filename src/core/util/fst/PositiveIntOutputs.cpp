#include "PositiveIntOutputs.h"

namespace Lucene {
namespace Util {
namespace FST {

const Long PositiveIntOutputs::NO_OUTPUT = boost::make_shared<long>(0);

bool operator>(const Long& lhs, int value)
{
	if (!lhs)
	{
		return false;
	}

	return *lhs > value;
}

bool operator>=(const Long& lhs, const Long& rhs)
{
	if (!lhs || !rhs)
	{
		return false;
	}

	return *lhs >= *rhs;
}

Long operator-(const Long& lhs, const Long& rhs)
{
	if (!lhs || !rhs)
	{
		return PositiveIntOutputs::NO_OUTPUT;
	}

	return boost::make_shared<long>(*lhs - *rhs);
}

Long operator+(const Long& lhs, const Long& rhs)
{
	if (!lhs || !rhs)
	{
		return PositiveIntOutputs::NO_OUTPUT;
	}

	return boost::make_shared<long>(*lhs + *rhs);
}

PositiveIntOutputs::PositiveIntOutputs(bool doShare) : _doShare(doShare)
{
}

const PositiveIntOutputsPtr& PositiveIntOutputs::getSingleton(bool doShare)
{
	static PositiveIntOutputsPtr singletonShare;
	static PositiveIntOutputsPtr singletonNoShare;

	if (doShare)
	{
		if (!singletonShare)
		{
			singletonShare.reset(new PositiveIntOutputs(true));
		}

		return singletonShare;
	}

	if (singletonNoShare)
	{
		singletonNoShare.reset(new PositiveIntOutputs(false));
	}

	return singletonNoShare;
}

Long PositiveIntOutputs::common(const Long& output1, const Long& output2)
{
	BOOST_ASSERT(valid(output1));
	BOOST_ASSERT(valid(output2));

	if (output1 == NO_OUTPUT || output2 == NO_OUTPUT)
	{
		return NO_OUTPUT;
	}
	else if (_doShare)
	{
		BOOST_ASSERT(output1 > 0);
		BOOST_ASSERT(output2 > 0);

		return boost::make_shared<long>(std::min(*output1, *output2));
	}
	else if (output1 == output2)
	{
		return output1;
	}
	else
	{
		return NO_OUTPUT;
	}
}

Long PositiveIntOutputs::subtract(const Long& output, const Long& inc)
{
	BOOST_ASSERT(valid(output));
	BOOST_ASSERT(valid(inc));
	BOOST_ASSERT(output >= inc);

	if (inc == NO_OUTPUT)
	{
		return output;
	}
	else if (output == inc)
	{
		return NO_OUTPUT;
	}
	else
	{
		return output - inc;
	}
}

Long PositiveIntOutputs::add(const Long& prefix, const Long& output)
{
	BOOST_ASSERT(valid(prefix));
	BOOST_ASSERT(valid(output));

	if (prefix == NO_OUTPUT)
	{
		return output;
	}
	else if (output == NO_OUTPUT)
	{
		return prefix;
	}
	else
	{
		return prefix + output;
	}
}

void PositiveIntOutputs::write(Long output, boost::shared_ptr<DataOutput> out)
{
	BOOST_ASSERT(valid(output));
	out->writeVLong(*output);
}

Long PositiveIntOutputs::read(DataInputPtr input)
{
	long v = input->readVLong();
	if (v == 0)
	{
		return NO_OUTPUT;
	}
	else
	{
		return boost::make_shared<long>(v);
	}
}

bool PositiveIntOutputs::valid(const Long& o)
{
	BOOST_ASSERT(o != nullptr);
	BOOST_ASSERT(o == NO_OUTPUT || o > 0);

	return true;
}

Long PositiveIntOutputs::getNoOutput()
{
	return NO_OUTPUT;
}

String PositiveIntOutputs::outputToString(Long output)
{
	return std::to_wstring(*output);
}

String PositiveIntOutputs::toString()
{
	String doShareValue = _doShare ? L"true" : L"false";
	return L"PositiveIntOutputs(doShare=" + doShareValue + L")";
}
}
}
}

#include <DataTypes/DataTypeString.h>
#include <Functions/FunctionFactory.h>
#include <Functions/FunctionStringOrArrayToT.h>
#include <Functions/EmptyImpl.h>


namespace DB
{
namespace
{

struct NameNotEmpty
{
    static constexpr auto name = "notEmpty";
};
using FunctionNotEmpty = FunctionStringOrArrayToT<EmptyImpl<true>, NameNotEmpty, UInt8>;

}

REGISTER_FUNCTION(NotEmpty)
{
    factory.registerFunction<FunctionNotEmpty>();
}

}

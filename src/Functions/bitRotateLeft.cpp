#include <Functions/FunctionFactory.h>
#include <Functions/FunctionBinaryArithmetic.h>

namespace DB
{
namespace ErrorCodes
{
    extern const int NOT_IMPLEMENTED;
    extern const int LOGICAL_ERROR;
}

namespace
{

template <typename A, typename B>
struct BitRotateLeftImpl
{
    using ResultType = typename NumberTraits::ResultOfBit<A, B>::Type;
    static const constexpr bool allow_fixed_string = false;

    template <typename Result = ResultType>
    static inline NO_SANITIZE_UNDEFINED Result apply(A a [[maybe_unused]], B b [[maybe_unused]])
    {
        if constexpr (is_big_int_v<A> || is_big_int_v<B>)
            throw Exception("Bit rotate is not implemented for big integers", ErrorCodes::NOT_IMPLEMENTED);
        else
            return (static_cast<Result>(a) << static_cast<Result>(b))
                | (static_cast<Result>(a) >> ((sizeof(Result) * 8) - static_cast<Result>(b)));
    }

#if USE_EMBEDDED_COMPILER
    static constexpr bool compilable = true;

    static inline llvm::Value * compile(llvm::IRBuilder<> & b, llvm::Value * left, llvm::Value * right, bool)
    {
        if (!left->getType()->isIntegerTy())
            throw Exception("BitRotateLeftImpl expected an integral type", ErrorCodes::LOGICAL_ERROR);
        auto * size = llvm::ConstantInt::get(left->getType(), left->getType()->getPrimitiveSizeInBits());
        /// XXX how is this supposed to behave in signed mode?
        return b.CreateOr(b.CreateShl(left, right), b.CreateLShr(left, b.CreateSub(size, right)));
    }
#endif
};

struct NameBitRotateLeft { static constexpr auto name = "bitRotateLeft"; };
using FunctionBitRotateLeft = BinaryArithmeticOverloadResolver<BitRotateLeftImpl, NameBitRotateLeft, true, false>;

}

REGISTER_FUNCTION(BitRotateLeft)
{
    factory.registerFunction<FunctionBitRotateLeft>();
}

}

#include <memory>
#include <iterator>

namespace Ranges
{
//give a strong guarantee if value_type is copy constructible 
template<typename InpIt, typename NoThrFwdIt>
constexpr NoThrFwdIt strong_guarantee_uninitialized_move_or_copy(InpIt first, InpIt last, NoThrFwdIt d_first)
{
    using value_type = typename std::iterator_traits<InpIt>::value_type;
    if constexpr (!std::is_copy_constructible<value_type>::value || std::is_nothrow_move_constructible<value_type>::value)
        return std::uninitialized_move(first, last, d_first);
    else
        return std::uninitialized_copy(first, last, d_first);   
}

template<typename FwdIt>
constexpr void uninitialized_default_construct(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    if constexpr (std::is_copy_constructible<value_type>::value)
        std::uninitialized_fill(first, last, value_type{});
    else
        std::uninitialized_default_construct(first, last);
}
} // namespace Ranges
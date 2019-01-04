#pragma once
#include <string>
#include <memory>
#include "buffer.h"

#include <tuple>      // std::tuple
#include <functional> // std::invoke

template <
    size_t Index = 0, // start iteration at 0 index
    typename TTuple,  // the tuple type
    size_t Size =
        std::tuple_size_v<
            std::remove_reference_t<TTuple>>, // tuple size
    typename TCallable, // the callable to bo invoked for each tuple item
    typename... TArgs   // other arguments to be passed to the callable 
>
void map_tuple(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
{
    if constexpr (Index < Size)
    {
        std::invoke(callable, args..., std::get<Index>(tuple));

        if constexpr (Index + 1 < Size)
            for_each<Index + 1>(
                std::forward<TTuple>(tuple),
                std::forward<TCallable>(callable),
                std::forward<TArgs>(args)...);
    }
}

template <class... Ts>
struct VertexData
{
	template <typename T>
	struct VertexDataBuffer
	{
		std::string name;
		std::shared_ptr<Buffer<T>> data;
	};

	template<class... Ts>
	using buffers = std::tuple<Ts...>;

	VertexData(Ts... ts)
	{
		buffers = std::make_tuple(ts...);
	}
};
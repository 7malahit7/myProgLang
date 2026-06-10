#pragma once
#include "base.hpp"
#include <concepts>
namespace mpl::ast
{
	template <typename T>
	concept ast_node = std::derived_from<T, Node>;
}
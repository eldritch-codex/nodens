/// @file Layer.cpp
/// @brief Implementation of the Layer base class constructor and destructor.
/// @ingroup Core

module Nodens.Layer;

import std;

namespace Nodens
{

ILayer::ILayer(const std::string& name) : m_Name(name)
{
}

} // namespace Nodens

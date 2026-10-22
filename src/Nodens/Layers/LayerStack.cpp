/// @file LayerStack.cpp
/// @brief Implementation of LayerStack push/pop operations.
/// @ingroup Core

module Nodens.LayerStack;

import std;

namespace Nodens
{
LayerStack::LayerStack()
{
}

LayerStack::~LayerStack()
{
    for (ILayer* layer : m_Layers)
    {
        layer->OnDetach();
        delete layer;
    }
}

void LayerStack::PushLayer(ILayer* layer)
{
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex++, layer);
}

void LayerStack::PushOverlay(ILayer* overlay)
{
    m_Layers.emplace_back(overlay);
}

void LayerStack::PopLayer(ILayer* layer)
{
    auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (it != m_Layers.end())
    {
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void LayerStack::PopOverlay(ILayer* overlay)
{
    auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
    if (it != m_Layers.end())
        m_Layers.erase(it);
}
} // namespace Nodens

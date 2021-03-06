#include "CriticalBufferHeap.hpp"

#include "MemoryObject.hpp"
#include "CopyCommandList.hpp"

#include <iostream>


namespace inl {
namespace gxeng {
namespace impl {



CriticalBufferHeap::CriticalBufferHeap(gxapi::IGraphicsApi * graphicsApi) :
	m_graphicsApi(graphicsApi)
{}


MemoryObjDesc CriticalBufferHeap::Allocate(gxapi::ResourceDesc desc, gxapi::ClearValue* clearValue) {
	MemoryObjDesc result = MemoryObjDesc(
		m_graphicsApi->CreateCommittedResource(
			gxapi::HeapProperties(gxapi::eHeapType::CUSTOM, gxapi::eCpuPageProperty::NOT_AVAILABLE, gxapi::eMemoryPool::DEDICATED),
			gxapi::eHeapFlags::NONE,
			desc,
			gxapi::eResourceState::COMMON,
			clearValue
		)
	);

	return result;
}


} // namespace impl
} // namespace gxeng
} // namespace inl

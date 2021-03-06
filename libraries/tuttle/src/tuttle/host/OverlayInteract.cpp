#include "OverlayInteract.hpp"

namespace tuttle {
namespace host {

OverlayInteract::OverlayInteract( Graph& userGraph, ImageEffectNode& effect, int bitDepthPerComponent, bool hasAlpha )
	: ofx::interact::OfxhInteract( effect.getOverlayDescriptor(), ( void* )( effect.getHandle() ) )
	, _instance( effect )
	, _bitDepthPerComponent( bitDepthPerComponent )
	, _hasAlpha( hasAlpha )
	, _time(0)
	, _procGraph( _emptyMemoryCache, ComputeOptions().setReturnBuffers( false ), userGraph, std::list<std::string>() )
{
	effect.initOverlayDescriptor( bitDepthPerComponent, hasAlpha );
	createInstanceAction();
}

void OverlayInteract::draw( const OfxPointD& renderScale )
{
	ofx::interact::OfxhInteract::drawAction( _time, renderScale );
}

}
}

   
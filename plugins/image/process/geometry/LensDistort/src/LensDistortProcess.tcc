#include "LensDistortPlugin.hpp"

#include <terry/sampler/all.hpp>
#include <tuttle/plugin/ImageGilProcessor.hpp>
#include <tuttle/plugin/image/resample.hpp>
#include <tuttle/plugin/numeric/rectOp.hpp>
#include <tuttle/plugin/image/resample.hpp>

namespace tuttle {
namespace plugin {
namespace lens {

template<class View>
LensDistortProcess<View>::LensDistortProcess( LensDistortPlugin& instance )
	: ImageGilFilterProcessor<View>( instance )
	, _plugin( instance )
{}

template<class View>
void LensDistortProcess<View>::setup( const OFX::RenderArguments& args )
{
	ImageGilFilterProcessor<View>::setup( args );

	_params = _plugin.getProcessParams();;

	OfxRectD srcRod = rectIntToDouble( this->_srcPixelRod );
	OfxRectD dstRod = rectIntToDouble( this->_dstPixelRod );
	if( _plugin._srcRefClip->isConnected() )
	{
		OfxRectD srcRefRod = rectIntToDouble( _plugin._srcRefClip->getPixelRod( args.time, args.renderScale ) );
		_p = _plugin.getProcessParams( srcRod, dstRod, srcRefRod, this->_clipDst->getPixelAspectRatio() );
	}
	else
	{
		_p = _plugin.getProcessParams( srcRod, dstRod, this->_clipDst->getPixelAspectRatio() );
	}
}

/**
 * @brief Function called by rendering thread each time a process must be done.
 * @param[in]   procWindowRoW     Processing window
 */
template<class View>
void LensDistortProcess<View>::multiThreadProcessImages( const OfxRectI& procWindowRoW )
{
	using namespace boost::gil;
	using namespace terry::sampler;
	OfxRectI procWindowOutput = this->translateRoWToOutputClipCoordinates( procWindowRoW );

	switch( _params._filter )
	{
		case eParamFilterNearest:
		{
			lensDistort<terry::sampler::nearest_neighbor_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterBilinear:
		{
			lensDistort<terry::sampler::bilinear_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterBC:
		{
			bc_sampler BCsampler;
			BCsampler.valB = _params._paramB;
			BCsampler.valC = _params._paramC;
			lensDistort( this->_srcView, this->_dstView, procWindowOutput, BCsampler );
			return;
		}
		case eParamFilterBicubic:
		{
			lensDistort<terry::sampler::bicubic_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterCatrom:
		{
			lensDistort<terry::sampler::catrom_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterMitchell:
		{
			lensDistort<terry::sampler::mitchell_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterParzen:
		{
			lensDistort<terry::sampler::parzen_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterKeys:
		{
			lensDistort<terry::sampler::keys_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterSimon:
		{
			lensDistort<terry::sampler::simon_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterRifman:
		{
			lensDistort<terry::sampler::rifman_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterLanczos:
		{
			lanczos_sampler lanczosSampler;
			lanczosSampler.size = _params._filterSize;
			lensDistort( this->_srcView, this->_dstView, procWindowOutput, lanczosSampler );
			return;
		}
		case eParamFilterLanczos3:
		{
			lensDistort<terry::sampler::lanczos3_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterLanczos4:
		{
			lensDistort<terry::sampler::lanczos4_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterLanczos6:
		{
			lensDistort<terry::sampler::lanczos6_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterLanczos12:
		{
			lensDistort<terry::sampler::lanczos12_sampler>( this->_srcView, this->_dstView, procWindowOutput );
			return;
		}
		case eParamFilterGaussian:
		{
			gaussian_sampler gaussianSampler;
			gaussianSampler.size  = _params._filterSize;
			gaussianSampler.sigma = _params._filterSigma;
			lensDistort( this->_srcView, this->_dstView, procWindowOutput, gaussianSampler );
			return;
		}
	}
	BOOST_THROW_EXCEPTION( exception::Bug()
		<< exception::user( "Interpolation method not recognize." ) );
}

template<class View>
template<class Sampler>
void LensDistortProcess<View>::lensDistort( View& srcView, View& dstView, const OfxRectI& procWindow, const Sampler& sampler )
{
	terry::sampler::EParamFilterOutOfImage outOfImageProcess = _params._outOfImageProcess;
	switch( _params._lensType )
	{
		case eParamLensTypeStandard:
		{
			if( _p._distort )
			{
				resample_pixels_progress( srcView, dstView, static_cast<NormalLensDistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			}
			else
			{
				resample_pixels_progress( srcView, dstView, static_cast<NormalLensUndistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			}
			return;
		}
		case eParamLensTypeFisheye:
		{
			if( _p._distort )
				resample_pixels_progress( srcView, dstView, static_cast<FisheyeLensDistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			else
				resample_pixels_progress( srcView, dstView, static_cast<FisheyeLensUndistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			return;
		}
		case eParamLensTypeAdvanced:
		{
			if( _p._distort )
				resample_pixels_progress( srcView, dstView, static_cast<AdvancedLensDistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			else
				resample_pixels_progress( srcView, dstView, static_cast<AdvancedLensUndistortParams<double>&>( _p ), procWindow, outOfImageProcess, this, sampler );
			return;
		}
	}
	BOOST_THROW_EXCEPTION( exception::Bug()
		<< exception::user( "Lens type not recognize." ) );
}

}
}
}



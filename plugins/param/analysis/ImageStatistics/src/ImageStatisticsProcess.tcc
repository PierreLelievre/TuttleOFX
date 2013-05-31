#include "ImageStatisticsPlugin.hpp"

#include <tuttle/plugin/global.hpp>
#include <terry/globals.hpp>
#include <tuttle/plugin/param/gilColor.hpp>
#include <terry/typedefs.hpp>

#include <terry/numeric/operations.hpp>
#include <terry/numeric/operations_assign.hpp>
#include <terry/numeric/assign.hpp>
#include <terry/numeric/assign_minmax.hpp>
#include <terry/numeric/minmax.hpp>
#include <terry/numeric/init.hpp>
#include <terry/numeric/pow.hpp>
#include <terry/numeric/sqrt.hpp>
#include <terry/color/components.hpp>
#include <boost/gil/extension/color/hsl.hpp>

#include <boost/units/pow.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/find.hpp>

/*
namespace boost {
namespace gil {

namespace details {

template <typename Colorspace, typename ChannelType>
// models pixel concept
struct without_alpha_channel_impl_t
{
	typedef pixel<ChannelType, layout<Colorspace> > Pixel;
	typedef Pixel PixelR;

	BOOST_STATIC_ASSERT( ( !contains_color<Pixel, alpha_t>::value ) );

	PixelR operator ()( const Pixel& src ) const
	{
		return src;
	}

};

template <typename ChannelType>
// models pixel concept
struct without_alpha_channel_impl_t<rgba_t, ChannelType>
{
	typedef rgba_t Colorspace;
	typedef pixel<ChannelType, layout<Colorspace> > Pixel;
	typedef pixel<ChannelType, layout<rgb_t> > PixelR;

	BOOST_STATIC_ASSERT( ( contains_color<Pixel, alpha_t>::value ) );

	PixelR operator ()( const Pixel& src ) const
	{
		PixelR dst;

		get_color( dst, red_t() )   = get_color( src, red_t() );
		get_color( dst, green_t() ) = get_color( src, green_t() );
		get_color( dst, blue_t() )  = get_color( src, blue_t() );
		return dst;
	}

};

}

template <typename Pixel>
// models pixel concept
struct without_alpha_channel_t
{
	typedef typename channel_type<Pixel>::type ChannelType;
	typedef typename color_space_type<Pixel>::type Colorspace;

	typedef details::without_alpha_channel_impl_t<Colorspace, ChannelType> Do;
	typedef typename Do::PixelR PixelR;

	PixelR operator ()( const Pixel& src ) const
	{
		return Do() ( src );
	}

};

GIL_DEFINE_ALL_TYPEDEFS( 64, hsl )
GIL_DEFINE_ALL_TYPEDEFS( 64s, hsl )
GIL_DEFINE_ALL_TYPEDEFS( 64f, hsl )

}
}
*/

namespace tuttle {
namespace plugin {
namespace imageStatistics {

template< typename CType, typename Layout >
struct premultiplied
{
	typedef boost::gil::pixel<CType, Layout> CPixel;
	
	//GIL_FORCEINLINE
	CPixel operator()( const CPixel& src );
	CPixel operator()( const CPixel& src, const CPixel& alpha );
};

template< typename CType, typename Layout >
boost::gil::pixel<CType, Layout > premultiplied<CType, Layout>::operator()( const boost::gil::pixel<CType, Layout >& src )
{
	typedef boost::gil::pixel<CType, Layout > CPixel;
	CPixel dst = src;
	return dst;
}

template< typename CType, typename Layout >
boost::gil::pixel<CType, Layout > premultiplied<CType, Layout>::operator()( const boost::gil::pixel<CType, Layout >& src, const boost::gil::pixel<CType, Layout >& alpha )
{
	typedef boost::gil::pixel<CType, Layout > CPixel;
	CPixel dst = src;
	return dst;
}

template< >
boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t > premultiplied< boost::gil::bits64f, boost::gil::rgba_layout_t >::operator()( const boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t >& src )
{
	using namespace boost::gil;
	typedef boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t > CPixel;
	CPixel dst;
	get_color( dst, red_t() )   = 1.f * get_color( src, red_t() )   * get_color( src, alpha_t() );
	get_color( dst, green_t() ) = 1.f * get_color( src, green_t() ) * get_color( src, alpha_t() );
	get_color( dst, blue_t() )  = 1.f * get_color( src, blue_t() )  * get_color( src, alpha_t() );
	get_color( dst, alpha_t() ) = get_color( src, alpha_t() );
	return dst;
}

template< >
boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t > premultiplied< boost::gil::bits64f, boost::gil::rgba_layout_t >::operator()( const boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t >& src, const boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t >& alpha )
{
	using namespace boost::gil;
	typedef boost::gil::pixel<boost::gil::bits64f, boost::gil::rgba_layout_t > CPixel;
	CPixel dst;
	get_color( dst, red_t() )   = 1.f * get_color( src, red_t() )   * get_color( alpha, alpha_t() );
	get_color( dst, green_t() ) = 1.f * get_color( src, green_t() ) * get_color( alpha, alpha_t() );
	get_color( dst, blue_t() )  = 1.f * get_color( src, blue_t() )  * get_color( alpha, alpha_t() );
	get_color( dst, alpha_t() ) = get_color( alpha, alpha_t() );
	return dst;
}

template<typename T>
T variance( const T v_mean, const T v_sum_p2, const std::size_t nb )
{
	using namespace boost::units;
	return ( v_sum_p2 / nb - pow<2>( v_mean ) );
}

template<typename T>
T variance( const T v_mean, const T v_sum_p2, const double nb )
{
	using namespace boost::units;
	return ( v_sum_p2 / nb - pow<2>( v_mean ) );
}

template<typename Pixel>
Pixel pixel_variance( const Pixel& v_mean, const Pixel& v_sum_p2, const std::size_t nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = variance( v_mean[i], v_sum_p2[i], nb );
	}
	return res;
}

template<typename Pixel>
Pixel pixel_variance( const Pixel& v_mean, const Pixel& v_sum_p2, const double nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = variance( v_mean[i], v_sum_p2[i], nb );
	}
	return res;
}

/**
 * @brief In probability theory and statistics, skewness is a measure of the
 * asymmetry of the probability distribution of a real-valued random variable.
 *
 * A zero value indicates that the values are relatively evenly distributed
 * on both sides of the mean, typically but not necessarily implying
 * a symmetric distribution.
 */
template<typename T>
T skewness( const T v_mean, const T v_standard_deviation, const T v_sum_p2, const T v_sum_p3, const std::size_t nb )
{
	using namespace boost::units;
	return ( ( v_sum_p3 - 3.0 * v_mean * v_sum_p2 ) / nb + 2.0 * pow<3>( v_mean ) ) / pow<3>( v_standard_deviation );
}

template<typename T>
T skewness( const T v_mean, const T v_standard_deviation, const T v_sum_p2, const T v_sum_p3, const double nb )
{
	using namespace boost::units;
	return ( ( v_sum_p3 - 3.0 * v_mean * v_sum_p2 ) / nb + 2.0 * pow<3>( v_mean ) ) / pow<3>( v_standard_deviation );
}

template<typename Pixel>
Pixel pixel_skewness( const Pixel& v_mean, const Pixel& v_standard_deviation, const Pixel& v_sum_p2, const Pixel& v_sum_p3, const std::size_t nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = skewness( v_mean[i], v_standard_deviation[i], v_sum_p2[i], v_sum_p3[i], nb );
	}
	return res;
}

template<typename Pixel>
Pixel pixel_skewness( const Pixel& v_mean, const Pixel& v_standard_deviation, const Pixel& v_sum_p2, const Pixel& v_sum_p3, const double nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = skewness( v_mean[i], v_standard_deviation[i], v_sum_p2[i], v_sum_p3[i], nb );
	}
	return res;
}

/**
 * @brief In probability theory and statistics, kurtosis is a measure of the
 * "peakedness" of the probability distribution of a real-valued random variable.
 *
 * Higher kurtosis means more of the variance is the result of infrequent extreme deviations,
 * as opposed to frequent modestly sized deviations.
 */
template<typename T>
T kurtosis( const T v_mean, const T v_standard_deviation, const T v_sum_p2, const T v_sum_p3, const T v_sum_p4, const std::size_t nb )
{
	using namespace boost::units;
	return ( ( ( v_sum_p4 - 4.0 * v_mean * v_sum_p3 + 6.0 * pow<2>( v_mean ) * v_sum_p2 ) / nb - 3.0 * pow<4>( v_mean ) ) / ( pow<4>( v_standard_deviation ) ) );
}

template<typename T>
T kurtosis( const T v_mean, const T v_standard_deviation, const T v_sum_p2, const T v_sum_p3, const T v_sum_p4, const double nb )
{
	using namespace boost::units;
	return ( ( ( v_sum_p4 - 4.0 * v_mean * v_sum_p3 + 6.0 * pow<2>( v_mean ) * v_sum_p2 ) / nb - 3.0 * pow<4>( v_mean ) ) / ( pow<4>( v_standard_deviation ) ) );
}

template<typename Pixel>
Pixel pixel_kurtosis( const Pixel& v_mean, const Pixel& v_standard_deviation, const Pixel& v_sum_p2, const Pixel& v_sum_p3, const Pixel& v_sum_p4, const std::size_t nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = kurtosis( v_mean[i], v_standard_deviation[i], v_sum_p2[i], v_sum_p3[i], v_sum_p4[i], nb );
	}
	return res;
}

template<typename Pixel>
Pixel pixel_kurtosis( const Pixel& v_mean, const Pixel& v_standard_deviation, const Pixel& v_sum_p2, const Pixel& v_sum_p3, const Pixel& v_sum_p4, const double nb )
{
	using namespace boost::gil;
	Pixel res;
	for( int i = 0; i < num_channels<Pixel>::type::value; ++i )
	{
		res[i] = kurtosis( v_mean[i], v_standard_deviation[i], v_sum_p2[i], v_sum_p3[i], v_sum_p4[i], nb );
	}
	return res;
}

template<class Pixel>
struct OutputParams
{
	OutputParams()
	{
		using namespace terry::numeric;
		pixel_zeros_t<Pixel>( )( _average );
		pixel_zeros_t<Pixel>( )( _variance );
		pixel_zeros_t<Pixel>( )( _stdDeviation );
		pixel_zeros_t<Pixel>( )( _skewness );
		pixel_zeros_t<Pixel>( )( _kurtosis );
		pixel_zeros_t<Pixel>( )( _channelMin );
		pixel_zeros_t<Pixel>( )( _channelMax );
		pixel_zeros_t<Pixel>( )( _luminosityMin );
		pixel_zeros_t<Pixel>( )( _luminosityMax );
	}

	Pixel _average;
	Pixel _variance;
	Pixel _stdDeviation;
	Pixel _skewness;
	Pixel _kurtosis;
	Pixel _channelMin;
	Pixel _channelMax;
	Pixel _luminosityMin;
	Pixel _luminosityMax;
};

template<class View, typename CType = boost::gil::bits64f>
struct ComputeOutputParams
{

	typedef typename View::value_type Pixel;
	typedef typename boost::gil::color_space_type<View>::type Colorspace;
	typedef boost::gil::pixel<typename boost::gil::channel_type<View>::type, boost::gil::layout<boost::gil::gray_t> > PixelGray; // grayscale pixel type (using the input channel_type)
	typedef boost::gil::pixel<CType, boost::gil::layout<Colorspace> > CPixel; // the pixel type use for computation (using input colorspace)

	typedef OutputParams<CPixel> Output;

	Output operator()( const View& image, ImageStatisticsPlugin& plugin )
	{
		using namespace terry::numeric;
		OutputParams<CPixel> output;

		const std::size_t nbPixels = image.width() * image.height();

		// declare values and init
		Pixel firstPixel = *image.begin(); // for initialization only
		PixelGray firstPixelGray;
		color_convert( firstPixel, firstPixelGray );

		Pixel channelMin            = firstPixel;
		Pixel channelMax            = firstPixel;
		Pixel luminosityMin         = firstPixel;
		Pixel luminosityMax         = firstPixel;
		PixelGray luminosityMinGray = firstPixelGray;
		PixelGray luminosityMaxGray = firstPixelGray;

		/*CPixel channelMinWgt
		CPixel channelMaxWgt
		CPixel luminosityMinWgt
		CPixel luminosityMaxWgt
		PixelGray luminosityMinGrayWgt
		PixelGray luminosityMaxGrayWgt

		pixel_assigns_t<Pixel, CPixel>( )( firstPixel, channelMinWgt );
		pixel_assigns_t<Pixel, CPixel>( )( firstPixel, channelMaxWgt );
		pixel_assigns_t<Pixel, CPixel>( )( firstPixel, luminosityMinWgt );
		pixel_assigns_t<Pixel, CPixel>( )( firstPixel, luminosityMaxWgt );
		pixel_assigns_t<PixelGray, PixelGray>( )( firstPixelGray, luminosityMinGray );
		pixel_assigns_t<PixelGray, PixelGray>( )( firstPixelGray, luminosityMaxGray );

		pixel_assigns_t<CPixel, CPixel>( )(premultiplied<CType, boost::gil::layout<Colorspace> >()( channelMinWgt ), channelMinWgt );*/

		CPixel sum;
		CPixel sum_p2;
		CPixel sum_p3;
		CPixel sum_p4;
		pixel_zeros_t<CPixel>( )( sum );
		pixel_zeros_t<CPixel>( )( sum_p2 );
		pixel_zeros_t<CPixel>( )( sum_p3 );
		pixel_zeros_t<CPixel>( )( sum_p4 );

		CPixel sumWgt;
		CPixel sumWgt_p2;
		CPixel sumWgt_p3;
		CPixel sumWgt_p4;
		pixel_zeros_t<CPixel>( )( sumWgt );
		pixel_zeros_t<CPixel>( )( sumWgt_p2 );
		pixel_zeros_t<CPixel>( )( sumWgt_p3 );
		pixel_zeros_t<CPixel>( )( sumWgt_p4 );

		for( int y = 0; y < image.height(); ++y )
		{
			typename View::x_iterator src_it = image.x_at( 0, y );

			for( int x = 0; x < image.width(); ++x, ++src_it )
			{
				CPixel pix;
				CPixel pix_p2;
				CPixel pix_p3;
				CPixel pix_p4;

				pixel_assigns_t<Pixel, CPixel>( )( * src_it, pix ); // pix = src_it;
				pixel_assigns_t<CPixel, CPixel>( )( pixel_pow_t<CPixel, 2>( )( pix ), pix_p2 ); // pix_p2 = pow<2>( pix );
				pixel_assigns_t<CPixel, CPixel>( )( pixel_multiplies_t<CPixel, CPixel, CPixel>( )( pix, pix_p2 ), pix_p3 ); // pix_p3 = pix * pix_p2;
				pixel_assigns_t<CPixel, CPixel>( )( pixel_multiplies_t<CPixel, CPixel, CPixel>( )( pix_p2, pix_p2 ), pix_p4 ); // pix_p4 = pix_p2 * pix_p2;

				pixel_plus_assign_t<CPixel, CPixel>( )( pix, sum ); // sum += pix;
				pixel_plus_assign_t<CPixel, CPixel>( )( pix_p2, sum_p2 ); // sum_p2 += pix_p2;
				pixel_plus_assign_t<CPixel, CPixel>( )( pix_p3, sum_p3 ); // sum_p3 += pix_p3;
				pixel_plus_assign_t<CPixel, CPixel>( )( pix_p4, sum_p4 ); // sum_p4 += pix_p4;

				CPixel pixWgt;
				CPixel pixWgt_p2;
				CPixel pixWgt_p3;
				CPixel pixWgt_p4;

				pixel_assigns_t<CPixel, CPixel>( )(premultiplied<CType, boost::gil::layout<Colorspace> >()( pix ), pixWgt );
				pixel_assigns_t<CPixel, CPixel>( )(premultiplied<CType, boost::gil::layout<Colorspace> >()( pix_p2, pix ), pixWgt_p2 );
				pixel_assigns_t<CPixel, CPixel>( )(premultiplied<CType, boost::gil::layout<Colorspace> >()( pix_p3, pix ), pixWgt_p3 );
				pixel_assigns_t<CPixel, CPixel>( )(premultiplied<CType, boost::gil::layout<Colorspace> >()( pix_p4, pix ), pixWgt_p4 ); 

				pixel_plus_assign_t<CPixel, CPixel>( )( pixWgt, sumWgt ); // sumWgt += pixWgt;
				pixel_plus_assign_t<CPixel, CPixel>( )( pixWgt_p2, sumWgt_p2 ); // sumWgt_p2 += pixWgt_p2;
				pixel_plus_assign_t<CPixel, CPixel>( )( pixWgt_p3, sumWgt_p3 ); // sumWgt_p3 += pixWgt_p3;
				pixel_plus_assign_t<CPixel, CPixel>( )( pixWgt_p4, sumWgt_p4 ); // sumWgt_p4 += pixWgt_p4;

				// search min for each channel
				pixel_assign_min_t<Pixel, Pixel>( )( * src_it, channelMin );
				//pixel_assign_min_t<Pixel, Pixel>( )( pixel_multiplies_scalar_t<Pixel, double>() ( * src_it, get_color( * src_it, alpha_t() )), channelMinWgt );
				// search max for each channel
				pixel_assign_max_t<Pixel, Pixel>( )( * src_it, channelMax );
				//pixel_assign_max_t<Pixel, Pixel>( )( pixel_multiplies_scalar_t<Pixel, double>() ( * src_it, get_color( * src_it, alpha_t() )), channelMaxWgt );

				PixelGray grayCurrentPixel; // current pixel in gray colorspace
				//PixelGray grayCurrentPixelWgt; // current pixelWght in gray colorspace
				color_convert( * src_it, grayCurrentPixel );
				//color_convert( pixel_multiplies_scalar_t<Pixel, double>() ( * src_it, get_color( * src_it, alpha_t() )), grayCurrentPixel );

				// search min luminosity
				if( get_color( grayCurrentPixel, gray_color_t() ) < get_color( luminosityMinGray, gray_color_t() ) )
				{
					luminosityMin     = *src_it;
					luminosityMinGray = grayCurrentPixel;
				}
				// search max luminosity
				if( get_color( grayCurrentPixel, gray_color_t() ) > get_color( luminosityMaxGray, gray_color_t() ) )
				{
					luminosityMax     = *src_it;
					luminosityMaxGray = grayCurrentPixel;
				}

				/*// search min luminosity Wgt
				if( get_color( grayCurrentPixelWgt, gray_color_t() ) < get_color( luminosityMinGrayWgt, gray_color_t() ) )
				{
					luminosityMinWgt     = pixel_multiplies_scalar_t<Pixel, double>() ( * src_it, get_color( * src_it, alpha_t() ));
					luminosityMinGrayWgt = grayCurrentPixelWgt;
				}
				// search max luminosity Wgt
				if( get_color( grayCurrentPixelWgt, gray_color_t() ) > get_color( luminosityMaxGrayWgt, gray_color_t() ) )
				{
					luminosityMaxWgt     = pixel_multiplies_scalar_t<Pixel, double>() ( * src_it, get_color( * src_it, alpha_t() ));
					luminosityMaxGrayWgt = grayCurrentPixelWgt;
				}*/
			}
		}

		if( plugin._paramUseAlphaAsAWeightMask->getValue() )
		{
			double nbPixelsWgt = get_color( sum, alpha_t() );
			output._average = pixel_divides_scalar_t<CPixel, double>() ( sumWgt, nbPixelsWgt );
			output._variance = pixel_variance( output._average, sumWgt_p2, nbPixelsWgt );
			output._stdDeviation = pixel_sqrt_t<CPixel, Pixel>()( output._variance );
			output._skewness = pixel_skewness( output._average, output._stdDeviation, sumWgt_p2, sumWgt_p3, nbPixelsWgt );
			output._kurtosis = pixel_kurtosis( output._average, output._stdDeviation, sumWgt_p2, sumWgt_p3, sumWgt_p4, nbPixelsWgt );
			output._channelMin    = channelMin;
			output._channelMax    = channelMax;
			output._luminosityMin = luminosityMin;
			output._luminosityMax = luminosityMax;
		}
		else
		{
			output._average = pixel_divides_scalar_t<CPixel, std::size_t>() ( sum, nbPixels );
			output._variance = pixel_variance( output._average, sum_p2, nbPixels );
			output._stdDeviation = pixel_sqrt_t<CPixel, Pixel>()( output._variance );
			output._skewness = pixel_skewness( output._average, output._stdDeviation, sum_p2, sum_p3, nbPixels );
			output._kurtosis = pixel_kurtosis( output._average, output._stdDeviation, sum_p2, sum_p3, sum_p4, nbPixels );
			output._channelMin    = channelMin;
		    output._channelMax    = channelMax;
		    output._luminosityMin = luminosityMin;
		    output._luminosityMax = luminosityMax;
		}

		return output;
	}

};


template <typename OutputParamsRGBA, typename OutputParamsHSL>
void setOutputParams( const OutputParamsRGBA& outputParamsRGBA, const OutputParamsHSL& outputParamsHSL, const OfxTime time, ImageStatisticsPlugin& plugin )
{
	setRGBAParamValuesAtTime( *plugin._paramOutputAverage, time, outputParamsRGBA._average );
	setRGBAParamValuesAtTime( *plugin._paramOutputVariance, time, outputParamsRGBA._variance );
	setRGBAParamValuesAtTime( *plugin._paramOutputStdDeviation, time, outputParamsRGBA._stdDeviation );
	setRGBAParamValuesAtTime( *plugin._paramOutputSkewness, time, outputParamsRGBA._skewness );
	setRGBAParamValuesAtTime( *plugin._paramOutputKurtosis, time, outputParamsRGBA._kurtosis );
	setRGBAParamValuesAtTime( *plugin._paramOutputChannelMin, time, outputParamsRGBA._channelMin );
	setRGBAParamValuesAtTime( *plugin._paramOutputChannelMax, time, outputParamsRGBA._channelMax );
	setRGBAParamValuesAtTime( *plugin._paramOutputLuminosityMin, time, outputParamsRGBA._luminosityMin );
	setRGBAParamValuesAtTime( *plugin._paramOutputLuminosityMax, time, outputParamsRGBA._luminosityMax );


	set012ParamValuesAtTime( *plugin._paramOutputAverageHSL, time, outputParamsHSL._average );
	set012ParamValuesAtTime( *plugin._paramOutputVarianceHSL, time, outputParamsHSL._variance );
	set012ParamValuesAtTime( *plugin._paramOutputStdDeviationHSL, time, outputParamsHSL._stdDeviation );
	set012ParamValuesAtTime( *plugin._paramOutputSkewnessHSL, time, outputParamsHSL._skewness );
	set012ParamValuesAtTime( *plugin._paramOutputKurtosisHSL, time, outputParamsHSL._kurtosis );
	set012ParamValuesAtTime( *plugin._paramOutputChannelMinHSL, time, outputParamsHSL._channelMin );
	set012ParamValuesAtTime( *plugin._paramOutputChannelMaxHSL, time, outputParamsHSL._channelMax );
	set012ParamValuesAtTime( *plugin._paramOutputLuminosityMinHSL, time, outputParamsHSL._luminosityMin );
	set012ParamValuesAtTime( *plugin._paramOutputLuminosityMaxHSL, time, outputParamsHSL._luminosityMax );

}

template<class View>
ImageStatisticsProcess<View>::ImageStatisticsProcess( ImageStatisticsPlugin& instance )
	: ImageGilFilterProcessor<View>( instance, eImageOrientationIndependant )
	, _plugin( instance )
{
	this->setNoMultiThreading();
}

template<class View>
void ImageStatisticsProcess<View>::setup( const OFX::RenderArguments& args )
{
	using namespace boost::gil;

	ImageGilFilterProcessor<View>::setup( args );
	_processParams = _plugin.getProcessParams( this->_srcPixelRod );

	View image = subimage_view( this->_srcView,
								_processParams._rect.x1,
								_processParams._rect.y1,
								_processParams._rect.x2 - _processParams._rect.x1,
								_processParams._rect.y2 - _processParams._rect.y1 );

	typedef ComputeOutputParams<View, boost::gil::bits64f> ComputeRGBA;
	typename ComputeRGBA::Output outputRGBA = ComputeRGBA() ( image, this->_plugin );

	typedef pixel<typename channel_type<View>::type, layout<hsl_t> > HSLPixel;
	typedef color_converted_view_type<View, HSLPixel> HSLConverter;
	typedef ComputeOutputParams<typename HSLConverter::type, boost::gil::bits64f> ComputeHSL;
	typename ComputeHSL::Output outputHSL = ComputeHSL() ( color_converted_view<HSLPixel>( image ), this->_plugin );

	setOutputParams( outputRGBA, outputHSL, args.time, this->_plugin );

	switch( _processParams._chooseOutput )
	{
		case eParamChooseOutputSource:
			break;
		case eParamChooseOutputAverage:
			color_convert( outputRGBA._average, _outputPixel );
			break;
		case eParamChooseOutputVariance:
			color_convert( outputRGBA._variance, _outputPixel );
			break;
		case eParamChooseOutputStdDeviation:
			color_convert( outputRGBA._stdDeviation, _outputPixel );
			break;
		case eParamChooseOutputChannelMin:
			color_convert( outputRGBA._channelMin, _outputPixel );
			break;
		case eParamChooseOutputChannelMax:
			color_convert( outputRGBA._channelMax, _outputPixel );
			break;
		case eParamChooseOutputLuminosityMin:
			color_convert( outputRGBA._luminosityMin, _outputPixel );
			break;
		case eParamChooseOutputLuminosityMax:
			color_convert( outputRGBA._luminosityMax, _outputPixel );
			break;
	}
}

/**
 * @param[in] procWindowRoW  Processing window in RoW
 */
template<class View>
void ImageStatisticsProcess<View>::multiThreadProcessImages( const OfxRectI& procWindowRoW )
{
	using namespace boost::gil;
	OfxRectI procWindowOutput = this->translateRoWToOutputClipCoordinates( procWindowRoW );
	const OfxPointI procWindowSize = {
		procWindowRoW.x2 - procWindowRoW.x1,
		procWindowRoW.y2 - procWindowRoW.y1 };
	
	if( _processParams._chooseOutput == eParamChooseOutputSource )
	{
		for( int y = procWindowOutput.y1;
			 y < procWindowOutput.y2;
			 ++y )
		{
			typename View::x_iterator src_it = this->_srcView.x_at( procWindowOutput.x1, y );
			typename View::x_iterator dst_it = this->_dstView.x_at( procWindowOutput.x1, y );
			for( int x = procWindowOutput.x1;
				 x < procWindowOutput.x2;
				 ++x, ++src_it, ++dst_it )
			{
				( *dst_it ) = ( *src_it );
			}
			if( this->progressForward( procWindowSize.x ) )
				return;
		}
	}
	else
	{
		for( int y = procWindowOutput.y1;
			 y < procWindowOutput.y2;
			 ++y )
		{
			typename View::x_iterator dst_it = this->_dstView.x_at( procWindowOutput.x1, y );
			for( int x = procWindowOutput.x1;
				 x < procWindowOutput.x2;
				 ++x, ++dst_it )
			{
				( *dst_it ) = _outputPixel;
			}
			if( this->progressForward( procWindowSize.x ) )
				return;
		}
	}
}

}
}
}

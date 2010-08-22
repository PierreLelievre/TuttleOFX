#ifndef _TUTTLE_PLUGIN_FFMPEG_WRITER_PLUGIN_HPP_
#define _TUTTLE_PLUGIN_FFMPEG_WRITER_PLUGIN_HPP_

#include <tuttle/common/utils/global.hpp>
#include <ofxsImageEffect.h>
#include <string>
#include <boost/gil/gil_all.hpp>
#include <boost/scoped_ptr.hpp>
#include <ffmpeg/VideoFFmpegWriter.hpp>

namespace tuttle {
namespace plugin {
namespace ffmpeg {
namespace writer {

struct FFMpegProcessParams
{
	std::string _filepath;      ///< Ffmpeg filepath
	int _format;				///< Format
	int _codec;					///< Codec
	int _bitrate;				///< Bit rate
};

/**
 * @brief FFMpeg plugin
 */
class FFMpegWriterPlugin : public OFX::ImageEffect
{
public:
    FFMpegWriterPlugin( OfxImageEffectHandle handle );

public:
	FFMpegProcessParams getProcessParams() const;

	void changedParam( const OFX::InstanceChangedArgs &args, const std::string &paramName );
	bool isIdentity( const OFX::RenderArguments &args, OFX::Clip * &identityClip, double &identityTime );

	void beginSequenceRender( const OFX::BeginSequenceRenderArguments& args );
    void render( const OFX::RenderArguments &args );
	void endSequenceRender( const OFX::EndSequenceRenderArguments& args );

public:
    // do not need to delete these, the ImageEffect is managing them for us
    OFX::Clip*          _clipSrc;       ///< Source image clip
    OFX::Clip*          _clipDst;       ///< Destination image clip
	OFX::StringParam*   _filepath;      ///< Ffmpeg filepath
	OFX::ChoiceParam*   _format;
	OFX::ChoiceParam*   _formatLong;
	OFX::ChoiceParam*   _codec;
	OFX::ChoiceParam*   _codecLong;
	OFX::IntParam*      _bitRate;
	OFX::BooleanParam*  _paramRenderAlways;     ///< Render always
	VideoFFmpegWriter	_writer;
};

}
}
}
}

#endif

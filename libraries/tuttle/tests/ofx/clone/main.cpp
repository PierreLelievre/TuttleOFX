#include <tuttle/common/utils/global.hpp>
#include <tuttle/host/core/Exception.hpp>
#include <tuttle/host/core/Core.hpp>
#include <tuttle/host/core/ImageEffectNode.hpp>

#include <iostream>

#define BOOST_TEST_MODULE properties_tests
#include <boost/test/unit_test.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( ofx_tests_suite01 )

BOOST_AUTO_TEST_CASE( ofx_imageEffect_clones )
{
	using namespace std;
	using namespace tuttle::host;

	tuttle::host::core::Core::instance().preload();

	// get some plugins examples
	tuttle::host::ofx::imageEffect::OfxhImageEffectPlugin* plugin = tuttle::host::core::Core::instance().getImageEffectPluginById( "fr.hd3d.tuttle.pngreader" );
	BOOST_CHECK( plugin != NULL );

	//	BOOST_CHECK_THROW( plugin->getProperties().fetchPointerProperty( kOfxPropInstanceData ), core::exception::LogicError );

	boost::scoped_ptr<tuttle::host::core::ImageEffectNode> ofxinst( dynamic_cast< tuttle::host::core::ImageEffectNode* >( plugin->createInstance( kOfxImageEffectContextGenerator ) ) );
	ofxinst->setName( "pluginTest" );
	BOOST_CHECK( ofxinst->getProperties().fetchPointerProperty( kOfxPropInstanceData ).getValue() != NULL );

	boost::scoped_ptr<tuttle::host::core::ImageEffectNode> ofxinstclone( ofxinst->clone() );

	BOOST_CHECK_NE( ofxinst.get(), ofxinstclone.get() );
	BOOST_CHECK( (*ofxinst == *ofxinstclone) );
	BOOST_CHECK( ofxinst->getProperties().fetchPointerProperty( kOfxPropInstanceData ).getValue() != NULL );
	BOOST_CHECK( ofxinstclone->getProperties().fetchPointerProperty( kOfxPropInstanceData ).getValue() != NULL );
	BOOST_CHECK( ofxinst->getProperties().fetchPointerProperty( kOfxPropInstanceData ).getValue() != ofxinstclone->getProperties().fetchPointerProperty( kOfxPropInstanceData ).getValue() );
}

BOOST_AUTO_TEST_SUITE_END()


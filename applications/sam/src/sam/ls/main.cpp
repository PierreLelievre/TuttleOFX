#include <sam/common/utility.hpp>
#include <sam/common/color.hpp>
#include <sam/common/options.hpp>

#include <tuttle/common/utils/global.hpp>

//#include <tuttle/common/clip/Sequence.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <sequence/parser/Browser.h>
#include <sequence/DisplayUtils.h>

#include <algorithm>
#include <iostream>
#include <iterator>

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
namespace bal = boost::algorithm;

namespace sam
{
	Color _color;
	bool wasSthgDumped = false;
	
	bool isDotFilename( const bfs::path& p )
	{
		return p.filename().string().at(0) == '.';
	}
	
	boost::regex convertFilterToRegex( std::string filter )
	{
		boost::cmatch match;
		boost::regex expression( "(.*[%])([0-9]{2})([d].*)" ); // match to pattern like : %04d
		if( boost::regex_match( filter.c_str(), match, expression ) )
		{
			std::string matched = match[1].second;
			matched.erase( 2 , matched.size()-2); // keep only numbers
			const int patternWidth = boost::lexical_cast<int>( matched );
			std::string replacing( patternWidth, '#' );
			filter = boost::regex_replace( filter, boost::regex( "\\%\\d{1,2}d" ), replacing );
		}
	
		filter = boost::regex_replace( filter, boost::regex( "\\*" ), "(.*)" );
		filter = boost::regex_replace( filter, boost::regex( "\\?" ), "(.)" );
		filter = boost::regex_replace( filter, boost::regex( "\\@" ), "[0-9]+" ); // one @ correspond to one or more digits
		filter = boost::regex_replace( filter, boost::regex( "\\#" ), "[0-9]" ); // each # in pattern correspond to a digit
		return boost::regex( filter );
	}
	
	std::vector<boost::regex> convertFilterToRegex( const std::vector<std::string>& filters )
	{
		std::vector<boost::regex> res;
		BOOST_FOREACH( const std::string& filter, filters )
		{
			res.push_back( convertFilterToRegex( filter ) );
		}
		return res;
	}
	
	bool isFilteredFilename( const std::string& filename, const std::vector<boost::regex>& filters )
	{
		if( filters.size() == 0 )
			return false;
	
		BOOST_FOREACH( const boost::regex& filter, filters )
		{
			if( boost::regex_match( filename, filter ) )
				return false;
		}
		return true;
	}
}

int main( int argc, char** argv )
{
	using namespace tuttle::common;
	using namespace sam;

	typedef std::vector<sequence::BrowseItem> Items;

	bool recursiveListing    = false;
	bool script              = false;
	bool enableColor         = false;
	bool listUnitFile        = false;
	bool listFolder          = false;
	bool maskSequences       = false;
	bool listDotFile         = false; // file starting with a dot (.filename)
	bool listLongListing     = false;
	bool listRelativePath    = false;
	bool listAbsolutePath    = false;
	
	//std::string       availableExtensions;
	std::vector<std::string> paths;
	std::vector<std::string> filters;

	// Declare the supported options.
	bpo::options_description mainOptions;
	mainOptions.add_options()
		(kAllOptionString          , kAllOptionMessage)
		(kDirectoriesOptionString  , kDirectoriesOptionMessage)
		(kExpressionOptionString   , bpo::value<std::string>(), kExpressionOptionMessage)
		(kFilesOptionString        , kFilesOptionMessage )
		(kHelpOptionString         , kHelpOptionMessage)
		(kLongListingOptionString  , kLongListingOptionMessage)
		(kIgnoreOptionString       , kIgnoreOptionMessage)
		//(kRelativePathOptionString , kRelativePathOptionMessage)
		(kRecursiveOptionString    , kRecursiveOptionMessage)
		(kPathOptionString         , kPathOptionMessage)
		(kColorOptionString        , kColorOptionMessage)
		(kFullDisplayOptionString  , kFullDisplayOptionMessage )
		(kScriptOptionString       , kScriptOptionMessage)
		(kBriefOptionString        , kBriefOptionMessage)
	;
	
	// describe hidden options
	bpo::options_description hidden;
	hidden.add_options()
		(kInputDirOptionString, bpo::value< std::vector<std::string> >(), kInputDirOptionMessage)
		(kEnableColorOptionString, bpo::value<std::string>(), kEnableColorOptionMessage)
	;
	
	// define default options 
	bpo::positional_options_description pod;
	pod.add(kInputDirOptionLongName, -1);
	
	bpo::options_description cmdline_options;
	cmdline_options.add(mainOptions).add(hidden);

	bpo::positional_options_description pd;
	pd.add("", -1);
	
	bpo::variables_map vm;

	try
	{
		//parse the command line, and put the result in vm
		bpo::store(bpo::command_line_parser(argc, argv).options(cmdline_options).positional(pod).run(), vm);

		// get environment options and parse them
		if( const char* env_ls_options = std::getenv("SAM_LS_OPTIONS") )
		{
			const std::vector<std::string> vecOptions = bpo::split_unix( env_ls_options, " " );
			bpo::store(bpo::command_line_parser(vecOptions).options(cmdline_options).positional(pod).run(), vm);
		}
		if( const char* env_ls_options = std::getenv("SAM_OPTIONS") )
		{
			const std::vector<std::string> vecOptions = bpo::split_unix( env_ls_options, " " );
			bpo::store(bpo::command_line_parser(vecOptions).options(cmdline_options).positional(pod).run(), vm);
		}
		bpo::notify(vm);
	}
	catch( const bpo::error& e)
	{
		TUTTLE_COUT("sam-ls: command line error: " << e.what() );
		exit( -2 );
	}
	catch(...)
	{
		TUTTLE_COUT("sam-ls: unknown error in command line.");
		exit( -2 );
	}

	if ( vm.count(kScriptOptionLongName) )
	{
		// disable color, disable directory printing and set relative path by default
		script = true;
	}

	if ( vm.count(kColorOptionLongName) && !script )
	{
		enableColor = true;
	}
	if ( vm.count(kEnableColorOptionLongName) && !script )
	{
		const std::string str = vm[kEnableColorOptionLongName].as<std::string>();
		enableColor = string_to_boolean( str );
	}

	if( enableColor )
	{
		_color.enable();
	}

	if (vm.count(kHelpOptionLongName))
	{
		TUTTLE_COUT( _color._blue  << "TuttleOFX project [http://sites.google.com/site/tuttleofx]" << _color._std << std::endl );
		TUTTLE_COUT( _color._blue  << "NAME" << _color._std );
		TUTTLE_COUT( _color._green << "\tsam-ls - list directory contents" << _color._std << std::endl);
		TUTTLE_COUT( _color._blue  << "SYNOPSIS" << _color._std );
		TUTTLE_COUT( _color._green << "\tsam-ls [options] [directories]" << _color._std << std::endl );
		TUTTLE_COUT( _color._blue  << "DESCRIPTION" << _color._std << std::endl );

		TUTTLE_COUT( "List information about the sequences, files and folders." );
		TUTTLE_COUT( "List the current directory by default, and only sequences." );
		TUTTLE_COUT( "The script option disable color, disable directory printing (in multi-directory case or recursive) and set relative path by default." << std::endl );

		TUTTLE_COUT( _color._blue  << "OPTIONS" << _color._std << std::endl );
		TUTTLE_COUT( mainOptions );
		return 0;
	}

	if ( vm.count(kBriefOptionLongName) )
	{
		TUTTLE_COUT( _color._green << "list directory contents" << _color._std);
		return 0;
	}

	if (vm.count(kExpressionOptionLongName))
	{
		TUTTLE_COUT( _color._red << "Expression: " << vm["expression"].as<std::string>() << _color._std );
		bal::split( filters, vm["expression"].as<std::string>(), bal::is_any_of(","));
	}

	if (vm.count(kDirectoriesOptionLongName))
	{
		listFolder = true;
	}
	
	if (vm.count(kFilesOptionLongName))
	{
		listUnitFile = true;
	}
	
	if (vm.count(kIgnoreOptionLongName))
	{
		maskSequences = true;
	}
	
	if (vm.count(kFullDisplayOptionLongName))
	{
		listFolder = true;
		listUnitFile = true;
		maskSequences = false;
	}

	if (vm.count(kAllOptionLongName))
	{
		// add .* files
		listDotFile = true;
	}
	
	if (vm.count(kLongListingOptionLongName))
	{
		listLongListing = true;
	}
	
	if (vm.count(kRelativePathOptionLongName) )
	{
		listRelativePath = true;
	}

	if(vm.count(kPathOptionLongName))
	{
		listAbsolutePath = true;
	}
	
	// defines paths, but if no directory specify in command line, we add the current path
	if (vm.count(kInputDirOptionLongName))
	{
		paths = vm[kInputDirOptionLongName].as< std::vector<std::string> >();
	}
	else
	{
		paths.push_back( "./" );
	}
	
	if (vm.count(kRecursiveOptionLongName))
	{
		recursiveListing = true;
	}
	
	// build filter into regex expression
	const std::vector<boost::regex> reFilters = convertFilterToRegex( filters );
	
	try
	{
		BOOST_FOREACH( bfs::path path, paths )
		{
			Items items = sequence::parser::browse( path.string().c_str(), recursiveListing );
			
			for( Items::iterator item = items.begin(); item != items.end(); item++ )
			{
				bfs::path p = (*item).path;
				if( listRelativePath )
					p = p.relative_path( );
				if( listAbsolutePath )
					p = bfs::absolute( p );
				
				switch ((*item).type) {
					case sequence::FOLDER:
					{
						if( listFolder && ( listDotFile || !isDotFilename( p ) ) && ! isFilteredFilename( p.string(), reFilters ) )
						{
							std::cout << ( listLongListing ? "d " : "") << _color._blue << p.make_preferred() << _color._std << std::endl;
							wasSthgDumped = true;
						}
						break;
					}
					case sequence::UNITFILE:
					{
					if( listUnitFile && ( listDotFile || !isDotFilename( p ) ) && ! isFilteredFilename( p.string(), reFilters ) )
						{
							std::cout << ( listLongListing ? "f " : "" ) << _color._green << p.make_preferred() << _color._std << std::endl;
							wasSthgDumped = true;
						}
						break;
					}
					case sequence::SEQUENCE:
					{
						const sequence::Sequence &sequence = (*item).sequence;
						if( !maskSequences && ( listDotFile || !isDotFilename( sequence.pattern.string() ) ) && ! isFilteredFilename( (p / sequence.pattern.string()).string(), reFilters ) )
						{
							std::cout << ( listLongListing ? "s " : "" ) << _color._green << (p / sequence.pattern.string()).make_preferred() << _color._std;
							std::cout << ' ' << sequence.range;
							if (sequence.step > 1)
								std::cout << " (" << sequence.step << ')';
							std::cout << std::endl;
							wasSthgDumped = true;
						}
						break;
					}
					case sequence::UNDEFINED:
					{
						std::cout << ( listLongListing ? "u " : "" ) << p.string() << std::endl;
						wasSthgDumped = true;
						break;
					}
					default:
					{
						std::cout << ( listLongListing ? "b " : "" ) << p.string() << std::endl;
						wasSthgDumped = true;
						//return "!BAD_TYPE!";
						break;
					}
				}
			}
		}
	}
	catch ( const bfs::filesystem_error& ex)
	{
		TUTTLE_COUT( ex.what() );
	}
	catch( ... )
	{
		TUTTLE_CERR ( boost::current_exception_diagnostic_information() );
	}
	if(!wasSthgDumped)
		TUTTLE_CERR ( _color._error << "No sequence found here." << _color._std );
	return 0;
}

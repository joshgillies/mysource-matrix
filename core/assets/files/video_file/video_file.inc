<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: video_file.inc,v 1.11 2012/08/30 01:09:07 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* Video_File
*
* Purpose
*    Custom File type for storing Video files. Allows Video files to be
*    handled as a separate file type where asset types need to be
*    selected (eg. in an Asset Listing). This asset also supports
*    auto-filling of video and audio metadata when the getID3()
*    external tool is available and properly set up.
*
*
* @author     Nicholas Hubbard <nic@zedsaid.com>
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.11 $
* @package    MySource_Matrix_Packages
* @subpackage __core__
*/
class Video_File extends File
{


	public $allowed_extensions = Array('mov', 'avi', 'wmv', 'asf', 'flv', 'mp4', 'm4v', 'mpg', 'mpeg', 'ogv', 'ogg', 'webm');


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The $replacement = value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @$replacement = array
	* @access public
	*/
	public function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_attribute_length'] = 'Length of File (in Minutes:Seconds)';
		$keywords['asset_attribute_length_seconds'] = 'Length of File (in Seconds)';
		
		// Add Alt Asset Keyword
		$keywords['asset_attribute_alt_asset_X'] = 'Alternate Asset Reference for Accessibility (replace X with keyword)';

		$keywords['asset_attribute_file_format_dynamic'] = 'Dynamic Keyword for File Format';
		$keywords['asset_attribute_audio_codec_dynamic'] = 'Dynamic Keyword for Audio Codec';
		$keywords['asset_attribute_channels_dynamic'] = 'Dynamic Keyword for Channels';
		$keywords['asset_attribute_channel_mode_dynamic'] = 'Dynamic Keyword for Channel Mode';
		$keywords['asset_attribute_bitrate_dynamic'] = 'Dynamic Keyword for Bit Rate';
		$keywords['asset_attribute_sample_rate_dynamic'] = 'Dynamic Keyword for Sample Rate';
		$keywords['asset_attribute_video_codec_dynamic'] = 'Dynamic Keyword for Video Codec';
		$keywords['asset_attribute_width_dynamic'] = 'Dynamic Keyword for Width';
		$keywords['asset_attribute_height_dynamic'] = 'Dynamic Keyword for Height';
		$keywords['asset_attribute_fps_dynamic'] = 'Dynamic Keyword for Frames Per Second';
		$keywords['asset_attribute_length_dynamic'] = 'Dynamic Keyword for Length (in Minutes:Seconds)';
		$keywords['asset_attribute_length_seconds_dynamic'] = 'Dynamic Keyword for Length (in Seconds)';
		$keywords['asset_attribute_video_title_dynamic'] = 'Dynamic Keyword for Video Title';
		$keywords['asset_attribute_artist_dynamic'] = 'Dynamic Keyword for Artist';
		$keywords['asset_attribute_album_dynamic'] = 'Dynamic Keyword for Album';
		$keywords['asset_attribute_genre_dynamic'] = 'Dynamic Keyword for Genre';
		$keywords['asset_attribute_comments_dynamic'] = 'Dynamic Keyword for Comments';
		$keywords['asset_attribute_copyright_dynamic'] = 'Dynamic Keyword for Copyright';
		$keywords['asset_attribute_year_dynamic'] = 'Dynamic Keyword for Year';
		
		// Remove set Alt Asset Keyword, replace with alternate keyword
		foreach ($keywords as $key => $val) {
			if ($key == 'asset_attribute_alt_asset') {
				unset($keywords[$key]);
				break;
			}//end if
		}//end foreach
		
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for dynamically extracting data from an
	* Video file and using it in place of the keyword.  Keywords data is
	* pulled from the file itself and does not rely on the data being
	* in the database.
	*
	* @param string	$keyword	Keyword to find a replacement for
	* @param string $additional_replacements    Additional keyword replacements for 'replace_keywords' modifier
	*
	* @$replacement = string
	* @access public
	*/
	function getKeywordReplacement($keyword, $additional_replacements=Array())
	{
		$replacement = NULL; 
		
		// Remove any modifiers from keyword...but save the full keyword to
		// pass to parent asset in case our results are fruitless
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);
		$contextid = extract_context_modifier($modifiers);

		if ($contextid !== NULL) {
			// If we were able to extract a context ID to change to, and it's
			// different to our current one, then change and then reload a copy
			// of the asset in that context (as we would need to do anyway)

			if ((int)$contextid !== $GLOBALS['SQ_SYSTEM']->getContextId()) {
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$contexted_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->id);

				// Get the keyword without any modifiers
				$replacement = $contexted_asset->getKeywordReplacement($keyword);

				// Then apply the ones we had remaining, then return it
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $contexted_asset->id));
				
				unset($contexted_asset);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return $replacement;

			}//end if contextid is not the currently active context

		}//end if contextid is not NULL

		// Static/overriding keywords
		switch ($keyword) {
			case 'asset_attribute_length' :
				// Length should be the time in minutes:seconds
				$length = $this->attr('length');
				$minutes = floor($length / 60);
				$seconds = $length % 60;
				$replacement = $minutes.':'.str_pad($seconds, 2, '0', STR_PAD_LEFT);
			break;

			case 'asset_attribute_length_seconds' :
				// Should also be able to provide time in seconds if we need to
				$replacement = $this->attr('length');
			break;

		}//end switch
		
		// Keywords for alternate video file asset
		if (strpos($keyword, 'asset_attribute_alt_asset_') !== FALSE) {
			$alt_asset_key = substr($keyword, 26);
			// Set our new asset object
			$alt_assetid = $this->attr('alt_asset');
			// Make sure we have an asset id
			if (!empty($alt_assetid) && is_numeric($alt_assetid)) {
				$alt_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($alt_assetid);
				$replacement = !empty($alt_asset) ? $alt_asset->getKeywordReplacement($alt_asset_key) : '';
			} else {
				$replacement = '';
			}//end else
		}//end if

		// Dynamic keywords lie below here. These strip the video information direct from
		// the files, ignoring what is currently stored in the asset.
		if (substr($keyword, -8) === '_dynamic') {
			// Extract tag information - if failed then pretend it returned nothing
			$tags = $this->extractID3Metadata();
			if ($tags === FALSE) $tags = Array();

			// Get the actual attribute being dynamically requested
			$sub_keyword = substr($keyword, 16, -8);

			switch ($sub_keyword) {
				case 'file_format':
					$format = array_get_index($tags, 'file_format', '');
					$format = ucfirst($format);
					$replacement = $format;
				break;

				case 'audio_codec':
				case 'channels':
				case 'sample_rate':
				case 'video_codec':
				case 'width':
				case 'height':
				case 'fps':
				case 'video_title':
				case 'artist':
				case 'album':
				case 'genre':
				case 'comments':
				case 'copyright':
				case 'year':
					$replacement = array_get_index($tags, $sub_keyword, '');
				break;

				case 'channel_mode':
					$channel_mode = array_get_index($tags, 'channel_mode', '');
					$channel_mode = ucfirst($channel_mode);
					$replacement = $channel_mode;
				break;

				case 'bitrate':
					$bitrate = array_get_index($tags, $sub_keyword, '');
					if (is_numeric($bitrate)) {
						// Bitrate is returned in bit/s, but we are displaying in
						// kbit/s (rounded down)
						$bitrate = floor($bitrate / 1000);
					}
					$replacement = $bitrate;
				break;

				case 'length_seconds':
					$length = array_get_index($tags, 'length', '');
					if (is_numeric($length)) {
						// Round off the number of seconds (up or down)
						$length = round($length);
					}
					$replacement = $length;
				break;

				case 'length':
					$length = array_get_index($tags, 'length', '');
					if (is_numeric($length)) {
						// Round off the number of seconds (up or down)
						$length = round($length);
						// Now format it
						$minutes = floor($length / 60);
						$seconds = $length % 60;
						$length = $minutes.':'.str_pad($seconds, 2, '0', STR_PAD_LEFT);
					}
					$replacement = $length;
				break;

			}//end switch

		}//end if dynamic keyword

		if (isset($replacement)) {
			if (count($modifiers) > 0) {
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			}

			return $replacement;
		} else {
			// Use the full keyword so modifiers still get used
			return parent::getKeywordReplacement($full_keyword);
		}

	}//end getKeywordReplacement()


	/**
	* Save attributes for this asset
	*
	* Overrides the setting of the extract_id3 attribute. If set, it will reset the
	* attribute, and set the appropriate metadata fields to the extracted video data.
	*
	* @param boolean	$dont_run_updated	If TRUE, saveAttributes() in Asset class
	*										won't run _updated() afterward. (Only to
	*										be called from create() function.)
	*
	* @return boolean
	*/
	public function saveAttributes($dont_run_updated=FALSE)
	{
		if ($this->attr('extract_id3')) {
			$tags = $this->extractID3Metadata();
			if ($tags !== FALSE) {

				// Go through the info we do have, setting what we can, and leaving
				// alone what is not there.
				foreach ($tags as $tag_name => $tag_value) {
					switch ($tag_name) {
						case 'file_format' :
							$this->setAttrValue('file_format', ucfirst($tag_value));
						break;

						case 'channel_mode' :
							$this->setAttrValue('channel_mode', ucfirst($tag_value));
						break;

						case 'bitrate' :
							// Bitrate is provided in bit/s. We are saving this as a rounded-down
							// bitrate in kbit/s instead.
							$this->setAttrValue($tag_name, floor($tag_value / 1000));
						break;
						
						// These are all the same, run them through the same function
						case 'channels' :
						case 'audio_codec' :
						case 'sample_rate' :
							// Attribute had no underscore.
						case 'video_codec' :
						case 'width' :
						case 'height' :
						case 'fps' :
						case 'video_title' :
						case 'artist' :
						case 'album' :
						case 'genre' :
						case 'comments' :
						case 'copyright' :
						case 'year' :
							$this->setAttrValue($tag_name, $tag_value);
						break;

						case 'length' :
							// Length is stored in fractional seconds. This will be rounded (up
							// or down) to the nearest whole second.
							$this->setAttrValue($tag_name, round($tag_value));
						break;
					}//end switch

				}//end foreach

			}//end if tags returned

			// Set our extraction ID3 option back to NO so that changed values don't get overwritten
			$this->setAttrValue('extract_id3', FALSE);

		}//end if extract ID3 is true

		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Pulls out metadata from a specified file using getID3() and
	* returns them in a more friendly format
	*
	* Returns FALSE if extraction is disabled, or there was a
	* problem with extraction.
	*
	* Video data returned: file format, audio codec, channels channel mode, bitrate,
	* sample rate, video codec, video width, video height, duration.
	*
	* @param string	$file_name	The file to check for metadata. Defaults
	*							To the current file.
	*
	* @return mixed
	*/
	public function extractID3Metadata($file_name=NULL)
	{
		if (is_null($file_name)) {
			if ($this->usePublicPath()) {
				$file_name = $this->data_path_public.'/'.$this->attr('name');
			} else {
				$file_name = $this->data_path.'/'.$this->attr('name');
			}
		}

		// getID3 tool disabled in External Tools? Silently fail.
		if (!SQ_TOOL_GETID3_ENABLED) return FALSE;

		// getID3 external tool directory does not exist?
		if (!is_dir(SQ_TOOL_GETID3_PATH)) {
			trigger_error('Cannot extract Video metadata; path to getID3() external tool does not exist or is not a directory', E_USER_WARNING);
			return FALSE;
		}

		// ...or is not a getID3 module at all?
		if (!is_file(SQ_TOOL_GETID3_PATH.'/getid3.php')) {
			trigger_error('Cannot extract Video metadata; path to getID3() external tool does not point to a valid getID3() install', E_USER_WARNING);
			return FALSE;
		}

		if (!is_file($file_name)) {
			trigger_error('Cannot extract Video metadata; Video file path provided does not exist', E_USER_WARNING);
		}

		require_once SQ_TOOL_GETID3_PATH.'/getid3.php';

		// Now get a new getID3() object and analyse the stored file
		$id3 = new getID3;
		$id3->Analyze($file_name);
		$use_ogg = FALSE;
		
		$file_duration = 0;
		$file_bitrate = 0;
		$file_samplerate = 0;

		// Check and see if we are using OGG, which getID3() does not support
		if ($id3->info['fileformat'] == 'ogg') {
			require_once SQ_DATA_PATH.'/private/conf/tools.inc';

			// Go away please
			if (!SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED) return FALSE;

			if (!is_file(SQ_TOOL_OGG_METADATA_EXTRACTION_PATH)) {
				trigger_error('Cannot extract OGG metadata; path to OGG Metadata Extraction external tool does not exist or is not a file', E_USER_WARNING);
				return FALSE;
			}

			$use_ogg = TRUE;

			// Include our support file which is our own class file
			// this class file extends the original ogg.class.php
			require_once SQ_CORE_PACKAGE_PATH.'/files/video_file/lib/Matrix_Ogg.inc';

			// We don't want to use the cache option
			$ogg = new Matrix_Ogg($file_name, NOCACHING);
			// Video
			$theora = isset($ogg->Streams['theora']) ? $ogg->Streams['theora'] : NULL;
			// Audio
			$vorbis = isset($ogg->Streams['vorbis']) ? $ogg->Streams['vorbis'] : NULL;

			// Data relating to the file itself.
			$file_duration = $ogg->Streams['duration'];
			$file_samplerate = isset($vorbis) ? $vorbis['samplerate'] : 0;

			if ($theora) {
				$file_bitrate = $theora['nombr'];
			} else if ($vorbis) {
				$file_bitrate = $vorbis['bitrate'];
			}

		} else {
			$file_duration = $id3->info['playtime_seconds'];
			$file_bitrate = $id3->info['bitrate'];
			$file_samplerate = isset($id3->info['audio']['sample_rate']) ? $id3->info['audio']['sample_rate'] : 0;
		}
		
		$audio_data = Array(
						'length'		=> $file_duration,
						'bitrate'		=> $file_bitrate,
						'sample_rate'	=> $file_samplerate,
						'file_format'	=> $id3->info['fileformat'],
					  );

		$id3_tag_data = Array();

		// Set our vars to grab info from the audio and video arrays
		$tag_audio =& $id3->info['audio'];
		$tag_video =& $id3->info['video'];
		
		$tag_quicktime = NULL;
		if (isset($id3->info['tags']['quicktime'])) {
			$tag_quicktime =& $id3->info['tags']['quicktime'];
		}

		// Grab the data and put it into our array
		if ($use_ogg) {
			if (!is_null($vorbis)) {
				$id3_tag_data['audio_codec'] = $vorbis['vendor'];
				$id3_tag_data['channels'] = $vorbis['channels'];
				$id3_tag_data['channel_mode'] = $vorbis['channels'] == 2 ? 'Stereo' : 'Mono';
			}
			if (!is_null($theora)) {
				$id3_tag_data['video_codec'] = $theora['vendor'];
				$id3_tag_data['width'] = $theora['width'];
				$id3_tag_data['height'] = $theora['height'];
				$id3_tag_data['fps'] = $theora['frate'];

				// Extra data fields
				if (isset($theora['comments'])) {
					foreach($theora['comments'] as $comment) {
						$comment_field = explode('=', $comment);
						$cf = $comment_field[1];
						switch ($comment_field[0]) {
							case 'title' :
								$id3_tag_data['video_title'] = $cf;
							break;
							case 'artist' :
								$id3_tag_data['artist'] = $cf;
							break;
							case 'album' :
								$id3_tag_data['album'] = $cf;
							break;
							case 'genre' :
								$id3_tag_data['genre'] = $cf;
							break;
							case 'description' :
								$id3_tag_data['comments'] = $cf;
							break;
							case 'copyright' :
								$id3_tag_data['copyright'] = $cf;
							break;
							case 'year' :
								$id3_tag_data['year'] = $cf;
							break;
						}//end switch
						
					}//end foreach
					
				}//end if (comments)
				
			}//end if (theora)
			
		} else {
			// Not ogg format
			
			if (isset($tag_audio['codec'])) {
				$id3_tag_data['audio_codec'] = $tag_audio['codec'];
			}
			if (isset($tag_audio['channels'])) {
				$id3_tag_data['channels'] = $tag_audio['channels'];
			}
			if (isset($tag_audio['channelmode'])) {
				$id3_tag_data['channel_mode'] = $tag_audio['channelmode'];
			}
			if (isset($tag_video['codec'])) {
				$id3_tag_data['video_codec'] = $tag_video['codec'];
			}
			if (isset($tag_video['resolution_x'])) {
				$id3_tag_data['width'] = $tag_video['resolution_x'];
			}
			if (isset($tag_video['resolution_y'])) {
				$id3_tag_data['height'] = $tag_video['resolution_y'];
			}
			// Support for FPS is sketchy across differnt video formats, but we will try
			// getID3() v1.7.9 must be used to return correct frame rates for Quicktime
			if (isset($tag_video['frame_rate'])) {
				$id3_tag_data['fps'] = $tag_video['frame_rate'];
			}
			// Extra data fields
			if (!is_null($tag_quicktime)) {
				if (isset($tag_quicktime['title'][0])) {
					$id3_tag_data['video_title'] = $tag_quicktime['title'][0];
				}
				if (isset($tag_quicktime['artist'][0])) {
					$id3_tag_data['artist'] = $tag_quicktime['artist'][0];
				}
				if (isset($tag_quicktime['album'][0])) {
					$id3_tag_data['album'] = $tag_quicktime['album'][0];
				}
				if (isset($tag_quicktime['genre'][0])) {
					$id3_tag_data['genre'] = $tag_quicktime['genre'][0];
				}
				if (isset($tag_quicktime['comment'][0])) {
					$id3_tag_data['comments'] = $tag_quicktime['comment'][0];
				}
				if (isset($tag_quicktime['copyright'][0])) {
					$id3_tag_data['copyright'] = $tag_quicktime['copyright'][0];
				}
				if (isset($tag_quicktime['creation_date'][0])) {
					$id3_tag_data['year'] = $tag_quicktime['creation_date'][0];
				}
			}
			
		}//end else (ogg)
		
		$data = array_merge($audio_data, $id3_tag_data);
		return $data;

	}//end extractID3Metadata()


}//end class
?>

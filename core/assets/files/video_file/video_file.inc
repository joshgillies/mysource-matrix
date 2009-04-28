<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: video_file.inc,v 1.1.4.2 2009/04/28 06:37:04 mbrydon Exp $
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
* @version    $Revision: 1.1.4.2 $
* @package    MySource_Matrix_Packages
* @subpackage __core__
*/
class Video_File extends File
{


	public $allowed_extensions = Array('mov', 'avi', 'wmv', 'asf', 'flv', 'mp4', 'm4v', 'mpg', 'mpeg');


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
	*
	* @$replacement = string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
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
					$replacement = array_get_index($tags, 'audio_codec', '');
				break;

				case 'channels':
					$replacement = array_get_index($tags, 'channels', '');
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

				case 'sample_rate':
					$replacement = array_get_index($tags, 'sample_rate', '');
				break;

				case 'length_seconds':
					$length = array_get_index($tags, 'length', '');
					if (is_numeric($length)) {
						// Round off the number of seconds (up or down)
						$length = round($length);
					}
					$replacement = $length;
				break;

				case 'video_codec':
					$replacement = array_get_index($tags, 'video_codec', '');
				break;

				case 'width':
					$replacement = array_get_index($tags, 'width', '');
				break;

				case 'height':
					$replacement = array_get_index($tags, 'height', '');
				break;

				case 'fps':
					$replacement = array_get_index($tags, 'fps', '');
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
			return $replacement;
		} else {
			return parent::getKeywordReplacement($keyword);
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

						case 'audio_codec' :
							$this->setAttrValue('audio_codec', $tag_value);
						break;

						case 'channels' :
							$this->setAttrValue('channels', $tag_value);
						break;

						case 'channel_mode' :
							$this->setAttrValue('channel_mode', ucfirst($tag_value));
						break;

						case 'bitrate' :
							// Bitrate is provided in bit/s. We are saving this as a rounded-down
							// bitrate in kbit/s instead.
							$this->setAttrValue($tag_name, floor($tag_value / 1000));
						break;

						case 'sample_rate' :
							// Attribute had no underscore.
							$this->setAttrValue('sample_rate', $tag_value);
						break;

						case 'video_codec' :
							$this->setAttrValue('video_codec', $tag_value);
						break;

						case 'width' :
							$this->setAttrValue('width', $tag_value);
						break;

						case 'height' :
							$this->setAttrValue('height', $tag_value);
						break;

						case 'fps' :
							$this->setAttrValue('fps', $tag_value);
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

		// Data relating to the file itself.
		$audio_data = Array(
						'length'		=> $id3->info['playtime_seconds'],
						'bitrate'		=> $id3->info['bitrate'],
						'sample_rate'	=> $id3->info['audio']['sample_rate'],
						'file_format'	=> $id3->info['fileformat'],
					  );

		$id3_tag_data = Array();

		// Set our vars to grab info from the audio and video arrays
		$tag_audio =& $id3->info['audio'];
		$tag_video =& $id3->info['video'];

		// Grab the data and put it into our array
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

		$data = array_merge($audio_data, $id3_tag_data);
		return $data;

	}//end extractID3Metadata()


}//end class
?>

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
* $Id: mp3_file.inc,v 1.3 2009/01/13 04:27:24 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';

/**
* MP3_File
*
* Purpose
*    Custom File type for storing MP3 files. Allows MP3 files to be
*    handled as a separate file type where asset types need to be
*    selected (eg. in an Asset Listing). This asset also supports
*    auto-filling of ID3 and other audio metadata when the getID3()
*    external tool is available and properly set up.
*
*
* @author     Nicholas Hubbard <nic@zedsaid.com>
* @author     Luke Wright <lwright@squiz.net>
* @version    $Revision: 1.3 $
* @package    MySource_Matrix_Packages
* @subpackage __core__
*/
class MP3_File extends File
{


	public $allowed_extensions = Array('mp3');


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}// end constructor
	
	
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

		$keywords['asset_attribute_song_dynamic'] = 'Dynamic MP3 Keyword for Songs Name';
		$keywords['asset_attribute_album_dynamic'] = 'Dynamic MP3 Keyword for Album';
		$keywords['asset_attribute_artist_dynamic'] = 'Dynamic MP3 Keyword for Artist';
		$keywords['asset_attribute_track_dynamic'] = 'Dynamic MP3 Keyword for Track';
		$keywords['asset_attribute_bitrate_dynamic'] = 'Dynamic MP3 Keyword for Bit Rate';
		$keywords['asset_attribute_length_dynamic'] = 'Dynamic MP3 Keyword for Length (in Minutes:Seconds)';
		$keywords['asset_attribute_length_seconds_dynamic'] = 'Dynamic MP3 Keyword for Length (in Seconds)';
		$keywords['asset_attribute_year_dynamic'] = 'Dynamic MP3 Keyword for Year';
		$keywords['asset_attribute_genre_dynamic'] = 'Dynamic MP3 Keyword for Genre';
		$keywords['asset_attribute_samplerate_dynamic'] = 'Dynamic MP3 Keyword for Sample Rate';

		return $keywords;

	}// end getAvailableKeywords()
	
	
	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for dynamically extracting ID3 data from an
	* MP3 file and using it in place of the keyword.  Keywords data is
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
		switch ($keyword)
		{
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

		// Dynamic keywords lie below here. These strip the ID3 tags direct from
		// the files, ignoring what is currently stored in the asset.
		if (substr($keyword, -8) === '_dynamic') {
			// Extract tag information - if failed then pretend it returned nothing
			$tags = $this->extractID3Metadata();
			if ($tags === FALSE) $tags = Array();
			
			// Get the actual attribute being dynamically requested
			$sub_keyword = substr($keyword, 16, -8);

			switch ($sub_keyword) {
				case 'song':
					$replacement = array_get_index($tags, 'title', '');
				break;

				case 'album':
				case 'artist':
				case 'track':
				case 'genre':
				case 'year':
					$replacement = array_get_index($tags, $sub_keyword, '');
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
		
				case 'samplerate':
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

	}// end getKeywordReplacement()


	/**
	* Save attributes for this asset
	*
	* Overrides the setting of the extract_id3 attribute. If set, it will reset the
	* attribute, and set the appropriate metadata fields to the extracted ID3 data.
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
						case 'title' :
							// Named 'title' in our extraction function. Attribute is 'song'
							// for backwards-compatibility reasons.
							$this->setAttrValue('song', $tag_value);
						break;

						case 'album' :
						case 'artist' :
						case 'track' :
						case 'year' :
						case 'genre' :
							// These just carry over to the respective attributes
							$this->setAttrValue($tag_name, $tag_value);
						break;

						case 'sample_rate' :
							// Attribute had no underscore.
							$this->setAttrValue('samplerate', $tag_value);
						break;

						case 'bitrate' :
							// Bitrate is provided in bit/s. We are saving this as a rounded-down
							// bitrate in kbit/s instead.
							$this->setAttrValue($tag_name, floor($tag_value / 1000));
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
	* ID3 tags returned: title, album, artist, track, year, genre.
	* ID3v2 tags are checked first, if they don't exist then ID3v1 is.
	* For ID3v2, the "content type" tag is either converted to an
	* ID3v1 genre if in the correct format, or the whole field is
	* returned as genre. Only those fields actually extracted are
	* returned.
	*
	* Audio-related tags returned (these will be returned even if
	* there is no ID3 data to return):
	* - length: Duration of track in seconds. Probably fractional.
	* - bitrate: The bitrate (or average bitrate if VBR) in bits/s.
	*   Likely fractional if VBR.
	* - sample_rate: The sample rate in Hz
	* - is_vbr: TRUE if encoded with variable bit rate
	* - channels: 1 for mono, 2 for stereo, 5/6 for MP3 Surround files
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
			trigger_error('Cannot extract ID3 metadata; path to getID3() external tool does not exist or is not a directory', E_USER_WARNING);
			return FALSE;
		}

		// ...or is not a getID3 module at all?
		if (!is_file(SQ_TOOL_GETID3_PATH.'/getid3.php')) {
			trigger_error('Cannot extract ID3 metadata; path to getID3() external tool does not point to a valid getID3() install', E_USER_WARNING);
			return FALSE;
		}
		
		if (!is_file($file_name)) {
			trigger_error('Cannot extract ID3 metadata; MP3 file path provided does not exist', E_USER_WARNING);
		}

		require_once SQ_TOOL_GETID3_PATH.'/getid3.php';

		// Now get a new getID3() object and analyse the stored file
		$id3 = new getID3;
		$id3->Analyze($file_name);

		if ($id3->info['fileformat'] !== 'mp3') {
			trigger_error('Cannot extract ID3 metadata; File path passed is not an MP3 file', E_USER_WARNING);
			return FALSE;
		}

		// Data relating to the audio itself.
		// Channels stored as number rather than "mono/stereo" flag - to accommodate
		// the rare MP3 Surround file that uses 5/6 channels.
		$audio_data = Array(
						'length'			=> $id3->info['playtime_seconds'],
						'bitrate'			=> $id3->info['bitrate'],
						'sample_rate'		=> $id3->info['audio']['sample_rate'],
						'is_vbr'			=> ($id3->info['audio']['bitrate_mode'] === 'vbr'),
						'channels'			=> $id3->info['audio']['channels'],
					  );

		$id3_tag_data = Array();

		if (isset($id3->info['tags']['id3v2'])) {
			$tag =& $id3->info['tags']['id3v2'];

			// Only use the first listed entry for each tag
			foreach	($tag as $tag_index => $tag_value) {
				if (is_array($tag_value)) {
					$tag[$tag_index] = $tag_value[0];
				}
			}

			if (isset($tag['content_type'])) {
				$genres = getID3_ID3v2::ParseID3v2GenreString($tag['content_type']);
				if (count($genres) > 0) {
					$tag['genre'] = $genres['genre'][0];
				}
			}

			if (isset($tag['track_number'])) {
        		$tag['track'] = $tag['track_number'];
			}
		} else if (isset($id3->info['tags']['id3v1'])) {
			$tag =& $id3->info['tags']['id3v1'];
			
			// Only use the first listed entry for each tag
			foreach	($tag as $tag_index => $tag_value) {
				if (is_array($tag_value)) {
					$tag[$tag_index] = $tag_value[0];
				}
			}
		}

		if (isset($tag)) {
			if (isset($tag['title'])) {
				$id3_tag_data['title'] = $tag['title'];
			}
			if (isset($tag['artist'])) {
				$id3_tag_data['artist'] = $tag['artist'];
			}
			if (isset($tag['album'])) {
				$id3_tag_data['album'] = $tag['album'];
			}
			if (isset($tag['track'])) {
				$id3_tag_data['track'] = $tag['track'];
			}
			if (isset($tag['genre'])) {
				$id3_tag_data['genre'] = $tag['genre'];
			}
			if (isset($tag['year'])) {
				$id3_tag_data['year'] = $tag['year'];
			}
		}

		$data = array_merge($audio_data, $id3_tag_data);
		return $data;

	}//end extractID3Metadata()


}// end class
?>

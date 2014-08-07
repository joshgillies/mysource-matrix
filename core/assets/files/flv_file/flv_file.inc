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
* $Id: flv_file.inc,v 1.9 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* FLV_File
*
* Purpose
*
*
* @author  Anh Ta <ata@squiz.co.uk>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class FLV_File extends File
{


	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	public $allowed_extensions = Array('flv');
	
	
	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);
		
		//make asset_attribute_<metadata_name> keywords available
		$metadata = $this->attr('attributes');
		foreach ($metadata as $meta_name => $meta_val) {
			$this->vars[$meta_name] = Array('value' => $meta_val, 'type' => 'text', 'attrid' => 0);
		}

	}//end constructor


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	public function getAvailableKeywords()
	{
		$keywords = parent::getAvailableKeywords();

		$keywords['asset_attribute_duration_string'] = 'Duration of the FLV file (in Minutes:Seconds)';
		
		$keywords['asset_attribute_duration_int'] = 'Rounded duration of the FLV file in seconds';
		$keywords['asset_attribute_width_int'] = 'The rounded width of the video in pixels';
		$keywords['asset_attribute_height_int'] = 'The rounded height of the video in pixels';
		$keywords['asset_attribute_framerate_int'] = 'Rounded number of frames per second';
		$keywords['asset_attribute_videodatarate_int'] = 'Rounded video bit rate in kilobits per second';
		$keywords['asset_attribute_videocodecid_int'] = 'The rounded video codec ID used in the file';
		$keywords['asset_attribute_audiodatarate_int'] = 'Rounded audio bit rate in kilobits per second';
		$keywords['asset_attribute_audiocodecid_int'] = 'The rounded audio codec ID used in the file';
		$keywords['asset_attribute_audiosamplerate_int'] = 'The rounded frequency at which the audio stream is replayed';
		$keywords['asset_attribute_audiosamplesize_int'] = 'The rounded resolution of a single audio sample';

		$keywords['asset_attribute_XXX'] = 'Generic FLV metadata keyword';
		$keywords['asset_attribute_XXX_int'] = 'Generic rounded FLV metadata keyword';
		
		return $keywords;

	}// end getAvailableKeywords()
	
	
	/**
	* Get bodycopy keyword replacement
	* 
	* The keywords begin with flv_metadata_ will return the FLV metadata from this asset attributes if they exist, otherwise
	* fresh metadata from the FLV file will be returned.
	* 
	* The keywords begin with dynamic_flv_metadata_ always return fresh metadata from the FLV file.
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	* @see _getKeywordReplacement
	*/
	public function getKeywordReplacement($keyword)
	{
		$replacement = NULL;

		// Remove any modifiers from keyword
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

		//only process keywords that start with asset_attribute_
		if ('asset_attribute_' == substr($keyword, 0, 16)) {
			if ($keyword == 'asset_attribute_duration_string') {
				//want a duration string Minutes:Seconds
				$replacement =  self::convertTimeToString($this->attr('duration'));
				
			} else {
				$is_int = FALSE;
				if ('_int' == substr($keyword, -4)) {
					$keyword = substr($keyword, 0, -4);
					$is_int = TRUE;
				}
				//get the replacement for keyword from parent asset
				$replacement = parent::getKeywordReplacement($keyword);
				
				if (is_numeric($replacement)) { //the replacement is a number
					//if the keyword ends with _int, round it 
					if ($is_int) {
						$replacement = (int)(round($replacement));
					}
					
				} else { //the replacement is a string
					//replace HTML special characters so that malicious users can not use FLV metadata to make a XSS attack
					$replacement = nl2br(htmlspecialchars($replacement));
				}
			}
		}

		if ($replacement !== NULL) {
			if (count($modifiers) > 0) {
				apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
			}			
		} else {
			// use full keyword so the modifiers still get used
			$replacement = parent::getKeywordReplacement($full_keyword);
		}

		return $replacement;
		
	}//end getKeywordReplacement()
	
	
	/**
	* Save attributes set with setAttrValue to the database
	* 
	* This method is overrided here because we want to use the freshly extracted metadata if the checkbox Extract Metadata
	* in the details screen is checked. Setting the extract_metadata attribute to TRUE will trigger this asset to get 
	* fresh metadata from the FLV file and store them in its attributes. 
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	public function saveAttributes($dont_run_updated=FALSE)
	{
		$stored_metadata = Array(
							'duration'			=> Array('attr_name' => 'duration', 'default' => 0),
							'width'				=> Array('attr_name' => 'width', 'default' => 0),
							'height'			=> Array('attr_name' => 'height', 'default' => 0),
							'framerate'			=> Array('attr_name' => 'framerate', 'default' => 0),
							'videodatarate'		=> Array('attr_name' => 'videodatarate', 'default' => 0),
							'videocodecid'		=> Array('attr_name' => 'videocodecid', 'default' => 0),
							'audiodatarate'		=> Array('attr_name' => 'audiodatarate', 'default' => 0),
							'audiocodecid'		=> Array('attr_name' => 'audiocodecid', 'default' => 0),
							'audiosamplerate'	=> Array('attr_name' => 'audiosamplerate', 'default' => 0),
							'audiosamplesize'	=> Array('attr_name' => 'audiosamplesize', 'default' => 0),
						   );

		//if the extract metadata attribute is set, set all the metadata attributes to new values from the FLV metadata
		if ($this->attr('extract_metadata')) {
			$metadata = $this->extractMetadata();
			if ($metadata !== FALSE) {
				//put all the metadata specified in the _store_metadata array to the corresponding attributes
				foreach ($stored_metadata as $meta_name => $attr) {
					if (array_key_exists($meta_name, $metadata)) {
						$this->setAttrValue($attr['attr_name'], $metadata[$meta_name]);
						unset($metadata[$meta_name]);
					} else {
						$this->setAttrValue($attr['attr_name'], $attr['default']);
					}
				}
				//save the rest of metadata to the attribute "attributes"
				$this->setAttrValue('attributes', $metadata);
			}
			
			//set the extract metadata attribute to FALSE again so that the default value of extract metadata checkbox is unchecked
			$this->setAttrValue('extract_metadata', FALSE);
		}
		
		return parent::saveAttributes($dont_run_updated);
		
	}//end saveAttributes()
	
	
	/**
	 * Return the metadata of the FLV file of this asset. The result array has the following format
	 * 
	 * Array (
	 * 			metadata name => metadata value
	 * 		 )
	 * 
	 * @return	mixed	Return FALSE if the getID3() tool is disabled or there is no FLV file; otherwise, return the array of FLV metadata
	 * @access	public
	 */
	public function extractMetadata()
	{
		//check if getID3() is enabled
		if (!SQ_TOOL_GETID3_ENABLED || !is_file(SQ_TOOL_GETID3_PATH.'/getid3.php')) {
			return FALSE;
		}

		require_once SQ_TOOL_GETID3_PATH.'/getid3.php';

		//get file info
		$file_info = $this->getExistingFile();
		if (empty($file_info)) {
			return FALSE;
		}

		$getID3 = new getID3();
		$file_info = $getID3->analyze($file_info['path']);

		$metadata = Array();

		if (isset($file_info['flv']['meta']['onMetaData'])) {
			foreach ($file_info['flv']['meta']['onMetaData'] as $key => $value) {
				$metadata[$key] = $value;
			}
		}

		//if duration metadata does not exist, use playtime_seconds instead
		if (!array_key_exists('duration', $metadata) && isset($file_info['playtime_seconds'])) {
			$metadata['duration'] = $file_info['playtime_seconds'];
		}

		//if there are no width or height metadata, try to use video resolution instead
		if (!array_key_exists('width', $metadata) && isset($file_info['video']['resolution_x'])) {
			$metadata['width'] = $file_info['video']['resolution_x'];
		}
		if (!array_key_exists('height', $metadata) && isset($file_info['video']['resolution_y'])) {
			$metadata['height'] = $file_info['video']['resolution_y'];
		}

		//if framerate metadata does not exist, use video frame rate instead
		if (!array_key_exists('framerate', $metadata) && isset($file_info['video']['frame_rate'])) {
			$metadata['framerate'] = $file_info['video']['frame_rate'];
		}

		return $metadata;

	}//end extractMetadata()
	
	
	/**
	 * Convert a float time in seconds to a time string with the format Minutes:Second
	 * 
	 * @param $time		the input time in float
	 * @return string	the return time
	 * @access public
	 */
	public static function convertTimeToString($time)
	{
		$minutes = floor($time / 60);
		$seconds = floor($time % 60);
		
		return $minutes.':'.str_pad($seconds, 2, '0', STR_PAD_LEFT);
		
	}//end convertTimeToString
	
	
}//end class

?>

<?php

require_once SQ_CORE_PACKAGE_PATH.'/image/image_edit_fns.inc';

/**
* Thumbnail_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Thumbnail_Edit_Fns extends Image_Edit_Fns
{
	
	/**
	* The maxium width a thumbnail can have (in pixels)
	* @var int
	*/
	var $max_width  = 100;

	/**
	* The maxium height a thumbnail can have (in pixels)
	* @var int
	*/
	var $max_height = 100;


	/**
	* Constructor
	*
	*/
	function Thumbnail_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}//end constructor


	/**
	* Prints the interface for uploading a file
	*
	* @param object Thumbnail			&$asset	the file asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		file_upload($prefix);

		?>
		<br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?>
		<br/><b>Note:</b> Image size is restricted to a maximum of <?php echo $this->max_width.' x '.$this->max_height?> pixels</span>
		<?php

		$this->_printExistingFileInfo($asset);
		return true;
	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	* @param array(string)				$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		// check that the image dimensions are not greater than our limits
		$size = @getimagesize($info['tmp_name']);
		if (!empty($size) && ($size[0] > $this->max_width || $size[1] > $this->max_height)) {
			trigger_error('Uploaded file was too large. You may only upload a file no more than '.$this->max_width.' x '.$this->max_height.' pixels', E_USER_WARNING);
			return false;
		}

		return parent::processFileUpload($asset, $o, $prefix, $info);

	}//end processFileUpload()

}//end class
?>

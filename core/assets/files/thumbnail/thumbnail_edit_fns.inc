<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: thumbnail_edit_fns.inc,v 1.18 2004/08/05 03:42:38 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/image/image_edit_fns.inc';

/**
* Thumbnail_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thumbnail_Edit_Fns extends Image_Edit_Fns
{

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
		if ($asset->writeAccess('attributes')) {
			file_upload($prefix);

			?>
			<br/><span style="font-size: 7pt"><b>Note:</b> You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?>
			<br/><b>Note:</b> Image size is restricted to a maximum of <?php echo $asset->max_width.' x '.$asset->max_height?> pixels</span>
			<?php
		}

		$this->printExistingFileInfo($asset);
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
		if (!$asset->writeAccess('attributes')) return false;

		if (empty($info)) {
			$info = get_file_upload_info($prefix);
			// return on failed or no upload
			if ($info === false || empty($info)) return false;
		}

		// check that the image dimensions are not greater than our limits
		if (!$asset->validFile($info['name'])) return false;

		return parent::processFileUpload($asset, $o, $prefix, $info);

	}//end processFileUpload()
	
	
	/**
	* Prints info about the current uploaded file
	*
	* @param object File	&$asset	the file asset
	*
	* @return void
	* @access public
	*/
	function printExistingFileInfo(&$asset)
	{
		$existing = $asset->_getExistingFile();
		if (!empty($existing)) {
			?>
			<table cellspacing="0" cellpadding="2" >
				<tr>
					<td valign="top"><?php $asset->printImageTag('Click CURRENT FILE to view the latest version of this thumbnail'); ?></td>
					<td valign="top">
					<?php file_edit_fns::printExistingFileInfo($asset); ?>
					<b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?> pixels<br/>
					</td>
				</tr>
			</table>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
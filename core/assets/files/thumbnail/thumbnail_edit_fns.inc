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
* $Id: thumbnail_edit_fns.inc,v 1.24 2005/05/04 17:04:26 gnoel Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/image/image_edit_fns.inc';

/**
* Thumbnail_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.24 $
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
		File_Edit_Fns::paintFileUpload($asset, $o, $prefix);
		if ($asset->writeAccess('attributes')) {
			?><br/><span style="font-size: 10px;">You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?><br/>
			<?php
		}

		return true;

	}//end paintFileUpload()


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
		$existing = $asset->getExistingFile();
		if (!empty($existing)) {
			?>
			<table cellspacing="0" cellpadding="2" >
				<tr>
					<td valign="top"><?php $asset->printImageTag('Click CURRENT FILE to view the latest version of this thumbnail'); ?></td>
					<td valign="top">
					<?php File_Edit_Fns::printExistingFileInfo($asset); ?>
					<b>Dimensions:</b> <?php echo $existing['width'].' x '.$existing['height']?> pixels<br/>
					</td>
				</tr>
			</table>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
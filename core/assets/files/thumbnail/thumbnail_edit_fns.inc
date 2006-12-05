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
* $Id: thumbnail_edit_fns.inc,v 1.27 2006/12/05 23:14:15 emcdonald Exp $
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
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thumbnail_Edit_Fns extends Image_Edit_Fns
{


	/**
	* Prints the interface for uploading a file
	*
	* @param object	&$asset	the thumbnail asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		File_Edit_Fns::paintFileUpload($asset, $o, $prefix);
		if ($asset->writeAccess('attributes')) {
			$o->note(translate('core_only_upload_files_of_type', $this->_getExtensionString($asset->allowed_extensions)));
		}

		return TRUE;

	}//end paintFileUpload()


	/**
	* Prints info about the current uploaded file
	*
	* @param object	&$asset	the file asset
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
					<?php File_Edit_Fns::printExistingFileInfo($asset);
					echo translate('core_dimensions', $existing['width'], $existing['height']); ?><br/>
					</td>
				</tr>
			</table>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
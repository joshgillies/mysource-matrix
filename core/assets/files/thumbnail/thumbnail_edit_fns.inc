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
* $Id: thumbnail_edit_fns.inc,v 1.29 2012/08/30 01:09:07 ewang Exp $
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
* @version $Revision: 1.29 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thumbnail_Edit_Fns extends Image_Edit_Fns
{


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
					echo sprintf(translate('core_dimensions'), $existing['width'], $existing['height']); ?><br/>
					</td>
				</tr>
			</table>
			<?php
		}

	}//end printExistingFileInfo()


}//end class

?>
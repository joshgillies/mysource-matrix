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
* $Id: page_link_manager_edit_fns.inc,v 1.3 2006/12/07 00:04:21 emcdonald Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';

/**
* Page_Link_Manager_Edit_Fns
*
* Purpose
*
*
* @author  David Heppell <dheppell@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Link_Manager_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Link_Manager_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint links to the defaut formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('edit_format') ?></td>
				<td class="sq-backend-table-header"><?php echo translate('note') ?></td>
			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('cms_listing_page_contents_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('cms_listing_no_results_bodycopy_desc'));
			$this->_paintDefaultFormatLink($asset->id, 'not_logged_in', translate('cms_link_manager_not_logged_in_bodycopy_desc'));

			$folder =& $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('cms_listing_default_asset_desc'));
			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


}//end class


?>

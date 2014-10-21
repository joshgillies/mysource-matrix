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
* $Id: page_link_manager_edit_fns.inc,v 1.6 2012/08/30 00:58:08 ewang Exp $
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
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_Link_Manager_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
				<td class="sq-backend-table-header"><?php echo translate('Edit Format') ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Note') ?></td>

			</tr>
			<?php
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('This format defines the layout of the page. Ensure you use the %%asset_listing%% keyword to include the list of assets on the page.'));

			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('This format defines the layout of the page when the <b>Customise No Results Bodycopy</b> option is turned on, and there are no assets to list.'));

			$this->_paintDefaultFormatLink($asset->id, 'not_logged_in', translate('This format defines the layout of the page when the user viewing the page is not logged in.'));


			$folder = $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('This is the default format for all assets in the list if they do not have a customised format for their asset type or position.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


}//end class


?>

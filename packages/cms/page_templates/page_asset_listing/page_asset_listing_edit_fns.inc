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
* $Id: page_asset_listing_edit_fns.inc,v 1.33 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/listing_engine/listing_engine_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Listing_Engine_Edit_Fns
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
	* Paint the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLinkTypes(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('attributes');

		$current = $asset->attr('link_types');
		$link_types = get_link_type_names();

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';

			$selected = Array();
			foreach ($link_types as $link_type => $link_type_name) {
				if ($current & $link_type) {
					$selected[] = $link_type;
				}
			}

			combo_box($prefix.'_link_types', $link_types, TRUE, $selected);
			hidden_field($prefix.'_link_types_submitted', '1');
		} else {
			if (($current == 0) && ($asset->attr('subs_only'))){
				echo '<p class="sq-backend-warning">'.translate('No link types have been selected - <b>no assets will be displayed</b>').'</p>';

			} else {
				echo '<ul>';
				foreach ($link_types as $link_type => $link_type_name) {
					if ($current & $link_type) {
						echo '<li>'.$link_type_name.'</li>';
					}
				}
				echo '</ul>';
			}
		}

		return $write_access;

	}//end paintLinkTypes()


	/**
	* Processes the interface for selecting searched statuses
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processLinkTypes(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (empty($_POST[$prefix.'_link_types_submitted'])) return FALSE;

		$link_types = 0;
		if (isset($_POST[$prefix.'_link_types'])) {
			foreach ($_POST[$prefix.'_link_types'] as $link_type) {
				$link_types |= $link_type;
			}
		}

		return $asset->setAttrValue('link_types', $link_types);

	}//end processLinkTypes()


}//end class

?>

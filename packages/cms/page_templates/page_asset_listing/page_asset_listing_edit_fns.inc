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
* $Id: page_asset_listing_edit_fns.inc,v 1.28.2.1 2006/10/26 06:46:33 tbarrett Exp $
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
* @version $Revision: 1.28.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Listing_Edit_Fns extends Listing_Engine_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Listing_Edit_Fns()
	{
		$this->Listing_Engine_Edit_Fns();

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
				echo '<p class="sq-backend-warning">'.translate('cms_listing_no_link_types_selected').'</p>';
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

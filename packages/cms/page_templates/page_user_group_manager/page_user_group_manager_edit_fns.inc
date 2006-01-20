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
* $Id: page_user_group_manager_edit_fns.inc,v 1.4 2006/01/20 04:16:58 sdanis Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Link_Manager_Edit_Fns
*
* Purpose
*
*
* @author  David Heppell <dheppell@squiz.net>
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Page_User_Group_Manager_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_User_Group_Manager_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for changing the dropdown lists options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRootNode(&$asset, &$o, $prefix)
	{
			if ($asset->writeAccess('attributes')) {
				asset_finder($prefix.'_lm_root_node', $asset->attr('root_node'));
			} else {
				$root_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($asset->attr('root_node')));
				if (!empty($root_info)) {
					echo get_asset_tag_line($asset->attr('root_node'));
				} else {
					echo translate('cms_no_root_selected');
				}
			}

	}//end paintRootNode()


	/**
	* Process the interface for editing the dropdown lists options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRootNode(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$root_node = $_REQUEST[$prefix.'_lm_root_node']['assetid'];
		$asset->setAttrValue('root_node', $root_node);

	}//end processRootNode()


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
	function paintStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$write_access = $asset->writeAccess('links');

		$current = $asset->attr('statuses');

		if ($write_access) {
			require_once SQ_LIB_PATH.'/html_form/html_form.inc';
			$statuses = get_constant_values('SQ_STATUS');

			$options = Array();
			foreach ($statuses as $constant => $value) {
				$options[$value] = get_status_description($value);
			}

			combo_box($prefix.'_statuses', $options, TRUE, $current);
		} else {
			if (empty($current)) {
				echo translate('no_status_selected');
			} else {
				echo '<ul>';
				foreach ($current as $status) {
					echo '<li>'.get_status_description($status).'</li>';
				}
				echo '</ul>';
			}
		}

		return $write_access;

	}//end paintStatus()


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
	function processStatus(&$asset, &$o, $prefix)
	{
		// need to have write access to make any changes
		if (!$asset->writeAccess('attributes')) return FALSE;

		$statuses = Array();
		if (isset($_POST[$prefix.'_statuses'])) {
			foreach ($_POST[$prefix.'_statuses'] as $status) {
				$statuses[] = $status;
			}
			$statuses = array_unique($statuses);
		}

		return $asset->setAttrValue('statuses', $statuses);

	}//end processStatus()


}//end class

?>

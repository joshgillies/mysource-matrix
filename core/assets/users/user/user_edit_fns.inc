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
* $Id: user_edit_fns.inc,v 1.17.2.4 2005/12/22 22:58:25 dmckee Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* User_Edit_Fns
*
* Purpose
*     Editing interfaces for the (standard) user asset - one that can log in
*     but has no backend editing privileges. This also defines the baseline
*     interface for all user assets in the system (whether they be standard,
*     backend, system admin, etc).
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.17.2.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class User_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function User_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Returns a list of groups that the user is linked to
	*
	* @return array the list of groups in getAssetInfo() format
	* @access private
	*/
	function _getLinkedGroups(&$asset)
	{
		$group_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, 1,'user_group', true, 'minor');

		$group_assetids = Array();
		foreach ($group_links as $group_link) {
			$group_assetids[] = $group_link['majorid'];
		}

		$groups_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($group_assetids);

		return $groups_info;

	}//end _getLinkedGroups()


	/**
	* Paints adding a new user condition
	*
	* @access public
	* @return void
	*/
	function paintUserConditions(&$asset, &$o, $prefix)
	{
		$groups = $this->_getLinkedGroups($asset);

		if (empty($groups)) {
			echo translate('user_cannot_be_restricted');
			return false;
		}

		if ($asset->adminAccess('attributes')) {

			foreach ($groups as $group_assetid => $group) {
				$group_menu[$group_assetid] = $group['name'];
			}

			$conditions = Array();
			foreach ($GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('condition') as $type_code) {
				$conditions[$type_code] = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
			}

			check_box($prefix.'_add_group');

			ob_start();
				combo_box($prefix.'_group_list', $group_menu, false, '');
				$group_list = ob_get_contents();
			ob_end_clean();

			ob_start();
				combo_box($prefix.'_restriction_list', $conditions, false, '');
				$restriction_list = ob_get_contents();
			ob_end_clean();

			echo translate('add_group_restriction', $group_list, $restriction_list).' ';
		}

		// get a list of directly linked groups
		$groups = $this->_getLinkedGroups($asset);

		$admin_access = $asset->adminAccess('attributes');

		// get a list of user restrictions
		$restrictions = $asset->attr('restrictions');

		foreach ($groups as $group_assetid => $group) {
			$o->closeField();
			$o->closeSection();

			$usergroup =& $GLOBALS['SQ_SYSTEM']->am->getAsset($group_assetid);

			$o->openSection(translate('group_membership_restrictions', $usergroup->name, $usergroup->id));
			$o->openField('');

			//if user has restrictions related to this group
			if (array_get_index($restrictions, $group_assetid, null)) {

				//get array of restrictions for this group
				foreach ($restrictions[$group_assetid] as $key => $value) {
					$condition_prefix = $prefix.'_condition_'.$group_assetid.'_'.$key;
					$type_code = $value['name'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);

					$o->openSection($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name'));

					$condition =& new $type_code();
					$edit_fns =& $condition->getEditFns();

					$edit_fns->paintEditInterface($condition,  $o, $condition_prefix, $value, $admin_access);

					if ($admin_access) {
						$o->openField(translate('delete_question'));
						check_box($condition_prefix.'[delete]', '1');
						$o->closeField();
					}

					$o->closeSection();
				}

			} else {
				echo translate('no_membership_conditions', $usergroup->name);
			}

		}//end foreach $groups

		// field closed by asset_edit_functions

		return true;

	}//end paintUserConditions()


	/**
	* Processses adding a new user condition
	*
	* @access public
	* @return void
	*/
	function processUserConditions(&$asset, &$o, $prefix)
	{
		// get a list of directly linked groups
		$groups = $this->_getLinkedGroups($asset);
		$admin_access = $asset->adminAccess('attributes');
		$restrictions = $asset->attr('restrictions');

		// -- PROCESS CONDITIONS --//

		foreach ($groups as $group_assetid => $group) {

			// if user has restrictions related to this group
			if (array_get_index($restrictions, $group_assetid, null)) {

				// get array of restrictions for this group
				foreach ($restrictions[$group_assetid] as $key => $value) {
					$condition_prefix = $prefix.'_condition_'.$group_assetid.'_'.$key;

					// if this condition has been marked to delete, unset it now
					if (array_get_index($_POST[$condition_prefix], 'delete', 0)) {
						unset($restrictions[$group_assetid][$key]);
					} else {
						//prepare condition data
						$type_code = $value['name'];
						$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
						$condition =& new $type_code();
						$edit_fns =& $condition->getEditFns();

						// query the condition to retrieve the paramaters it needs
						$results = $edit_fns->processEditInterface($condition,  $o, $condition_prefix, $value, $admin_access);
						$restrictions[$group_assetid][$key]['condition_data'] = array_get_index($results, 'condition_data', Array());
						$restrictions[$group_assetid][$key]['match'] = array_get_index($results, 'match', '1');
					}
				}
			}
		}


		// -- ADD CONDITIONS --//

		// make sure we did want to add a condition
		if (isset($_POST[$prefix.'_add_group'])) {
			$restrictions = $asset->attr('restrictions');
			$group_list = $_POST[$prefix.'_group_list'];
			$restrictions[$group_list][] = Array(
											'name'				=> $_POST[$prefix.'_restriction_list'],
											'condition_data'	=> Array(),
											'match'				=> true,
										   );
		}


		return $asset->setAttrValue('restrictions', $restrictions);

	}//end processUserConditions()


	/**
	* Interface painting function for locale settings
	*
	* @param object User				&$asset	the user asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintLocale(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		require SQ_FUDGE_PATH.'/standards_lists/locales.inc';
		$current_locale = $asset->attr('locale');

		if ($write_access) {
			$standards_lists_locales = array_reverse($standards_lists_locales);
			$standards_lists_locales[''] = '** Use System Default **';
			$standards_lists_locales = array_reverse($standards_lists_locales);
			combo_box($prefix.'_locale', $standards_lists_locales, false, $current_locale);
		} else {
			if (isset($standards_lists_locales[$current_locale])) {
				echo $standards_lists_locales[$current_locale];
			} else if ($current_locale == '') {
				// add system default to this message when it is determined
				echo translate('no_locale_set_for_user', $standards_lists_locales[SQ_CONF_DEFAULT_BACKEND_LOCALE]);
			} else {
				echo translate('unknown_locale', $current_locale);
			}
		}

		return $write_access;

	}//end paintLocale()


	/**
	* Processes the interface for locale settings
	*
	* @param object User				&$asset	the user asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processLocale(&$asset, &$o, $prefix)
	{
		return false;

	}//end processLocale()


}//end class

?>

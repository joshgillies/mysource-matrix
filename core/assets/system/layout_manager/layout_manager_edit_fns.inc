<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: layout_manager_edit_fns.inc,v 1.2 2003/12/17 04:42:56 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Layout_Manager_Edit_Fns
*
* Edit functions for the layout manager.
*
* @see Asset_Edit_Fns
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	* 
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Layout_Manager_Edit_Fns()
	{
		$this->static_screens['details']['force_unlock'] = false;
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param &object Asset				$asset		the asset of the layout manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintAddLayout(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$layouts = array_diff($types, array_keys($asset->attr('layouts')));
		asort($layouts);
		
		$uc_layouts = Array();
		foreach ($layouts as $layout) {
			$uc_layouts[$layout] = ucwords(str_replace('_',' ', $layout));
		}
		combo_box($prefix.'_layouts', $uc_layouts, true, '', 6);
		
		return true;

	}//end paintAddLayout()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param &object Asset				$asset		the asset of the layout manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processAddLayout(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_layouts']) && !empty($_POST[$prefix.'_layouts'])) {
			$layouts = $asset->attr('layouts');
			foreach ($_POST[$prefix.'_layouts'] as $type_code) {
				// this should never happen
				if (in_array($type_code, $asset->attr('layouts'))) return false;
				$layouts[$type_code] = Array();
			}
			$asset->setAttrValue('layouts', $layouts);
			$asset->saveAttributes();
		}
		return true;
	
	}//end processAddLayout()


	/**
	* Paints the interface to add new screens to type_codes and layouts to screens
	*
	* @param &object Asset				$asset		the asset of the layout manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintEditLayouts(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$layouts = $asset->attr('layouts');
		if (empty($layouts)) {
			echo 'There are no Layouts Defined <br />';
			return false;
		}

		// print the current type_codes that we have defined in the manager
		foreach ($asset->attr('layouts') as $type_code => $screens) {

			$uc_type_code = ucwords(str_replace('_', ' ', $type_code));
			$o->openSection($uc_type_code);
				$o->openField('');
				
				check_box($prefix.'_remove_type_code['.$type_code.']');
				?>Remove Customisations for <?php
				echo $uc_type_code;
				
				$o->closeField();
				$o->openField('');

				if (empty($screens)) {
					echo 'No Layouts defined for any of '.$uc_type_code.'\'s Screens <br />';
				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" width="33%">Asset Type</td>
							<td class="sq-backend-table-header" width="33%">Choose Layout</td>
							<td align="center" class="sq-backend-table-header" width="33%">Remove</td>
						</tr>
						
					<?php

						// print the current screens that are defined for this type_code
						foreach ($screens as $screen => $assetid) {
							?>
							<tr>
								<td class="sq-backend-table-cell">
									<?php echo ucfirst($screen) ?>
								</td>
								<td class="sq-backend-table-cell">
									<?php 
										asset_finder($prefix.'_find['.$type_code.'@'.$screen.']', $assetid, Array('layout' => 'I'));
									?>
								</td>
								<td align="center" class="sq-backend-table-cell">
								 <?php check_box($prefix.'_remove_screen['.$type_code.'@'.$screen.']'); ?>
								</td>
							</tr>
							<?php
						}//end foreach screen

						?>
					</table>
				<?php
				}//end if
				?>
				<br />Add a layout for screen:
				<?php
				$ei = new Asset_Edit_Interface($type_code);

				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$as =& new $type_code();
				$ef = &$as->getEditFns();

				$screens = Array(''); // first option is blank
				foreach (array_keys($ef->static_screens) as $screen) $screens[] = $screen;
				foreach (array_keys($ei->getScreens()) as $screen) $screens[] = $screen;

				// remove any screens that have already been defined
				$screens = array_unique(array_diff($screens, array_keys($layouts[$type_code])));

				$uc_screens = Array();
				foreach ($screens as $screen) {
					$uc_screens[$screen] = ucfirst($screen);
				}
				// print a box to add more screens
				combo_box($prefix.'_screens['.$type_code.']', $uc_screens, false, Array());
		
				$o->closeField();
			$o->closeSection();

		}//end foreach type_code

		return true;

	}//end paintEditLayouts()


	/**
	* Accepts requests to add new screens to type_codes, and layouts to screens
	*
	* @param &object Asset				$asset		the asset of the layout manager
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processEditLayouts(&$asset, &$o, $prefix)
	{
		$save = false;

		// get any new screens added to a type code
		if (isset($_POST[$prefix.'_screens'])) {
			$layouts = $asset->attr('layouts');
			foreach ($_POST[$prefix.'_screens'] as $type_code => $screen) {
				if (!$screen) continue;
				if (!isset($layouts[$type_code])) $layouts[$type_code] = Array();
				$layouts[$type_code][$screen] = 0; // set assetid to 0
				$save = true;
			}
		}
		// get any layouts that have been allocated to a screen
		if (isset($_POST[$prefix.'_find'])) {
			foreach ($_POST[$prefix.'_find']  as $info => $assetid) {
				if (!isset($assetid['assetid']) || !$assetid['assetid']) continue;
				list($type_code, $screen) = explode('@', $info);
				$layouts[$type_code][$screen] = $assetid['assetid'];
				$save = true;
			}
		}
		// check to see if any screens have been flagged for removal
		// which will ignore any uncommited customisations to the screen
		if (isset($_POST[$prefix.'_remove_screen'])) {
			foreach (array_keys($_POST[$prefix.'_remove_screen']) as $info) {
				list($type_code, $screen) = explode('@', $info);
				unset($layouts[$type_code][$screen]);
				$save = true;
			}
		}
		// remove all customisations
		// which will overide any uncommited customisations, and any flagged removal of screens
		if (isset($_POST[$prefix.'_remove_type_code'])) {
			foreach (array_keys($_POST[$prefix.'_remove_type_code']) as $type_code) {
				unset($layouts[$type_code]);
				$save = true;
			}
		}
		// save only if we need to
		if ($save) {
			$asset->setAttrValue('layouts', $layouts);
			$asset->saveAttributes();
		}

		return true;

	}//end processEditLayouts()


}//end class
?>
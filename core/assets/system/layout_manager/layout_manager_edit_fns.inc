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
* $Id: layout_manager_edit_fns.inc,v 1.16 2012/08/30 01:09:11 ewang Exp $
*
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
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		$this->static_screens['details']['force_unlock'] = FALSE;
		parent::__construct();

		unset($this->static_screens['settings']);
		unset($this->static_screens['preview']);
		unset($this->static_screens['lookupValues']);
		unset($this->static_screens['web_paths']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['tagging']);
		unset($this->static_screens['roles']);
		unset($this->static_screens['permissions']);
		unset($this->static_screens['dependants']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['layouts']);

	}//end constructor


	/**
	* Paints the interface to add new type_codes to define layouts for
	*
	* @param Layout_Manager		$asset	the asset of the layout manager
	* @param Backend_Outputter	$o		the Backend Outputter class
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function paintAddLayout(Layout_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		asset_type_chooser($prefix.'_layouts', TRUE, Array());

		return TRUE;

	}//end paintAddLayout()


	/**
	* Accepts requests to add a type_code to define layouts for
	*
	* @param Layout_Manager		$asset	the asset of the layout manager
	* @param Backend_Outputter	$o		the Backend Outputter class
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function processAddLayout(Layout_Manager $asset, Backend_Outputter $o, $prefix)
	{
		if (isset($_POST[$prefix.'_layouts']) && !empty($_POST[$prefix.'_layouts'])) {
			$layouts = $asset->attr('layouts');
			foreach ($_POST[$prefix.'_layouts'] as $type_code) {
				// this should never happen
				if (in_array($type_code, $asset->attr('layouts'))) {
					return FALSE;
				}
				if (!empty($type_code)) $layouts[$type_code] = Array();
			}
			$asset->setAttrValue('layouts', $layouts);
			$asset->saveAttributes();
		}
		return TRUE;

	}//end processAddLayout()


	/**
	* Paints the interface to add new screens to type_codes and layouts to screens
	*
	* @param Layout_Manager		$asset	the asset of the layout manager
	* @param Backend_Outputter	$o		the Backend Outputter class
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function paintEditLayouts(Layout_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$layouts = $asset->attr('layouts');
		if (empty($layouts)) {
			$o->note(translate('There are no Layouts Defined'));

			return FALSE;
		}

		// print the current type_codes that we have defined in the manager
		foreach ($layouts as $type_code => $screens) {

			$uc_type_code = ucwords(str_replace('_', ' ', $type_code));
			$o->openSection($uc_type_code);

				if ($write_access) {
					$o->openField('');

					check_box($prefix.'_remove_type_code['.$type_code.']');
					label(sprintf(translate('Remove Customisations for %s'), $uc_type_code), $prefix.'_remove_type_code['.$type_code.']');


					$o->closeField();
				}

				$o->openField('');

				if (empty($screens)) {
					$o->note(sprintf(translate('No Layouts defined for any of %s\'s Screens'), $uc_type_code));

				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td class="sq-backend-table-header" width="33%"><?php echo translate('Screen Name'); ?></td>

							<td class="sq-backend-table-header" width="33%"><?php echo translate('Layout'); ?></td>

							<?php if ($write_access) {
								?>
								<td align="center" class="sq-backend-table-header" width="33%"><?php echo translate('Remove'); ?></td>

							<?php }
							?>
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
										if ($write_access) {
											asset_finder($prefix.'_find['.$type_code.'@'.$screen.']', $assetid, Array('layout' => 'I'));
										} else {
											if (empty($assetid)) {
												echo '<em>'.translate('None').'</em>';

											} else {
												echo get_asset_tag_line($assetid);
											}
										}
									?>
								</td>
								<?php if ($write_access) {
									?>
									<td align="center" class="sq-backend-table-cell">
										<?php check_box($prefix.'_remove_screen['.$type_code.'@'.$screen.']'); ?>
									</td>
								<?php }
								?>
							</tr>
							<?php
						}//end foreach screen

						?>
					</table>
				<?php
				}//end else

				if ($write_access) {
					?>
					<br /><?php echo translate('Add a layout for screen'); ?>:

					<?php
					$ei = new Asset_Edit_Interface($type_code);

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
					$as = new $type_code();
					$ef = $as->getEditFns();

					$screens = Array(''); // first option is blank
					foreach (array_keys($ef->static_screens) as $screen) {
						$screens[] = $screen;
					}
					foreach (array_keys($ei->getScreens()) as $screen) {
						$screens[] = $screen;
					}

					// remove any screens that have already been defined
					$screens = array_unique(array_diff($screens, array_keys($layouts[$type_code])));

					$uc_screens = Array();
					foreach ($screens as $screen) {
						$uc_screens[$screen] = ucfirst($screen);
					}
					// print a box to add more screens
					combo_box($prefix.'_screens['.$type_code.']', $uc_screens, FALSE, Array());
				}

				$o->closeField();
			$o->closeSection();

		}//end foreach type_code

		return $write_access;

	}//end paintEditLayouts()


	/**
	* Accepts requests to add new screens to type_codes, and layouts to screens
	*
	* @param Layout_Manager		$asset	the asset of the layout manager
	* @param Backend_Outputter	$o		the Backend Outputter class
	* @param string				$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	public function processEditLayouts(Layout_Manager $asset, Backend_Outputter $o, $prefix)
	{
		$save = FALSE;

		// get any new screens added to a type code
		if (isset($_POST[$prefix.'_screens'])) {
			$layouts = $asset->attr('layouts');
			foreach ($_POST[$prefix.'_screens'] as $type_code => $screen) {
				if (!$screen) continue;
				if (!isset($layouts[$type_code])) {
					$layouts[$type_code] = Array();
				}
				$layouts[$type_code][$screen] = 0; // set assetid to 0
				$save = TRUE;
			}
		}
		// get any layouts that have been allocated to a screen
		if (isset($_POST[$prefix.'_find'])) {
			foreach ($_POST[$prefix.'_find']  as $info => $assetid) {
				if (!isset($assetid['assetid']) || !$assetid['assetid']) {
					continue;
				}
				list($type_code, $screen) = explode('@', $info);
				$layouts[$type_code][$screen] = $assetid['assetid'];
				$save = TRUE;
			}
		}
		// check to see if any screens have been flagged for removal
		// which will ignore any uncommited customisations to the screen
		if (isset($_POST[$prefix.'_remove_screen'])) {
			foreach (array_keys($_POST[$prefix.'_remove_screen']) as $info) {
				list($type_code, $screen) = explode('@', $info);
				unset($layouts[$type_code][$screen]);
				$save = TRUE;
			}
		}
		// remove all customisations
		// which will overide any uncommited customisations, and any flagged removal of screens
		if (isset($_POST[$prefix.'_remove_type_code'])) {
			foreach (array_keys($_POST[$prefix.'_remove_type_code']) as $type_code) {
				unset($layouts[$type_code]);
				$save = TRUE;
			}
		}
		// save only if we need to
		if ($save) {
			$asset->setAttrValue('layouts', $layouts);
			$asset->saveAttributes();
		}

		return TRUE;

	}//end processEditLayouts()


}//end class
?>

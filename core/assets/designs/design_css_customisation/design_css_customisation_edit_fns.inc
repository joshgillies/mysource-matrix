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
* $Id: design_css_customisation_edit_fns.inc,v 1.23 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_css/design_css_edit_fns.inc';

/**
* Design_Css_Customisation_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Customisation_Edit_Fns extends Design_Css_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['lock_type'] = 'all';

	}//end constructor


	/**
	* Paints the interface for customising the design areas
	*
	* @param Design_CSS_Customisation	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCustomisedDesignAreas(Design_CSS_Customisation $asset, Backend_Outputter $o, $prefix)
	{
		$customised_areas = $asset->getCustomisedAreas();
		if (empty($customised_areas)) {
			echo translate('core_design_customisation_no_design_areas_customised');
		} else {
			$this->_paintDesignAreaList($customised_areas, translate('core_remove_customisation_question'), $prefix.'_remove_customisation', $asset->writeAccess('links'), TRUE, TRUE);
		}

	}//end paintCustomisedDesignAreas()


	/**
	* Processes the customising of design areas
	*
	* @param Design_CSS_Customisation	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCustomisedDesignAreas(Design_CSS_Customisation $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_customisation'])) {
			return FALSE;
		}

		$parent_design = $asset->getParentDesign();
		if (is_null($parent_design)) return FALSE;

		$data = Array();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($_POST[$prefix.'_remove_customisation'] as $id_name) {
			$parents_link = $parent_design->getDesignAreaLink($id_name);
			if (empty($parents_link)) continue;

			$data[$id_name] = Array($parents_link['minorid'], $parents_link['minor_type_code']);
		}// end foreach

		if ($data && !$this->_updateDesignAreaLink($asset, $data)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// regenerate the design_file.php and css_file.php here (because the processBackend function
		// doesn't do it automatically any more
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array(
		    'assetid'	=> $asset->id,
		    'parentid'	=> $parent_design->id,
		);
		$hh->queueHipo('hipo_job_regenerate_design', $vars);

		return TRUE;

	}//end processCustomisedDesignAreas()


	/**
	* Paints the interface for customising the design areas
	*
	* @param Design_CSS_Customisation	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintUnCustomisedDesignAreas(Design_CSS_Customisation $asset, Backend_Outputter $o, $prefix)
	{
		$uncustomised_areas = $asset->getUnCustomisedAreas();
		if (empty($uncustomised_areas)) {
			echo translate('core_design_customisation_all_design_areas_customised');
		} else {
			$this->_paintDesignAreaList($uncustomised_areas, translate('core_customise_question'), $prefix.'_customise', $asset->writeAccess('links'));
		}

	}//end paintUnCustomisedDesignAreas()


	/**
	* Processes the customising of design areas
	*
	* @param Design_CSS_Customisation	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processUnCustomisedDesignAreas(Design_CSS_Customisation $asset, Backend_Outputter $o, $prefix)
	{
		if (empty($_POST[$prefix.'_customise'])) return FALSE;

		$data = Array();

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$map  = Array();
		foreach ($_POST[$prefix.'_customise'] as $id_name) {
			$link = $asset->getDesignAreaLink($id_name);
			if (is_null($link)) continue;
			$da = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code'], FALSE);
			if (is_null($da)) continue;

			$create_link = Array (
							'asset'			=> &$asset,
							'link_type'		=> SQ_LINK_TYPE_3,
							'value'			=> $link['value'],
							'sort_order'	=> NULL,
							'is_dependant'	=> 1,
							'is_exclusive'	=> 0,
						   );

			$GLOBALS['SQ_SYSTEM']->am->acquireLock($da->id, 'all');

			// First let's duplicate the DA, ignoring the directory with the files
			$clone = $GLOBALS['SQ_SYSTEM']->am->cloneAsset($da, $create_link, $map, Array('attributes', 'permissions', 'roles'), TRUE);

			$GLOBALS['SQ_SYSTEM']->am->releaseLock($da->id, 'all');
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($clone->id, 'all');

			if (is_null($clone)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$data[$id_name] = Array($clone->id, $clone->type());

		}//end foreach

		if ($data && !$this->_updateDesignAreaLink($asset, $data)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}//end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end processUnCustomisedDesignAreas()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	*
	* @param Asset	$asset	the asset to link the file to
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	public function _processUploadedFile(Asset $asset, Array $info=Array())
	{
		// if we already have a file with the same name TYPE_3 linked to us,
		// delete this link so we link up this new file
		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($existing as $link) {
			$existing_ids[$link['minorid']] = $link['linkid'];
		}
		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));
		foreach ($existing_info as $assetid => $existing_info) {
			// if the name is the same, delete the link
			if ($existing_info['name'] == strtolower($info['name'])) {
				$linkid = $existing_ids[$assetid];
				$asset->deleteExistingLink($linkid);

				// We need to update the lookups of the old linked asset
				$minor_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$minor_asset->updateLookups();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($minor_asset);
			}
		}

		return parent::_processUploadedFile($asset, $info);

	}//end _processUploadedFile()


	/**
	* Recursivly updates all links to a design area from under this asset
	*
	* @param Asset	$asset	the asset to which we belong
	* @param array	$data	array of Array([id_name] => Array([assetid], [type_code]));
	*						where key is the id_name, whose link needs to be updated and the value
	*						is an array with assetid and type_code
	*
	* @return boolean
	* @access public
	*/
	public function _updateDesignAreaLink(Asset $asset, Array $data)
	{
		$da_links = $asset->getDesignAreaLink();
		$old_da_ids = Array();
		foreach ($da_links as $da_link) {
			$id_name = $da_link['value'];
			// skip this design area if it is not in the list of design areas to update
			if (empty($data[$id_name])) continue;

			// okay, this design area is in the list of design areas to update, but
			// skip it if it is already link'dup correctly
			if ($data[$id_name][0] == $da_link['minorid']) {
				continue;
			}

			$old_da_ids[] = $da_link['minorid'];

			// remove the old link (the one from the original design)
			if (!$asset->deleteLink($da_link['linkid'])) {
				return FALSE;
			}

			// create the new link if required (wont be required if we created during a customisation)
			$existing_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $data[$id_name][0]);
			if (empty($existing_link)) {
				// now create the new link
				$da = $GLOBALS['SQ_SYSTEM']->am->getAsset($data[$id_name][0], $data[$id_name][1]);
				if (is_null($da)) return FALSE;
				if (!$asset->createLink($da, SQ_LINK_TYPE_3, $id_name, NULL, '1')) {
					return FALSE;
				}
			}

			// remove any cached versions
			if (isset($asset->_tmp['design_areas'])) {
				unset($asset->_tmp['design_areas'][$id_name]);
			}

		}//end foreach

		// Update our customisations to use our DAs where appropriate
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_css_customisation', TRUE, 'major', 'customisation');
		foreach ($customisation_links as $customisation_link) {
			$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($customisation_link['minorid'], $customisation_link['minor_type_code']);
			if (is_null($customisation)) continue;

			// We don't want to overwrite already-customised design areas in the
			// customisation, so we filter the update array to remove them.
			// We do this by applying the rule that we can only unlink DAs
			// from the customisation if we just unlinked them from ourselves
			$cust_update_data = $data; // copy
			foreach ($customisation->getCustomisedAreas() as $link) {
				if (isset($cust_update_data[$link['value']])) {
					if (!in_array($link['minorid'], $old_da_ids)) {
						// this customisation has its own version of this DA already
						// so don't cascade our DA to it
						unset($cust_update_data[$link['value']]);
					}
				}
			}
			if (!$this->_updateDesignAreaLink($customisation, $cust_update_data)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end _updateDesignAreaLink()


	/**
	* Paints a list of design areas out
	*
	* @param array		$links			the links pointing to the design areas to paint
	* @param string		$check_box_col	the title to put in the check box column
	* @param string		$check_box_name	the form name to give the check box for each design area entry
	* @param boolean	$write_access	if you have write access or not
	* @param boolean	$link_name		whether to link the name to the edit interface for the design area
	*
	* @return void
	* @access private
	*/
	protected function _paintDesignAreaList(Array $links, $check_box_col, $check_box_name, $write_access, $link_name=FALSE)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		?>
		<table class="sq-backend-table">
			<tr>
				<th class="sq-backend-table-header" width="33%"><?php echo translate('type'); ?></th>
				<th class="sq-backend-table-header" width="33%"><?php echo translate('core_design_area'); ?></th>
			<?php
				if ($write_access) {
			?>
				<th class="sq-backend-table-header" width="34%" style="text-align: center;"><?php echo $check_box_col; ?></th>
			<?php
				}// end if
			?>
			</tr>
		<?php
		uasort($links, Array(get_class($this), 'linkCompare'));
		$current_type = '';
		foreach ($links as $link) {
			$da = $am->getAsset($link['minorid'], $link['minor_type_code']);

			if ($da->attr('id_name') == '') {
				trigger_localised_error('CORE0154', E_USER_NOTICE, $da->id);
				foreach ($links as $link) {
					if ($link['minorid'] == $da->id) {
						if (!empty($link['value'])) {
							$da->setAttrValue('id_name', $link['value']);
							$da->saveAttributes();
						} else {
							trigger_localised_error('CORE0149', E_USER_WARNING, $da->id);
						}
						break;
					}
				}
			}

			if (is_null($da)) continue;
		?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					if ($current_type != $da->type()) {
						$current_type = $da->type();
						echo '<i>', substr($current_type, 12), '</i>'; // take of the 'design_area_'
					} else {
						echo '&nbsp;';
					}
				?>
				</td>
				<td class="sq-backend-table-cell" style="white-space: nowrap;">
					<?php
					if ($link_name) {
						echo get_asset_tag_line($da->id, 'details');
					} else {
						echo get_asset_tag_line($da->id, '');
					}
					?>
				</td>
			<?php
				if ($write_access) {
			?>
				<td class="sq-backend-table-cell" style="text-align: center;">
					<input type="checkbox" name="<?php echo $check_box_name; ?>[]" value="<?php echo $da->attr('id_name');; ?>">
				</td>
			<?php
				}// end if
			?>
			</tr>
		<?php
		}//end foreach
		?>
		</table>
		<?php

	}//end _paintDesignAreaList()


	/**
	* This is the static comparing function called by uasort() in _paintDesignAreaList()
	*
	* @param array	$a	array of link information
	* @param array	$b	array of link information
	*
	* @return int
	* @access private
	* @see _paintDesignAreaList()
	*/
	protected function linkCompare($a, $b)
	{
		// if they don't match, return the value
		if ($c = strcmp($a['minor_type_code'], $b['minor_type_code'])) {
			return $c;
		}
		// otherwise the types are equal, so compare by id_name
		return strcmp(strtolower($a['value']), strtolower($b['value']));

	}//end linkCompare()


}//end class

?>

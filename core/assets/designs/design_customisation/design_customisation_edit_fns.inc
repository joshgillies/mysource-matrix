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
* $Id: design_customisation_edit_fns.inc,v 1.24 2004/04/19 01:09:11 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design_edit_fns.inc';

/**
* Design_Customisation_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Customisation_Edit_Fns extends Design_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Design_Customisation_Edit_Fns()
	{
		$this->Design_Edit_Fns();
		$this->static_screens['details']['lock_type'] = 'all';

	}//end constructor


	/**
	* Paints the interface for customising the design areas
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCustomisedDesignAreas(&$asset, &$o, $prefix)
	{
		$customised_areas = $asset->getCustomisedAreas();
		if (empty($customised_areas)) {
			?> No Design Areas are customised yet. <?php
		} else {
			$this->_paintDesignAreaList($customised_areas, 'Remove Customisation ?', $prefix.'_remove_customisation', $asset->writeAccess('links'), true, true);
		}

	}//end paintCustomisedDesignAreas()


	/**
	* Processes the customising of design areas
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCustomisedDesignAreas(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_customisation'])) return false;

		$parent_design = &$asset->getParentDesign();
		if (is_null($parent_design)) return false;

		$data = Array();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach($_POST[$prefix.'_remove_customisation'] as $id_name) {
			$parents_link = $parent_design->getDesignAreaLink($id_name);
			if (empty($parents_link)) continue;

			$data[$id_name] = Array($parents_link['minorid'], $parents_link['minor_type_code']);
		}// end foreach

		if ($data && !$this->_updateDesignAreaLink($asset, $data)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processCustomisedDesignAreas()


	/**
	* Paints the interface for customising the design areas
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintUnCustomisedDesignAreas(&$asset, &$o, $prefix)
	{
		$uncustomised_areas = $asset->getUnCustomisedAreas();
		if (empty($uncustomised_areas)) {
			?> All Design Areas have been customised. <?php
		} else {
			$this->_paintDesignAreaList($uncustomised_areas, 'Customise ?', $prefix.'_customise', $asset->writeAccess('links'));
		}

	}//end paintUnCustomisedDesignAreas()


	/**
	* Processes the customising of design areas
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processUnCustomisedDesignAreas(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_customise'])) return false;

		$data = Array();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$map  = Array();
		foreach($_POST[$prefix.'_customise'] as $id_name) {
			$link = $asset->getDesignAreaLink($id_name);
			if (is_null($link)) continue;
			$da = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code'], false);
			if (is_null($da)) continue;
			
			$create_link = Array (	'asset' => &$asset,
									'link_type'  => SQ_LINK_TYPE_3,
									'value'      => $link['value'],
									'sort_order' => null,
									'dependant'  => 1,
									'exclusive'  => 0
								 );

			// First let's duplicate the DA, ignoring the directory with the files
			$clone = &$GLOBALS['SQ_SYSTEM']->am->cloneAsset($da, $create_link, $map, Array('attributes', 'permissions'), true);
			if (is_null($clone)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$data[$id_name] = Array($clone->id, $clone->type());

		}// end foreach

		if ($data && !$this->_updateDesignAreaLink($asset, $data)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processUnCustomisedDesignAreas()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	* @param object Asset	$asset	the asset to link the file to
	* @param array			$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function _processUploadedFile(&$asset, $info=Array())
	{
		// if we already have a file with the same name TYPE_3 linked to us,
		// delete this link so we link up this new file
		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, 'file', false);
		foreach ($existing as $link) $existing_ids[$link['minorid']] = $link['linkid'];
		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));
		foreach ($existing_info as $existing_info) {
			// if the name is the same, delete the link
			if ($existing_info['name'] == $info['name']) {
				$linkid = $existing_ids[$existing_info['assetid']];
				$asset->deleteLink($linkid);
			}
		}

		return parent::_processUploadedFile($asset, $info);

	}//end _processUploadedFile()


	/**
	* Recursivly updates all links to a design area from under this asset
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param Array			$data	array of Array([id_name] => Array([assetid], [type_code]));
	*								where key is the id_name, whose link needs to be updated and the value
	*								is an array with assetid and type_code
	*
	* @return boolean
	* @access public
	*/
	function _updateDesignAreaLink(&$asset, $data)
	{
		$da_links = $asset->getDesignAreaLink();
		foreach($da_links as $da_link) {
			$id_name = $da_link['value'];
			// skip this design area if it is not in the list of design areas to update
			if (empty($data[$id_name])) continue;
			
			// okay, this design area is in the list of design areas to update, but
			// skip it if it is already link'dup correctly
			if ($data[$id_name][0] == $da_link['minorid']) continue;

			// remove the old link (the one from the original design)
			if (!$asset->deleteLink($da_link['linkid'])) return false;
			
			// create the new link if required (wont be required if we created during a customisation)
			$existing_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($asset->id, $data[$id_name][0]);
			if (empty($existing_link)) {
				// now create the new link
				$da = &$GLOBALS['SQ_SYSTEM']->am->getAsset($data[$id_name][0], $data[$id_name][1]);
				if (is_null($da)) return false;
				if (!$asset->createLink($da, SQ_LINK_TYPE_3, $id_name, null, '1')) return false;
			}

			// remove any cached versions
			if (isset($asset->_tmp['design_areas'])) unset($asset->_tmp['design_areas'][$id_name]);

		}// end foreach

		$customisation_links = &$GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
		foreach($customisation_links as $customisation_link) {
			$customisation = &$GLOBALS['SQ_SYSTEM']->am->getAsset($customisation_link['minorid'], $customisation_link['minor_type_code']);
			if (is_null($customisation)) continue;
			if (!$this->_updateDesignAreaLink($customisation, $data)) return false;
		}

		return true;

	}//end _updateDesignAreaLink()


	/**
	* Paints a list of design areas out
	*
	* @param Array	$links			the links pointing to the design areas to paint
	* @param string	$check_box_col	the title to put in the check box column
	* @param string	$check_box_name	the form name to give the check box for each design area entry
	* @param bool	$link_name		whether to link the name to the edit interface for the design area
	*
	* @access private
	*/
	function _paintDesignAreaList($links, $check_box_col, $check_box_name, $write_access, $link_name=false, $show_specialised=false)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		?>
		<table cellspacing="2" cellpadding="2" border="0" width="100%">
			<tr>
				<td class="sq-backend-data" width="25%"><b>Type</b></td>
				<td class="sq-backend-data" width="25%"><b>Design Area</b></td>
			<?php if ($write_access) { ?>
				<td class="sq-backend-data" width="25%" style="text-align: center;"><b><?php echo $check_box_col; ?></b></td>
			<?php }// end if ?>
				<td class="sq-backend-data" width="25%" style="text-align: center;"><b><?php echo ($show_specialised) ? 'Specialised ?' : '&nbsp;'; ?></b></td>
			</tr>
		<?php
		uasort($links, Array(get_class($this), 'linkCompare'));
		$current_type = '';
		foreach($links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['minor_type_code']);

			if ($da->attr('id_name') == '') {
				trigger_error('Design Area #'.$da->id.' does not have an id_name set. Attempting to recover.', E_USER_NOTICE);
				foreach ($links as $link) {
					if ($link['minorid'] == $da->id) {
						if (!empty($link['value'])) {
							$da->setAttrValue('id_name', $link['value']);
							$da->saveAttributes();
						} else {
							trigger_error('Could not recover id_name for design area #'.$da->id, E_USER_WARNING);
						}
						break;
					}
				}
			}

			if (is_null($da)) continue;
		?>
			<tr>
				<td class="sq-backend-data">
				<?php
					if ($current_type != $da->type()) {
						$current_type = $da->type();
						echo '<i>', substr($current_type, 12), '</i>'; // take of the 'design_area_'
					} else {
						echo '&nbsp;';
					}
				?>
				</td>
				<td class="sq-backend-data" style="white-space: nowrap;">
					<?php
					echo $da->id, ' - ';
					if ($link_name) echo '<a href="', $da->getBackendHref(), '">';
					echo $da->name;
					if ($link_name) echo '</a>';
					?>
				</td>
			<?php if ($write_access) { ?>
				<td class="sq-backend-data" style="text-align: center;">
					<input type="checkbox" name="<?php echo $check_box_name; ?>[]" value="<?php echo $da->attr('id_name');; ?>">
				</td>
			<?php }// end if ?>
				<td class="sq-backend-data" style="text-align: center;">
				<?php if ($show_specialised && $da->attr('specialised')) { ?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/tick.gif" width="15" height="15" />
				<?php } else { ?>
					&nbsp;
				<?php }// end if ?>
				</td>
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
	* @param Array	$a	array of link information
	* @param Array	$b	array of link information
	*
	* @return int
	* @see _paintDesignAreaList()
	*/
	function linkCompare($a, $b)
	{
		// if they don't match, return the value
		if ($c = strcmp($a['minor_type_code'], $b['minor_type_code'])) return $c;
		// otherwise the types are equal, so compare by id_name
		return strcmp(strtolower($a['value']), strtolower($b['value']));

	}//end linkCompare()


}//end class

?>
<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design_edit_fns.inc';

/**
* Design_Customisation_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Customisation_Edit_Fns extends Design_Edit_Fns
{

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
			$this->_paintDesignAreaList($customised_areas, 'Remove Customisation ?', $prefix.'_remove_customisation', $asset->writeAccess(), true, true);
		}

	}// end paintCustomisedDesignAreas()

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
			if (is_null($parents_link)) continue;

			$data[$id_name] = $parents_link['minorid'];

		}// end foreach

		if ($data) {
			if ($this->_updateDesignAreaLink($asset, $data)) {
				$asset->_tmp['regenerate_design'] = true;
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}// end if
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end processCustomisedDesignAreas()

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
			$this->_paintDesignAreaList($uncustomised_areas, 'Customise ?', $prefix.'_customise', $asset->writeAccess());
		}
	}// end paintUnCustomisedDesignAreas()

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
		$create_link = Array(); // because we have to bugger about with this a bit, don't link it anywhere yet
		foreach($_POST[$prefix.'_customise'] as $id_name) {
			$link = $asset->getDesignAreaLink($id_name);
			if (is_null($link)) continue;
			$da = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code'], false);
			if (is_null($da)) continue;

			// First let's duplicate the DA, ignoring the directory with the files
			$dupe = &$da->duplicate($create_link, $map, true, false);
			if (is_null($dupe)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}

			$data[$id_name] = $dupe->id;

		}// end foreach

		if ($data) {
			if ($this->_updateDesignAreaLink($asset, $data)) {
				$asset->_tmp['regenerate_design'] = true;
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}// end if
		}// end if

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end processUnCustomisedDesignAreas()



	/**
	* Recursivly updates all links to a design area from under this asset
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param Array			$data	array of Array(id_name => 1324)); where key is the id_name, whose link needs to be updated and the value is the new minorid
	*
	* @return boolean
	* @access public
	*/
	function _updateDesignAreaLink(&$asset, $data)
	{

		$da_links = $asset->getDesignAreaLink();
		foreach($da_links as $da_link) {
			$id_name = $da_link['value'];
			if (empty($data[$id_name])) continue;
			// Now link our parents DA to the design customisation
			if (!$asset->_updateLink($da_link['linkid'], $data[$id_name])) return false;

			// remove any cached versions
			if (isset($asset->_tmp['design_areas'])) unset($asset->_tmp['design_areas'][$id_name]);

		}// end foreach

		$customisation_links = &$asset->getLinks(SQ_LINK_UNITE, 'design_customisation', true, 'major', 'customisation');
		foreach($customisation_links as $customisation_link) {
			$customisation = &$GLOBALS['SQ_SYSTEM']->am->getAsset($customisation_link['minorid'], $customisation_link['type_code']);
			if (is_null($customisation)) continue;

			if (!$this->_updateDesignAreaLink($customisation, $data)) return false;

		}// end foreach

		return true;

	}// end _updateDesignAreaLink()


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
			$da = &$am->getAsset($link['minorid'], $link['type_code']);
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
	}// end _paintDesignAreaList()

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
        if ($c = strcmp($a['type_code'], $b['type_code'])) return $c;
		// otherwis the types are equal, so compare by id_name
		return strcmp(strtolower($a['value']), strtolower($b['value']));
    }

}//end class
?>

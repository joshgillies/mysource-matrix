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
* $Id: page_asset_builder_edit_fns.inc,v 1.1 2004/01/15 05:15:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Asset_Builder_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the interface for choosing the type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			if ($asset->attr('create_type') == '') {
				echo 'No asset type selected';
			} else {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->attr('create_type'));
				echo $info['name'];
			}
		} else {
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			$options = Array('' => '-- select asset type --');
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, false, $asset->attr('create_type'));
		}

	}//end paintCreateType()


	/**
	* Processes the interface for choosing the type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateType(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_type'])) return false;
		$create_type = trim($_POST[$prefix.'_create_type']);
		return $asset->setAttrValue('create_type', $create_type);

	}//end processCreateType()


	/**
	* Paint the interface for choosing the link type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLink(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (!$asset->writeAccess('attributes')) {
			echo link_type_name($asset->attr('create_link_type'));
		} else {
			$link_options = get_bit_values('SQ_LINK_');
			$options = Array();
			foreach($link_options as $link_name => $bit_value) {
				$options[$bit_value] = link_type_name($bit_value);
			}
			combo_box($prefix.'_create_link_type', $options, false, $asset->attr('create_link_type'));
		}

	}//end paintCreateLink()


	/**
	* Processes the interface for choosing the link type of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLink(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_link_type'])) return false;
		$create_link_type = (int)$_POST[$prefix.'_create_link_type'];
		return $asset->setAttrValue('create_link_type', $create_link_type);

	}//end processCreateLink()


	/**
	* Paint the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLocation(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_location');

		echo '<p>';
		if (empty($locations)) {
			echo 'No create locations have been specified. The user creating the asset will be able to select the location manually.';
		} else if (count($locations) == 1) {
			echo 'All new assets will be created under ';
			$locationid = array_pop($locations);
			$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
			if (!is_null($location)) {
				echo '"'.$location->name.'" [#'.$location->id.']';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
			}
			echo ' ';
			check_box($prefix.'_delete_location['.$location->id.']');
			echo ' Delete ?';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">ID</td>
					<td class="sq-backend-table-header">Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($locations as $locationid => $null) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $location->id; ?></td>
						<td class="sq-backend-table-cell"><?php echo $location->name; ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_location['.$location->id.']'); ?></td><?php
						}
						?>
					</tr>
					<?php
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
				}
				?>
			</table>
			<?php
		}
		echo '</p>';

		if ($write_access) {
			$o->closeField();
			$o->openField('Add New Location');
			asset_finder($prefix.'_new_create_location');
		}

		return $write_access;

	}//end paintCreateLocation()


	/**
	* Processes the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateLocation(&$asset, &$o, $prefix)
	{
		$old_locations = $asset->attr('create_location');
		$new_locations = $old_locations;

		// add a new create location
		if (isset($_POST[$prefix.'_new_create_location'])) {
			$new_location =(int) $_POST[$prefix.'_new_create_location']['assetid'];
			if ($new_location && !isset($new_locations[$new_location])) {
				$new_locations[$new_location] = $new_location;
			}
		}

		// delete existing create locations
		if (isset($_POST[$prefix.'_delete_location'])) {
			foreach ($_POST[$prefix.'_delete_location'] as $locationid => $on) {
				if (isset($new_locations[$locationid])) unset($new_locations[$locationid]);
			}
		}

		if ($old_locations != $new_locations) return $asset->setAttrValue('create_location', $new_locations);
		return false;

	}//end processCreateLocation()


	/**
	* Paint the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateStatus(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		if (!$asset->writeAccess('attributes')) {
			echo get_status_description($asset->attr('create_status'));
		} else {
			$status_options = get_bit_values('SQ_STATUS_');
			$options = Array();
			foreach($status_options as $status_name => $bit_value) {
				$options[$bit_value] = get_status_description($bit_value);
			}
			combo_box($prefix.'_create_status', $options, false, $asset->attr('create_status'));
		}

	}//end paintCreateStatus()


	/**
	* Processes the interface for choosing the status of newly created assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCreateStatus(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_create_status'])) return false;
		$create_status = (int)$_POST[$prefix.'_create_status'];
		return $asset->setAttrValue('create_status', $create_status);

	}//end processCreateStatus()


	/**
	* Returns true if the asset builder allows multiple create locations
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function definesMultipleLocations(&$asset)
	{
		return (count($asset->attr('create_location')) > 1);

	}//end definesMultipleLocations()


}//end class

?>
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
* $Id: design_area_asset_lineage_edit_fns.inc,v 1.20 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Asset_Lineage_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage_Edit_Fns extends Design_Area_Edit_Fns
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
	* Loops through the contents array and creates the necessary objects
	*
	* @param Design_Area_Asset_Lineage	$asset		the asset to which we belong
	* @param array						&$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	public function _processContents(Design_Area_Asset_Lineage $asset, Array &$contents)
	{
		if (!$this->_setVariables($asset, $contents)) {
			return FALSE;
		}

		$asset_index	= -1;
		$divider_index	= -1;
		// see if our page element has any sub's in it
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			switch ($element['operation']) {
				case 'asset' :
					$asset_index = $index;
				break;

				case 'divider' :
					$divider_index = $index;
				break;

			}//end switch

		}//end foreach

		// OK, so we have some asset contents, so extract it
		if ($asset_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$asset_index]['contents'])) {
				$contents[$asset_index]['contents'] = $this->_getDefaultContents($asset, 'asset');
			}

			if (!$asset->setAttrValue('asset_contents', $contents[$asset_index]['contents'])) {
				return FALSE;
			}

			// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
			$contents[$asset_index] = Array(
										'_type'				=> 'TAG',
										'attributes'		=> Array(),
										'operation'			=> 'asset',
										'self_terminating'	=> 1,
									  );

		}//end if

		// OK, so we have some divider contents, so extract it
		if ($divider_index >= 0) {

			if (!$asset->setAttrValue('divider_contents', $contents[$divider_index]['contents'])) {
				return FALSE;
			}

			// Now we want to remove the divider section
			array_splice($contents, $divider_index, 1);

		} else {
			// we dont have a tag, so lets use the default value instead
			if (!$asset->setAttrValue('divider_contents', $this->_getDefaultContents($asset, 'divider'))) {
				return FALSE;
			}
		}

		$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'][$asset->attr('id_name')] = TRUE;
		return $asset->setAttrValue('contents', $contents);

	}//end _processContents()


	/**
	* Paint the interface for choosing asset types to exclude from the lineage
	*
	* @param Design_Area_Asset_Lineage	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintExcludedTypes(Design_Area_Asset_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		$excluded_types = $asset->attr('unwanted_asset_types');
		if ($asset->writeAccess('attributes')) {
			asset_type_chooser($prefix.'_excluded_types', TRUE, $excluded_types);
		} else {
			if (empty($excluded_types)) {
				echo 'No types are excluded';
			} else {
				?>
				<ul>
				<?php
				foreach ($GLOBALS['SQ_SYSTEM']->am->getTypeInfo($excluded_types, 'name') as $type_name) {
					?>
					<li><?php echo $type_name; ?></li>
					<?php
				}
				?>
				</ul>
				<?php
			}
		}

	}//end paintExcludedTypes()


	/**
	* Paint the interface for choosing asset types to exclude from the lineage
	*
	* @param Design_Area_Asset_Lineage	$asset	the asset whose interface we are painting
	* @param Backend_Outputter			$o		the Backend Outputter object
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	public function processExcludedTypes(Design_Area_Asset_Lineage $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			// bug #2712: asset chooser returns an array of empty strings
			// instead of an empty array
			$exclude_list = Array();
			foreach ($_POST[$prefix.'_excluded_types'] as $type) {
				if ($type != '') $exclude_list[] = $type;
			}
			return $asset->setAttrValue('unwanted_asset_types', $exclude_list);
		} else {
			return FALSE;
		}

	}//end processExcludedTypes()


}//end class
?>
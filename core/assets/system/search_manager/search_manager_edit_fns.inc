<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Search_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Search_Manager_Edit_Fns extends Asset_Edit_Fns
{
	var $static_screens = Array(
		'details' => Array(
			'name' => 'Details',
			'force_unlock' => true
		),
	);

	/**
	* Constructor
	*
	*/
	function Search_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}//end constructor



	function paintBackendAssetWeights(&$owner, &$o)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$o->setHeading("Asset Attribute Relevance Weighting");

		$typeHierarchy = $am->getAssetTypeHierarchy();

		$this->paintAssetTypeSection($typeHierarchy, $o);

		$o->commitButton();
	}

	function paintAssetTypeSection($typeHierarchy, &$o) {

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		require_once(SQ_INCLUDE_PATH."/asset_attribute.inc");

		foreach ($typeHierarchy as $typeCode => $assetType) {
			$attributeids = $am->getAssetTypeAttributes($typeCode);

			if(empty($attributeids)) {
				continue;
			}
				
			$o->openSection($assetType['name']);
	
			foreach ($attributeids as $attributeid) {
				$attribute = Asset_Attribute::loadAttribute($attributeid);

				$fieldPrefix = $o->openField(ucfirst(str_replace("_", " ", $attribute->name)));

				$o->addHiddenField("weights[". $typeCode. "][". $attributeid. "]", $attribute->weighting);
				echo text_box("new_weights[". $typeCode. "][". $attributeid. "]", $attribute->weighting, 1);

				$o->closeField();

			}
			$o->closeSection();

			if (!empty($assetType['subs'])) {
				$this->paintAssetTypeSection($assetType['subs'], $o);
			}
		}

	}

	function processBackendAssetWeights(&$owner, $o)
	{
		// go through the list and apply the weights to the table
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		require_once(SQ_INCLUDE_PATH."/asset_attribute.inc");

		$oldWeights = $_REQUEST['weights'];
		$newWeights = $_REQUEST['new_weights'];

		$changedAttributeIds = array();

		foreach ($newWeights as $typeCode => $attributeWeights) {
			foreach ($attributeWeights as $attributeid => $weighting) {
				if ($weighting != $oldWeights[$typeCode][$attributeid]) {
					$changedAttributeIds[$attributeid] = $weighting;
				}
			}
		}

		foreach ($changedAttributeIds as $attributeid => $weight) {
			$attribute = Asset_Attribute::loadAttribute($attributeid);
			$sql = "UPDATE ". SQ_TABLE_PREFIX. "asset_attribute 
					SET weighting = '". addslashes($weight) ."' 
					WHERE attributeid = '". addslashes($attributeid) ."'";

			$db->query($sql);

			$o->addMessage(SQ_BO_MSG_NOTICE, "Weighting changed for attribute ". $attribute->name. " ($attributeid)");
		}
	}

}//end class
?>

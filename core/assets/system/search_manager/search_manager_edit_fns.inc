<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager_edit_fns.inc,v 1.4 2003/10/15 11:05:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Search_Manager_Edit_Fns
*
* Edit functions for the search manager.
*
* @see Asset_Edit_Fns
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.4 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Search_Manager_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	* 
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Search_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}//end constructor


	/**
	* Paints the interface for assigning weights to asset types' attributes.
	* 
	* @param object Search_Manager		&$asset	the search manager asset
	* @param object Backend_Outputter	&$o		the backend outputter
	*
	* @return void
	* @access public
	*/
	function paintAssetWeights(&$owner, &$o)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		$o->setHeading("Asset Attribute Relevance Weighting");

		$typeHierarchy = $am->getAssetTypeHierarchy();

		$this->paintAssetTypeSection($typeHierarchy, $o);

		$o->commitButton();
	} // paintBackendAssetWeights()

	
	/** 
	* Paints a section for an asset type and its attributes and their weights.
	* (recursive)
	*
	* @param Array						$typeHierarchy	The asset type hierarchy. 
	* @param object Backend_Outputter	&$o				The backend outputter.
	* 
	* @access public
	* @return void
	*/
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

	} // end paintAssetTypeSection

	/**
	* Paints the interface for assigning weights to asset types' attributes.
	* 
	* @param object Search_Manager		&$asset	the search manager asset
	* @param object Backend_Outputter	&$o		the backend outputter
	*
	* @return void
	* @access public
	*/
	function processAssetWeights(&$owner, $o)
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
	} // end processBackendAssetWeights()

	/** 
	* Paints the interface for queuing a HIPO to reindex the system. 
	* 
	*
	* @param object Search_Manager		&$asset the search manager asset
	* @param object	Backend_Outputter	&$o
	*
	* @access public
	* @returns boolean
	*/
	function paintReindexing(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			
		} else {
			asset_finder($prefix."_reindexing_root_assetid", '');
		}
	}

	/** 
	* Processes the interface for queuing a HIPO to reindex the system. 
	* 
	*
	* @param object Search_Manager		&$asset the search manager asset
	* @param object	Backend_Outputter	&$o
	*
	* @access public
	* @returns boolean
	*/
	function processReindexing(&$asset, &$o, $prefix)
	{
		$root_assetid = $_POST[$prefix."_reindexing_root_assetid"];
		if ($root_assetid > 0) {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_reindex', Array('root_assetid' => $root_assetid));
			return true;
		}

		return false;
	}

	

}//end class
?>

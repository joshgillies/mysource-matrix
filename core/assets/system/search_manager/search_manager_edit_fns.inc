<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager_edit_fns.inc,v 1.6 2003/10/21 06:30:43 dwong Exp $
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
* @version $Revision: 1.6 $ - 1.0
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



	function paintWeights(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$typeHierarchy = $am->getAssetTypeHierarchy();

		$this->paintAssetTypeSection($asset, $o, $prefix, $typeHierarchy);
	}

	function paintAssetTypeSection(&$asset, &$o, $prefix, $typeHierarchy)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$wa = $asset->writeAccess('attributes');

		foreach ($typeHierarchy as $typeCode => $assetType) {
			$weightings = $am->getTypeDefaultWeightings($typeCode);
			// filter out the dead ones
			foreach ($weightings as $component => $data) {
				if ($data['weighting'] < 0)
					unset($weightings[$component]);
			}
			if (!empty($weightings)) {
				$o->openSection($assetType['name']);
				foreach ($weightings as $component => $data) {
					$o->openField($data['description']);

					if (!$wa) {
						echo $data['weighting'];
					} else {

						if ($data['weighting'] == 0) {
							$disabled = '1';
						} else {
							$disabled = '0';
						}
						combo_box($prefix. "disable[$typeCode][$component]", Array(
							'1' => 'Do not include in searches',
							'0' => 'Include in searches'
						), false, $disabled);
						
						if (!$disabled) {
							echo "&nbsp;Weight: ";
							text_box ($prefix. "weights[$typeCode][$component]", $data['weighting'], 3);
						} else {
							$o->addHiddenField($prefix. "weights[$typeCode][$component]", 1);
						}
					}
					$o->closeField();
				}
				$o->closeSection();
			}

			if (!empty($assetType['subs'])) {
				$this->paintAssetTypeSection($asset, $o, $prefix, $assetType['subs']);
			}
		}		
	} // end paintAssetTypeSection()

	/** 
	* Processes the request for setting of default weights.
	* 
	* @param object Search_Manager		&$asset the search manager asset
	* @param object	Backend_Outputter	&$o		the backend outputter object
	* @param string						$prefix the prefix for field
	*
	* @access public
	* @returns boolean
	*/
	function processWeights (&$asset, &$o, $prefix)
	{
		$weights = $_POST[$prefix. 'weights'];
		$disabled = $_POST[$prefix. 'disable'];
		$im =& $GLOBALS['SQ_SYSTEM']->getIndexingManager();

		foreach ($weights as $typeCode => $weightData) {
			foreach ($weightData as $component => $weighting) {
				if ($disabled[$typeCode][$component] || $weighting < 0)
					$weighting = 0;
				$im->setComponentDefaultWeighting ($typeCode, $component, $weighting);
			}
		}
		return true;
	} // end processWeights();

	/** 
	* Paints the interface for queuing a HIPO to reindex the system. 
	* 
	*
	* @param object Search_Manager		&$asset the search manager asset
	* @param object	Backend_Outputter	&$o		the backend outputter object
	* @param string						$prefix the prefix for field
	*
	* @access public
	* @returns void
	*/
	function paintReindexing(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
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

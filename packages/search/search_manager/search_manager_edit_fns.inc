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
* $Id: search_manager_edit_fns.inc,v 1.5 2004/09/02 06:36:37 arailean Exp $
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
* @version $Revision: 1.5 $ - 1.0
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
	* Paint the interface for painting default weights for each registered asset type
	*
	* @param object Search_Mananger		&$asset		the search manager asset
	* @param object Backend_Outputter	&$o			the backend outputter
	* @param string						$prefix		the prefix for fields
	*
	* @return boolean
	* @access public
	*/
	function paintWeights(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$typeHierarchy = $am->getAssetTypeHierarchy();

		$this->paintAssetTypeSection($asset, $o, $prefix, $typeHierarchy);
		return true;

	}//end paintWeights()


	/**
	* Paint the interface for painting default weights for a particular asset type
	*
	* @param object Search_Mananger		&$asset		the search manager asset
	* @param object Backend_Outputter	&$o			the backend outputter
	* @param string						$prefix		the prefix for fields
	*
	* @return boolean
	* @access public
	*/
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

	}//end paintAssetTypeSection()


	/**
	* Processes the request for setting of default weights.
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processWeights(&$asset, &$o, $prefix)
	{
		$weights = $_POST[$prefix. 'weights'];
		$disabled = $_POST[$prefix. 'disable'];

		foreach ($weights as $typeCode => $weightData) {
			foreach ($weightData as $component => $weighting) {
				if ($disabled[$typeCode][$component] || $weighting < 0)
					$weighting = 0;
					$asset->setComponentDefaultWeighting($typeCode, $component, $weighting);
			}
		}
		return true;

	}//end processWeights()


	/**
	* Paint the interface for queuing a HIPO to reindex the system
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintReindexing(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			asset_finder($prefix.'_reindexing_root_assetid', '');
			echo '&nbsp;&nbsp;&nbsp;<b>OR</b>';
			$o->closeField();
			$o->openField('System Wide');
			check_box($prefix.'_reindexing_root_folder');
			echo 'Reindex all assets in the system';
			return true;
		}
		return false;

	}//end paintReindexing()


	/**
	* Process the interface for queuing a HIPO to reindex the system
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processReindexing(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_reindexing_root_assetid']['assetid'])) {
			$root_assetid = $_POST[$prefix.'_reindexing_root_assetid']['assetid'];
			if ($root_assetid > 0) {
				$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->queueHipo('hipo_job_reindex', Array('root_assetid' => $root_assetid), '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
				return true;
			}
		}
		if (isset($_POST[$prefix.'_reindexing_root_folder'])) {
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$hh->queueHipo('hipo_job_reindex', Array('root_assetid' => '1'), '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
			return true;
		}
		return false;

	}//end processReindexing()


	/**
	* Paint the interface for Noise Words
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintNoiseWords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$noise_words = $asset->attr('noise_word_list');
		$noise_words = (is_array($noise_words))? implode(' ', $noise_words):'';

		if ($write_access) {
			text_area($prefix.'_noise_words', $noise_words, 60, 5);
		} else {
			echo $noise_words;
		}
		return $write_access;

	}//end paintNoiseWords()


	/**
	* Process the interface for adding noise words
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processNoiseWords(&$asset, &$o, $prefix)
	{

		$words = array_get_index($_POST, $prefix.'_noise_words', false);

		$matches	= Array();
		$new_words	= Array();

		if (preg_match_all('/\w+/i', $words, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match){
				$word = array_pop($match);
				$new_words[$word] = $word;
			}
		}

		$asset->setAttrValue('noise_word_list', $new_words);

		return true;

	}//end processNoiseWords()


	/**
	* Paint the interface for White Words
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function paintWhiteWords(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$words = $asset->attr('white_word_list');
		$words = (is_array($words))? implode(' ', $words):'';

		if ($write_access) {
			text_area($prefix.'_white_words', $words, 60, 5);
		} else {
			echo $words;
		}
		return $write_access;

	}//end paintWhiteWords()


	/**
	* Process the interface for adding white words
	*
	* @param object Search_Manager		&$asset		the search manager asset
	* @param object	Backend_Outputter	&$o			the backend outputter object
	* @param string						$prefix		the prefix for field
	*
	* @return boolean
	* @access public
	*/
	function processWhiteWords(&$asset, &$o, $prefix)
	{

		$words = array_get_index($_POST, $prefix.'_white_words', false);

		$matches	= Array();
		$new_words	= Array();

		if (preg_match_all('/\w+/i', $words, $matches, PREG_SET_ORDER)) {
			foreach ($matches as $match){
				$word = array_pop($match);
				$new_words[$word] = $word;
			}
		}

		$asset->setAttrValue('white_word_list', $new_words);

		return true;

	}//end processWhiteWords()


}//end class
?>
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
* $Id: design_area_asset_lineage_edit_fns.inc,v 1.5 2003/12/12 15:01:35 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Asset_Lineage_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Returns an array of vars that are setable, used to create the parseString
	*
	* @param object Asset	$asset	the asset whose interface we are painting
	*
	* @return Array
	* @access public
	* @see createParseString()
	*/
	function _getSetableVars(&$asset)
	{
		$vars = parent::_getSetableVars($asset);
		unset($vars['asset_contents']);
		unset($vars['divider_contents']);
		return $vars;

	}// end _getSetableVars()


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, &$contents)
	{
		if (!$this->_setVariables($asset, $contents)) return false;

		$asset_index	= -1;
		$divider_index	= -1;
		// see if our page element has any sub's in it
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG') continue;

			switch ($element['operation']) {
				case 'asset' :
					$asset_index = $index;
				break;

				case 'divider' :
					$divider_index = $index;
				break;

			}// end switch

		}//end foreach

		// OK, so we have some asset contents, so extract it
		if ($asset_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$asset_index]['contents'])) {
				$contents[$asset_index]['contents'] = $this->_getDefaultContents($asset, 'asset');
			}

			if (!$asset->setAttrValue('asset_contents', $contents[$asset_index]['contents'])) {
				return false;
			}

			// Now what we do is morph it so that this element doesn't contain all the un-necessary contents stuff
			$contents[$asset_index] = Array(
											'_type' => 'TAG',
											'attributes'  => Array (),
											'operation' => 'asset',
											'self_terminating' => 1
										);

		}// end if

		// OK, so we have some divider contents, so extract it
		if ($divider_index >= 0) {

			// if there is no contents, then see if there is a default
			if (empty($contents[$divider_index]['contents'])) {
				$contents[$divider_index]['contents'] = $this->_getDefaultContents($asset, 'divider');
			}

			if (!$asset->setAttrValue('divider_contents', $contents[$divider_index]['contents'])) {
				return false;
			}

			// Now we want to remove the divider section
			array_splice($contents, $divider_index, 1);

		}// end if

		return $asset->setAttrValue('contents', $contents);

	}// end _processContents()


	/**
	* Used by _outputContents() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	* @see Design_Area_Edit_Fns::_outputContents()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'asset' :

				// because the page lineage array contains the site index, all we need to do to have or not have
				// the home link is start the loop at a different position
				$start_index = ($asset->attr('prefix_with_home_link')) ? 0 : 1;

				$total = 'COUNT($ASSET_LINEAGE)';
				// because the page lineage array contains the current page, all we need to do to have or not have
				// the current link is end the loop at a different position
				if (!$asset->attr('suffix_with_current_link')) {
					$total .= ' - 1';
				}

				// if we have pages to print, we just adjust the total count factor
				if (($levels_to_print = (int) $asset->attr('levels_to_print')) > 0) {
					$total = 'min('.$total.', '.($levels_to_print + $start_index).')';
				}

				if ($divider_contents = $asset->attr('divider_contents')) {
					ob_start();
					if (!$this->_outputContents($divider_contents, $asset, $design)) return false;
					$divider = ob_get_contents();
					ob_end_clean();
				} else {
					$divider = '';
				}

				if ($asset->attr('prefix_with_divider')) {
					echo $divider;
				}

				echo '<', '?php

				for($'.$asset->attr('id_name').'_i = '.$start_index.', $'.$asset->attr('id_name').'_total = '.$total.';
					$'.$asset->attr('id_name').'_i < $'.$asset->attr('id_name').'_total;
					$'.$asset->attr('id_name').'_i++) {
					';

				if ($asset_contents = $asset->attr('asset_contents')) {
					echo '?','>';
					if (!$this->_outputContents($asset_contents, $asset, $design)) return false;
					echo '<','?php ';
				}

				// if there is not divider at the end we need to check that we aren't at the end of the list
				if (!$asset->attr('suffix_with_divider')) {
					echo '
						if ($'.$asset->attr('id_name').'_i != $'.$asset->attr('id_name').'_total - 1) {
						';
				}

				echo '?','>', $divider, '<','?php ';

				// if there is not divider at the end we need to check that we aren't at the end of the list
				if (!$asset->attr('suffix_with_divider')) {
					echo '
						}// end if';
				}

				echo '
				}// end for
				?','>';

				return true;

			default :
				return parent::_outputElement($element, $asset, $design);
		}// end switch

	}// end _outputElement()


}//end class
?>

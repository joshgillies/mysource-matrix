<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Menu_Type_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design_Area_Menu_Type_Edit_Fns extends Design_Area_Edit_Fns
{

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
		if (!$asset->setAttrValue('settings', Array())) return false;
		if (!parent::_processContents($asset, $contents)) return false;
		$settings = $asset->attr('settings');
		// Now just make sure that current, hierarchy and normal entries all exist for the settings
		for(reset($settings); NULL !== ($name = key($settings)); next($settings)) {
			if (!isset($settings[$name]['normal'])) {
				trigger_error('Setting "'.$name.'" has no corresponding "normal", in "'.$asset->name.'" design area', E_USER_WARNING);
				return false;
			}

			if (!isset($settings[$name]['hierarchy'])) {
				$settings[$name]['hierarchy'] = (isset($settings[$name]['current'])) ? $settings[$name]['current'] : $settings[$name]['normal'];
			}

			if (!isset($settings[$name]['current'])) {
				$settings[$name]['current'] = (isset($settings[$name]['hierarchy'])) ? $settings[$name]['hierarchy'] : $settings[$name]['normal'];
			}

		}// end foreach

		return $asset->setAttrValue('settings', $settings);

	}// end _processContents()


	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		echo '<', '?php
		$page_lineage = $am->getLineageFromURL();
		$level = '.((int) $asset->attr('level')).';
		if (!empty($page_lineage) && !empty($page_lineage[$level])) {
			$settings = '.var_export($asset->attr('settings'), true).';
			$parent = &$am->getAsset($page_lineage[$level][\'assetid\'], $page_lineage[$level][\'type_code\']);
			if (!is_null($parent)) {
				$links = $parent->getLinks(SQ_LINK_NAV);
				if (!empty($links)) {
		#			pre_echo($links);
		?', '>';
		parent::_createDesignFileOutput($contents, $asset, $design);
		echo '<', '?php
				}// end if not empty links
			}// end if null parent
		}// end if page_lineage found
		?', '>';

		return true;

	}// end _createDesignFileOutput()

	/**
	* Used by _createDesignFileOutput() to deal with a print tag
	*
	* @param array			$element	the print element tag
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @access private
	* @see _createDesignFileOutput()
	*/
	function _outputElement($element, &$asset, &$design)
	{
		switch($element['operation']) {
			case 'page' :
				echo '<', '?php 
				// is the asset that is printing this body is a site, 
				// then use it\'s index page for the lineage asset
				if ($ASSET->type() == \'site\') { 
					$link = $ASSET->getSpecialPageLink(\'index\');
					$lineage_assetid = (empty($link)) ? 0 : $link[\'minorid\'];
					$lineage_setting = \'current\';
				} else {
					$lineage_assetid = $page_lineage[$level + 1][\'assetid\'];
					$lineage_setting = (count($page_lineage) - 1 == $level + 1) ? \'current\' : \'hierarchy\';
				}
				pre_echo("Lineage Setting : $lineage_setting");
				pre_echo("Lineage assetid : $lineage_assetid");
				foreach($links as $data) { 
					$kid = &$am->getAsset($data[\'minorid\'], $data[\'type_code\']);
					$current_setting = ($lineage_assetid == $kid->id) ? $lineage_setting : \'normal\';
				?', '>';
				parent::_createDesignFileOutput($element['contents'], $asset, $design);
				echo '<', '?php 
				}// end foreach links
				?', '> ';

			break;

			default :
				parent::_outputElement($element, $asset, $design);
		}// end switch

	}// end _outputElement()


}//end class
?>

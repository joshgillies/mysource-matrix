<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Metadata_Schema_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Metadata_Schema_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Metadata_Schema_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end construtor()


	/**
	* Paint the interface for filling in metadata values
	*
	* @param object Metadata_Schema		&$asset			the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o				the outputter class
	* @param array						$values			current metadata values set for this asset
	* @param boolean					$write_access	does the current user have write access to the asset
	*													that we are setting metadata for (NOT THE SAME AS $asset)
	*
	* @return boolean
	* @access public
	*/
	function paintInlineEditInterface(&$asset, &$o, $values, $write_access)
	{
		$o->openSection($asset->name);
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!isset($values['sections'][$section->name])) $values['sections'][$section->name] = Array();
			if (!$edit_fns->paintInlineEditInterface($section, $o, $values['sections'][$section->name], $write_access)) return false;
		}
		$o->closeSection();

		return true;

	}//end paintInlineEditInterface()


	/**
	* Process the interface for filling in metadata values
	*
	* @param object Metadata_Schema	&$asset			the asset whose interface we are painting
	* @param array					&$new_values	an array we can store the new values in
	*
	* @return boolean
	* @access public
	*/
	function processInlineEditInterface(&$asset, &$new_values)
	{
		$new_values[$asset->name] = Array();
		$new_values[$asset->name]['sections'] = Array();
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->processInlineEditInterface($section, $new_values[$asset->name]['sections'])) return false;
		}

		return true;

	}//end processInlineEditInterface()


	/**
	* Output the complete metadata tag for this schema
	*
	* @param object Metadata_Field	&$asset		the asset whose interface we are painting
	* @param array					$values		the current metadata values
	* @param array					&$keywords	an array we can put keywords in that need replacing
	*
	* @return boolean
	* @access public
	*/
	function generateMetadata(&$asset, $values, &$keywords)
	{
		$current_links = $asset->getLinks(SQ_LINK_TYPE_2, 'metadata_section', true, 'major', null, true);
		foreach ($current_links as $link) {
			$section = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);
			if (is_null($section)) continue;
			$edit_fns = $section->getEditFns();
			if (!$edit_fns->generateMetadata($section, $values['sections'][$section->name], $keywords)) return false;
		}

		return true;

	}//end generateMetadata()

}//end class

?>
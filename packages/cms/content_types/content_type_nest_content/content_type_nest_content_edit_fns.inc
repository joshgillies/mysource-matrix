<?php

require_once dirname(__FILE__).'/../../content_type/content_type_edit_fns.inc';

/**
* Content_Type_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::cms
*/
class Content_Type_Nest_Content_Edit_Fns extends Content_Type_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Content_Type_Nest_Content_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}

	
	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	* @param string								$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		echo '<b>Nested AssetID:</b> ';
		text_box($prefix.'_nested_assetid', $asset->attr('nested_assetid'), 10);
		
		$nested_assetid = $asset->attr('nested_assetid');
		$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', true);
		if ($nested_asset->id) {
			// print some details about the nested asset
			echo ' <-- <i>'.$nested_asset->attr('name').' [ '.$nested_assetid.' ]</i><br>';
		}

		echo '<br><b>The contents of the nested asset are printed below</b><hr>';
		$this->paint($asset);
		return true;
	}


	/**
	* Just calls printBody on the nested asset and lets it take care of things
	*
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset) {
		$nested_assetid = $asset->attr('nested_assetid');
		// get the asset - we cant pass the type in because it could be anything
		// we also mute errors because we check later on
		$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', true);
		if (!$nested_asset->id) return;
		$nested_asset->printBody();
	}


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array(string)						$link	information used to create the initial link
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	* @param string								$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		$current_assetid = $asset->attr('nested_assetid');
		$assetid = (isset($_POST[$prefix.'_nested_assetid'])) ? $_POST[$prefix.'_nested_assetid'] : 0;
		if ($assetid != $current_assetid) {
			if (!$asset->setAttrValue('nested_assetid', $assetid)) return false;
			
			// get the html contents of the nested asset so
			// id we change the type of the cell, other content
			// types can use the html
			ob_start();
			$this->paint($asset);
			$html = ob_get_contents();
			$asset->setAttrValue('html', $html);
			ob_end_clean();
			return true;
			
		}
		return false;
	}

}//end class
?>

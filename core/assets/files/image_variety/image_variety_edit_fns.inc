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
* $Id: image_variety_edit_fns.inc,v 1.2 2005/01/17 18:01:49 brobertson Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Image_Variety_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Image_Variety_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Image_Variety_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		// override static screens - only display details because the others don't apply
		$this->static_screens = Array(
									'details' => Array(
													'name'			=> 'Details',
													'force_unlock'	=> false,
													'lock_type'		=> 'none',
												),
								);

	}//end constructor


	/**
	* Paint the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintInlineInterface(&$asset, &$o, $prefix)
	{
		require_once 'XML/Tree/Node.php';
		$wa = $asset->writeAccess('attributes');

		if (!$asset->id && !$wa) return false;

		$int_edit_params = new XML_Tree_Node('int', '', Array('width' => 8));

		$o->openSection(($asset->name) ? $asset->name : 'New');
		if ($asset->id) {
			$o->openField('Current File');
				?>
					<a href="<?php echo current_url(true, true).'?a='.$asset->id?>&now=<?php echo time()?>" target="_blank">Click to open in a new window</a>
				<?php
			$o->closeField();
		}

		if ($asset->name == '') {
			$o->openField('Name');
				$text_edit_params = new XML_Tree_Node('text', '', Array('width' => 15));
				$name = &$asset->getAttribute('name');
				$name->setEditParams($text_edit_params);
				$name->paint($prefix.'_'.$name->name, !$wa);
			$o->closeField();
		}

			$o->openField('Width');
				$width = &$asset->getAttribute('width');
				$width->setEditParams($int_edit_params);
				$width->paint($prefix.'_'.$width->name, !$wa);
			$o->closeField();
			$o->openField('Height');
				$height = &$asset->getAttribute('height');
				$height->setEditParams($int_edit_params);
				$height->paint($prefix.'_'.$height->name, !$wa);
			$o->closeField();
			$o->openField('');
				$o->note('Either the width or height attributes can be set to zero.<br>This will mean that this dimension is not important and will force the aspect ratio of the original image to be maintained');
			$o->closeField();
		$o->closeSection();

		return true;

	}// end paintInlineValueInterface();


	/**
	* Process the interface for the image variety
	*
	* @param object Image_Variety		&$asset		the asset whose interface we are processing
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInlineInterface(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$changes_made = false;

		$vars = Array('width', 'height');
		if ($asset->attr('name') == '') array_unshift($vars, 'name');

		foreach($vars as $attr_name) {
			$attr = &$asset->getAttribute($attr_name);
			$attr->process($prefix.'_'.$attr->name);
			if ($attr->processed && $asset->setAttrValue($attr->name, $attr->value)) {
				$changes_made = true;
			}
		}

		if (!$changes_made) return false;
		else return $asset->saveAttributes();

	}// end processInlineValueInterface();


}//end class
?>

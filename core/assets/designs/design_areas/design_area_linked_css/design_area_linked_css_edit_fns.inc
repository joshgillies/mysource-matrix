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
* $Id: design_area_linked_css_edit_fns.inc,v 1.2 2004/09/07 10:52:20 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Linked_Css_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css_Edit_Fns extends Design_Area_Edit_Fns
{

	/**
	* Prints the interface for linking to an assetid
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAssetid(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('links')) {
			$type_code_restrictions['design_css'] = 'D';
			$type_code_restrictions['design_css_customisation'] = 'D';
			asset_finder($prefix.'_assetid', $asset->_getLinkedCSSInfo('minorid'), $type_code_restrictions);

		} elseif ($asset->_getLinkedCSSInfo('minorid')) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_getLinkedCSSInfo('minorid'));
			if (!is_null($link_asset)) echo $link_asset->name, '(Id: #', $link_asset->id, ')';

		}// end if

		return true;

	}//end paintAssetid()


	/**
	* Prints the interface for selecting a tag type
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintTagType(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$options = Array();
			$options['link_tag'] = 'Link tag';
			$options['import_tag'] = '@import tag';
			combo_box($prefix.'_tag_type', $options, false, $asset->attr('tag_type'));
		} else {
			switch ($asset->attr('tag_type')) {
				case 'link_tag' :
					echo 'Link Tag';
					break;
				case 'import_tag':
					echo '@import tag';
					break;
			}
		}// end if

		return true;

	}//end paintTagType()
	
	/**
	* Prints the interface for selecting a tag type
	*
	* @param object Asset				&$asset	the owning asset
	* @param object Backend_Outputter	&$o		the backend outputter
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintMedia(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$options = Array();
			$options['all'] = 'All';
			$options['aural'] = 'Aural';
			$options['braille'] = 'Braille';
			$options['embossed'] = 'Embossed';
			$options['handheld'] = 'Handheld';
			$options['print'] = 'Print';
			$options['projection'] = 'Projection';
			$options['screen'] = 'Screen';
			$options['tty'] = 'TTY';
			$options['tv'] = 'TV';
			combo_box($prefix.'_media', $options, false, $asset->attr('media'));
		} else {
			echo $asset->attr('media');
		}// end if

		return true;

	}//end painMedia()


	/**
	* Processes the interface for setting the tag type
	*
	* @param object Asset				&$asset	the owning asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processTagType(&$asset, &$o, $prefix)
	{
		$tag_type  = (int) (isset($_POST[$prefix.'_tag_type'])) ? $_POST[$prefix.'_tag_type'] : '';
		return $asset->setAttrValue('tag_type', $tag_type);

	}//end processTagType()


	/**
	* Processes the interface for setting the media
	*
	* @param object Asset				&$asset	the owning asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processMedia(&$asset, &$o, $prefix)
	{
		$media  = (int) (isset($_POST[$prefix.'_media'])) ? $_POST[$prefix.'_media'] : '';
		return $asset->setAttrValue('media', $media);

	}//end processMedia()


	/**
	* Processes the interface for linking to an asset
	*
	* @param object Asset				&$asset	the owning asset
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processAssetid(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) return false;

		if ($asset->_getLinkedCSSInfo('minorid') == $_POST[$prefix.'_assetid']['assetid']) return false;

		$assetid = (int) $_POST[$prefix.'_assetid']['assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// remove the old link first
		if ($old_linkid = $asset->_getLinkedCSSInfo('linkid')) {
			if (!$asset->deleteLink($old_linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// now create the new link
		if ($assetid) {
			$link_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($link_asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$asset->createLink($link_asset, SQ_LINK_NOTICE, 'linked_css_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processAssetid()

}//end class

?>
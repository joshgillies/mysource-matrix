<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: design_area_linked_css_edit_fns.inc,v 1.11 2007/12/14 23:47:00 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Area_Linked_Css_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the interface for linking to an assetid
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintAssetid(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('links')) {
			$type_code_restrictions['design_css'] = 'D';
			$type_code_restrictions['design_css_customisation'] = 'D';
			asset_finder($prefix.'_assetid', $asset->getLinkedCSSInfo('minorid'), $type_code_restrictions);

		} else if ($asset->getLinkedCSSInfo('minorid')) {
			$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->getLinkedCSSInfo('minorid'));
			if (!is_null($link_asset)) {
				echo get_asset_tag_line($link_asset->id);
			}

		}//end if

		return TRUE;

	}//end paintAssetid()


	/**
	* Prints the interface for selecting a tag type
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintTagType(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$options = Array();
			$options['link_tag'] = 'Link tag';
			$options['import_tag'] = '@import tag';
			combo_box($prefix.'_tag_type', $options, FALSE, $asset->attr('tag_type'));
		} else {
			switch ($asset->attr('tag_type')) {
				case 'link_tag' :
					echo 'Link Tag';
				break;
				case 'import_tag':
					echo '@import tag';
				break;
			}
		}//end if

		return TRUE;

	}//end paintTagType()


	/**
	* Prints the interface for selecting a tag type
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function paintMedia(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
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
			combo_box($prefix.'_media', $options, FALSE, $asset->attr('media'));
		} else {
			echo $asset->attr('media');
		}//end if

		return TRUE;

	}//end paintMedia()


	/**
	* Processes the interface for setting the tag type
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processTagType(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
	{
		$tag_type  = (int) (isset($_POST[$prefix.'_tag_type'])) ? $_POST[$prefix.'_tag_type'] : '';
		return $asset->setAttrValue('tag_type', $tag_type);

	}//end processTagType()


	/**
	* Processes the interface for setting the media
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processMedia(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
	{
		$media  = (int) (isset($_POST[$prefix.'_media'])) ? $_POST[$prefix.'_media'] : '';
		return $asset->setAttrValue('media', $media);

	}//end processMedia()


	/**
	* Processes the interface for linking to an asset
	*
	* @param Design_Area_Linked_CSS	$asset	the owning asset
	* @param Backend_Outputter		$o		the backend outputter
	* @param string					$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processAssetid(Design_Area_Linked_CSS $asset, Backend_Outputter $o, $prefix)
	{
		if (!isset($_POST[$prefix.'_assetid']['assetid'])) {
			return FALSE;
		}

		if ($asset->getLinkedCSSInfo('minorid') == $_POST[$prefix.'_assetid']['assetid']) {
			return FALSE;
		}

		$assetid = (int) $_POST[$prefix.'_assetid']['assetid'];

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// remove the old link first
		if ($old_linkid = $asset->getLinkedCSSInfo('linkid')) {
			if (!$asset->deleteLink($old_linkid)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		// now create the new link
		if ($assetid) {
			$link_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			if (is_null($link_asset)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			if (!$asset->createLink($link_asset, SQ_LINK_NOTICE, 'linked_css_asset')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}

		// If we get this far, then it's all OK
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processAssetid()


}//end class
?>

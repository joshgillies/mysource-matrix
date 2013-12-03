<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: paint_layout_page.inc,v 1.16 2012/08/30 01:09:09 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page_templates/page_standard/page_standard.inc';
require_once SQ_CORE_PACKAGE_PATH.'/paint_layout/paint_layout_bodycopy/paint_layout_bodycopy.inc';

define('SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE', 'paint_layout_bodycopy');
define('SQ_PAINT_LAYOUT_REQUEST_VAR_ASSET', 'sqpa');
define('SQ_PAINT_LAYOUT_DEFAULT_FORMAT_NAME', 'default_format');


/**
* Paint_Layout_Page
*
* This asset is used to represent other assets in the system.
*
* The representation of each asset type is done by a different bodycopy which
* has access to all the keywords of the asset.
*
* Those bodycopies also support conditions.
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Page extends Page_Standard
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset listings create bopycopy and folder asset when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _createAdditional(Array &$link)
	{
		if (!parent::_createAdditional($link, Array('content' => '%paint_layout%'))) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset(SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE);

		// 1. create type formats folder
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('folder');

		$sub_assets= Array(
						'type_formats'	=> 'folder',
					 );

		$type_formats = NULL;
		foreach ($sub_assets as $name => $type) {
			$asset = new $type();
			$copy_link = Array('asset' => &$this, 'value' => $name ,'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$asset->setAttrValue('name', ucwords(str_replace('_',' ', $name)));
			if (!$asset->create($copy_link)) return FALSE;

			if ($name == 'type_formats') $type_formats = $asset;

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			unset($asset);
		}

		// 2. create default format bodycopy
		$type_format_asset_type_code = SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE;
		$asset = new $type_format_asset_type_code();

		$copy_link = Array(
						'asset'			=> &$type_formats,
						'value'			=> SQ_PAINT_LAYOUT_DEFAULT_FORMAT_NAME,
						'link_type'		=> SQ_LINK_TYPE_2,
						'is_dependant'	=> 1,
						'is_exclusive'	=> 1,
					 );

		$asset->setAttrValue('name', 'Default Format');

		return $asset->create($copy_link, Array('content' => '%asset_contents%'));

	}//end _createAdditional()


	/**
	* Queue the setting of the value of an attribute
	*
	* This function sets the value of the attribute locally, but does not database updating.
	* The change is queued awaiting the call of saveAttributes()
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	public function setAttrValue($name, $value)
	{
		if ($name == 'types' && !empty($value)) {
			// because of way that we use the 'types' attribute in _getBodycopyFromAssetTypeCode() we need to make sure
			// that all the types that inherit from other types are at the start of the list
			// otherwise if we have 'page' and 'page_standard' in the list in that order, and we are viewing a page_standard asset
			// it will pick the 'page' format bodycopy because 'page_standard' inherits from 'page'
			$orig_value = $value;
			$value = Array();
			$levels = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($orig_value), 'lvl');
			arsort($levels);
			foreach ($levels as $type_code => $lvl) {
				$value[$type_code] = $orig_value[$type_code];
			}

		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		$assetid = $this->_getAssetToPaint();
		$this->paintAssetFromId($assetid);

	}//end printBody()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	public function onRequestKeywords(Asset $broadcaster, Array $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);

		// if we're being handled here (instead of one of our extensions),
		// it must be from one of our format bodycopies, or something else altogether,
		// so we'll try to find out which of the format types it is.
		$folder = $this->getFolder();
		if (is_null($folder)) return;

		if (isset($parents[$folder->id])) {
			$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE);

			foreach ($type_links as $link_info) {
				if (isset($parents[$link_info['minorid']])) {
					$type_code = $link_info['value'];
					if ($type_code == SQ_PAINT_LAYOUT_DEFAULT_FORMAT_NAME) {
						$type_code = '';
					}

					$type_format = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_info['minorid']);
					if (!empty($type_format)) {
						$type_format->getKeywordsDescription($vars['keywords'], $type_code);
					}

					break;
				}
			}

		} else {
			// must be our page contents bodycopy
			$vars['keywords']['paint_layout'] = 'Paint Layout';
		}

	}//end onRequestKeywords()


	/**
	* Returns the id of the asset to paint.
	*
	* @return int
	* @access private
	*/
	protected function _getAssetToPaint()
	{
		$ret = 0;

		if (!empty($_REQUEST[SQ_PAINT_LAYOUT_REQUEST_VAR_ASSET])) {
			$ret = $_REQUEST[SQ_PAINT_LAYOUT_REQUEST_VAR_ASSET];
		}

		return $ret;

	}//end _getAssetToPaint()


	/**
	* Paint an asset
	*
	* @param int	$assetid	the id of the asset to paint
	*
	* @return void
	* @access public
	*/
	public function paintAssetFromId($assetid=0)
	{
		if (!empty($assetid)) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			$this->paint($asset);
		}


	}//end paintAssetFromId()


	/**
	* Paints out our layout
	*
	* @param Asset	$asset	the asset that we are going to print the body for
	*
	* @return void
	* @access public
	*/
	public function paint(Asset $asset)
	{
		if (!empty($asset)) {
			$bodycopy = $this->getBodycopy();
			if (is_null($bodycopy) || !$bodycopy->readAccess()) {
				$asset->printBody();
			} else {
				$keywords = $bodycopy->getKeywords();
				$replacements = $this->_getKeywordReplacements($keywords, $asset);
				$bodycopy->setKeywordReplacements($replacements);
				$bodycopy->printBody();
			}
		}

	}//end paint()


	/**
	* Returns replacements for the page bodycopy keywords (array of keyword => value)
	*
	* @param array	$keywords	the keywords to replace
	* @param object	&$asset		the asset to represent
	*
	* @return array
	* @access private
	*/
	protected function &_getKeywordReplacements($keywords, &$asset)
	{
		$ret = Array();

		if (in_array('paint_layout', $keywords)) {
			$asset_type_code = $asset->type();
			$bodycopy = $this->_getBodycopyFromAssetTypeCode($asset_type_code);

			$ret['paint_layout'] = $bodycopy->paintAsset($asset);
		}

		return $ret;

	}//end _getKeywordReplacements()


	/**
	* Returns a reference to the format bodycopy corresponding to
	* $asset_type_code
	*
	* @param string	$asset_type_code	type of the asset to represent
	*
	* @return Bodycopy
	* @access private
	*/
	protected function _getBodycopyFromAssetTypeCode($asset_type_code)
	{
		$ret = NULL;

		$supported_types = $this->attr('types');

		$format_type = '';
		foreach ($supported_types as $supported_type => $inherited) {
			$supported_type_and_children = ($inherited) ? $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($supported_type, TRUE) : Array($supported_type);
			if (in_array($asset_type_code, $supported_type_and_children)) {
				$format_type = $supported_type;
				break;
			}
		}

		$folder = $this->getFolder();

		if (!empty($format_type)) {
			$format_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE, TRUE, $format_type);
		} else {
			// not a customized type, so let's take the default format
			$format_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE, TRUE, SQ_PAINT_LAYOUT_DEFAULT_FORMAT_NAME);
		}

		if (!empty($format_link)) {
			$ret = $GLOBALS['SQ_SYSTEM']->am->getAsset($format_link['minorid'], $format_link['minor_type_code']);
		}

		return $ret;

	}//end _getBodycopyFromAssetTypeCode()


	/**
	* Return a reference to a folder linked to this asset determined by the passed in value on the link
	*
	* @param string	$type	the value on the link for this folder
	*
	* @return Folder
	* @access public
	*/
	public function getFolder($type='type_formats')
	{
		$null = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'folder', TRUE, $type);
		if (empty($link)) return $null;

		$folder = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
		if (is_null($folder)) return $null;

		return $folder;

	}//end getFolder()


	/**
	* Get a list of all formats of a certain type
	*
	* The results of this function are cached for the length of the script execution
	*
	* @param string	$type	the value on the link for the formats folder
	*
	* @return array
	* @access public
	*/
	public function getFormats($type='type_formats')
	{
		if (!isset($this->_tmp['formats'][$type])) {
			$folder = $this->getFolder($type);
			$format_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE, TRUE);
			if (empty($format_links)) return Array();

			$formats = Array();
			foreach ($format_links as $link_data) {
				$formats[$link_data['minorid']] = $link_data['value'];
			}
			$this->_tmp['formats'][$type] = $formats;
		}
		return $this->_tmp['formats'][$type];

	}//end getFormats()


	/**
	* Create a new type format/bodycopy in the Type Format Folder.
	*
	* Return TRUE if successful
	*
	* @param string	$type	an asset type code that the new type format must implement
	*
	* @return boolean
	* @access public
	*/
	public function createTypeFormatBodycopy($type)
	{
		$ret = FALSE;

		// create new bodycopy for that type
		$folder = $this->getFolder('type_formats');
		$new_type = $type;
		// check that this type isnt already customised
		$current_link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE, TRUE, $new_type);
		if (empty($current_link)) {
			// create a new format bodycopy for this type
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset(SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE);

			$copy_link = Array('asset' => &$folder, 'value' => $new_type, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1);

			$type_format_asset_type_code = SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE;
			$bodycopy = new $type_format_asset_type_code();
			$bodycopy->setAttrValue('name', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_type, 'name').' Format');
			$bodycopy->setAttrValue('associated_asset_type', $type);

			if ($bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				$ret = TRUE;
			} else {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			}
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		} else {
			$ret = TRUE;
		}

		return $ret;

	}//end createTypeFormatBodycopy()

	
	/**
	*
	* Get available conditions of the asset its parent site
	* This asset doesn't support conditions, return empty array
	* 
	* @return array
	* @access public
	*/
	function getAvailableConditions()
	{
	    return Array();
	}//end getAvailableConditions

	/**
	*
	* Get all the conditions set for the asset
	* This asset doesn't support conditions, return empty array
	*
	* @return array
	* @access public
	*/
	function getConditions()
	{
	    return Array();
	}//end getConditions
	
}//end class

?>

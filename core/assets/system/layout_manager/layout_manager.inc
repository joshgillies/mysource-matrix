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
* $Id: layout_manager.inc,v 1.22 2012/05/22 00:58:52 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Layout Manager
*
* Manages Layouts for type_codes for limbo mode
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Layout_Manager($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* Returns either the linkid or FALSE on failure
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return mixed int|boolean
	* @access public
	* @see Asset::create()
	*/
	public function create(Array &$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	*/
	protected function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	public function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* Return FALSE always because we don't want to be able to clone a layout manager
	*
	* @return boolean
	* @access public
	*/
	public function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns a Layout Asset for a type_code and its screen (or NULL on error)
	*
	* OR, if no screen is specified, an array of layouts for all the defined screens for the type_code in the form:
	* <pre>
	*  Array(
	*     'screen_name1' => &$layout_asset1,
	*     'screen_name2' => &$layout_asset2,
	*  );
	* </pre>
	*
	* @param object	$asset		the asset for which the layout is wanted
	* @param string	$screen		the name of the screen for which the layout is defined
	*
	* @return object
	* @access public
	*/
	public function getLayout($asset, $screen='')
	{
		$null = NULL; // for returning a reference

		// layouts should only be used on the frontend
		if (!SQ_IN_LIMBO) return $null;

		if (is_null($asset)) return $null;

		// Go and find if there is a layout on this asset, first.
		// Layouts defined by the Layouts screen on the asset
		$layout = $this->_getAssetLayout($asset, $screen);

		if (!is_null($layout)) return $layout;

		// If no Asset Layout found, continue to use Layout Manager for a layout
		$type_code = $asset->type();
		$layouts = $this->attr('layouts');

		// if there are no layouts directly applied for this type code will
		// will check the type ancestors to look for one
		if (!isset($layouts[$type_code]) || (!empty($screen) && !isset($layouts[$type_code][$screen]))) {
			if (NULL === ($type_code = $this->getAncestorLayoutTypeCode($type_code, $screen))) {
				return $null;
			}
		}

		// no layouts defined for this type_code's screens
		if (empty($layouts[$type_code])) return $null;

		// if they have not specified a screen, return an array of references to all the screen layouts
		if ($screen == '') {
			$layout = Array();
			foreach ($layouts[$type_code] as $screen => $assetid) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'layout');
				if (is_null($asset)) continue;
				$layout[$screen] = $asset;
			}
			if (empty($layout)) return $null;
		} else {
			if (!isset($layouts[$type_code][$screen])) {
				return $null;
			}
			$layout = NULL;
			if (!empty($layouts[$type_code][$screen])) {
				$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($layouts[$type_code][$screen], 'layout');
			}
			return $layout;
		}
		return $layout;

	}//end getLayout()


	/**
	* Returns the layout type code of ancestor for a given type code, or NULL if none exits
	*
	* @param string	$type_code	the type_code to find an ancestor type code for
	* @param string	$screen		the name of the screen for which the layout is defined
	*
	* @return string
	* @access public
	*/
	public function getAncestorLayoutTypeCode($type_code, $screen='')
	{
		$layouts = $this->attr('layouts');
		$ancestor_type_code = '';

		$ancestors = $GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($type_code, FALSE);
		foreach ($ancestors as $type) {
			if (isset($layouts[$type])) {
				if (!empty($screen) && !isset($layouts[$type][$screen])) {
					continue;
				}
				$ancestor_type_code = $type;
				break;
			}
		}
		return (!empty($ancestor_type_code)) ? $ancestor_type_code : NULL;

	}//end getAncestorLayoutTypeCode()


	/**
	* returns true if a layout is defined for a particular type_code and screen
	*
	* @param string	$asset		the asset to check
	* @param string	$screen		the screen of the type_code to check
	*
	* @return boolean
	* @access public
	*/
	public function hasLayout($asset, $screen='details')
	{
		// Check for errors first
		if (is_null($asset)) return FALSE;

		// Check for a valid asset layout set by the Layouts screen of the asset
		$layout = $this->_hasAssetLayout($asset, $screen);

		// If a layout found, return, otherwise continue
		if ($layout) return TRUE;

		$type_code = $asset->type();
		$layouts = $this->attr('layouts');
		if (!isset($layouts[$type_code][$screen])) {
			// if there is no screen defined for this type_code, check to see if there is
			// a layout defined for its ancestor
			if (NULL === ($type_code = $this->getAncestorLayoutTypeCode($type_code))) {
				return FALSE;
			}
			if (!isset($layouts[$type_code][$screen])) {
				return FALSE;
			}
		}
		return TRUE;

	}//end hasLayout()


	/**
	* Returns a Layout Asset for a type_code and its screen (or NULL on error)
	*
	* OR, if no screen is specified, an array of layouts for all the defined screens for the type_code in the form:
	* <pre>
	*  Array(
	*     'screen_name1' => &$layout_asset1,
	*     'screen_name2' => &$layout_asset2,
	*  );
	* </pre>
	*
	* @param object	$asset		the asset for which the layout is wanted
	* @param string	$screen		the name of the screen for which the layout is defined
	*
	* @return object
	* @access public
	*/
	public function _getAssetLayout($asset, $screen='')
	{
		$layout_def = 'layout::'.$asset->type().'::'.$screen;
		$layouts = $asset->getLookupValues(TRUE, $layout_def);
		$url = preg_replace('|^https?://|', '', $asset->getURL());

		if (!empty($layouts)) {
			$layout = NULL;
			if (isset($layouts[$url][$layout_def]['value']) && !empty($layouts[$url][$layout_def]['value'])) {
					$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($layouts[$url][$layout_def]['value']);
			}//end if

			if (is_null($layout)) {
				return NULL;
			} else {
				return $layout;
			}
		}//end if

		return NULL;

	}//end _getAssetLayout()


	/**
	* returns true if a layout is defined for a particular type_code and screen
	*
	* @param string	$asset		the asset to check
	* @param string	$screen		the screen of the type_code to check
	*
	* @return boolean
	* @access public
	*/
	public function _hasAssetLayout($asset, $screen='details')
	{
		$layout = $asset->getLookupValues(TRUE, 'layout::'.$asset->type().'::'.$screen);

		if (!empty($layout)) return TRUE;

		return FALSE;

	}//end _hasAssetLayout()


}//end class
?>

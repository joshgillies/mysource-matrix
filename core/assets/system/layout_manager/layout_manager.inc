<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: layout_manager.inc,v 1.1 2003/12/02 03:33:47 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Layout Manager
*
* Manages Layouts for type_codes for limbo mode
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.1 $ - 0.1
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
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	} // end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) return false;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this))  {
				$linkid = false;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName();


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return false;

	}//end canDelete()


	/**
	* Returns a Layout Asset for a type_code and its screen
	*
	* If no screen is specified, an array of layouts for all the defined screens for the type_code in the form:
	*
	* <PRE>
	*  Array(
	*     'screen_name1' => &$layout_asset1,
	*     'screen_name2' => &$layout_asset2,
	*  );
	* </PRE>
	*
	* @param string $type_code the type_code for which the layout is wanted
	* @param string $screen the name of the screen for which the layout is defined for
	*
	* @return &object Layout $layout (or an array of layouts if no screen is specified). NULL on error
	* @access public
	*/
	function &getLayout($type_code, $screen='')
	{
		$null = null; // for returning a reference
		if ($type_code == '') return $null;
		$layouts = $this->attr('layouts');
	
		// no layouts for this type_code
		if (!in_array($type_code, array_keys($layouts)))return $null;
		// no layouts defined for this type_code's screens
		if (empty($layouts[$type_code])) return $null;

		// if they have not specified a screen, return an array of references to all the screen layouts
		if ($screen == '') {
			$layout = Array();
			foreach ($layouts[$type_code] as $screen => $assetid) {
				$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, 'layout');
				if (is_null($asset)) continue;
				$layout[$screen] = &$asset;
			}
			if (empty($layout)) return $null;
		} else {
			if (!isset($layouts[$type_code][$screen])) return $null;
			$layout = &$GLOBALS['SQ_SYSTEM']->am->getAsset($layouts[$type_code][$screen], 'layout');
			if (is_null($layout)) return $null;
		}
		return $layout;
	
	}//end getLayout()


	/**
	* Returns TRUE if a layout is defined for a particualt type_code and screen
	*
	* @param string $type_code the type_code to check
	* @param string $screen the screen of the type_code to check
	*
	* @return boolean
	* @access public
	*/
	function hasLayout($type_code, $screen='details')
	{
		$layouts = $this->attr('layouts');
		return (isset($layouts[$type_code][$screen])) ? true : false;
	
	}//end hasLayout()


}//end class
?>

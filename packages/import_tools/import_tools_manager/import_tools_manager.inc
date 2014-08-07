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
* $Id: import_tools_manager.inc,v 1.8 2012/08/30 00:58:50 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Import_Tools_Manager
*
* Manages searching and relevance weights for attributes of asset types.
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage import_tools
*/
class Import_Tools_Manager extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Import_Tools_Manager($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	* @see Asset::create()
	*/
	function create(&$link)
	{
		require_once SQ_CORE_PACKAGE_PATH.'/system/system_asset_fns.inc';
		if (!system_asset_fns_create_pre_check($this)) {
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if ($linkid = parent::create($link)) {
			if (!system_asset_fns_create_cleanup($this)) {
				$linkid = FALSE;
			}
		}

		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$this->installConverters();
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		}

		return $linkid;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');

	}//end _getName()


	/**
	* Can this asset have its last significant link removed (putting it in the trash)?
	*
	* @return boolean
	* @access public
	*/
	function canDelete()
	{
		return FALSE;

	}//end canDelete()


	/**
	* returns FALSE always because we don't want to be able to clone a search manager
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$itm_links = parent::_getAllowedLinks();
		$itm_links[SQ_LINK_TYPE_1]['import_tool_converter'] = Array('card' => 'M', 'exclusive' => TRUE);

		return $itm_links;

	}//end _getAllowedLinks()


	/**
	* Checks for new converters
	*
	* @return boolean
	* @access private
	*/
	function installConverters()
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;

		$converter_types = $am->getTypeDescendants('import_tool_converter');
		$current_converters = $am->getLinks($this->id, SQ_LINK_TYPE_1, 'import_tool_converter', FALSE);

		$converter_types = array_flip($converter_types);

		foreach ($current_converters as $data) {
			$installed_type_code = $data['minor_type_code'];
			if (isset($converter_types[$installed_type_code])) {
				unset($converter_types[$installed_type_code]);
			}
		}

		foreach ($converter_types as $type_code => $id) {
			$this->installConverter($type_code);
		}

	}//end installConverters()


	/**
	* Checks for new converters
	*
	* @param string	$type_code	Type code of the converter to install
	*
	* @return boolean
	* @access private
	*/
	function installConverter($type_code)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$am->includeAsset($type_code);

		$import_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_1, 'is_exclusive' => 1);

		$new_converter = new $type_code;

		// Child Classes should have a default name set, so there is no need to set it here
		$new_converter->create($import_link);

	}//end installConverter()


}//end class

?>

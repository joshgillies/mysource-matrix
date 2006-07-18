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
* $Id: soap_wsdl_schema.inc,v 1.1 2006/07/18 06:41:56 bcaldwell Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'SOAP/Client.php';


/**
* SOAP_WSDL_Schema
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
register_implementation('soap_wsdl_schema', 'bridge');
class SOAP_WSDL_Schema extends Asset
{


	/* {{{ START ASSET FUNCTIONS */


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function SOAP_WSDL_Schema($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

		//bam($this->callWsdlOperation('doSpellingSuggestion', array('vpLh4/dQFHJEQ52GT2/zhJVcyTmTZKIC', 'andreui')));
		//bam($this->getWsdlOperationLinks());

	}//end constructor


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
	function setAttrValue($name, $value)
	{
		// might be some custom code for this attribute
		$set_attr_fn = 'set'.str_replace(' ', '', ucwords(str_replace('_', ' ', $name))).'AttrValue';
		if (method_exists($this, $set_attr_fn)) {
			return $this->$set_attr_fn($name, $value);
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrvalue()


	/* END ASSET FUNCTIONS }}} */

	/* {{{ START BRIDGE FUNCTIONS */


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$operation_links = $this->getWsdlOperationLinks();

		$asset_map_links = Array();
		foreach ($operation_links as $operation_link) {
			// re-arrage this bugger so that the asset map is happy
			$asset_map_link = Array(
								'url'				=> '',
								'path'				=> '',
								'num_kids'			=> 0,
								'accessible'		=> 1,

								'majorid'			=> $operation_link['majorid'],
								'minorid'			=> $operation_link['minorid'],
								'assetid'			=> $operation_link['minorid'],
								'minor_type_code'	=> $operation_link['minor_type_code'],
								'type_code'			=> $operation_link['minor_type_code'],
								'linkid'			=> $operation_link['linkid'],

								'name'				=> $operation_link['name'],
								'short_name'		=> $operation_link['name'],
								'sort_order'		=> $operation_link['name'],

								'link_type'			=> SQ_LINK_TYPE_1,
								'status'			=> $this->status,
							   );
			$asset_map_links[] = $asset_map_link;
		}
		return $asset_map_links;

	}//end getAssetMapLinks()


	/**
	* Returns a reference to the asset represented by the passed assetid
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return mixed object|NULL
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		// bah, we care not about the silly shadow asset id
		$id_parts = explode(':', $shadowid);
		if (!isset($id_parts[1])) {
			return $null = NULL;
		}
		$shadow_assetid = $id_parts[1];

		// we can use this to map parts from the shadow id to what they represent
		$format = Array('operation');

		$shadow_parts = Array();
		foreach (explode('/', $shadow_assetid) as $element_type => $element_name) {
			$shadow_parts[$format[$element_type]] = $element_name;
		}

		// extract the information we need to get the asset
		$element_type = end(array_keys($shadow_parts));
		$element_name = end($shadow_parts);

		$get_element_func = 'getWsdl'.ucwords($element_type).'Asset';
		if (method_exists($this, $get_element_func)) {
			return $this->$get_element_func($element_name);
		}

		$null = NULL;
		return $null;

	}//end getAsset()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	* @param boolean	$dependant			Whether we want dependant links, non-dependent or (NULL) don't care
	* @param boolean	$exclusive			Whether we want exclusive links, non-exclusive or (NULL) don't care
	*
	* @return array
	* @access public
	* @see Asset_Manager::getLinks()
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL, $dependant=NULL, $exclusive=NULL)
	{
		return Array();

	}//end getLinks()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'User', 'User_Group', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	* @see Asset_Manager::getParents()
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		return Array();

	}//end getParents()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* @param int			$assetid			the id of the asset to get its children for
	* @param string|array	$type_code			the type of asset that is linked
	*											(eg 'Page', 'File', etc)
	*											if an array returns link if matches any
	*											of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	* @param boolean		$dependant			if TRUE, results will be filtered to assets that are dependants.
	*											If FALSE, results will all not be dependants.
	*											If NULL, results will not be filtered
	* @param string			$sort_by			a field in the asset table you would like the results
	*											sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=TRUE, $dependant=NULL, $sort_by=NULL)
	{
		return Array();

	}//end getChildren()


	/* END START BRIDGE FUNCTIONS }}} */

	/* {{{ START SOAP_WSDL_SCHEMA FUNCTIONS */


	/**
	* Perform some extra actions when setting the value of the 'wsdl_uri' attribute
	*
	* This method will automatically parse and cache the file located at 'wsdl_uri' when
	* it's value is being set.
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	* @see saveAttributes()
	*/
	function setWsdlUriAttrValue($name, $value)
	{
		// use pear soap to parse the wsdl file
		$wsdl =& new SOAP_WSDL($value);
		if (isset($wsdl->fault)) {
			// todo: trigger error
			return FALSE;
		}
		return $this->setAttrValue('wsdl_cache', $wsdl) && parent::setAttrValue($name, $value);

	}// end setWsdlUriAttrValue()


	/**
	* Attempts to extract the specified information from the wsdl object
	*
	* @param string	$element_type	the type of element (i.e. service, binding)
	* @param string	$element_name	optionally the name of the element
	*
	* @access public
	* @return array
	*/
	function getWsdlElementDescription($element_type, $element_name='')
	{
		// we'll need to deal with the wsdl object
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return Array();

		// element type needs to make sense
		if (empty($element_type) || !isset($wsdl->$element_type)) {
			return Array();
		}
		$element_desc = $wsdl->$element_type;

		// attempt to return a specific element, if we can
		if (!empty($element_name)) {
			$element_desc = array_get_index($element_desc, $element_name);
			if (is_null($element_desc)) return Array();
		}
		return $element_desc;

	}//end getWsdlElementDescription()


	/**
	* This will actually call the specified operation using generated proxy code
	*
	* @param string	$operation	The name of the operation to call
	* @param array	$parameters	An array of parameters for the operation
	*
	* @access public
	* @return mixed
	*/
	function callWsdlOperation($operation, $parameters)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return NULL;

		$proxy = $wsdl->getProxy();
		if (!method_exists($proxy, $operation)) return NULL;

		return call_user_func_array(Array($proxy, $operation), $parameters);

	}//end callWsdlOperation()


	/**
	*/
	function getWsdlOperationList()
	{
		$port_list = $this->getWsdlElementDescription('portTypes');
		if (empty($port_list)) return Array();

		$operation_list = Array();
		foreach ($port_list as $port_name => $port_desc) {
			$operation_list = array_merge($operation_list, array_keys($port_desc));
		}
		return $operation_list;

	}//end getWsdlOperationList()


	/**
	*/
	function getWsdlOperationLinks()
	{
		$operation_list = $this->getWsdlOperationList();
		if (empty($operation_list)) return Array();

		$operation_links = Array();
		foreach ($operation_list as $operation_name) {
			// generate a simple shadow id
			$shadow_assetid = $this->id.':'.$operation_name;

			// build a link array for this operation
			$operation_link = Array(
								'linkid'			=> $shadow_assetid,

								'majorid'			=> $this->id,
								'minorid'			=> $shadow_assetid,
								'major_type_code'	=> 'soap_wsdl_schema',
								'minor_type_code'	=> 'soap_wsdl_operation',

								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> FALSE,
								'is_exclusive'		=> FALSE,

								'name'				=> $operation_name,
								'sort_order'		=> $operation_name,
							  );
			$operation_links[] = $operation_link;
		}
		return $operation_links;

	}//end getWsdlOperationLinks()


	/**
	*/
	function &getWsdlOperationAsset($operation_name)
	{
		if (empty($operation_name)) return $null = NULL;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_operation');

		$operation =& new SOAP_WSDL_Operation($operation_name);
		return $operation;

	}//end getWsdlOperationAsset()


	/* END SOAP_WSDL_SCHEMA FUNCTIONS }}} */


}//end class

?>
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
* $Id: soap_wsdl_schema.inc,v 1.4 2006/07/20 04:57:31 bcaldwell Exp $
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
* @version $Revision: 1.4 $
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
		$links = Array();

		$element_links = $this->getWsdlServiceLinks();
		foreach ($element_links as $element_link) {
			$link = Array();

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = count($this->getWsdlBindingLinks($element_link['minorid']));
			$link['accessible'] = 1;

			$link['majorid'] = $element_link['majorid'];
			$link['minorid'] = $element_link['minorid'];
			$link['assetid'] = $element_link['minorid'];
			$link['minor_type_code'] = $element_link['minor_type_code'];
			$link['type_code'] = $element_link['minor_type_code'];
			$link['linkid'] = $element_link['minorid'];

			// make name and short name the same
			$link['name'] = $this->getWsdlElementDescription($element_link['minorid'], 'service');
			$link['short_name'] = $this->getWsdlElementDescription($element_link['minorid'], 'service');
			$link['status'] = $this->status;

			$link['link_type'] = $element_link['link_type'];

			$link['sort_order'] = $element_link['sort_order'];

			$links[] = $link;
		}
		return $links;

	}//end getAssetMapLinks()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	/*function getAssetMapAssetInfo($assetid)
	{
		$asset_info = Array();

		$id_parts = explode(':', $assetid);
		if (empty($id_parts[1])) return $asset_info;

		$asset_info['assetid']		= $assetid;
		$asset_info['name']			= $assetid;
		$asset_info['short_name']	= $assetid;
		$asset_info['version']		= $this->version;
		$asset_info['status']		= $this->status;
		$asset_info['type_code']	= 'soap_wsdl_binding';
		$asset_info['num_kids']		= 1;
		$asset_info['accessible']	= 1;
		$asset_info['url']			= '';
		$asset_info['web_path']		= '';

		return $asset_info;

	}//end getAssetMapAssetInfo()*/


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
		$assetid_parts = $this->getWsdlElementDescription($shadowid);
		if (is_null($assetid_parts)) return $null = NULL;

		// extract the information we need to get the asset
		$element_type = end(array_keys($assetid_parts));
		$element_name = end($assetid_parts);

		$get_element_func = 'getWsdl'.ucwords($element_type).'Asset';
		log_dump($get_element_func);
		if (method_exists($this, $get_element_func)) {
			return $this->$get_element_func($shadowid, $element_name);
		}

		$null = NULL;
		return $null;

	}//end getAsset()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
		return Array();

	}//end getLink()


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
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not NULL)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{

	}//end getLinkByAsset()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the desired link
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{

	}//end getLinkById()


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
		return $this->getWsdlBindingLinks($assetid);

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
	function getWsdlElementDefinition($element_type, $element_name='')
	{
		// we'll need to deal with the wsdl object
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return Array();

		// element type needs to make sense
		if (empty($element_type) || !isset($wsdl->$element_type)) {
			return Array();
		}
		$element_def = $wsdl->$element_type;

		// attempt to return a specific element, if we can
		if (!empty($element_name)) {
			$element_def = array_get_index($element_def, $element_name);
			if (is_null($element_def)) return Array();
		}
		return $element_def;

	}//end getWsdlElementDefinition()


	/**
	*/
	function getWsdlElementDescription($element_assetid, $element_type='')
	{
		// bah, we care not about the silly shadow asset id
		$id_parts = explode(':', $element_assetid);
		if (!isset($id_parts[1])) {
			return $null = NULL;
		}
		$element_assetid = $id_parts[1];

		// we can use this to map parts from the shadow id to what they represent
		$format = Array('service', 'binding', 'port', 'operation');

		$element_parts = Array();
		foreach (explode('/', $element_assetid) as $type => $name) {
			$element_parts[$format[$type]] = $name;
		}

		if (!empty($element_type)) {
			return array_get_index($element_parts, $element_type);
		}
		return $element_parts;

	}//end getWsdlElementDescription()


	/**
	*/
	function getWsdlServiceLinks()
	{
		$service_def = $this->getWsdlElementDefinition('services');

		$service_links = Array();
		foreach (array_keys($service_def) as $service_name) {
			// this is the id of this service
			$service_assetid = implode(':', Array($this->id, $service_name));

			// build the link
			$service_links[] = Array(
								'linkid'			=> $service_assetid,

								'majorid'			=> $this->id,
								'minorid'			=> $service_assetid,
								'major_type_code'	=> 'soap_wsdl_schema',
								'minor_type_code'	=> 'soap_wsdl_service',

								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,

								'sort_order'		=> $service_assetid,
							   );

		}
		return $service_links;

	}//end getWsdlServiceLinks()


	/**
	*/
	function &getWsdlServiceAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_service');

		$service =& new SOAP_WSDL_Service($assetid);
		return $service;

	}//end getWsdlServiceAsset();


	/**
	*/
	function getWsdlBindingLinks($service_assetid)
	{
		$service_name = $this->getWsdlElementDescription($service_assetid, 'service');
		$service_def = $this->getWsdlElementDefinition('services', $service_name);

		// the ports for this service
		$ports = array_get_index($service_def, 'ports');
		if (is_null($ports)) return Array();

		$binding_links = Array();
		foreach ($ports as $port_name => $port_def) {
			$binding_assetid = implode('/', Array($service_assetid, array_get_index($port_def, 'binding')));

			// build the link
			$binding_link = Array(
								'linkid'			=> $binding_assetid,

								'majorid'			=> $service_assetid,
								'minorid'			=> $binding_assetid,
								'major_type_code'	=> 'soap_wsdl_service',
								'minor_type_code'	=> 'soap_wsdl_binding',

								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,

								'sort_order'		=> $binding_assetid,
							   );
			$binding_links[] = $binding_link;
		}
		return $binding_links;

	}//end getWsdlBindingLinks()


	/**
	*/
	function &getWsdlBindingAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_binding');

		$binding =& new SOAP_WSDL_Binding($assetid);
		return $binding;

	}//end getWsdlBindingAsset()


	/**
	*/
	function getWsdlPortLinks($binding_assetid)
	{
		$binding_name = $this->getWsdlElementDescription($binding_assetid, 'binding');
		$binding_def = $this->getWsdlElementDefinition('bindings', $binding_name);

		$port_name = array_get_index($binding_def, 'type');
		if (is_null($port_name)) return Array();

		$port_assetid = implode('/', Array($binding_assetid, $port_name));

		// build the link
		$port_link = Array(
							'linkid'			=> $port_assetid,

							'majorid'			=> $binding_assetid,
							'minorid'			=> $port_assetid,
							'major_type_code'	=> 'soap_wsdl_binding',
							'minor_type_code'	=> 'soap_wsdl_port_type',

							'link_type'			=> SQ_LINK_TYPE_2,
							'is_dependant'		=> TRUE,
							'is_exclusive'		=> FALSE,

							'sort_order'		=> $port_assetid,
						   );

		return Array($port_link);

	}//end getWsdlPortLinks()


	/**
	*/
	function &getWsdlPortAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_port_type');

		$port_type =& new SOAP_WSDL_Port_Type($assetid);
		return $port_type;

	}//end getWsdlPortAsset()


	/**
	*/
	function getWsdlOperationLinks($port_assetid)
	{
		$port_name = $this->getWsdlElementDescription($port_assetid, 'port');
		$port_def = $this->getWsdlElementDefinition('portTypes', $port_name);

		$operation_links = Array();
		foreach (array_keys($port_def) as $operation_name) {
			// generate the id of the operation
			$operation_assetid = implode('/', Array($port_assetid, $operation_name));

			// the link
			$operation_link = Array(
								'linkid'			=> $operation_assetid,

								'majorid'			=> $port_assetid,
								'minorid'			=> $operation_assetid,
								'major_type_code'	=> 'soap_wsdl_port_type',
								'minor_type_code'	=> 'soap_wsdl_operation',

								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,

								'sort_order'		=> $operation_assetid,
							   );
			$operation_links[] = $operation_link;
		}
		return $operation_links;

	}//end getWsdlOperationLinks()


	/**
	*/
	function &getWsdlOperationAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_operation');

		$operation =& new SOAP_WSDL_Operation($assetid);
		return $operation;

	}//end getWsdlOperationAsset()


	/* END SOAP_WSDL_SCHEMA FUNCTIONS }}} */


}//end class

?>
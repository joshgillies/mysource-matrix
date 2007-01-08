<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: soap_wsdl_schema.inc,v 1.16 2007/01/08 04:01:42 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once 'SOAP/Client.php';

define('SQ_SOAP_WSDL_SERVICE', 'service');
define('SQ_SOAP_WSDL_BINDING', 'binding');
define('SQ_SOAP_WSDL_PORT_TYPE', 'port');
define('SQ_SOAP_WSDL_OPERATION', 'operation');
define('SQ_SOAP_WSDL_MESSAGE', 'message');
define('SQ_SOAP_WSDL_ELEMENT', 'element');
define('SQ_SOAP_WSDL_TYPE', 'type');
define('SQ_SOAP_WSDL_NAMESPACE', 'namespace');
define('SQ_SOAP_WSDL_NAME', 'name');

define('SQ_SOAP_WSDL_INPUT_MESSAGE', 'input');
define('SQ_SOAP_WSDL_OUTPUT_MESSAGE', 'output');
define('SQ_SOAP_WSDL_FAULT_MESSAGE', 'fault');

define('SQ_SOAP_WSDL_COMPONENT_TYPE', 'asset');

/**
* SOAP_WSDL_Schema
*
* Purpose
*
*
* @author  Ben Caldwell <bcaldwell@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
register_implementation('soap_wsdl_schema', 'bridge');
class SOAP_WSDL_Schema extends Asset
{


	/**
	* The allowed structure of the shadow asset ids for this bridge
	*
	* DO NOT change the order of the elements in this array, it reflects
	* the native hierarchy of the WSDL file and is represented in the id
	* of our shadow assets.
	*
	* @var array
	* @access public
	*/
	var $assetid_format = Array(
							SQ_SOAP_WSDL_SERVICE,
							SQ_SOAP_WSDL_BINDING,
							SQ_SOAP_WSDL_PORT_TYPE,
							SQ_SOAP_WSDL_OPERATION,
							SQ_SOAP_WSDL_MESSAGE,
						  );


	/**
	* The different types of message that we can handle in a WSDL
	*
	* @var array
	* @access public
	*/
	var $message_types = Array(
							SQ_SOAP_WSDL_INPUT_MESSAGE,
							SQ_SOAP_WSDL_OUTPUT_MESSAGE,
							SQ_SOAP_WSDL_FAULT_MESSAGE,
						 );


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


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		$service_links = $this->getServiceLinks();
		return $this->_getAssetMapLinks($service_links);

	}//end getAssetMapLinks()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		$asset_info = Array();

		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
		if (is_null($asset)) return $asset_info;

		$asset_info['assetid']		= $asset->id;
		$asset_info['name']			= $asset->name;
		$asset_info['short_name']	= $asset->short_name;
		$asset_info['version']		= $this->version;
		$asset_info['status']		= $this->status;
		$asset_info['type_code']	= get_class($asset); // todo: find another way to do this
		$asset_info['num_kids']		= 1;
		$asset_info['accessible']	= 1;
		$asset_info['url']			= '';
		$asset_info['web_path']		= '';

		return $asset_info;

	}//end getAssetMapAssetInfo()


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
		$component_type = $this->getComponentType($shadowid);

		$get_component_func = '_get'.ucwords($component_type).'Asset';
		if (method_exists($this, $get_component_func)) {
			return $this->$get_component_func($shadowid);
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
		return Array();

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
		return Array();

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
		return Array();

	}//end getChildren()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		$name = parent::_getName($short_name);
		$wsdl = $this->attr('wsdl_cache');
		if (!empty($wsdl)) $name = basename($wsdl->uri);

		return $name;

	}//end _getName()


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
			trigger_error($wsdl->fault->message, E_USER_NOTICE);
			return FALSE;
		}
		return $this->setAttrValue('wsdl_cache', $wsdl) && parent::setAttrValue($name, $value);

	}// end setWsdlUriAttrValue()


	/**
	*/
	function _getAssetMapLinks($links=Array())
	{
		if (empty($links)) return Array();

		$asset_map_links = Array();
		foreach ($links as $link) {

			$link_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid']);

			$asset_map_links[] = Array(
									'url'				=> '',
									'path'				=> '',
									'num_kids'			=> 1, // todo: don't always assume kids
									'accessible'		=> 1,

									'majorid'			=> $link['majorid'],
									'minorid'			=> $link['minorid'],
									'assetid'			=> $link['minorid'],
									'minor_type_code'	=> $link['minor_type_code'],
									'type_code'			=> $link['minor_type_code'],
									'linkid'			=> $link['linkid'],

									'name'				=> $link_asset->_getName(),
									'short_name'		=> $link_asset->_getName(TRUE),

									'status'			=> $this->status,
									'sort_order'		=> $link['sort_order'],
									'link_type'			=> $link['link_type'],
								  );
			error_log($link['minorid']);
		}
		return $asset_map_links;

	}//end _getAssetMapLinks()


	/**
	* Calls the operation using the PEAR SOAP package.
	*
	* @param string	$port_name		the name of the port this operation is on
	* @param string $opeartion_name	the name of the operation to call
	* @param array	$parameters		the parameters for this operation
	*/
	function callOperation($port_name, $operation_name, $parameters)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return NULL;

		// todo: repair multi-proxy support. specify port name as first parameter
		$proxy = $wsdl->getProxy();

		// todo: change what we return here (it should not be an element in the result)
		$result = call_user_func_array(array(&$proxy, $operation_name), $parameters);
		return $result;

	}//end callOperation()


	/**
	* Returns the asset id of the shadow asset with the specified type
	*
	* @param string	$assetid	the full asset id
	* @param string	$type		the type of asset you want the id for
	*
	* @return string
	* @access public
	*/
	function getComponent($assetid, $type='')
	{
		$components = $this->getComponents($assetid);

		if (empty($type)) $type = $this->getComponentType($assetid);
		return array_get_index($components, $type);

	}//end getComponent()


	/**
	* Breaks apart a shadow asset id and rebuilds shadow asset ids for it's parents
	*
	* @param string	$assetid	the full asset id
	*
	* @access public
	* @return array
	*/
	function getComponents($assetid)
	{
		$id_parts = explode(':', $assetid);
		if (!isset($id_parts[1])) {
			return $null = NULL;
		}
		$bridge_assetid = $id_parts[0];
		$component_path = $id_parts[1];

		$component_parts = $this->_getComponents($component_path);

		$components = Array();
		foreach (array_reverse(array_keys($component_parts)) as $type) {
			$components[$type] = $bridge_assetid.':'.implode('/', $component_parts);
			array_pop($component_parts);
		}
		return $components;

	}//end getComponents()


	/**
	* Given a string, will break it down into the names of each individual component
	*
	* @param string	$component_path	the string that defines the component, no bridge id
	*
	* @access private
	* @return array
	*/
	function _getComponents($component_path)
	{
		$component_parts = Array();
		foreach (explode(',', $component_path) as $component_ref) {
			$id_parts = explode('=', $component_ref);

			$component_id = array_get_index($id_parts, 1);
			if (is_null($component_id)) continue;

			$component_type = array_get_index($id_parts, 0);
			$component_parts[$component_type] = $component_id;
		}
		return $component_parts;

	}//end _getComponents()


	/**
	* Returns the WSDL defined name of the component of the specified type based on assetid
	*
	* @param string	$assetid	the full asset id of the component
	* @param string	$type		the type of component you want the name of
	*
	* @access public
	* @return string
	*/
	function getComponentName($assetid, $type='')
	{
		if (empty($type)) $type = $this->getComponentType($assetid);

		$id_parts = explode(':', $assetid);
		if (!isset($id_parts[1])) {
			return '';
		}
		$component_path = $id_parts[1];

		$component_parts = $this->_getComponents($component_path);
		return array_get_index($component_parts, $type);

	}//end getComponentName()


	/**
	* Given an asset id, returns the type of asset
	*
	* @param string	$assetid	the full id of the asset
	*
	* @access public
	* @return string
	*/
	function getComponentType($assetid)
	{
		$id_parts = explode(':', $assetid);
		if (!isset($id_parts[1])) {
			return '';
		}
		$component_path = $id_parts[1];

		$component_parts = $this->_getComponents($component_path);
		return array_get_index($component_parts, SQ_SOAP_WSDL_COMPONENT_TYPE);

	}//end getComponentType()


	/**
	* Given an asset id, and the desired type of component, returns the wsdl definition
	*
	* @param string	$assetid	the full id of the component
	* @param string	$type		the type of component you want the definition of
	*
	* @access public
	* @return array
	*/
	function getComponentWsdl($assetid, $type='')
	{
		if (empty($type)) $type = $this->getComponentType($assetid);

		$component_wsdl_fn = '_get'.ucwords($type).'Wsdl';
		if (method_exists($this, $component_wsdl_fn)) {
			return $this->$component_wsdl_fn($assetid);
		}
		return Array();

	}//end getComponentWsdl()


	/**
	*/
	function getComponentAssetId($id_parts, $type='')
	{
		$component_assetid = Array();
		foreach ($id_parts as $component_type => $component_id) {
			$component_assetid[] = $component_type.'='.$component_id;
		}
		$component_assetid[] = SQ_SOAP_WSDL_COMPONENT_TYPE.'='.$type;

		return $this->id.':'.implode(',', $component_assetid);

	}//end getComponentAssetId()


	/**
	* Returns a reference to the asset of the type specified
	*
	* @param string	$assetid	the full asset id
	* @param string	$type		the type of asset you want a reference to
	*
	* @access public
	* @return string
	*/
	function &getComponentAsset($assetid, $type='')
	{
		$component_assetid = $this->getComponent($assetid, $type);
		if (is_null($component_assetid)) return $null = NULL;

		$component_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($component_assetid);
		return $component_asset;

	}//end getComponentAsset()


	/**
	* Returns a reference to the service asset with the supplied id
	*
	* @param string	$assetid	the full asset id of the service
	*
	* @access private
	* @return &object
	* @see &getAsset()
	*/
	function &_getServiceAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_service');

		$service =& new SOAP_WSDL_Service($assetid);
		return $service;

	}//end _getServiceAsset()


	/**
	*/
	function getServiceLinks()
	{
		// we want the wsdl for all services, so we do not supply an assetid
		$service_def = $this->getComponentWsdl(NULL, SQ_SOAP_WSDL_SERVICE);

		$service_links = Array();
		foreach (array_keys($service_def) as $service_name) {
			// this is the id of this service
			$id_parts = Array(SQ_SOAP_WSDL_SERVICE => $service_name);
			$service_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_SERVICE);

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

	}//end getServiceLinks()


	/**
	*/
	function _getServiceWsdl($assetid)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return NULL;

		$service_wsdl = $wsdl->services;
		if (!empty($assetid)) {
			$service_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_SERVICE);
			$service_wsdl = array_get_index($service_wsdl, $service_name);
		}
		return $service_wsdl;

	}//end _getServiceWsdl()


	/**
	* Returns a reference to the binding asset with the supplied id
	*
	* @param string	$assetid	the full id of the binding asset
	*
	* @access private
	* @return &object
	* @see &getAsset()
	*/
	function &_getBindingAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_binding');

		$binding =& new SOAP_WSDL_Binding($assetid);
		return $binding;

	}//end _getBindingAsset()


	/**
	*/
	function getBindingLinks($service_assetid)
	{
		$service_def = $this->getComponentWsdl($service_assetid, SQ_SOAP_WSDL_SERVICE);

		// the ports for this service
		$ports = array_get_index($service_def, 'ports');
		if (is_null($ports)) return Array();

		$binding_links = Array();
		foreach ($ports as $port_name => $port_def) {
			// the id of this binding
			$id_parts = Array(
							SQ_SOAP_WSDL_SERVICE => $this->getComponentName($service_assetid, SQ_SOAP_WSDL_SERVICE),
							SQ_SOAP_WSDL_BINDING => array_get_index($port_def, 'binding'),
						);
			$binding_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_BINDING);

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

	}//end getBindingLinks()


	/**
	*/
	function _getBindingWsdl($assetid)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return NULL;

		$binding_wsdl = $wsdl->bindings;
		if (!empty($assetid)) {
			$binding_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_BINDING);
			$binding_wsdl = array_get_index($binding_wsdl, $binding_name);
		}
		return $binding_wsdl;

	}//end _getBindingWsdl()


	/**
	* Returns a reference to the port asset with the supplied id
	*
	* @param string	$asestid	the full asset id of the port
	*
	* @access private
	* @return &object
	* @see &getAsset()
	*/
	function &_getPortAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_port_type');

		$port_type =& new SOAP_WSDL_Port_Type($assetid);
		return $port_type;

	}//end _getWsdlAsset()


	/**
	*/
	function getPortLinks($binding_assetid)
	{
		$binding_def = $this->getComponentWsdl($binding_assetid, SQ_SOAP_WSDL_BINDING);

		$port_name = array_get_index($binding_def, 'type');
		if (is_null($port_name)) return Array();

		// the id of this port type
		$id_parts = Array(
						SQ_SOAP_WSDL_SERVICE => $this->getComponentName($binding_assetid, SQ_SOAP_WSDL_SERVICE),
						SQ_SOAP_WSDL_BINDING => $this->getComponentName($binding_assetid, SQ_SOAP_WSDL_BINDING),
						SQ_SOAP_WSDL_PORT_TYPE => $port_name,
					);
		$port_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_PORT_TYPE);

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

	}//end getPortLinks()


	/**
	*/
	function _getPortWsdl($assetid)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return NULL;

		$port_wsdl = $wsdl->portTypes;
		if (!empty($assetid)) {
			$port_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_PORT_TYPE);
			$port_wsdl = array_get_index($port_wsdl, $port_name);
		}
		return $port_wsdl;

	}//end _getPortWsdl()


	/**
	* Returns a reference to the operation asset with the suppiled id
	*
	* @param string	$assetid	the full asset id of the operation
	*
	* @access private
	* @return &object
	* @see &getAsset()
	*/
	function &_getOperationAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_operation');

		$operation =& new SOAP_WSDL_Operation($assetid);
		return $operation;

	}//end _getOperationAsset()


	/**
	*/
	function getOperationLinks($port_assetid)
	{
		$port_def = $this->getComponentWsdl($port_assetid);

		$operation_links = Array();
		foreach (array_keys($port_def) as $operation_name) {
			// the id of this port type
			$id_parts = Array(
							SQ_SOAP_WSDL_SERVICE	=> $this->getComponentName($port_assetid, SQ_SOAP_WSDL_SERVICE),
							SQ_SOAP_WSDL_BINDING	=> $this->getComponentName($port_assetid, SQ_SOAP_WSDL_BINDING),
							SQ_SOAP_WSDL_PORT_TYPE	=> $this->getComponentName($port_assetid, SQ_SOAP_WSDL_PORT_TYPE),
							SQ_SOAP_WSDL_OPERATION	=> $operation_name,
						);
			$operation_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_OPERATION);

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

	}//end getOperationLinks()


	/**
	*/
	function _getOperationWsdl($assetid)
	{
		$operation_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_OPERATION);

		$port_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_PORT_TYPE);
		$port_wsdl = $this->getComponentWsdl($assetid, SQ_SOAP_WSDL_PORT_TYPE);

		$operation_desc = array_get_index($port_wsdl, $operation_name);
		return $operation_desc;

	}//end _getOperationWsdl()


	/**
	*/
	function &_getMessageAsset($assetid)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('soap_wsdl_message');

		$message_name = $this->getComponentName($assetid);
		$message_types = $this->message_types;

		$class_name = 'SOAP_WSDL_Message';

		$operation_wsdl = $this->getComponentWsdl($assetid, SQ_SOAP_WSDL_OPERATION);
		foreach ($operation_wsdl as $message_type => $message_wsdl) {
			if (!in_array($message_type, $message_types)) continue;

			$name = array_get_index($message_wsdl, 'message');
			if ($name == $message_name) {
				$class_name = 'SOAP_WSDL_Message_'.ucwords($message_type);
			}
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset(strtolower($class_name));

		$message =& new $class_name($assetid);
		return $message;

	}//end _getMessageAsset()


	/**
	*/
	function getMessageLinks($operation_assetid)
	{
		$operation_wsdl = $this->getComponentWsdl($operation_assetid);

		$message_types = $this->message_types;

		$message_links = Array();
		foreach ($operation_wsdl as $message_type => $message_wsdl) {
			// check to see if this is actually a message
			if (!in_array($message_type, $message_types)) continue;
			$message_name = array_get_index($message_wsdl, 'message');

			// the id of this port type
			$id_parts = Array(
					SQ_SOAP_WSDL_SERVICE => $this->getComponentName($operation_assetid, SQ_SOAP_WSDL_SERVICE),
					SQ_SOAP_WSDL_BINDING => $this->getComponentName($operation_assetid, SQ_SOAP_WSDL_BINDING),
					SQ_SOAP_WSDL_PORT_TYPE => $this->getComponentName($operation_assetid, SQ_SOAP_WSDL_PORT_TYPE),
					SQ_SOAP_WSDL_OPERATION => $this->getComponentName($operation_assetid, SQ_SOAP_WSDL_OPERATION),
					SQ_SOAP_WSDL_MESSAGE => $message_name,
				);
			$message_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_MESSAGE);

			// the type of the message asset
			$message_type_code = 'soap_wsdl_message_'.strtolower($message_type);

			// the link
			$message_link = Array(
								'linkid'			=> $message_assetid,

								'majorid'			=> $operation_assetid,
								'minorid'			=> $message_assetid,
								'major_type_code'	=> 'soap_wsdl_operation',
								'minor_type_code'	=> $message_type_code,

								'link_type'			=> SQ_LINK_TYPE_2,
								'is_dependant'		=> TRUE,
								'is_exclusive'		=> FALSE,

								'sort_order'		=> $message_assetid,
							   );
			$message_links[] = $message_link;
		}
		return $message_links;

	}//end getMessageLinks()


	/**
	*/
	function _getMessageWsdl($assetid)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return Array();

		$message_wsdl = $wsdl->messages;
		if (!empty($wsdl)) {
			$message_name = $this->getComponentName($assetid, SQ_SOAP_WSDL_MESSAGE);
			$message_wsdl = array_get_index($message_wsdl, $message_name);
		}
		return $message_wsdl;

	}//end _getMessageWsdl()


	/**
	*/
	function &_getTypeAsset($assetid)
	{
		$type_classname = 'SOAP_WSDL_Type';

		$wsdl = $this->getComponentWsdl($assetid);

		$type = array_get_index($wsdl, 'type');
		if (!is_null($type) && $GLOBALS['SQ_SYSTEM']->am->installed(strtolower($type_classname.'_'.$type))) {
			$type_classname = $type_classname.'_'.ucwords($type);
		}
		$GLOBALS['SQ_SYSTEM']->am->includeAsset(strtolower($type_classname));

		$type =& new $type_classname($assetid);
		return $type;

	}//end _getElementAsest()


	/**
	*/
	function getTypeLinks($component_assetid)
	{
		$component_wsdl = $this->getComponentWsdl($component_assetid);

		$component_elements = array_get_index($component_wsdl, 'elements');
		if (!is_null($component_elements)) $component_wsdl = $component_elements;

		$type_links = Array();
		foreach ($component_wsdl as $param_name => $param_wsdl) {

			$id_parts = Array(
							SQ_SOAP_WSDL_NAME		=> array_get_index($param_wsdl, 'name'),
							SQ_SOAP_WSDL_TYPE		=> array_get_index($param_wsdl, 'type'),
							//SQ_SOAP_WSDL_ELEMENT	=> array_get_index($param_wsdl, 'element'),
							SQ_SOAP_WSDL_NAMESPACE	=> array_get_index($param_wsdl, 'namespace'),
						);

			$type_assetid = $this->getComponentAssetId($id_parts, SQ_SOAP_WSDL_TYPE);

			// the link
			$type_link = Array(
							'linkid'			=> $type_assetid,

							'majorid'			=> $component_assetid,
							'minorid'			=> $type_assetid,
							'major_type_code'	=> 'soap_wsdl_message', // todo: not always a message
							'minor_type_code'	=> 'soap_wsdl_type_struct',

							'link_type'			=> SQ_LINK_TYPE_2,
							'is_dependant'		=> TRUE,
							'is_exclusive'		=> FALSE,

							'sort_order'		=> $type_assetid,
						 );
			$type_links[] = $type_link;
		}
		return $type_links;

	}//end getTypeLinks()


	/**
	*/
	function _getTypeWsdl($assetid)
	{
		$wsdl = $this->attr('wsdl_cache');
		if (empty($wsdl)) return Array();

		$type_name	= $this->getComponentName($assetid, SQ_SOAP_WSDL_TYPE);
		$type_ns	= $this->getComponentName($assetid, SQ_SOAP_WSDL_NAMESPACE);

		$ns = array_get_index($wsdl->complexTypes, $type_ns);
		if (is_null($ns) || empty($ns)) return Array();

		$type_wsdl = array_get_index($ns, $type_name);
		if (is_null($type_wsdl)) return Array();

		return $type_wsdl;

	}//end _getTypeWsdl()


}//end class

?>
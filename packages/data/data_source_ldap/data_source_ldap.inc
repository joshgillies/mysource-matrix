<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ldap.inc,v 1.1 2011/03/23 07:05:42 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';


/**
* Data_Source_Ldap
*
* Purpose
* 	Get the LDAP entries from the given LDAP directory based on the given
* 	search filter query string and populate the LDAP entries as shadow assets
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Ldap extends Data_Source
{
	
	/*
	* LDAP bridge asset
	*
	* @var object
	*/
	var $ldap_bridge;
	

	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{	
		$this->ldap_bridge = NULL;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of the result of LDAP search query execution
	*
	* @return array
	* @access public
	*/
	function getResultSet()
	{
		$query_string = isset($this->_tmp['search_filter_string']) ? $this->_tmp['search_filter_string'] : $this->getSearchFilterString();
		if (empty($query_string)) {
			return Array();
		}		
		
		$hash = md5($query_string);
		// Check the local cache
		if (!isset($this->_tmp[$hash])) {
		
			// Check the system cache
			$result = $this->getCachedResultSet($hash);
			if ($result !== FALSE) {
				$this->_tmp[$hash] = $result;
			} else {
				// Not cached at all, get it from LDAP bridge
				$this->_tmp[$hash] = $this->_executeQuery($query_string);
				$this->cacheResultSet($this->_tmp[$hash], $hash);
			}
		}

		return $this->_tmp[$hash];
		
	}//end getResultSet()
	

	/**
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @param boolean $reload_ldap_bridge
	*
	* @return object
	* @access public
	*/
	function connectToLdap($reload_ldap_bridge=FALSE)
	{	
		if (is_null($this->ldap_bridge) || $reload_ldap_bridge) {
			// If using an existing LDAP bridge
			$ldap_bridge_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'ldap_bridge', FALSE, 'ldap_bridge');		
			$ldap_bridge_assetid = isset($ldap_bridge_link['minorid']) ? $ldap_bridge_link['minorid'] : FALSE;
			$ldap_bridge_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ldap_bridge_assetid);
			
			if (!empty($ldap_bridge_info) && $ldap_bridge_info[$ldap_bridge_assetid]['type_code'] == 'ldap_bridge') {
				$this->ldap_bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($ldap_bridge_assetid);
			} else {
				//If using private and exclusive LDAP bridge
				$ldap_bridge_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_3, 'ldap_bridge', FALSE, 'ldap_bridge_exclusive');
				$ldap_bridge_assetid = isset($ldap_bridge_link['minorid']) ? $ldap_bridge_link['minorid'] : FALSE;
				$ldap_bridge_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ldap_bridge_assetid);
				
				if (!empty($ldap_bridge_info) && $ldap_bridge_info[$ldap_bridge_assetid]['type_code'] == 'ldap_bridge') {
					$this->ldap_bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($ldap_bridge_assetid);
				} else {
					
					$GLOBALS['SQ_SYSTEM']->am->includeAsset('ldap_bridge');					
					$ldap_bridge = new Ldap_Bridge();					
					$ldap_bridge->setAttrValue('name', 'ldap_ds_ldap_bridge');
					$create_link = Array(
									'asset'		=> $this,
									'value'		=> 'ldap_bridge_exclusive',
									'link_type'	=> SQ_LINK_TYPE_3,
								);
					
					if (!$ldap_bridge->create($create_link)) {
						trigger_localised_error('DATA0003', E_USER_WARNING);
						return NULL;
					}
					
					$this->ldap_bridge = $ldap_bridge;
				}
				
				if ($reload_ldap_bridge) {				
					$this->_syncLdapBridgeAttrs();
				}				
			}//end else
		}//end if
		
		if (is_null($this->ldap_bridge)) {
			return NULL;
		}
		
		return $this->ldap_bridge->connectToLdap();

	}//end connectToLdap()


	/**
	* Updates all the attributes of LDAP bridge using the attributes set on
	* this asset for LDAP connection
	*
	* @return boolean
	* @access private
	*/	
	function _syncLdapBridgeAttrs()
	{
		if (is_null($this->ldap_bridge)) {
			return FALSE;
		}
						
		$attrs = Array('ldap_options', 'aliases', 'host', 'port', 'root_dn', 'bind_dn', 'password');
		
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($this->ldap_bridge->id, 'attributes')) {
			trigger_localised_error('DATA0004', E_USER_WARNING, $this->ldap_bridge->id);			
			return FALSE;
		}
		
		foreach($attrs as $attr) {		
			$new_value = $this->attr($attr);			
			$this->ldap_bridge->setAttrValue($attr, $new_value);
		}
		
		$success = $this->ldap_bridge->saveAttributes();		
		$GLOBALS['SQ_SYSTEM']->am->releaseLock($this->ldap_bridge->id, 'attributes');
		
		return $success;
		
	}// end _syncLdapBridgeAttrs()

	
	/**
	* Function that executes the LDAP query using the given search filter
	*
	* @param string	$search_filter	LDAP search filter string
	*
	* @return array
	* @access public
	*/
	function _executeQuery($search_filter)
	{	
		$base_dn = $this->attr('root_dn');
		if (empty($base_dn)) {			
			return Array();
		}
		
		$result = Array();		
		
		$ldap = @$this->connectToLdap();		
		if (!is_null($ldap)) {			
			$sort_by = $this->attr('sort_by');
			
			if (!empty($search_filter)) {
				$search_result = $ldap->search($base_dn, $search_filter, $sort_by);
				
				if (!empty($search_result)) {
					$entries = $ldap->getEntries($search_result);
					$ldap->disconnect();
					
					if (!empty($entries)) {
						$shadow_assetids = Array();
						$ldap_results = Array();
						foreach($entries as $data) {
							if (!$data['count']) {
								continue;
							}						
							unset($data['count']);							
							unset($data['objectclass']['count']);
							foreach ($data['objectclass'] as $class) {
								$class = strtolower($class);
								if (isset($this->ldap_bridge->_group_types[$class]) || isset($this->ldap_bridge->_user_types[$class])) {
									$shadow_assetids[] = $this->id.':'.$data['dn'];
									$ldap_results[$this->id.':'.$data['dn']] = $data;
								}//end if
							}//end foreach
						}//end foreach
						
						$shadow_assetids = array_unique($shadow_assetids);
						$shadow_assets_info = $this->ldap_bridge->getAssetInfo($shadow_assetids);
						
						foreach($shadow_assets_info as $assetid => $asset_info) {							
							if (is_array($asset_info)) {
								$ds_record = Array();

								//Populate ldap info
								if (isset($ldap_results[$assetid])) {
									foreach($ldap_results[$assetid] as $ldap_key => $ldap_val) {
										if (!is_int($ldap_key)) {											
											$ds_record[$ldap_key] = is_array($ldap_val) ? (isset($ldap_val[0]) ? $ldap_val[0] : '') : $ldap_val;
										}
									}//end foreach
								}//end if
								
								//Populate shadow asset info as well
								foreach($asset_info as $data_key => $data_val) {
									$ds_record[$data_key] = $data_val;
								}//end foreach
								$result[] = $ds_record;								
							}//end if						
						}//end foreach						
						
					}//end if
					
				} else {
					$ldap->disconnect();
				}
			}//end if
			
			
			
		}//end if		
		
		return $result;

	}//end _executeQuery()


	/**
	* Returns an array of defined user-customisable LDAP connection options.
	*
	* The options below are customisable by the user. If the user chooses to
	* customise one of the below options, it's value will be set according
	* to the array below. If an option is not customised, the default is assumed.
	*
	* @return array
	* @access public
	*/
	function getConnectionOptions()
	{
		// an array of customisable ldap connection options to their customised value
		// defaults:
		// LDAP_OPT_PROTOCOL_VERSION default = 2
		// LDAP_OPT_REFERRALS = 1
		return Array(
				'LDAP_OPT_PROTOCOL_VERSION'	=> 3,
				'LDAP_OPT_REFERRALS'		=> 0,
			   );

	}//end getConnectionOptions()


	/**
	* Processes and returns the search filter string
	*
	* @return string
	* @access public
	*/
	function getSearchFilterString()
	{
		$search_filter_string = $this->attr('search_filter');
		$keyword_wrapper = '%%';
		$keyword_pattern = '('.$keyword_wrapper.'([a-zA-Z_\-0-9\.]+)'.$keyword_wrapper.')';

		// insert the dynamic parameters into the search filter string
		preg_match_all ('/'.$keyword_pattern.'/', $search_filter_string, $matches, PREG_PATTERN_ORDER);
		if (empty($matches[1])) {
			$this->_tmp['search_filter_string'] = $search_filter_string;
			return $this->_tmp['search_filter_string'];
		}

		$raw_keywords = $matches[1];
		$keywords = $matches[2];

		foreach ($keywords as $keyword) {
			$value = $this->getDynamicVariableValue($keyword);
			$replacements[] = str_replace('$', '\$', urldecode($value));
			$patterns[] = '/('.$keyword_wrapper.$keyword.$keyword_wrapper.')/';
		}

		$search_filter_string = preg_replace($patterns, $replacements, $search_filter_string);
		$this->_tmp['search_filter_string'] = trim($search_filter_string);
		
		return $this->_tmp['search_filter_string'];

	}//end getSearchFilterString()
	

	/**
	* Get a value of a dynamic variable
	*
	* @param string	$variable_name	Name of variable
	*
	* @return string
	* @access public
	*/
	function getDynamicVariableValue($variable_name)
	{
		$dynamic_vars = $this->attr('dynamic_vars');
		if (!isset($dynamic_vars[$variable_name])) return '';
		$parameter_map = $this->getAttribute('parameter_map');
		$value = $parameter_map->getParameterValue($variable_name);

		if (empty($value)) {
			return array_get_index($dynamic_vars, $variable_name, '');
		}

		return urlencode($value);

	}//end getDynamicVariableValue()
	
	
	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()	


}//end class

?>

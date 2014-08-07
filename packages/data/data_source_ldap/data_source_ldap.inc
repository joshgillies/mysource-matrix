<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd   |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ldap.inc,v 1.13 2012/12/07 01:15:28 cupreti Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/../include/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_FUDGE_PATH.'/ldap/ldap.inc';


/**
* Data_Source_Ldap
*
* Purpose
* 	Get the LDAP entries from the given LDAP directory based on the given
* 	search filter query string and populate the LDAP entries as shadow assets
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.13 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_Ldap extends Data_Source
{

	/*
	* Array having details to connect to an LDAP directory 
	*
	*	Array(
	*		'host',
	*		'port',
	*		'root_dn',
	*		'bind_dn',
	*		'password',
	*		'ldap_options',
	*		'aliases'
	*	)
	*
	* @var array
	*/	
	var $_ldap_dir;
	

	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{	
		$this->_ldap_dir = Array();
		
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
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);
		$hash = md5($query_string.$this->attr('attributes').';'.$this->attr('binary_attributes'));
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
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);
		return $this->_tmp[$hash];
		
	}//end getResultSet()
	

	/**
	* Load LDAP connection details from external LDAP Bridge
	* or from its own LDAP settings
	*
	* @return void
	* @access private
	*/
	function _loadLdapVars()
	{
		// If using an existing LDAP bridge
		$ldap_bridge_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'ldap_bridge', FALSE, 'ldap_bridge');		
		$ldap_bridge_assetid = isset($ldap_bridge_link['minorid']) ? $ldap_bridge_link['minorid'] : FALSE;
		if ($ldap_bridge_assetid) {
			$ldap_bridge_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($ldap_bridge_assetid);
		}
			
		if (!empty($ldap_bridge_info) && $ldap_bridge_info[$ldap_bridge_assetid]['type_code'] == 'ldap_bridge') {
			$ldap_bridge = $GLOBALS['SQ_SYSTEM']->am->getAsset($ldap_bridge_assetid);
			
			$this->_ldap_dir['host'] = $ldap_bridge->attr('host');
			$this->_ldap_dir['port'] = $ldap_bridge->attr('port');
			$this->_ldap_dir['root_dn'] = $ldap_bridge->attr('root_dn');
			$this->_ldap_dir['bind_dn'] = $ldap_bridge->attr('bind_dn');
			$this->_ldap_dir['password'] = $ldap_bridge->attr('password');
			$this->_ldap_dir['ldap_options'] = $ldap_bridge->attr('ldap_options');
			$this->_ldap_dir['aliases'] = $ldap_bridge->attr('aliases');
			
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($ldap_bridge_asset);			
		} else {
			$this->_ldap_dir['host'] = $this->attr('host');
			$this->_ldap_dir['port'] = $this->attr('port');
			$this->_ldap_dir['root_dn'] = $this->attr('root_dn');
			$this->_ldap_dir['bind_dn'] = $this->attr('bind_dn');
			$this->_ldap_dir['password'] = $this->attr('password');
			$this->_ldap_dir['ldap_options'] = $this->attr('ldap_options');
			$this->_ldap_dir['aliases'] = $this->attr('aliases');			
		}	
	
	}//end _loadLDAPVars()
	
	/**
	* Connect and bind to the LDAP directory that this asset bridges to
	*
	* @return object
	* @access public
	*/
	function connectToLdap()
	{	
		// Load the LDAP connection details
		$this->_loadLdapVars();
		
		if (empty($this->_ldap_dir['host'])) return NULL;

		$ldap = new Ldap();
		if (!$ldap->connect($this->_ldap_dir['host'], $this->_ldap_dir['port'])) {
			return NULL;
		}

		// Any non-default ldap connection settings set here
		foreach ($this->_ldap_dir['ldap_options'] as $option => $value) {
			ldap_set_option($ldap->ptr, constant($option), $value);
		}

		// Alias dereferencing setting here
		ldap_set_option($ldap->ptr, LDAP_OPT_DEREF, constant($this->_ldap_dir['aliases']));

		// If the bind DN is empty, anonymous access is assumed
		if (!empty($this->_ldap_dir['bind_dn'])) {
			if (!$ldap->bind($this->_ldap_dir['bind_dn'], $this->_ldap_dir['password'])) {
				return NULL;
			}
		}
		
		return $ldap;		
	
	}//end connectToLdap()


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
		$search_filter = trim($search_filter);
		if (empty($search_filter)) {
			return Array();
		}
		
		$result = Array();
		
		$ldap = @$this->connectToLdap();		
		if (!is_null($ldap)) {		
			$base_dn = $this->_ldap_dir['root_dn'];			
			$sort_by = trim($this->attr('sort_by'));
			
			$required_attributes = $this->parseStringValues($this->attr('attributes').','.$this->attr('binary_attributes'));
			
			$search_result = $ldap->search($base_dn, $search_filter, empty($sort_by) ? NULL : $sort_by, TRUE, TRUE, $required_attributes);
			
			if (!empty($search_result)) {
				$entries = $ldap->getEntries($search_result, $this->parseStringValues($this->attr('binary_attributes')));
				
				if (!empty($entries)) {					
					unset($entries['count']);
					foreach($entries as $data) {							
						unset($data['count']);
						$ds_record = Array();
						foreach($data as $ldap_key => $ldap_val) {								
							if (!is_int($ldap_key)) {
								if (is_array($ldap_val)) {
									unset($ldap_val['count']);										
									$ds_record[$ldap_key] = implode(";", $ldap_val);
								} else {
									$ds_record[$ldap_key] = $ldap_val;
								}
							}//end if
						}//end foreach
						
						$result[] = $ds_record;
						
					}//end foreach
				}//end if					
			}//end if
		}//end if
	
		if (!is_null($ldap)) {
			$ldap->disconnect();
		}
		
		return $result;

	}//end _executeQuery()


	/**
	 * Parse a delimiter-separated string and return the array containing the unique values.
	 * 
	 * @param string $str			The delimiter-separated string to parse
	 * @param string $delimiter		The delimiter that splits the values in the string
	 * 
	 * @return array				Return the array containing the unique values from the parsed string
	 * @access public
	 */
	function parseStringValues($str, $delimiter = ',')
	{
		$values = explode($delimiter, $str);
		$new_values = Array();
		foreach ($values as $value) {
			$val = trim($value);
			if (($val != '') && !in_array($val, $new_values)) {
				$new_values[] = $val;
			}
		}
		
		return $new_values;
		
	}//end parseStringValues()


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


	/**
	* Returns name of LDAP attributes defined as binary data
	*
	* @return array
	* @access public
	*/
	function getBinaryDataAttributes()
	{
		return $this->parseStringValues($this->attr('binary_attributes'));

	}//end getBinaryDataAttributes()


}//end class

?>

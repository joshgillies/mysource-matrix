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
* $Id: ldap.inc,v 1.17 2012/08/30 00:56:51 ewang Exp $
*
*/


/**
* LDAP
*
* Purpose
*     To provide an interface to an LDAP directory server for basic functionality
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.17 $
* @package Fudge
* @subpackage ldap
*/
class Ldap
{

	/**
	* The host used to connect to the LDAP server
	* Abreviated to 'h' throughout the member functions
	*
	* @var string
	*/
	var $host = '';

	/**
	* The port used to connect to the LDAP server
	* Abreviated to 'p' throughout the member functions
	*
	* @var int
	*/
	var $port = 0;

	/**
	* The current LDAP server pointer
	*
	* @var resource
	*/
	var $ptr = null;


	/**
	* Constructor
	*
	* @param string	$h		the host used to connect to the LDAP server
	* @param int	$p		the port used to connect to the LDAP server
	* @param string	$pass	the password used to connect to the LDAP server
	* @param string	$bdn	the DN used to bind to the LDAP server
	*
	*/
	function Ldap($h=null, $p=null, $pass=null, $bdn='')
	{
		// if a host has been specified, connect
		if (!is_null($h) && !empty($h)) {
			$this->connect($h, $p, $pass, $bdn);
			$this->bind($bdn, $pass);
		}

	}//end constructor


	/**
	* Connect to a certain LDAP server and bind to it
	*
	* @param string		$h	the host used to connect to the LDAP server
	* @param int		$p	the port used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function connect($h, $p=null)
	{
		// bail out safely if we have no ldap functions
		if (!extension_loaded('ldap')) {
			$this->ptr = null;
			return false;
		}

		if ($h != $this->host || $p != $this->port) {
			// close old connection
			if ($this->ptr) ldap_close($this->ptr);

			if (!($this->ptr = ldap_connect($h, $p))) {
				trigger_error('Unable to connect to LDAP server: '.$h.' on port '.$p, E_USER_WARNING);
				return false;
			}
			$this->host = $h;
			$this->port = $p;
		}
		return true;

	}//end connect()


	/**
	* Bind to a LDAP server
	*
	* @param string		$bdn	the DN used to bind to the LDAP server
	* @param string		$pass	the password used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function bind($bdn='', $pass=null)
	{
		if (!$this->ptr) return false;

		// bind to the server
		if (is_null($pass)) {
			// attempting an anonymous bind
			if (!($r = ldap_bind($this->ptr))) {
				trigger_error('Unable to Anonomously bind to LDAP server: '.$this->host, E_USER_WARNING);
				unset($this->ptr);
				return false;
			}
		} else {
			if (!($r = ldap_bind($this->ptr, $bdn, $pass))) {
				trigger_error('Unable to bind to LDAP server: '.$this->host.' BDN: '.$bdn.' Using Password: '.(($pass) ? 'YES' : 'NO'), E_USER_WARNING);
				$this->ptr = null;
				return false;
			}
		}

		return $r;

	}//end bind()


	/**
	* Disconnect from the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function disconnect()
	{
		if ($this->ptr) {
			return ldap_close($this->ptr);
		}
		return true;

	}//end disconnect()


	/**
	* Connect to a certain LDAP server and bind to it as a particular user
	*
	* @param string		$bdn	the root DN used to connect to the LDAP server
	* @param string		$pass	the password used to connect to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function connectAsUser($bdn, $pass)
	{
		$conn_result = $this->connect($this->host, $this->port);
		if (!$conn_result) return false;
		return $this->bind($bdn, $pass);

	}//end connectAsUser()


	/**
	* Search the server starting at $startdn using $filter
	*
	* @param string		$startdn		the DN to start searching at
	* @param string		$filter			search filter
	* @param boolean	$multi_level	search multiple levels
	* @param boolean	$multi_result	search for multiple entries
	* @param array		$attributes		array of the attributes we want to return for the returned objects
	*
	* @return resource (search result identifier) | zero (on error)
	* @access public
	*/
	function search($startdn, $filter, $sort_by=null, $multi_level=true, $multi_result=true, $attributes=Array())
	{
		if (!$this->ptr) return 0;

		if(!empty($attributes)) {
			if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter, $attributes);
			else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter, $attributes);
			else $sr = @ldap_read($this->ptr, $startdn, $filter, $attributes);
		} else {
			if ($multi_level && $multi_result) $sr = @ldap_search($this->ptr, $startdn, $filter);
			else if ($multi_result) $sr = @ldap_list($this->ptr, $startdn, $filter);
			else $sr = @ldap_read($this->ptr, $startdn, $filter);
		}

		if (!$sr) {
			log_dump(ldap_err2str(ldap_errno($this->ptr)));
			log_dump($filter);
			return 0;
		} else {
			if (!is_null($sort_by)) ldap_sort($this->ptr, $sr,  $sort_by);
			return $sr;
		}

	}//end search()


	/**
	* Get the entries in a given search result in a multi-dim array
	*
	* @param resource	$result	the search result identifier
	* @param array		$binary_attributes the attributes that need to be extracted under binary format
	*
	* @return array
	* @access public
	*/
	function getEntries($result, $binary_attributes = Array())
	{
		if (!($info = ldap_get_entries($this->ptr, $result))) {
			trigger_error('Unable to get entries for LDAP search result ['.$result.'] :'.ldap_error($this->ptr), E_USER_WARNING);
			return Array();
		} else {
			// ldap_get_entries() function can not handle binary data so we use ldap_get_values_len() to
			// put binary data to the binary attributes of $info
			if (!empty($binary_attributes) && ($info['count'] > 0)) {
				// There is no document confirming that the order of entries returned by ldap_get_entries()
				// is the same with that of ldap_first_entry()/ldap_next_entry() so we use the dn as identifier
				$entry_order = Array();
				for ($i = 0; $i < $info['count']; $i++) {
					$entry_order[$info[$i]['dn']] = $i;
				}
				// Set binary data for each entry
				for ($entry_identifier = ldap_first_entry($this->ptr, $result); $entry_identifier !== FALSE; $entry_identifier = ldap_next_entry($this->ptr, $entry_identifier)) {
					$dn = ldap_get_dn($this->ptr, $entry_identifier);
					if (($dn !== FALSE) && isset($entry_order[$dn])) {
						$entry_index = $entry_order[$dn];
						foreach ($binary_attributes as $attribute) {
							// ldap_get_entries() lowercases attribute indexes
							$lowercased_attribute = strtolower($attribute);
							// ldap_get_entries() can still get the first few bytes of the binary data, so we can speed this function
							// a bit by checking if there is a value of this attribute before extracting its actual binary data
							if (isset($info[$entry_index][$lowercased_attribute])) {
								// ldap_get_values_len() function throws a warning if there is no value for the attribute so we suppress the warning with @
								$binary_value = @ldap_get_values_len($this->ptr, $entry_identifier, $attribute);
								if ($binary_value !== FALSE) {
									$info[$entry_index][$lowercased_attribute] = $binary_value;
								}
							}
						}
					}
				}
			}
			
			return $info;
		}

	}//end getEntries()


	/**
	* Return the number of entries in a given search result
	*
	* @param resource	$result	the search result identifier
	*
	* @return int
	* @access public
	*/
	function getNumEntries($result)
	{
		if (!$result) return 0;
		return ldap_count_entries($this->ptr, $result);

	}//end getNumEntries()


	/**
	* Insert an entry into LDAP
	*
	* @param string	$dn			the level to add the entry at
	* @param array	$details	the details for new entry
	*
	* @return boolean
	* @access public
	*/
	function addEntry($dn, $details)
	{
		if (!ldap_add($this->ptr, $dn, $details)) {
			trigger_error('Unable to INSERT entry into LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
			return false;
		}
		return true;

	}//end addEntry()


	/**
	* Modify an LDAP entry
	*
	* @param string	$dn			the DN of the entry to modify
	* @param array	$details	the new details for entry
	*
	* @return boolean
	* @access public
	*/
	function modifyEntry($dn, $details)
	{
		if (!ldap_modify($this->ptr, $dn, $details)) {
			trigger_error('Unable to MODIFY entry in LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
		}
		return true;

	}//end modifyEntry()


	/**
	* Delete an entry from LDAP
	*
	* @param string	$dn		the DN of the entry to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteEntry($dn)
	{
		if (!ldap_delete($this->ptr, $dn)) {
			trigger_error('Unable to DELETE entry from LDAP at DN: ['.$dn.'] '.ldap_error($this->ptr), E_USER_WARNING);
		}
		return true;

	}//end deleteEntry()
	
	
	/** 
	* Escapes the given string according to RFC 2254 so that they can be safely used in LDAP filters.
	* 
	* @param string	$str	the string to escape
	* 
	* @return string
	* @access public
	*/
	function escape_filter_value($str){
		
	    $metaChars = array('\\', '(', ')', '*');
	    $quotedMetaChars = array();
	    foreach ($metaChars as $key => $value) $quotedMetaChars[$key] = '\\'.dechex(ord($value));
	    $str=str_replace($metaChars,$quotedMetaChars,$str); //replace them
	    return ($str);
	    
	}


}//end class

?>

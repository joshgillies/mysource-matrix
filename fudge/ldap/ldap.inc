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
* $Id: ldap.inc,v 1.3 2004/04/14 01:58:30 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* LDAP
*
* Purpose
*     To provide an interface to an LDAP directory server for basic functionality
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
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
	* @param string	$bdn	the root DN used to connect to the LDAP server
	* @param string	$rdn	the DN used to bind to the LDAP server
	*
	*/
	function Ldap($h=null, $p=null, $pass=null, $bdn='', $rdn='')
	{
		// if a host has been specified, connect
		if (!is_null($h) && !empty($h)) $this->connect($h, $p, $pass, $bdn, $rdn);

	}//end constructor



	/**
	* Connect to a certain LDAP server and bind to it
	*
	* @param string		$h		the host used to connect to the LDAP server
	* @param int		$p		the port used to connect to the LDAP server
	* @param string		$pass	the password used to connect to the LDAP server
	* @param string		$bdn	the root DN used to connect to the LDAP server
	* @param string		$rdn	the DN used to bind to the LDAP server
	*
	* @return boolean
	* @access public
	*/
	function connect($h, $p=null, $pass=null, $bdn='', $rdn='')
	{
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

		// bind to the server
		if (is_null($pass)) {
			// attempting an anonymous bind
			if (!($r = ldap_bind($this->ptr))) {
				trigger_error('Unable to Anonomously bind to LDAP server: '.$h, E_USER_WARNING);
				unset($this->ptr);
				return false;
			}
		} else {
			if (!($r = ldap_bind($this->ptr, $bdn, $pass))) {
				trigger_error('Unable to bind to LDAP server: '.$h.' BDN: '.$bindto.' Using Password: '.(($pass) ? 'YES' : 'NO'), E_USER_WARNING);
				$this->ptr = null;
				return false;
			}
		}

		$this->host   = $h;
		$this->port   = $p;

		return $r;

	}//end connect()


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
		$conn_result = $this->connect($this->host, $this->port, $pass, $bdn);
		return $conn_result;

	}//end connectAsUser()


	/**
	* Search the server starting at $startdn using $filter
	*
	* @param string		$startdn		the DN to start searching at
	* @param string		$filter			search filter
	* @param boolean	$multi_level	search multiple levels
	* @param boolean	$multi_result	search for multiple entries
	*
	* @return resource (search result identifier) | zero (on error)
	* @access public
	*/
	function search($startdn, $filter, $multi_level=true, $multi_result=true)
	{
		if (!$this->ptr) return 0;

		if ($multi_level && $multi_result) $sr = ldap_search($this->ptr, $startdn, $filter);
		else if ($multi_result) $sr = ldap_list($this->ptr, $startdn, $filter);
		else $sr = ldap_read($this->ptr, $startdn, $filter);

		if (!$sr) {
			trigger_error('Unable to SEARCH ['.$startdn.'] with Filter ['.$filter.']: '.ldap_error($this->ptr), E_USER_WARNING);
			return 0;
		} else {
			return $sr;
		}

	}//end search()


	/**
	* Get the entries in a given search result in a multi-dim array
	*
	* @param resource	$result	the search result identifier
	*
	* @return array
	* @access public
	*/
	function getEntries($result)
	{
		if (!($info = ldap_get_entries($this->ptr, $result))) {
			trigger_error('Unable to get entries for LDAP search result ['.$result.'] :'.ldap_error($this->ptr), E_USER_WARNING);
			return Array();
		} else {
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


}//end class

?>
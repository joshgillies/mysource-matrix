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
* $Id: site_network.inc,v 1.2 2005/02/28 03:11:55 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/folder/folder.inc';

/**
* Site_Network
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Site_Network extends Folder
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Site_Network($assetid=0)
	{
		$this->Folder($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Site Network's dont do anything, but Folder's do, so we need to override this method.
	*
	* @param array	&$link	information used to create the initial link
	*
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		return true;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{

		return Array(SQ_LINK_TYPE_1 => Array('site' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_TYPE_2 => Array('site' => Array('card' => 'M', 'exclusive' => false)),
					 SQ_LINK_NOTICE => Array('site' => Array('card' => 'M', 'exclusive' => false)),
					);

	}//end _getAllowedLinks()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* Site Network enforce the rule that a site can only belong to a signle network
	*
	* @param object Asset	$minor		the minor asset that we are linking to
	* @param string			$link_type	the type of link this is
	* @param int			$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed TRUE or error message string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		$result = parent::canCreateLink($minor, $link_type, $exclusive);
		if ($result !== true) return $result;

		if (is_a($minor, 'site')) {
			// site assets can only be in a single site network
			$network_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_ALL, 'site_network', false, 'minor');
			if (!empty($network_link)) return 'Sites can only be linked under one Site Network';
		}

		return true;

	}//end canCreateLink()


	/**
	* Get the primary URL for this network
	*
	* @access public
	* @return string
	*/
	function getPrimaryURL()
	{
		if (!isset($this->_tmp['primary_url'])) {
			$site_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'site', false, 'primary_site');
			if (empty($site_link)) return '';
	
			$sql = 'SELECT url, http, https
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_url
					';
			$where = 'assetid = '.$GLOBALS['SQ_SYSTEM']->db->quote($site_link['minorid']);
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
			$sql .= $where.'ORDER BY urlid LIMIT 1';
	
			$primary_url_data = $GLOBALS['SQ_SYSTEM']->db->getRow($sql);
			assert_valid_db_result($primary_url_data);
	
			if (empty($primary_url_data)) return '';
			$primary_url = $primary_url_data['url'];
	
			if ($primary_url['https'] == '1') $primary_url = 'https://'.$primary_url;
			else $primary_url = 'http://'.$primary_url;

			$this->_tmp['primary_url'] = $primary_url;
		}

		return $this->_tmp['primary_url'];

	}//end getPrimaryURL()
	
	
	/**
	* Unserialises a session file
	*
	* It breaks each of the session variables up and stores them as the key elements
	* of an array with their values as the values of the array elements.
	*
	* @param string	$session_file	the session file to unserialise
	*
	* @return array
	* @access public
	*/
	function unserialiseSessionFile($session_file)
	{
		$session_str = file_get_contents($session_file);
		// break the session at the word boundaries and the pipes
		$parts = preg_split('/\w+\|/', $session_str, -1, PREG_SPLIT_OFFSET_CAPTURE);
		$session_arr = Array();
		
		for ($i = 0; $i < count($parts); $i++) {
			if ($i == count($parts) - 1) continue;
			$offset = $parts[$i][1] + strlen($parts[$i][0]);
			$len = $parts[$i + 1][1] - 1 - $offset;
			
			$key = substr($session_str, $offset, $len);
			$session_arr[$key] = unserialize($parts[$i + 1][0]);
		}
		return $session_arr;
	
	}//end unserialiseSessionFile()
	
	
	/**
	* Serialises the specified session contents and writes it to the specified session file
	*
	* @param string	$session_file		the session file to write to
	* @param array	$session_contents	the contents to write to file
	*
	* @return boolean
	* @access public
	*/
	function serialiseSessionFile($session_file, $session_contents)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (!file_exists($session_file)) {
			trigger_error('Session file "'.$session_file.'" does not exist', E_USER_WARNING);
			return false;
		}
		
		if (!is_array($session_contents)) {
			trigger_error('Array expected for second argument, got '.gettype($session_contents), E_USER_WARNING);
			return false;
		}
	
		$session_str = '';
		foreach ($session_contents as $key => $val) {
			$session_str .= $key.'|'.serialize($val);
		}

		if (!string_to_file($session_str, $session_file)) {
			trigger_error('Could not write to session file "'.$session_file.'"', E_USER_WARNING);
			return false;
		}
		return true;
	
	}//end serialiseSessionFile()
	
	
	/**
	* Synchronises the specified primary session with the current (secondary) session
	*
	* The current session will be overridden with the primary session if the
	* primary's timestamp is the most recent, and vice versa. $_SESSION is
	* used for the current session, and will be written to at script completion.
	*
	* @param string	$pri_sessionid	the sessionid of the primary session
	*
	* @return boolean
	* @access public
	*/
	function syncSessionFile($pri_sessionid)
	{
		$pri_sess = $this->unserialiseSessionFile(SQ_CACHE_PATH.'/sess_'.$pri_sessionid);
	
		if (!is_array($pri_sess)) {
			// something is definately wrong
			trigger_error('Primary session is currupt', E_USER_ERROR);
			return false;
		}
	
		$pri_timestamp = array_get_index($pri_sess, 'SQ_SESSION_TIMESTAMP', -1);
		$sec_timestamp = array_get_index($_SESSION, 'SQ_SESSION_TIMESTAMP', -1);
		$pri_login_key = array_get_index($pri_sess, 'SQ_LOGIN_KEY', null);
		$sec_login_key = array_get_index($_SESSION, 'SQ_LOGIN_KEY', null);
	
		// if primary is younger
		if ($pri_timestamp > $sec_timestamp || $pri_timestamp == -1) {
			// copy primary to secondary
			$_SESSION = $pri_sess;
		} else {
			// copy secondary to primary
			$pri_sess = $_SESSION;
		}
	
		$now = time();
		$pri_sess['SQ_SESSION_TIMESTAMP'] = $now;
		$_SESSION['SQ_SESSION_TIMESTAMP'] = $now;
	
		// preserve login keys
		if (!is_null($pri_login_key)) $pri_sess['SQ_LOGIN_KEY'] = $pri_login_key;
		if (!is_null($sec_login_key)) $_SESSION['SQ_LOGIN_KEY'] = $sec_login_key;
	
		// save the sessionid of the primary so that we
		// know that we have run this script before. We won't have to
		// do anther browser refresh if we know this.
		$_SESSION['PRIMARY_SESSIONID'] = $pri_sessionid;
	
		// *** JEDI MIND TRICK *** you did not see us doing this... move along
		if (!$this->serialiseSessionFile(SQ_CACHE_PATH.'/sess_'.$pri_sessionid, $pri_sess)) {
			trigger_error('Could not write session back to primary session file', E_USER_ERROR);
			return false;
		}
		return true;
	
	}//end syncSessionFile()


}//end class

?>

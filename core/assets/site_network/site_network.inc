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
* $Id: site_network.inc,v 1.14 2012/08/30 01:09:09 ewang Exp $
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
* @version $Revision: 1.14 $
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
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		return TRUE;

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{

		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'site'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'site'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'site'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns TRUE if the passed minor asset can be linked to this asset, or a string with the error msg
	*
	* Site Network enforce the rule that a site can only belong to a signle network
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param string	$link_type	the type of link this is
	* @param int	$exclusive	the exclusive status of the link (gets passed to canLinkToType)
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canCreateLink(&$minor, $link_type, $exclusive)
	{
		$result = parent::canCreateLink($minor, $link_type, $exclusive);
		if ($result !== TRUE) return $result;

		if (($minor instanceof Site) && ($link_type != SQ_LINK_NOTICE)) {
			// site assets can only be in a single site network
			$network_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'site_network', FALSE, 'minor');
			if (!empty($network_link)) {
				return translate('Sites can only be linked under one Site Network');

			}
		}

		return TRUE;

	}//end canCreateLink()


	/**
	* Returns TRUE if the passed minor asset can be move-linked to this asset, or a string with the error msg
	*
	* Differs from canCreateLink() in that if we are being moved from a site network,
	* we have to allow one network link, being the asset we are being moved from
	*
	* @param object	&$minor		the minor asset that we are linking to
	* @param object	&$old_major	the major asset that we are being moved from
	* @param string	$link_type	the type of link this is
	*
	* @return mixed boolean|string
	* @access public
	*/
	function canMoveLink(&$minor, &$old_major, $link_type)
	{
		if (($minor instanceof Site) && ($old_major instanceof Site_Network) && ($link_type != SQ_LINK_NOTICE)) {
			// if we are moving from a site network, we can tolerate one existing
			// network link - the one being moved from
			$network_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($minor->id, SQ_SC_LINK_SIGNIFICANT, 'site_network', FALSE, 'minor');
			if (count($network_link) > 1) {
				return translate('Sites can only be linked under one Site Network');

			}
		} else {
			// if we aren't moving from a site network, the usual check applies
			return parent::canMoveLink($minor, $old_major, $link_type);
		}

		return TRUE;

	}//end canMoveLink()


	/**
	* Get the primary URL for this network, with the protocol
	*
	* @return string
	* @access public
	*/
	function getPrimaryURL()
	{
		if (!isset($this->_tmp['primary_url'])) {
			$site_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'site', FALSE, 'primary_site');
			if (empty($site_link)) return '';

			try {
				$bind_vars = Array('assetid'=> $site_link['minorid']);
				$primary_url_data = MatrixDAL::executeAll('site_network', 'getPrimaryURL', $bind_vars);
			} catch (DALException $e) {
				throw new Exception('Unable to get primary URL due to database error: '.$e->getMessage());
			}

			if (empty($primary_url_data)) return '';
			$primary_url = $primary_url_data[0]['url'];

			if ($primary_url_data[0]['https'] == '1') {
				$primary_url = 'https://'.$primary_url;
			} else {
				$primary_url = 'http://'.$primary_url;
			}

			$primary_url = sq_root_url($primary_url);
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
			trigger_localised_error('CORE0072', sprintf(translate('Session file "%s" does not exist'), $session_file), E_USER_WARNING);
			return FALSE;
		}

		if (!is_array($session_contents)) {
			trigger_localised_error('CORE0004', sprintf(translate('Array expected for second argument, got %s'), gettype($session_contents)), E_USER_WARNING);
			return FALSE;
		}

		$session_str = '';
		foreach ($session_contents as $key => $val) {
			$session_str .= $key.'|'.serialize($val);
		}

		if (!string_to_file($session_str, $session_file)) {
			trigger_localised_error('CORE0021', sprintf(translate('Unable to write to session file "%s"'), $session_file), E_USER_WARNING);
			return FALSE;
		}
		return TRUE;

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
			trigger_localised_error('CORE0071', translate('Primary session is corrupt'), E_USER_ERROR);
			return FALSE;
		}

		$pri_timestamp = array_get_index($pri_sess, 'SQ_SESSION_TIMESTAMP', -1);
		$sec_timestamp = array_get_index($_SESSION, 'SQ_SESSION_TIMESTAMP', -1);
		$pri_login_key = array_get_index($pri_sess, 'SQ_LOGIN_KEY', NULL);
		$sec_login_key = array_get_index($_SESSION, 'SQ_LOGIN_KEY', NULL);

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
		if (!is_null($pri_login_key)) {
			$pri_sess['SQ_LOGIN_KEY'] = $pri_login_key;
		}
		if (!is_null($sec_login_key)) {
			$_SESSION['SQ_LOGIN_KEY'] = $sec_login_key;
		}

		// save the sessionid of the primary so that we
		// know that we have run this script before. We won't have to
		// do anther browser refresh if we know this.
		$_SESSION['PRIMARY_SESSIONID'] = $pri_sessionid;
		$pri_sess['PRIMARY_SESSIONID'] = $pri_sessionid;

		// *** JEDI MIND TRICK *** you did not see us doing this... move along
		if (!$this->serialiseSessionFile(SQ_CACHE_PATH.'/sess_'.$pri_sessionid, $pri_sess)) {
			trigger_localised_error('CORE0020', translate('Unable to write session back to primary session file'), E_USER_ERROR);
			return FALSE;
		}
		return TRUE;

	}//end syncSessionFile()


}//end class

?>

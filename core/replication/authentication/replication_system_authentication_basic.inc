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
* $Id: replication_system_authentication_basic.inc,v 1.1.2.10 2004/02/21 14:47:05 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_system_authentication.inc';

/**
* Replication_System_Authentication_Basic
*
* Purpose
*
*    Handle the authentication of connections between Master Server and Slaves using Basic authentication
*    [TODO: define what "Basic Authentication" actually is]
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_System_Authentication_Basic extends Replication_System_Authentication
{

	/**
	* Password the Slave sends to the Master when authenticating
	* @var string
	*/
	var $slave_to_master_phrase = '';

	/**
	* Password the Master sends to the Slave when authenticating
	* @var string
	*/
	var $master_to_slave_phrase = '';

	/**
	* If not a blank string, then to authenticate the remote system must be this IP
	* @var string
	*/
	var $ip_address = '';


	/**
	* Returns an array of data that can be later used with loadParams() to re-initiate this authentication type
	* The reason we do this rather than just serialising the whole object is so that it can be edited by
	* hand if need be
	*
	* @access public
	* @return Array()
	* @see loadParams()
	*/
	function extractParams()
	{
		return Array(
			'slave_to_master_phrase' => $this->slave_to_master_phrase,
			'master_to_slave_phrase' => $this->master_to_slave_phrase,
			'ip_address'             => $this->ip_address,
		);
	
	}//end extractParams()


	/**
	* Loads an array of data that came from extractParams() to re-initiate this authentication type
	*
	* @param Array $params	the params array as retrieved from extractParams()
	*
	* @access public
	* @return Array()
	* @see extractParams()
	*/
	function loadParams($params)
	{
		if (isset($params['slave_to_master_phrase'])) $this->slave_to_master_phrase = $params['slave_to_master_phrase'];
		if (isset($params['master_to_slave_phrase'])) $this->master_to_slave_phrase = $params['master_to_slave_phrase'];
		if (isset($params['ip_address']))             $this->ip_address             = $params['ip_address'];
		return true;
	
	}//end loadParams()


	/**
	* Attempts to authenticate with the connection with $fd
	*
	* @param object Replication_System_Transport	$transport	the socket file desriptor resource to read from
	* @param boolean								$initiator	whether this end of the connection initiated the connection or not
	*
	* @return boolean
	* @access public
	*/
	function authenticate($transport, $initiator)
	{
		if ($this->slave_to_master_phrase == '' || $this->master_to_slave_phrase == '') {
			trigger_error('Unable to authenticate without two pass phrases', E_USER_WARNING);
			return false;
		}

		if (SQ_REPLICATION_IS_MASTER) {
			$our_phrase   = $this->master_to_slave_phrase;
			$their_phrase = $this->slave_to_master_phrase;
		} else {
			$our_phrase   = $this->slave_to_master_phrase;
			$their_phrase = $this->master_to_slave_phrase;
		}

		// now because we initiated the connection the other end will be the one challenging us 
		// with the authentication, so we wait for them
		if ($initiator) {
			if (!$this->_sendPhrase($transport)) return false;
			if (!$this->_requestPhrase($transport)) return false;

		} else {
			// only need to check the ip on the side that didn't initiate because the side that initated
			// know's where it initiated the connection to :)
			if ($this->ip_address != '') {
				if ($this->ip_address != $transport->getPeerInfo(true)) {
					trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : IP Address Incorrect', E_USER_WARNING);
					// just being polite really, but we don't really care if it doesn't get there, we quitting anyway..
					$transport->writeVar('Authentication Failed');
					return false;
				}
			}
			if (!$this->_requestPhrase($transport)) return false;
			if (!$this->_sendPhrase($transport)) return false;
		}// end if

		return true;

	}//end authenticate()


	/**
	* We are going to be asking the other side for a passphase, this takes care of it.
	* Works together with _sendPhrase()
	*
	* @param object Replication_System_Transport	$transport	the socket file desriptor resource to read from
	*
	* @return boolean	true if the phrase the remote server presented was what we expected
	* @access public
	* @see _sendPhrase()
	*/
	function _requestPhrase($transport)
	{
		$their_phrase = (SQ_REPLICATION_IS_MASTER) ? $this->slave_to_master_phrase : $this->master_to_slave_phrase;

		if (true !== ($err_msg = $transport->sendAndReceiveVar('Phrase:', $data=null, $headers=null))) {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : '.$err_msg, E_USER_WARNING);
			return false;
		}
		if ($data != $their_phrase) {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : Supplied Phrase was incorrect', E_USER_WARNING);
			// just being polite really, but we don't really care if it doesn't get there, we are quitting anyway..
			$transport->writeVar('Authentication Failed');
			return false;
		}
		if (!$transport->writeVar('Authenticated')) {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : Data Write Error', E_USER_WARNING);
			return false;
		}

		return true;

	}//end _requestPhrase()

	
	/**
	* We are expecting to be asked to send a passphrase, this takes care of it.
	* Works together with _requestPhrase()
	*
	* @param object Replication_System_Transport	$transport	the socket file desriptor resource to read from
	*
	* @return boolean	true if our phrase was authenticated by the remote server
	* @access public
	* @see _requestPhrase()
	*/
	function _sendPhrase($transport)
	{
		$our_phrase   = (SQ_REPLICATION_IS_MASTER) ? $this->master_to_slave_phrase : $this->slave_to_master_phrase;

		if (true !== ($err_msg = $transport->receiveAndSendVar('Phrase:', $our_phrase))) {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : '.$err_msg, E_USER_WARNING);
			return false;
		}

		if (!$transport->readVar($data=null,$headers=null)) {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : Data Read Error', E_USER_WARNING);
			return false;
		}

		if ($data != 'Authenticated') {
			trigger_error('Unable to Authenticate ('.$transport->getPeerInfo().') : Expected "Authenticated", got "'.$data.'"', E_USER_WARNING);
			return false;
		}

		return true;

	}//end _sendPhrase()

}//end class

?>
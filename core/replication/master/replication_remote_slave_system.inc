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
* $Id: replication_remote_slave_system.inc,v 1.1.2.3 2004/03/09 18:17:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/replication/replication_remote_system.inc';

/**
* Replication_Remote_Slave_System
*
* Purpose
*
*    To represent a remote system in the replication framework of the master system
*    Includes taking care of Authentication and Managing the connections between the systems
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_Remote_Slave_System extends Replication_Remote_System
{
	/**
	* The timestamp for when this system was created
	* @var int
	*/
	var $created = null;

	/**
	* The timestamp for when this system was retired
	* @var int
	*/
	var $retired = null;


	/**
	* Constructor
	*
	* @param string	$systemid	the id of the system that we are representing
	*
	*/
	function Replication_Remote_Slave_System($systemid='')
	{
		$this->Replication_Remote_System();
		if($systemid) $this->load($systemid);

	}//end constructor


	/**
	* Creates a new Remote System by creating a config file for it
	*
	* @param string	$name				the text name of the system that we will be representing
	* @param string	$auth_type			the authentication type that is going to be used
	* @param string	$transport_method	the transport method that is going to be used
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function create($name, $auth_type, $transport_method)
	{
		if (true !== ($err_msg = Replicator::validSystemName($name))) {
			trigger_error('Unable to create Replication Remote System "'.$name.'" : System Name Invalid ('.$err_msg.')', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT COUNT(*) 
				FROM '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				WHERE name = '.$db->quote($name);

		$result = $db->getOne($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		if (!empty($result)) {
			trigger_error('Unable to create Replication Remote System "'.$name.'" : System Name has already been used', E_USER_WARNING);
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$systemid = $db->nextId(SQ_TABLE_PREFIX.'sequence_replication_slave');
		if (DB::isError($systemid)) {
			trigger_error($systemid->getMessage().'<br/>'.$systemid->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$now = time();

		$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				(systemid, name, created, retired)
				VALUES
				('.$db->quote($systemid).', '.$db->quote($name).', '.$db->quote(ts_iso8601($now)).', NULL)';
		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->id      = $systemid;
		$this->name    = $name;
		$this->created = $now;
		$this->retired = null;

		if (!$this->setAuthType($auth_type)) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : Unable to set Authentication Type', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		if (!$this->setTransportMethod($transport_method)) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : Unable to set Transport Method', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		// make sure the directory exists
		if (!create_directory($this->dataPath())) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : unable to create data directory', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		if (!$this->save()) {
			trigger_error('Unable to create Replication Remote System "'.$this->id.'" : unable to save', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = null;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end create()


	/**
	* Returns the conf file for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	*/
	function confFile()
	{
		if (is_null($this->id)) {
			trigger_error('Unable to get config filename : No System Id', E_USER_WARNING);
			return '';
		}
		return SQ_DATA_PATH.'/private/conf/replication/'.$this->id.'_'.$this->name.'.inc';

	}// end confFile()


	/**
	* Returns the path to a data directory especially for this remote system
	*
	* @return string
	* @access public
	* @see save(), load()
	* @abstract
	*/
	function dataPath()
	{
		if (is_null($this->id)) {
			trigger_error('Unable to get data path : No System Id', E_USER_WARNING);
			return '';
		}
		return SQ_DATA_PATH.'/private/replication/remote_systems/'.$this->id;

	}// end dataPath()


	/**
	* Loads a remote system into this instance
	*
	* @param string	$systemid	the id of the system to load
	*
	* @return boolean
	* @access public
	* @see save()
	*/
	function load($systemid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$sql = 'SELECT systemid, name, created, retired
				FROM '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				WHERE systemid = '.$db->quote($systemid);

		$result = $db->getRow($sql, null, DB_FETCHMODE_ORDERED);
		if (DB::isError($result)) trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);

		if (is_null($result)) return;

		list($this->id,
			 $this->name,
			 $created,
			 $retired) = $result;
		$this->created = iso8601_ts($created);
		$this->retired = ($retired != '') ? iso8601_ts($retired) : null;
		unset($result);

		if (is_null($this->retired)) {
			return parent::load();
		} else {
			return true;
		}// end if

	}// end load()


	/**
	* Retires a remote system out of server
	*
	* @return boolean	indicating success or not
	* @access public
	*/
	function retire()
	{
		if (is_null($this->id)) return false;

		if ($this->isRetired()) {
			trigger_error('Unable to retire Replication Remote System "'.$this->id.'" : System Already Retired', E_USER_WARNING);
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$now = time();
		$sql = 'UPDATE '.SQ_TABLE_PREFIX.'replication_remote_slave_system
				  SET retired = '.$db->quote(ts_iso8601($now)).'
				WHERE systemid = '.$db->quote($this->id);

		$result = $db->query($sql);
		if (DB::isError($result)) {
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$conf_file = $this->confFile();
		if ($conf_file != '' && file_exists($conf_file) && !unlink($conf_file)) {
			trigger_error('Unable to retire Replication Remote System "'.$this->id.'" : unable to remove config file', E_USER_WARNING);
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$this->retired = $now;

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end retire()


	/**
	* Is this remote system retired ?
	*
	* @return boolean	true if retired, false if not
	* @access public
	*/
	function isRetired()
	{
		return !is_null($this->retired);
	}// end isRetired()


	/**
	* Install a slave from ourselves, basically do a few checks and send all our data to the slave
	*
	* @return void
	* @access public
	*/
	function install()
	{
		trigger_error('NOT WORKING', E_USER_ERROR);
#
#		$this->_log("Starting Slave Install", 'debug');
#		$this->_log("Do Some Locking Here : ".__FILE__.'::'.__LINE__, 'general');
#
#		$e = 'Slave Install Error ('.$this->name.' ['.$this->transport->getPeerInfo().']) : ';
#
#		if (true !== ($err_msg = $this->transport->sendAndReceiveVar('Start Checks', $ack=null, $headers=null))) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return;
#		}
#		if ($ack != 'ack') {
#			trigger_error($e.'Did not receive "ack" reply', E_USER_WARNING);
#			return;
#		}
#
#		$checks = Array();
#		
#		// check PHP
#		$checks['php'] = Array(
#							'version' => PHP_VERSION,
#							);
#
#		// check system version
#		$checks['system'] = Array(
#								'version' => SQ_SYSTEM_VERSION,
#							);
#
#		// check crypt()
#		$checks['crypt'] = Array(
#								'text'      => SQ_SYSTEM_SHORT_NAME,
#								'encrypted' => crypt(SQ_SYSTEM_SHORT_NAME), // let salt be generated
#								);
#
#		// check db 
#		$checks['db'] = Array(
#							'phptype'      => $GLOBALS['SQ_SYSTEM']->db->phptype,
#							);
#
#		// check asset tree
#		$checks['asset_tree'] = Array(
#								'SQ_CONF_ASSET_TREE_BASE' => SQ_CONF_ASSET_TREE_BASE,
#								'SQ_CONF_ASSET_TREE_SIZE' => SQ_CONF_ASSET_TREE_SIZE,
#								);
#
#
#		if (true !== ($err_msg = $this->transport->sendAndReceiveVar($checks, $ack=null, $headers=null))) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return;
#		}
#		$this->_log("CHECKS RESULT : ".$ack, 'debug');
#		if ($ack != 'Checks OK') {
#			trigger_error($e.'Checks Failed', E_USER_WARNING);
#			return;
#		}
#
#		$this->_log("SEND DATA", 'debug');
#
#		// First thing to do is create a file with all our assetids and versions that will be sent to the slave 
#		// to permorm the checking
#		$versions_file = $this->dataPath().'/asset_versions.txt';
#		$f = fopen($versions_file, 'wb');
#		if (!$f) {
#			trigger_error($e.'Unable to open file "'.$versions_file.'" for writing', E_USER_WARNING);
#			return;
#		}
#
#		$sql = 'SELECT assetid, version
#				FROM '.SQ_TABLE_PREFIX.'asset
#				ORDER BY assetid';
#		$result = $GLOBALS['SQ_SYSTEM']->db->query($sql);
#		if (DB::isError($result)) {
#			trigger_error($e.$result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
#			fclose($f);
#			return;
#		}
#
#		if (!$this->transport->writeVar((int) $result->numRows())) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			$result->free();
#			fclose($f);
#			return;
#		}
#
#
#		$all_ok = true;
#		while (null !== ($row = $result->fetchRow())) {
#			if (!fwrite($f, $row['assetid']."\t".$row['version']."\n")) {
#				trigger_error($e.'Error writing to "'.$versions_file.'"', E_USER_WARNING);
#				$all_ok = false;
#				break;
#			}
#		}//end while
#		$result->free();
#		fclose($f);
#
#		if (!$all_ok) return;
#
#		if (!$this->transport->writeFile($versions_file)) {
#			trigger_error($e.$err_msg, E_USER_WARNING);
#			return;
#		}

	}// end install()


	/**
	* Get's all the assetids and versions from the slave just after it has done it's initial import of data
	*
	* @return void
	* @access public
	*/
	function versions_init()
	{
		$this->_log("Starting Slave Version Init", 'debug');
		$this->_log("Do Some Locking Here : ".__FILE__.'::'.__LINE__, 'general');

		$e = 'Slave Version Init Error ('.$this->name.' ['.$this->transport->getPeerInfo().']) : ';

		if (true !== ($err_msg = $this->transport->sendAndReceiveVar('Num Assets', $num_assets=null, $headers=null))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			return;
		}
		if (!is_int($num_assets)) {
			trigger_error($e.'Num Assets is not an integer', E_USER_WARNING);
			return;
		}

		if (true !== ($err_msg = $this->transport->sendAndReceiveVar('Send Assets', $ack=null, $headers=null))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			return;
		}
		if ($ack != 'ack') {
			trigger_error($e.'Did not receive "ack" reply', E_USER_WARNING);
			return;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		for($i = 0; $i < $num_assets; $i++) {
			if (true !== ($err_msg = $this->transport->readVar($row=null, $headers=null))) {
				trigger_error($e.$err_msg, E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return;
			}
			if (!is_array($row) || empty($row['assetid']) || empty($row['version'])) {
				trigger_error($e.'Invalid from slave, expecting assetid and version in array, got : '.print_r($row, 1), E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return;
			}

			$sql = 'INSERT INTO '.SQ_TABLE_PREFIX.'replication_remote_slave_asset_versions
					(systemid, assetid, version)
					VALUES
					('.$db->quote($this->id).', '.$db->quote($row['assetid']).', '.$db->quote($row['version']).')';
$this->_log($sql, 'debug');
			$result = $db->query($sql);
			if (DB::isError($result)) {
				trigger_error($e.$result->getMessage()."\n".$result->getUserInfo(), E_USER_ERROR);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return;
			}

		}// end for

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		if (true !== ($err_msg = $this->transport->sendAndReceiveVar('Got Assets', $ack=null, $headers=null))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			return;
		}
		if ($ack != 'ack') {
			trigger_error($e.'Did not receive "ack" reply', E_USER_WARNING);
			return;
		}

	}// end versions_init()


	/**
	* Replicate with the slave
	*
	* General Process is list this :
	* <ul>
	*  <li> Tell slave to start getting assets changed on it's system
	*  <li> Create a point in time on this system
	*  <li> Getting assets changed from the last replication in time until our point in time
	*  <li> Collect the assets that have changed from the slave system
	*  <li> Check for assets that have changed in both systems
	*  <li> 
	* </ul>
	*
	* @return void
	* @access public
	*/
	function replicate()
	{
		$this->_log("Starting Slave Replication....", 'debug');
		$this->_log("Do Some Locking Here : ".__FILE__.'::'.__LINE__, 'general');

		$e = 'Slave Replication Error ('.$this->name.' ['.$this->transport->getPeerInfo().']) : ';

		// tell them to create a list of assets that have changed
		if (true !== ($err_msg = $this->transport->sendAndReceiveVar('Find Changed Assets', $ack=null, $headers=null))) {
			trigger_error($e.$err_msg, E_USER_WARNING);
			return;
		}
		if ($ack != 'ack') {
			trigger_error($e.'Did not receive "ack" reply', E_USER_WARNING);
			return;
		}

		$point_in_time = time();

		$our_changed_assets = $this->getChangedAssets($point_in_time);


	}// end replicate()


	/**
	* Return a list of assets that have changed since replication was last run for this slave
	*
	* @param int	$timestamp	the point in time that we are checking up until
	*
	* @return Array('[assetid]' => Array('current_version' => '[the current version according to us]', 'old_version' => '[the version at the last tiem we replicated]'))
	* @access public
	*/
	function getChangedAssets($timestamp) 
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// NOTE: if the sav.* is null it means that the asset was created since the last time we ran this
		$sql = 'SELECT a.assetid, a.version as current_version, sav.version as old_version
				FROM '.SQ_TABLE_ROLLBACK_PREFIX.'asset a
						LEFT OUTER JOIN '.SQ_TABLE_PREFIX.'replication_remote_slave_asset_versions sav ON a.assetid = sav.assetid
				WHERE a.sq_effective_to IS NULL
				  AND (a.'.SQ_TABLE_PREFIX.'effective_to IS NULL
							OR a.'.SQ_TABLE_PREFIX.'effective_to > '.$this->db->quote(ts_iso8601($timestamp)).')
				  AND (sav.systemid IS NULL OR sav.systemid = '.$this->db->quote($this->id).')
				  AND (sav.version IS NULL OR a.version != sav.version)';
pre_echo($sql);

		$result = $db->getAssoc($sql);
		if (DB::isError($result)) {
			trigger_error($e.$result->getMessage()."\n".$result->getUserInfo(), E_USER_ERROR);
			return false;
		}

		pre_echo($result);

		return $result;		

	}// end getChangedAssets()

}//end class

?>
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
* $Id: replication_config.inc,v 1.1.2.3 2004/02/13 17:18:58 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_SYSTEM_ROOT.'/core/replication/replicator.inc';

/**
* Replication_Config
*
* Purpose
*
*    Looks after the creation and editing of the hipo config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Replication_Config extends Config
{
	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
		'SQ_REPLICATION_ENABLED'	=> Array('editable' => 0, 'default' =>  false),
		'SQ_REPLICATION_IS_MASTER'	=> Array('editable' => 0, 'default' =>  true),
//
//		// Following only applicable to slaves
//		'SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID'	=> Array('editable' => 1, 'default' => ''),
//		'SQ_REPLICATION_MASTER_SERVER_SYSTEMID'			=> Array('editable' => 1, 'default' => ''),
//		'SQ_REPLICATION_MASTER_SERVER_DOMAIN'			=> Array('editable' => 1, 'default' => ''),
//		'SQ_REPLICATION_MASTER_SERVER_PORT'				=> Array('editable' => 1, 'default' =>  0),
//		'SQ_REPLICATION_MASTER_AUTH'					=> Array('editable' => 1, 'default' => 'basic'),
//		'SQ_REPLICATION_MASTER_TRANSPORT'				=> Array('editable' => 1, 'default' => 'plain_text'),
	

	);


	/**
	* Constructor
	*
	*/
	function Replication_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/replication.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false)
	{

	pre_echo("SAVE");
//		// if we are a master server then we don't need all the SQ_REPLICATION_MASTER_* crap
//		// so we will remove it
//		$is_master = $this->config_vars['SQ_REPLICATION_IS_MASTER']['default'];
//
//		// are they attempting to set it
//		if (isset($vars['SQ_REPLICATION_IS_MASTER'])) {
//			$is_master = $vars['SQ_REPLICATION_IS_MASTER'];
//
//		// is there something alreadyt set ?
//		} else if (file_exists($this->config_file)) {
//			require_once $this->config_file;
//			if (defined('SQ_REPLICATION_IS_MASTER')) $is_master = SQ_REPLICATION_IS_MASTER;
//		}
//		// Now just make sure they aren't trying to set anything that they shouldn't
//		if ($is_master) {
//			unset($this->config_vars['SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID']);
//			unset($this->config_vars['SQ_REPLICATION_MASTER_SERVER_SYSTEMID']);
//			unset($this->config_vars['SQ_REPLICATION_MASTER_SERVER_DOMAIN']);
//			unset($this->config_vars['SQ_REPLICATION_MASTER_SERVER_PORT']);
//			unset($this->config_vars['SQ_REPLICATION_MASTER_AUTH']);
//			unset($this->config_vars['SQ_REPLICATION_MASTER_TRANSPORT']);
//
//		} else {
//			if (isset($vars['SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID']) && $vars['SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID'] != '') {
//				if (true !== ($err_msg = Replicator::validSystemId($vars['SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID']))) {
//					trigger_error('Unable to save '.$this->name.' : Master Replication System Id not valid ('.$err_msg.')', E_USER_WARNING);
//					return false;
//				}
//			}
//
//		}//end if

		return parent::save($vars, $backup_existing);

	}//end save()


	/**
	* Paint's out the code for the config file for the passed config
	*
	* This allows extra things to be added for special cases, and extra checks to be made if necessary on the value
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value)
	{
		switch($var_name) {
//			case 'SQ_REPLICATION_SYSTEM_ID' :
//			case 'SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID' :
//
//				if ($value != '') {
//					if (true !== ($err_msg = Replicator::validSystemId($value))) {
//						$disp_name = ucwords(strtolower(str_replace('_', ' ', substr($var_name, 15))));
//						trigger_error('Unable to save '.$this->name.' : Invalid '.$disp_name.' ('.$err_msg.')', E_USER_WARNING);
//						return false;
//					}
//				}
//				return parent::_paintConfigVar($var_name, $value);
//				return true;
//				break;

			default :
				return parent::_paintConfigVar($var_name, $value);

		}// end switch

	}//end _paintConfigVar()



	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		// need to be root
		return $GLOBALS['SQ_SYSTEM']->userRoot();
	}// end canAcquireLock()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!parent::writeAccess()) return false;
		// need to be root
		if ($GLOBALS['SQ_SYSTEM']->userRoot()) return true;
		if (!empty($GLOBALS['SQ_INSTALL'])) return true;
		return false;

	}// end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class($this);

		$o->openSection('Replication Configuration');

			// These Fields are not editable
			$o->openField('Replication Enabled');
			echo (SQ_REPLICATION_ENABLED) ? 'Yes' : 'No';

			$o->openField('Is this the Master System ?');
			echo (SQ_REPLICATION_IS_MASTER) ? 'Yes' : 'No';

		$o->closeSection();


		$replicator = &$GLOBALS['SQ_SYSTEM']->getSystemReplicator();
		$replicator->paintBackend($o, $write_access);

//		if (SQ_REPLICATION_IS_MASTER) {
//			$o->openSection('Slave System\'s Connection Settings');
//
//			$slave_list_str = $this->_getVal('SQ_REPLICATION_SLAVE_LIST');
//
//			if (trim($this->_getVal('SQ_REPLICATION_SLAVE_LIST')) != '') {
//				$slave_list = explode("\n", $slave_list_str);
//				foreach($slave_list as $slave_systemid) {
//					if($write_access) {
//						text_box($class.'[SQ_REPLICATION_SLAVE_LIST_ARR][]', $slave_systemid, 30);
//					} else {
//						echo $slave_systemid;
//					}
//				}// end foreach
//
//			}// end if
//
//		} else {
//
//			$o->openSection('Master System Connection Settings');
//
//				$o->openField('Replication System ID');
//				echo $this->_getVal('SQ_REPLICATION_MASTER_REPLICATION_SYSTEM_ID').'<br>';
//				$o->note('This is the Replication System Id of the System that the Master to this Slave System');
//
//				$o->openField('Squiz Server System ID');
//				if($write_access) {
//					text_box($class.'[SQ_REPLICATION_MASTER_SERVER_SYSTEMID]', $this->_getVal('SQ_REPLICATION_MASTER_SERVER_SYSTEMID'), 30);
//				} else {
//					echo $this->_getVal('SQ_REPLICATION_MASTER_SERVER_SYSTEMID');
//				}
//				$o->note('This is the Squiz Server System ID that the Master System uses to connect to it\'s squiz server');
//
//				$o->openField('Domain');
//				if($write_access) {
//					text_box($class.'[SQ_REPLICATION_MASTER_SERVER_DOMAIN]', $this->_getVal('SQ_REPLICATION_MASTER_SERVER_DOMAIN'), 30);
//				} else {
//					echo $this->_getVal('SQ_REPLICATION_MASTER_SERVER_DOMAIN');
//				}
//
//				$o->openField('Port');
//				if($write_access) {
//					text_box($class.'[SQ_REPLICATION_MASTER_SERVER_PORT]', $this->_getVal('SQ_REPLICATION_MASTER_SERVER_PORT'), 10);
//				} else {
//					echo $this->_getVal('SQ_REPLICATION_MASTER_SERVER_PORT');
//				}
//
//				$o->openField('Authentication Type');
//				$auth_types = Replicator::getAuthTypes();
//				if($write_access) {
//					combo_box($class.'[SQ_REPLICATION_MASTER_AUTH]', $auth_types, false, $this->_getVal('SQ_REPLICATION_MASTER_AUTH'));
//				} else {
//					echo $auth_types[$this->_getVal('SQ_REPLICATION_MASTER_AUTH')];
//				}
//
//				$o->openField('Transport Method');
//				$transport_methods = Replicator::getTransportMethods();
//				if($write_access) {
//					combo_box($class.'[SQ_REPLICATION_MASTER_TRANSPORT]', $transport_methods, false, $this->_getVal('SQ_REPLICATION_MASTER_TRANSPORT'));
//				} else {
//					echo $transport_methods[$this->_getVal('SQ_REPLICATION_MASTER_TRANSPORT')];
//				}
//
//			$o->closeSection();
//
//		}// end if !is master

		if ($write_access) $o->commitButton('Commit', true);


	}// end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean	false on error, true on everything else (including nothing happening)
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		if (!empty($_POST['process_form'])) {
			if ($this->writeAccess()) {
				$replicator = &$GLOBALS['SQ_SYSTEM']->getSystemReplicator();
				if (!$replicator->processBackend($o))  {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			} else {
				trigger_error('Permission Denied. You do not have write access.', E_USER_WARNING);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end if

		if (!parent::processBackend($o)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end processBackend()


}//end class

?>
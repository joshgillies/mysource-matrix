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
* $Id: replication_config.inc,v 1.1.2.1 2004/01/21 18:43:56 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

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
		'SQ_REPLICATION_ENABLED'	=> Array('editable' => 0, 'default' =>  0),
		'SQ_REPLICATION_SYSTEM_ID'	=> Array('editable' => 0, 'default' => ''),
		'SQ_REPLICATION_IS_MASTER'	=> Array('editable' => 0, 'default' =>  0),

		// Following only applicable to masters
		'SQ_REPLICATION_SLAVE_LIST'	=> Array('editable' => 1, 'default' => ''),

		// Following only applicable to slaves
		'SQ_REPLICATION_MASTER_SERVER_SYSTEMID'	=> Array('editable' => 1, 'default' => ''),
		'SQ_REPLICATION_MASTER_DOMAIN'			=> Array('editable' => 1, 'default' => ''),
		'SQ_REPLICATION_MASTER_PORT'			=> Array('editable' => 1, 'default' =>  0),
		'SQ_REPLICATION_MASTER_AUTH'			=> Array('editable' => 1, 'default' => 'basic'),
		'SQ_REPLICATION_MASTER_TRANSPORT'		=> Array('editable' => 1, 'default' => 'plain_text'),
	

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
		// if we are a master server then we don't need all the SQ_REPLICATION_MASTER_* crap
		// so we will remove it
		$is_master = $this->config_vars['SQ_REPLICATION_IS_MASTER']['default'];

		// are they attempting to set it
		if (isset($vars['SQ_REPLICATION_IS_MASTER'])) {
			$is_master = $vars['SQ_REPLICATION_IS_MASTER'];

		// is there something alreadyt set ?
		} else if (file_exists($this->config_file)) {
			require_once $this->config_file;
			if (defined('SQ_REPLICATION_IS_MASTER')) $is_master = SQ_REPLICATION_IS_MASTER;
		}
		// Now just make sure they aren't trying to set anything that they shouldn't
		if ($is_master) {
			unset($this->config_vars['SQ_REPLICATION_MASTER_SERVER_SYSTEMID']);
			unset($this->config_vars['SQ_REPLICATION_MASTER_DOMAIN']);
			unset($this->config_vars['SQ_REPLICATION_MASTER_PORT']);
			unset($this->config_vars['SQ_REPLICATION_MASTER_AUTH']);
			unset($this->config_vars['SQ_REPLICATION_MASTER_TRANSPORT']);

			// Now do some validation
			if (isset($vars['SQ_REPLICATION_SLAVE_LIST'])) {
				$vars['SQ_REPLICATION_SLAVE_LIST'] = str_replace("\r", '', $vars['SQ_REPLICATION_SLAVE_LIST']);
			}

		} else {
			unset($this->config_vars['SQ_REPLICATION_SLAVE_LIST']);
		}//end if

		return parent::save($vars, $backup_existing);

	}//end save()


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

			$o->openField('Replication System Id');
			echo SQ_REPLICATION_SYSTEM_ID;

			$o->openField('Is this the Master System ?');
			echo (SQ_REPLICATION_IS_MASTER) ? 'Yes' : 'No';

		$o->closeSection();

		if (SQ_REPLICATION_IS_MASTER) {
			$o->openSection('Slave System\'s Connection Settings');

			$slave_list_str = $this->_getVal('SQ_REPLICATION_SLAVE_LIST');

			if (trim($this->_getVal('SQ_REPLICATION_SLAVE_LIST')) != '') {
				$slave_list = explode("\n", $slave_list_str);
				foreach($slave_list as $slave_systemid) {
					if($write_access) {
						text_box($class.'[SQ_REPLICATION_SLAVE_LIST_ARR][]', $slave_systemid, 30);
					} else {
						echo $slave_systemid;
					}
				}// end foreach

			}// end if

		} else {

			$o->openSection('Master System Connection Settings');

				$o->openField('Squiz Server System ID');
				if($write_access) {
					text_box($class.'[SQ_REPLICATION_MASTER_SERVER_SYSTEMID]', $this->_getVal('SQ_REPLICATION_MASTER_SERVER_SYSTEMID'), 30);
				} else {
					echo $this->_getVal('SQ_REPLICATION_MASTER_SERVER_SYSTEMID');
				}
				$o->note('This is the Squiz Server System ID that the Master System uses to connect to it\'s squiz server');

				$o->openField('Domain');
				if($write_access) {
					text_box($class.'[SQ_REPLICATION_MASTER_DOMAIN]', $this->_getVal('SQ_REPLICATION_MASTER_DOMAIN'), 30);
				} else {
					echo $this->_getVal('SQ_REPLICATION_MASTER_DOMAIN');
				}

				$o->openField('Port');
				if($write_access) {
					text_box($class.'[SQ_REPLICATION_MASTER_PORT]', $this->_getVal('SQ_REPLICATION_MASTER_PORT'), 10);
				} else {
					echo $this->_getVal('SQ_REPLICATION_MASTER_PORT');
				}

				require_once SQ_SYSTEM_ROOT.'/core/replication/replicator.inc';

				$o->openField('Authentication Type');
				$auth_types = Replicator::getAuthTypes();
				if($write_access) {
					combo_box($class.'[SQ_REPLICATION_MASTER_AUTH]', $auth_types, false, $this->_getVal('SQ_REPLICATION_MASTER_AUTH'));
				} else {
					echo $auth_types[$this->_getVal('SQ_REPLICATION_MASTER_AUTH')];
				}

				$o->openField('Transport Method');
				$transport_methods = Replicator::getTransportMethods();
				if($write_access) {
					combo_box($class.'[SQ_REPLICATION_MASTER_TRANSPORT]', $transport_methods, false, $this->_getVal('SQ_REPLICATION_MASTER_TRANSPORT'));
				} else {
					echo $transport_methods[$this->_getVal('SQ_REPLICATION_MASTER_TRANSPORT')];
				}

			$o->closeSection();

		}// end if !is master

		if ($write_access) $o->commitButton('Commit', true);


	}// end paintBackend()



}//end class

?>
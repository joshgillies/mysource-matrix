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
* $Id: replication_config.inc,v 1.1.2.4 2004/02/21 14:48:55 brobertson Exp $
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
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
* $Id: squiz_server_config.inc,v 1.3 2004/01/15 21:55:32 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* Squiz_Server_Config
*
* Purpose
*
*    Looks after the creation and editing of the squiz server config
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Squiz_Server_Config extends Config
{
	var $config_vars = Array(
		'SQ_SERVER_SYSTEM_ROOT'	=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_SYSTEMID'	=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_DOMAIN'		=> Array('editable' => 1, 'default' => ''),
		'SQ_SERVER_PORT'		=> Array('editable' => 1, 'default' => 0),
	);

	/**
	* Constructor
	*
	*/
	function Squiz_Server_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/squiz_server.inc';

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

	}//end canAcquireLock()


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
		return $GLOBALS['SQ_SYSTEM']->userRoot();

	}//end writeAccess()


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
		
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();

		$class = get_class($this);

		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		$o->openSection('Squiz Server Configuration');

			$o->openField('Squiz Server System Root');
			if($write_access) {
				text_box($class.'[SQ_SERVER_SYSTEM_ROOT]', SQ_SERVER_SYSTEM_ROOT, 60);
			} else {
				echo SQ_SERVER_SYSTEM_ROOT;
			}

			$o->openField('Squiz Server System ID');
			if($write_access) {
				text_box($class.'[SQ_SERVER_SYSTEMID]', SQ_SERVER_SYSTEMID, 30);
			} else {
				echo SQ_SERVER_SYSTEMID;
			}

			$o->openField('Squiz Server Domain');
			if($write_access) {
				text_box($class.'[SQ_SERVER_DOMAIN]', SQ_SERVER_DOMAIN, 30);
			} else {
				echo SQ_SERVER_DOMAIN;
			}

			$o->openField('Squiz Server Port');
			if($write_access) {
				text_box($class.'[SQ_SERVER_PORT]', SQ_SERVER_PORT, 10);
			} else {
				echo SQ_SERVER_PORT;
			}

		$o->closeSection();

		if ($write_access) $o->commitButton('Commit', true);


	}//end paintBackend()


}//end class

?>
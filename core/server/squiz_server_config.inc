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
* $Id: squiz_server_config.inc,v 1.17 2012/08/30 01:09:22 ewang Exp $
*
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
* @version $Revision: 1.17 $
* @package MySource_Matrix
*/
class Squiz_Server_Config extends Config
{


	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
						'SQ_SERVER_SYSTEM_ROOT'	=> Array('editable'	=> 1, 'default'	=> ''),
						'SQ_SERVER_SYSTEMID'	=> Array('editable'	=> 1, 'default'	=> ''),
						'SQ_SERVER_DOMAIN'		=> Array('editable'	=> 1, 'default'	=> ''),
						'SQ_SERVER_PORT'		=> Array('editable'	=> 1, 'default'	=> 0),
					   );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/squiz_server.inc';
		$this->name = translate('Squiz Server Configuration');

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
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
		}

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
		$class = get_class_lower($this);

		$o->openSection(translate('Squiz Server Configuration'));


			$o->openField(translate('System Root'));

			if ($write_access) {
				text_box($class.'[SQ_SERVER_SYSTEM_ROOT]', SQ_SERVER_SYSTEM_ROOT, 60);
			} else {
				echo SQ_SERVER_SYSTEM_ROOT;
			}
			$o->note(translate('This is the path to the local Squiz Server\'s base directory'));


			$o->openField(translate('System ID'));

			if ($write_access) {
				text_box($class.'[SQ_SERVER_SYSTEMID]', SQ_SERVER_SYSTEMID, 30);
			} else {
				echo SQ_SERVER_SYSTEMID;
			}

			$o->openField(translate('Domain'));

			if ($write_access) {
				text_box($class.'[SQ_SERVER_DOMAIN]', SQ_SERVER_DOMAIN, 30);
			} else {
				echo SQ_SERVER_DOMAIN;
			}

			$o->openField(translate('Port'));

			if ($write_access) {
				text_box($class.'[SQ_SERVER_PORT]', SQ_SERVER_PORT, 10);
			} else {
				echo SQ_SERVER_PORT;
			}

		$o->closeSection();

		if ($write_access) $o->commitButton('', true);


	}//end paintBackend()


}//end class

?>

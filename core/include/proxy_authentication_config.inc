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
* $Id: proxy_authentication_config.inc,v 1.1 2006/04/10 23:06:33 sdanis Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Messaging_Service_Config
*
* Purpose
*
*    Looks after the creation and editing of the messaging service config
*
* @author  Sertan Danis <sdanis@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Proxy_Authentication_Config extends Config
{

	/**
	* The array of default messaging config settings
	* @var array
	*/
	var $config_vars = Array(
						'SQ_PA_ENABLED'		=> Array('editable' => 1, 'default' => FALSE),
						'SQ_PA_HOSTNAME'	=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_PORT'		=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_USERNAME'	=> Array('editable' => 1, 'default' => ''),
						'SQ_PA_PASSWORD'	=> Array('editable' => 1, 'default' => ''),
					   );


	/**
	* Constructor
	*
	*/
	function Proxy_Authentication_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/proxy_authentication.inc';

	}//end constructor


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	Backend_Outputter, reference to the backend outputter
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

		$o->openSection(translate('proxy_authentication_config'));
			$o->openField(translate('proxy_authentication_status'));
			if (SQ_PA_ENABLED) {
				require_once(SQ_INCLUDE_PATH.'/general_occasional.inc');
				$socket = openSocketConnection('http://matrix.squiz.net', $errno, $errstr);
				if (!is_null($socket)) {
					$response = fread($socket, 128);
					if (preg_match('|407 proxy authentication required|i', $response)) {
						echo '<font color="red"><strong>'.translate('proxy_authentication_failed').'</strong></font>';
					} else {
						echo '<font color="green"><strong>'.translate('proxy_authentication_passed').'</strong></font>';
					}
				}
			} else {
				echo '<strong>'.translate('proxy_authentication_disabled').'</font>';
			}
			$o->closeField();

			$o->openField(translate('proxy_authentication_enable'));
			if ($write_access) {
				combo_box($class.'[SQ_PA_ENABLED]', Array(translate('no'), translate('yes')), FALSE, SQ_PA_ENABLED);
			} else {
				echo (SQ_PA_ENABLED ? translate('yes') : translate('no'));
			}
			$o->closeField();
			$o->openField(translate('hostname'));
			if ($write_access) {
				text_box($class.'[SQ_PA_HOSTNAME]', SQ_PA_HOSTNAME, 25);
			} else {
				echo SQ_PA_HOSTNAME;
			}
			$o->closeField();
			$o->openField(translate('port'));
			if ($write_access) {
				text_box($class.'[SQ_PA_PORT]', SQ_PA_PORT, 5);
			} else {
				echo SQ_PA_PORT;
			}
			$o->closeField();
			$o->openField(translate('username'));
			if ($write_access) {
				text_box($class.'[SQ_PA_USERNAME]', SQ_PA_USERNAME, 15);
			} else {
				echo SQ_PA_USERNAME;
			}
			$o->closeField();
			$o->openField(translate('password'));
			if ($write_access) {
				password_box($class.'[SQ_PA_PASSWORD]', base64_decode(SQ_PA_PASSWORD), 15);
			} else {
				if (strlen(SQ_PA_PASSWORD)) {
					echo translate('password_not_shown');
				}
			}
			$o->closeField();

		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


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
		log_dump($vars);

		//encode password
		if (isset($vars['SQ_PA_PASSWORD']) && !empty($vars['SQ_PA_PASSWORD'])) {
			$vars['SQ_PA_PASSWORD'] = base64_encode($vars['SQ_PA_PASSWORD']);
		}

		return parent::save($vars, $backup_existing);

	}//end save()


}//end class

?>

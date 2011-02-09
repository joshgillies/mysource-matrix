<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: tool_clear_squid_cache_prefs.inc,v 1.6.4.1 2011/02/09 22:18:28 akarelia Exp $
*
*/

require_once SQ_LIB_PATH.'/config/prefs.inc';

if (!defined('SQ_SQUID_CACHE_SLASH_ALLOW')) {
	define ('SQ_SQUID_CACHE_SLASH_ALLOW', 1);
}

if (!defined('SQ_SQUID_CACHE_SLASH_DENY')) {
	define ('SQ_SQUID_CACHE_SLASH_DENY', 0);
}

/**
* Tool_Clear_Squid_Cache_Prefs
*
* Purpose
*
*    List of host names to be used when purging the Squid Cache in the Global Prefs
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.6.4.1 $
* @package MySource_Matrix
* @subpackage squid
*/
class Tool_Clear_Squid_Cache_Prefs extends Prefs
{

	var $pref_vars = Array(
						'SQ_SQUID_CACHE_HOSTNAMES'	=> Array(
														'name'			=> 'squid_tool_hostname',
														'description'	=> 'A list of newline separated host names to be used when purging the Squid Cache',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_PATH'		=> Array(
														'name'			=> 'squid_tool_path_field',
														'description'	=> 'The path value to the squidclient',
														'default'		=> '/usr/bin/squidclient',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_PORT'		=> Array(
														'name'			=> 'squid_tool_port_field',
														'description'	=> 'A list of newline separated host names to be used when purging the Squid Cache',
														'default'		=> '3128',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_OPTION'		=> Array(
														'name'			=> 'squid_tool_option_field',
														'description'	=> 'Additonal options to be included in the purge command',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_SLASH'		=> Array(
														'name'			=> 'squid_tool_slash_field',
														'description'	=> 'URLs with trailing slash are also processed by the purge command',
														'default'		=> FALSE,
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_URL_PORT'			=> Array(
														'name'			=> 'squid_tool_url_port_field',
														'description'	=> 'You can specify a port to add to each URL when submitted to Squid. If you are running Squiz Matrix on a non-standard port behind Squid, you may need to add the port back to the URL to clear the cache.',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
					 );


	/**
	* Constructor
	*
	*/
	function Tool_Clear_Squid_Cache_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_HOSTNAMES') {
			if (is_null($pref)) $o->openField(translate('squid_tool_hostname'));
			if ($have_lock && $is_admin) {
				// enter hostnames, separated by newline
				text_area('prefs[SQ_SQUID_CACHE_HOSTNAMES]', $this->pref_vars['SQ_SQUID_CACHE_HOSTNAMES']['default'], 80, 4);
			} else {
				echo nl2br($this->pref_vars['SQ_SQUID_CACHE_HOSTNAMES']['default']);
			}
			 echo $o->note(translate('squid_tool_pref_note'));
			if (is_null($pref)) $o->closeField();
		}


		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_PATH') {
			// specify the path to squidclient
			if (is_null($pref)) {
				$o->openField(translate('squid_tool_path_field'));
			}
			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_SQUID_CACHE_PATH]', $this->pref_vars['SQ_SQUID_CACHE_PATH']['default'], 30);
			} else {
				echo nl2br($this->pref_vars['SQ_SQUID_CACHE_PATH']['default']);
			}
			if (is_null($pref)) $o->closeField();
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_PORT') {
			// default port number
			if (is_null($pref)) {
				$o->openField(translate('squid_tool_port_field'));
			}
			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_SQUID_CACHE_PORT]', $this->pref_vars['SQ_SQUID_CACHE_PORT']['default'], 30);
			} else {
				echo nl2br($this->pref_vars['SQ_SQUID_CACHE_PORT']['default']);
			}
			if (is_null($pref)) $o->closeField();
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_OPTION') {
			// additional options to be included in the command, can be left blank
			if (is_null($pref)) {
				$o->openField(translate('squid_tool_option_field'));
			}
			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_SQUID_CACHE_OPTION]', $this->pref_vars['SQ_SQUID_CACHE_OPTION']['default'], 30);
				$o->note(translate('squid_tool_option_note'));
			} else {
				echo nl2br($this->pref_vars['SQ_SQUID_CACHE_OPTION']['default']);
			}
			if (is_null($pref)) $o->closeField();
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_SLASH') {
			// Whether URLs with trailing slash should also be included in the PURGE command
			if (is_null($pref)) {
				$o->openField(translate('squid_tool_slash_field'));
			}
			$val = $this->pref_vars['SQ_SQUID_CACHE_SLASH']['default'];
			if ($have_lock && $is_admin) {
				$selection = Array(
								SQ_SQUID_CACHE_SLASH_ALLOW	=> translate('true'),
								SQ_SQUID_CACHE_SLASH_DENY	=> translate('false'),
							 );
				combo_box('prefs[SQ_SQUID_CACHE_SLASH]', $selection, FALSE, $val);
			} else {
				?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $val ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
			}
			$o->note(translate('squid_tool_slash_note'));
			if (is_null($pref)) $o->closeField();
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_URL_PORT') {
			// additional options to be included in the command, can be left blank
			if (is_null($pref)) {
				$o->openField(translate('squid_tool_url_port_field'));
			}
			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_SQUID_URL_PORT]', $this->pref_vars['SQ_SQUID_URL_PORT']['default'], 10);
				$o->note(translate('squid_tool_url_port_note'));
			} else {
				echo nl2br($this->pref_vars['SQ_SQUID_URL_PORT']['default']);
			}
			if (is_null($pref)) $o->closeField();
		}

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_HOSTNAMES') {
			if (isset($_POST['prefs']['SQ_SQUID_CACHE_HOSTNAMES'])) {
				$this->pref_vars['SQ_SQUID_CACHE_HOSTNAMES']['default'] = strtolower($_POST['prefs']['SQ_SQUID_CACHE_HOSTNAMES']);
			}
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_PATH') {
			if (isset($_POST['prefs']['SQ_SQUID_CACHE_PATH'])) {
				$this->pref_vars['SQ_SQUID_CACHE_PATH']['default'] = $_POST['prefs']['SQ_SQUID_CACHE_PATH'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_PORT') {
			if (isset($_POST['prefs']['SQ_SQUID_CACHE_PORT'])) {
				$this->pref_vars['SQ_SQUID_CACHE_PORT']['default'] = $_POST['prefs']['SQ_SQUID_CACHE_PORT'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_OPTION') {
			if (isset($_POST['prefs']['SQ_SQUID_CACHE_OPTION'])) {
				$this->pref_vars['SQ_SQUID_CACHE_OPTION']['default'] = $_POST['prefs']['SQ_SQUID_CACHE_OPTION'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_CACHE_SLASH') {
			if (isset($_POST['prefs']['SQ_SQUID_CACHE_SLASH'])) {
				$this->pref_vars['SQ_SQUID_CACHE_SLASH']['default'] = $_POST['prefs']['SQ_SQUID_CACHE_SLASH'];
			}
		}

		if (is_null($pref) || $pref == 'SQ_SQUID_URL_PORT') {
			if (isset($_POST['prefs']['SQ_SQUID_URL_PORT'])) {
				$this->pref_vars['SQ_SQUID_URL_PORT']['default'] = $_POST['prefs']['SQ_SQUID_URL_PORT'];
			}
		}

		return TRUE;

	}//end processBackend()


}//end class

?>

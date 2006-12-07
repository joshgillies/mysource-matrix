<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: tool_clear_squid_cache_prefs.inc,v 1.2 2006/12/07 02:54:14 bcaldwell Exp $
*
*/

require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Tool_Clear_Squid_Cache_Prefs
*
* Purpose
*
*    List of host names to be used when purging the Squid Cache in the Global Prefs
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix
* @subpackage squid
*/
class Tool_Clear_Squid_Cache_Prefs extends Prefs
{

	var $pref_vars = Array(
						'SQ_SQUID_CACHE_HOSTNAMES'	=> Array(
														'name'			=> 'Squid Cache Host Names',
														'description'	=> 'A list of newline separated host names to be used when purging the Squid Cache',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_PATH'		=> Array(
														'name'			=> 'Path to squidclient',
														'description'	=> 'The path value to the squidclient',
														'default'		=> '/usr/bin/squidclient',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_PORT'		=> Array(
														'name'			=> 'Port Number',
														'description'	=> 'A list of newline separated host names to be used when purging the Squid Cache',
														'default'		=> '3128',
														'protected'		=> FALSE,
													   ),
						'SQ_SQUID_CACHE_OPTION'		=> Array(
														'name'			=> 'Additional Options',
														'description'	=> 'Additonal options to be included in the purge command',
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
			if (is_null($pref)) $o->openField('Host Names');
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

		return TRUE;

	}//end processBackend()


}//end class

?>
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
* $Id: file_prefs.inc,v 1.8.2.1 2006/03/16 23:17:04 rong Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Content_Type_WYSIWYG_Prefs
*
* Purpose
*
*    Looks after the creation the WYSIWYG preferences
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8.2.1 $
* @package MySource_Matrix
*/
class File_Prefs extends Prefs
{

	var $pref_vars = Array(
						'SQ_FILE_MAX_SIZE'	=> Array(
												'name'			=> 'Max File Size',
												'description'	=> 'This preference allows you to change the maximum size of files that can be uploaded',
												'default'		=> 0,
												'protected'		=> FALSE,
											   ),
					 );


	/**
	* Constructor
	*
	*/
	function File_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the Backend Outputter object
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

		if (is_null($pref) || $pref == 'SQ_FILE_MAX_SIZE') {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			// work out the max file size that PHP is allowing
			$ini_size = strtolower(ini_get('upload_max_filesize'));
			if (substr($ini_size, -1) == 'k') {
				$ini_size = $ini_size * 1024;
			} else if (substr($ini_size, -1) == 'm') {
				$ini_size = $ini_size * 1024 * 1024;
			}

			// work out the max post size that PHP is allowing
			$post_size = strtolower(ini_get('post_max_size'));
			if (substr($post_size, -1) == 'k') {
				$post_size = $post_size * 1024;
			} else if (substr($post_size, -1) == 'm') {
				$post_size = $post_size * 1024 * 1024;
			}

			$mem_limit_size = strtolower(ini_get('memory_limit'));
			if (substr($mem_limit_size, -1) == 'k') {
				$mem_limit_size = $mem_limit_size * 1024;
			} else if (substr($mem_limit_size, -1) == 'm') {
				$mem_limit_size = $mem_limit_size * 1024 * 1024;
			}
			$php_max_file_size = min($ini_size, $post_size, $mem_limit_size);

			if (is_null($pref)) {
				$o->openField(translate('max_file_upload'));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_MAX_SIZE]', $this->pref_vars['SQ_FILE_MAX_SIZE']['default'], 10);
				echo $o->note(translate('warn_max_settable_file_size', '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>'));
			} else {
				if (!$this->pref_vars['SQ_FILE_MAX_SIZE']['default']) {
					echo translate('warn_set_max_file_size', '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>');
				} else {
					$max_file_size = strtolower($this->pref_vars['SQ_FILE_MAX_SIZE']['default']);
					if (substr($max_file_size, -1) == 'k') {
						$max_file_size = $max_file_size * 1024;
					} else if (substr($max_file_size, -1) == 'm') {
						$max_file_size = $max_file_size * 1024 * 1024;
					}

					if ($max_file_size > $php_max_file_size) {
						echo translate('file_size_above_limit', '<b>'.strtoupper(easy_filesize($max_file_size)).'</b>', '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>');
					} else {
						echo strtoupper(easy_filesize($max_file_size));
					}
				}
			}
			if (is_null($pref)) $o->closeField();

		}//end if

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the Backend Outputter object
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

		if (is_null($pref) || $pref == 'SQ_FILE_MAX_SIZE') {
			if (isset($_POST['prefs']['SQ_FILE_MAX_SIZE'])) {
				$max_size = strtoupper(trim($_POST['prefs']['SQ_FILE_MAX_SIZE']));
				if (empty($max_size)) $max_size = 0;

				if (!empty($max_size) && !preg_match('|^[0-9]+[KM]$|', $max_size)) {
					trigger_localised_error('CORE0002', E_USER_WARNING, $max_size);
				} else {
					$this->pref_vars['SQ_FILE_MAX_SIZE']['default'] = $max_size;
				}
			}
		}

		return TRUE;

	}//end processBackend()


	/**
	* Merges two preference arrays for a content type
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		$merged_prefs = Array();
		$pref_vars = Array('SQ_FILE_MAX_SIZE');

		foreach ($pref_vars as $var_name) {
			if (isset($prefs1[$var_name]) && !isset($prefs2[$var_name])) {
				// this is only set in prefs1
				$merged_prefs[$var_name] = $prefs1[$var_name];
			} else if (isset($prefs2[$var_name]) && !isset($prefs1[$var_name])) {
				// this is only set in prefs2
				$merged_prefs[$var_name] = $prefs2[$var_name];
			} else if (isset($prefs1[$var_name]) && isset($prefs2[$var_name])) {
				// this is set in both prefs
				switch ($var_name) {
					case 'SQ_FILE_MAX_SIZE' :
						// use the maximum allowed file size
						$pref_1_size = strtolower($prefs1[$var_name]['default']);
						if (substr($pref_1_size, -1) == 'k') {
							$pref_1_size = $pref_1_size * 1024;
						} else if (substr($pref_1_size, -1) == 'm') {
							$pref_1_size = $pref_1_size * 1024 * 1024;
						}

						$pref_2_size = strtolower($prefs2[$var_name]['default']);
						if (substr($pref_1_size, -1) == 'k') {
							$pref_1_size = $pref_1_size * 1024;
						} else if (substr($pref_1_size, -1) == 'm') {
							$pref_1_size = $pref_1_size * 1024 * 1024;
						}

						if ($pref_1_size > $pref_2_size) {
							$merged_prefs[$var_name] = $prefs1[$var_name];
						} else {
							$merged_prefs[$var_name] = $prefs2[$var_name];
						}
					break;
				}

				$merged_prefs[$var_name]['name']        = $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description'] = $prefs1[$var_name]['description'];
				$merged_prefs[$var_name]['protected']   = ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
			}

		}//end foreach

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>
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
* $Id: file_prefs.inc,v 1.1 2004/11/29 02:12:40 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class File_Prefs extends Prefs
{

	var $pref_vars = Array(
		'SQ_FILE_MAX_SIZE'   => Array(	'name'			=> 'Max File Size',
										'description'	=> 'This preference allows you to change the maximum size of files that can be uploaded',
										'default'		=> 0,
										'protected'		=> false
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
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=null)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_FILE_MAX_SIZE') {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			
			// work out the max file size that PHP is allowing
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
			$php_max_file_size = min($post_size, $mem_limit_size);

			if (is_null($pref)) $o->openField('Max File Upload');

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_MAX_SIZE]', $this->pref_vars['SQ_FILE_MAX_SIZE']['default'], 10);
				echo "<p>The maximum file size can be entered in shorthand notation: 'K' for kilobytes, 'M' for megabytes. The maximum file size allowed by PHP on this sytem is <b>".strtoupper(easy_filesize($php_max_file_size))."</b></p>";
			} else {
				if (!$this->pref_vars['SQ_FILE_MAX_SIZE']['default']) {
					echo 'You have not set a maximum file upload size. The current PHP limit of <b>'.strtoupper(easy_filesize($php_max_file_size)).'</b> will be applied to all file uploads.';
				} else {
					$max_file_size = strtolower($this->pref_vars['SQ_FILE_MAX_SIZE']['default']);
					if (substr($max_file_size, -1) == 'k') {
						$max_file_size = $max_file_size * 1024;
					} else if (substr($max_file_size, -1) == 'm') {
						$max_file_size = $max_file_size * 1024 * 1024;
					}

					if ($max_file_size > $php_max_file_size) {
						echo 'The current setting of <b>'.strtoupper(easy_filesize($max_file_size)).'</b> is greater than than the PHP limit of <b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>. The PHP limit will be applied to all file uploads.';
					} else {
						echo strtoupper(easy_filesize($max_file_size));
					}
				}
			}
			if (is_null($pref)) $o->closeField();
		}

		return true;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=null)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return false;

		if (is_null($pref) || $pref == 'SQ_FILE_MAX_SIZE') {
			if (isset($_POST['prefs']['SQ_FILE_MAX_SIZE'])) {
				$max_size = strtoupper(trim($_POST['prefs']['SQ_FILE_MAX_SIZE']));
				if (empty($max_size)) $max_size = 0;

				if (!empty($max_size) && !preg_match('|^[0-9]+[KM]$|', $max_size)) {
					trigger_error('"'.$max_size.'" is not a valid file size', E_USER_WARNING);
				} else {
					$this->pref_vars['SQ_FILE_MAX_SIZE']['default'] = $max_size;
				}
			}
		}
	
		return true;

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

		foreach ($this->pref_vars as $var_name => $var_data) {
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

						if ($pref_1_size > $pref_2_size) $merged_prefs[$var_name] = $prefs1[$var_name];
						else $merged_prefs[$var_name] = $prefs2[$var_name];
					break;
				}

				$merged_prefs[$var_name]['name']        = $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description'] = $prefs1[$var_name]['description'];
				$merged_prefs[$var_name]['protected']   = ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
			}
		}

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>
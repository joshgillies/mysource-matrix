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
* $Id: file_prefs.inc,v 1.19 2012/08/30 01:09:07 ewang Exp $
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
* @version $Revision: 1.19 $
* @package MySource_Matrix
*/
class File_Prefs extends Prefs
{

	var $pref_vars = Array(
						'SQ_FILE_MAX_SIZE'					=> Array(
																'name'			=> 'max_file_upload',
																'description'	=> 'This preference allows you to change the maximum size of files that can be uploaded',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_PREUPLOADED_MAX_SIZE'		=> Array(
																'name'			=> 'preuploaded_max_file_upload',
																'description'	=> 'This preference allows you to change the maximum size of files that can be created from pre-uploaded ones',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOW_NO_EXTENSION'		=> Array(
																'name'			=> 'allow_file_with_no_extension',
																'description'	=> 'This preference allows you to use file with no extension.',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOWED_TYPES'				=> Array(
																'name'			=> 'allowed_file_extensions_to_upload',
																'description'	=> 'This preference allows you to restrict the allowed types of files to upload',
																'default'		=> 'jpg,jpeg,gif,png,zip,tar,rar,mov,mkv,avi,flv,fla,psd,css,js,txt,html,pdf,xls,rtf,doc,docx,ppt',
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOWED_PREUPLOADED_TYPES'	=> Array(
																'name'			=> 'allowed_file_extentions_for_pre_uploaded_files',
																'description'	=> 'This preference allows you to restrict the allowed types of pre-uploaded files',
																'default'		=> 'jpg,jpeg,gif,png,zip,tar,rar,mov,mkv,avi,flv,fla,psd,css,js,txt,html,pdf,xls,rtf,doc,docx,ppt',
																'protected'		=> FALSE,
															   ),

						'SQ_FILE_ASSET_SUMMARY'				=> Array(
																'name'			=> 'file_asset_summary',
																'description'	=> 'This preference allows user to customize the content for asset_summary_X keyword replacement',
																'default'		=> '%file_type^uppercase%, %asset_file_size_in_bytes^divide:1024^round:2% KB',
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

		// Max File Size Upload
		if (is_null($pref) || $pref == 'SQ_FILE_MAX_SIZE') {
			if (is_null($pref)) {
				$o->openField(translate( $this->pref_vars['SQ_FILE_MAX_SIZE']['name']));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_MAX_SIZE]', $this->pref_vars['SQ_FILE_MAX_SIZE']['default'], 10);
				echo $o->note(sprintf(translate('warn_max_settable_file_size'), '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>'));
				if ($ini_size > $post_size) {
					echo $o->note(sprintf(translate('warn_max_settable_file_size_warning'), '<b>'.strtoupper(easy_filesize($post_size)).'</b>'));
				}
			} else {
				if (!$this->pref_vars['SQ_FILE_MAX_SIZE']['default']) {
					echo sprintf(translate('warn_set_max_file_size'), '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>');
				} else {
					$max_file_size = strtolower($this->pref_vars['SQ_FILE_MAX_SIZE']['default']);
					if (substr($max_file_size, -1) == 'k') {
						$max_file_size = $max_file_size * 1024;
					} else if (substr($max_file_size, -1) == 'm') {
						$max_file_size = $max_file_size * 1024 * 1024;
					}

					if ($max_file_size > $php_max_file_size) {
						echo sprintf(translate('file_size_above_limit'), '<b>'.strtoupper(easy_filesize($max_file_size)).'</b>', '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>');
					} else {
						echo strtoupper(easy_filesize($max_file_size));
					}
					if ($ini_size > $post_size) {
						echo $o->note(sprintf(translate('warn_max_settable_file_size_warning'), '<b>'.strtoupper(easy_filesize($post_size)).'</b>'));
					}
				}
			}
			if (is_null($pref)) $o->closeField();
		}

		// Max Pre-uploaded File Size
		if (is_null($pref) || $pref == 'SQ_FILE_PREUPLOADED_MAX_SIZE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['name']));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_PREUPLOADED_MAX_SIZE]', $this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['default'], 10);
				echo $o->note(sprintf(translate('warn_preuploaded_max_settable_file_size'), '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>'));
			} else {
				if (!$this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['default']) {
					echo sprintf(translate('warn_preuploaded_max_settable_file_size'), '<b>'.strtoupper(easy_filesize($php_max_file_size)).'</b>');
				} else {
					$max_file_size = strtolower($this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['default']);
					if (substr($max_file_size, -1) == 'k') {
						$max_file_size = $max_file_size * 1024;
					} else if (substr($max_file_size, -1) == 'm') {
						$max_file_size = $max_file_size * 1024 * 1024;
					}
					echo strtoupper(easy_filesize($max_file_size));
				}
			}
			if (is_null($pref)) $o->closeField();
		}

		// Allow No Extension
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOW_NO_EXTENSION') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_FILE_ALLOW_NO_EXTENSION']['name']));
			}

			$value = $this->pref_vars['SQ_FILE_ALLOW_NO_EXTENSION']['default'];
			if ($have_lock && $is_admin) {
				$options = Array(1 => 'Yes', 0 => 'No');
				combo_box('prefs[SQ_FILE_ALLOW_NO_EXTENSION]', $options, FALSE, $value);
			} else {
				echo ($value) ? translate('yes') : translate('no');
			}
			if (is_null($pref)) $o->closeField();
		}

		// Allowed File Extensions
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOWED_TYPES') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_FILE_ALLOWED_TYPES']['name']));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_ALLOWED_TYPES]', $this->pref_vars['SQ_FILE_ALLOWED_TYPES']['default'], 20);
				echo $o->note(translate('allowed_file_extensions_note'));
			} else {
				$value = $this->pref_vars['SQ_FILE_ALLOWED_TYPES']['default'];
				if (empty($value)) {
					echo translate('allowed_file_extensions_not_specified');
				} else {
					$exts = explode(',', $value);
					$num_exts = count($exts);
					if ($num_exts == 1) {
						$ext_str = array_pop($exts);
					} else if ($num_exts > 1) {
						$ext_str = implode(', ', array_slice($exts, 0, -1)).' and '.$exts[$num_exts - 1];
					}
					echo sprintf(translate('allowed_file_extensions_list'), (($num_exts > 1) ? 's are ' : ' is ').$ext_str);
				}
			}
			if (is_null($pref)) $o->closeField();
		}

		// Allowed File Extensions for Pre-uploaded
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['name']));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_FILE_ALLOWED_PREUPLOADED_TYPES]', $this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['default'], 20);
				echo $o->note(translate('allowed_file_extensions_note'));
			} else {
				$value = $this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['default'];
				if (empty($value)) {
					echo translate('allowed_file_extensions_not_specified');
				} else {
					$exts = explode(',', $value);
					$num_exts = count($exts);
					if ($num_exts == 1) {
						$ext_str = array_pop($exts);
					} else if ($num_exts > 1) {
						$ext_str = implode(', ', array_slice($exts, 0, -1)).' and '.$exts[$num_exts - 1];
					}
					echo sprintf(translate('allowed_file_extensions_list'), (($num_exts > 1) ? 's are ' : ' is ').$ext_str);
				}
			}
			if (is_null($pref)) $o->closeField();
		}

		// File summary defination
		if (is_null($pref) || $pref == 'SQ_FILE_ASSET_SUMMARY') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_FILE_ASSET_SUMMARY']['name']));
			}

			if ($have_lock && $is_admin) {
				text_area('prefs[SQ_FILE_ASSET_SUMMARY]', $this->pref_vars['SQ_FILE_ASSET_SUMMARY']['default'], 60, 5);
				echo $o->note(translate('file_asset_summary_note'));
			} else {
				$value = $this->pref_vars['SQ_FILE_ASSET_SUMMARY']['default'];
				echo $value;
			}
			if (is_null($pref)) $o->closeField();
		}

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

		// Max File Size Upload
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

		// Max Pre-uploaded File Size
		if (is_null($pref) || $pref == 'SQ_FILE_PREUPLOADED_MAX_SIZE') {
			if (isset($_POST['prefs']['SQ_FILE_PREUPLOADED_MAX_SIZE'])) {
				$max_size = strtoupper(trim($_POST['prefs']['SQ_FILE_PREUPLOADED_MAX_SIZE']));
				if (empty($max_size)) $max_size = 0;

				if (!empty($max_size) && !preg_match('|^[0-9]+[KM]$|', $max_size)) {
					trigger_localised_error('CORE0002', E_USER_WARNING, $max_size);
				} else {
					$this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['default'] = $max_size;
				}
			}
		}

		// Allow No Extension
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOW_NO_EXTENSION') {
			if (isset($_POST['prefs']['SQ_FILE_ALLOW_NO_EXTENSION'])) {
				$allow = strtoupper(trim($_POST['prefs']['SQ_FILE_ALLOW_NO_EXTENSION']));
				$this->pref_vars['SQ_FILE_ALLOW_NO_EXTENSION']['default'] = $allow;
			}
		}

		// Allowed File Extensions
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOWED_TYPES') {
			if (isset($_POST['prefs']['SQ_FILE_ALLOWED_TYPES'])) {
				$result = $this->_tidyFileTypeString($_POST['prefs']['SQ_FILE_ALLOWED_TYPES']);
				$this->pref_vars['SQ_FILE_ALLOWED_TYPES']['default'] = $result;
			}
		}

		// Allowed File Extensions for Pre-uploaded
		if (is_null($pref) || $pref == 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES') {
			if (isset($_POST['prefs']['SQ_FILE_ALLOWED_PREUPLOADED_TYPES'])) {
				$result = $this->_tidyFileTypeString($_POST['prefs']['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']);
				$this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['default'] = $result;
			}
		}

		// File summary defination
		if (is_null($pref) || $pref == 'SQ_FILE_ASSET_SUMMARY') {
			if (isset($_POST['prefs']['SQ_FILE_ASSET_SUMMARY'])) {
				$this->pref_vars['SQ_FILE_ASSET_SUMMARY']['default'] = $_POST['prefs']['SQ_FILE_ASSET_SUMMARY'];
			}
		}

		return TRUE;

	}//end processBackend()


	/**
	* Clean up the user input string for file types
	*
	* Never trust user input, let's get our hands dirty here.
	*
	* @param string	$str	input string
	*
	* @return string
	* @access private
	*/
	function _tidyFileTypeString($str)
	{
		$str = str_replace(' ', '', strtolower(trim($str)));
		if (empty($str)) return '';

		if (substr($str, -1) == ',') {
			$str = substr($str, 0, -1);
		}

		if (strpos($str, ',') !== FALSE) {
			$tmp = explode(',', $str);
			$new_tokens = Array();
			foreach ($tmp as $token) {
				if (!empty($token)) $new_tokens[] = $token;
			}
		} else {
			return $str;
		}

		if (empty($new_tokens)) {
			return '';
		} else {
			$new_tokens = array_unique($new_tokens);
		}
		return implode(',', $new_tokens);

	}//end _tidyFileTypeString()


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
		$pref_vars = Array(
						'SQ_FILE_MAX_SIZE',
						'SQ_FILE_PREUPLOADED_MAX_SIZE',
						'SQ_FILE_ALLOW_NO_EXTENSION',
						'SQ_FILE_ALLOWED_TYPES',
						'SQ_FILE_ALLOWED_PREUPLOADED_TYPES',
					 );

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
					case 'SQ_FILE_PREUPLOADED_MAX_SIZE' :
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

						// Go for smaller size
						if ($pref_1_size > $pref_2_size) {
							$merged_prefs[$var_name] = $prefs1[$var_name];
						} else {
							$merged_prefs[$var_name] = $prefs2[$var_name];
						}
					break;
					case 'SQ_FILE_ALLOW_NO_EXTENSION' :
						$pref_1_val = $prefs1[$var_name]['default'];
						$pref_2_val = $prefs2[$var_name]['default'];
						$merged_prefs[$var_name]['default'] = ($pref_1_val || $pref_2_val);
					break;
					case 'SQ_FILE_ALLOWED_TYPES' :
					case 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES' :
						if (is_array($prefs1[$var_name]) && isset($prefs1[$var_name]['default'])) {
							$pref_1_exts_str = $prefs1[$var_name]['default'];
						} else {
							$pref_1_exts_str = $prefs1[$var_name];
						}
						if (is_array($prefs2[$var_name]) && isset($prefs2[$var_name]['default'])) {
							$pref_2_exts_str = $prefs2[$var_name]['default'];
						} else {
							$pref_2_exts_str = $prefs2[$var_name];
						}
						$pref_merge_str = $pref_1_exts_str.','.$pref_2_exts_str;
						$merged_prefs[$var_name]['default'] = implode(',', array_unique(explode(',', $pref_merge_str)));
					break;
				}//end switch

				$merged_prefs[$var_name]['name']        = $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description'] = $prefs1[$var_name]['description'];

				if (isset($prefs2[$var_name]['protected'])) {
					$merged_prefs[$var_name]['protected'] = ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
				} else {
					$merged_prefs[$var_name]['protected'] = $prefs1[$var_name]['protected'];
				}
			}//end if

		}//end foreach

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>

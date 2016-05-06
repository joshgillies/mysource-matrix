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
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_PREUPLOADED_MAX_SIZE'		=> Array(
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOW_NO_EXTENSION'		=> Array(
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOWED_TYPES'				=> Array(
																'default'		=> 'jpg,jpeg,gif,png,zip,tar,rar,mov,mkv,avi,flv,fla,psd,css,js,txt,html,pdf,xls,rtf,doc,docx,ppt',
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ALLOWED_PREUPLOADED_TYPES'	=> Array(
																'default'		=> 'jpg,jpeg,gif,png,zip,tar,rar,mov,mkv,avi,flv,fla,psd,css,js,txt,html,pdf,xls,rtf,doc,docx,ppt',
																'protected'		=> FALSE,
															   ),
						'SQ_FILE_ASSET_SUMMARY'				=> Array(
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
        require_once SQ_FUDGE_PATH.'/general/file_system.inc';    
		$this->Prefs($pref_file);

		// work out the max file size that PHP is allowing
		$file_sizes = $this->_getMaxFileSizes();
		$this->pref_vars['SQ_FILE_MAX_SIZE']['name'] = translate('Max File Upload');
		$this->pref_vars['SQ_FILE_MAX_SIZE']['desc'] = sprintf(
		    translate('The maximum file size can be entered in shorthand notation: \'K\' for kilobytes, \'M\' for megabytes. The maximum file size allowed by PHP on this system is "<strong>%s</strong>"'),
		    strtoupper(easy_filesize($file_sizes['effective_max_size']))
		);

		$this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['name'] = translate('Pre-uploaded Max File Size');
		$this->pref_vars['SQ_FILE_PREUPLOADED_MAX_SIZE']['desc'] = translate('The maximum file size can be created from the pre-uploaded files. This can be entered in shorthand notation: \'K\' for kilobytes, \'M\' for megabytes. When the value is zero, this option will be ignored.');

		$this->pref_vars['SQ_FILE_ALLOW_NO_EXTENSION']['name'] = translate('Allow Files with No Extension?');

		$this->pref_vars['SQ_FILE_ALLOWED_TYPES']['name'] = translate('Allowed File Extensions to Upload');
		$this->pref_vars['SQ_FILE_ALLOWED_TYPES']['desc'] = translate('Type the allowed file extensions, separated by commas. For example: jpg,png,txt,mov');

		$this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['name'] = translate('Allowed File Extensions for Pre-Uploaded files');
		$this->pref_vars['SQ_FILE_ALLOWED_PREUPLOADED_TYPES']['desc'] = translate('Type the allowed file extensions, separated by commas. For example: jpg,png,txt,mov');

		$this->pref_vars['SQ_FILE_ASSET_SUMMARY']['name'] = translate('File Summary Format');
		$this->pref_vars['SQ_FILE_ASSET_SUMMARY']['desc'] = translate('File summary format for \'asset_summary_X\' keyword replacement');

	}//end constructor


	private function _getMaxFileSizes() {
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

		return Array(
		    'upload_max_filesize' => $ini_size,
		    'post_max_size'       => $post_size,
		    'memory_limit'        => $mem_limit_size,
		    'effective_max_size'  => $php_max_file_size,
		);

	}//end _getMaxFileSizes()


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
		$ini_sizes = $this->_getMaxFileSizes();

		$this->paintTextField($o, $have_lock, $pref, 'SQ_FILE_MAX_SIZE', 10, 1, function($max_file_size) use ($ini_sizes) {
            if ((int) $max_file_size === 0) {
                return sprintf(
                    translate('You have not set a maximum file upload size. The current PHP limit of <strong>%s</strong> will be applied to all file uploads.'),
                    strtoupper(easy_filesize($ini_sizes['effective_max_size']))
                );
            } else {
                if (substr($max_file_size, -1) == 'k') {
                    $max_file_size = $max_file_size * 1024;
                } else if (substr($max_file_size, -1) == 'm') {
                    $max_file_size = $max_file_size * 1024 * 1024;
                }

                if ($max_file_size > $ini_sizes['effective_max_size']) {
                    return sprintf(
                        translate('The current setting of <strong>%1$s</strong> is greater than than the PHP limit of <strong>%2$s</strong>. The PHP limit will be applied to all file uploads.'),
                        strtoupper(easy_filesize($max_file_size)),
                        strtoupper(easy_filesize($ini_sizes['effective_max_size']))
                    );
                } else {
                    return strtoupper(easy_filesize($max_file_size));
                }
            }
        });

        // If the maximum POST size is too small, throw a message about it
        if ($ini_sizes['upload_max_filesize'] > $ini_sizes['post_max_size']) {
            $o->openField('');
            echo '<div class="sq-warning-message">';
            echo sprintf(
                '<strong>'.translate('Warning: Your PHP settings relating to uploads appear to be misconfigured.').'</strong><br/>'.
                    translate('The setting \'max_post_size\' (<strong>%s</strong>) is smaller than \'upload_max_filesize\' (<strong>%s</strong>).').' '.
                    translate('The \'max_post_size\' setting should be the larger of the two - otherwise silent errors may be introduced, as PHP will blank out POST data if an uploaded file is larger than the \'max_post_size\' setting.'),
                strtoupper(easy_filesize($ini_sizes['post_max_size'])),
                strtoupper(easy_filesize($ini_sizes['upload_max_filesize']))
            );
            echo '</div>';
            $o->closeField();
        }

        $this->paintTextField($o, $have_lock, $pref, 'SQ_FILE_PREUPLOADED_MAX_SIZE', 20, 1, function($max_file_size) {
            $max_file_size = strtolower($max_file_size);
            if ((int) $max_file_size === 0) {
                return translate('Unlimited');
            } else {
                if (substr($max_file_size, -1) == 'k') {
                    $max_file_size = (int) $max_file_size * 1024;
                } else if (substr($max_file_size, -1) == 'm') {
                    $max_file_size = (int) $max_file_size * 1024 * 1024;
                }
                return strtoupper(easy_filesize($max_file_size));
            }
        });

        // Allow File assets with no extension?
        $options = Array(
                       1 => translate('Yes'),
                       0 => translate('No'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_FILE_ALLOW_NO_EXTENSION', $options);

        // Allowed file extensions - for uploaded and pre-uploaded files.
        $restrictionFn = function($value) {
            if (empty($value)) {
                echo translate('No file type restriction.');
            } else {
                $exts = explode(',', $value);
                $num_exts = count($exts);
                if ($num_exts == 1) {
                    $ext_str = array_pop($exts);
                } else if ($num_exts > 1) {
                    $ext_str = implode(', ', array_slice($exts, 0, -1)).' and '.$exts[$num_exts - 1];
                }

                // TODO: Translate plural.
                if ($num_exts > 1) {
                    return sprintf(translate('Allowed extensions are %s'), $ext_str);
                } else {
                    return sprintf(translate('Allowed extension is %s'), $ext_str);
                }
            }
        };
        $this->paintTextField($o, $have_lock, $pref, 'SQ_FILE_ALLOWED_TYPES', 20, 1, $restrictionFn);
        $this->paintTextField($o, $have_lock, $pref, 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES', 20, 1, $restrictionFn);
        $this->paintTextField($o, $have_lock, $pref, 'SQ_FILE_ASSET_SUMMARY', 60, 5);

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
					trigger_localised_error('CORE0002', sprintf(translate('"%1$s" is not a valid file size'), $max_size), E_USER_WARNING);
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
					trigger_localised_error('CORE0002', sprintf(translate('"%1$s" is not a valid file size'), $max_size), E_USER_WARNING);
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
				if (is_array($prefs1[$var_name]) && isset($prefs1[$var_name]['default'])) {
					$pref1_def_value = $prefs1[$var_name]['default'];
				} else {
					$pref1_def_value = $prefs1[$var_name];
				}
				if (is_array($prefs2[$var_name]) && isset($prefs2[$var_name]['default'])) {
					$pref2_def_value = $prefs2[$var_name]['default'];
				} else {
					$pref2_def_value = $prefs2[$var_name];
				}

				switch ($var_name) {
					case 'SQ_FILE_MAX_SIZE' :
					case 'SQ_FILE_PREUPLOADED_MAX_SIZE' :
						// use the maximum allowed file size
						$pref_1_size = strtolower($pref1_def_value);

						if (substr($pref_1_size, -1) == 'k') {
							$pref_1_size = $pref_1_size * 1024;
						} else if (substr($pref_1_size, -1) == 'm') {
							$pref_1_size = $pref_1_size * 1024 * 1024;
						}

						$pref_2_size = strtolower($pref2_def_value);
						if (substr($pref_2_size, -1) == 'k') {
							$pref_2_size = $pref_2_size * 1024;
						} else if (substr($pref_2_size, -1) == 'm') {
							$pref_2_size = $pref_2_size * 1024 * 1024;
						}

						// Go for smaller size
						if ($pref_1_size > $pref_2_size) {
							$merged_prefs[$var_name]['default'] = $prefs1[$var_name];
						} else {
							$merged_prefs[$var_name]['default'] = $prefs2[$var_name];
						}
					break;
					case 'SQ_FILE_ALLOW_NO_EXTENSION' :
						$merged_prefs[$var_name]['default'] = ($pref1_def_value || $pref1_def_value);
					break;
					case 'SQ_FILE_ALLOWED_TYPES' :
					case 'SQ_FILE_ALLOWED_PREUPLOADED_TYPES' :
						$pref_1_exts_str = $pref1_def_value;
						$pref_2_exts_str = $pref2_def_value;

						$pref_merge_str = $pref_1_exts_str.','.$pref_2_exts_str;
						$merged_prefs[$var_name]['default'] = implode(',', array_unique(explode(',', $pref_merge_str)));
					break;
				}//end switch

				$merged_prefs[$var_name]['name']        = $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description'] = isset($prefs1[$var_name]['description']) ? $prefs1[$var_name]['description'] : '';

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

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
* $Id: external_tools_config.inc,v 1.46 2013/08/19 04:28:03 cupreti Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';

/**
* System_Config
*
* Purpose
*
*    Looks after the creation and editing of the location of external tools
*    e.g. HTML Tidy, PDFtoHTML, Antiword, etc.
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.46 $
* @package MySource_Matrix
*/
class External_Tools_Config extends Config
{


	/**
	* Holds all the vars for this that config vars for this config
	* In the form : Array([var name] => Array('editable' => [boolean], 'default' => [mixed scalar]))
	* @var Array()
	*/
	var $config_vars = Array(
						'SQ_TOOL_HTML_TIDY_PATH'				=> Array('editable' => 1, 'default' => '/usr/bin/tidy'),
						'SQ_TOOL_HTML_TIDY_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL'	=> Array('editable' => 1, 'default' => 0),
						'SQ_TOOL_HTML_TIDY_HTML_STANDARD'		=> Array('editable' => 1, 'default' => 'HTML_4.01_Transitional'),
						'SQ_TOOL_PDFTOHTML_PATH'				=> Array('editable' => 1, 'default' => '/usr/bin/pdftohtml'),
						'SQ_TOOL_PDFTOHTML_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_ANTIWORD_PATH'					=> Array('editable' => 1, 'default' => '/usr/bin/antiword'),
						'SQ_TOOL_ANTIWORD_MAPPING_PATH'			=> Array('editable' => 1, 'default' => '/usr/share/antiword'),
						'SQ_TOOL_ANTIWORD_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_JPEG_TOOLKIT_PATH'				=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_JPEG_TOOLKIT_ENABLED'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_SPELL_CHECKER_LANG'			=> Array('editable' => 1, 'default' => 'en'),
						'SQ_TOOL_MARKDOWN_PATH'					=> Array('editable' => 1, 'default' => '/usr/bin/markdown.php'),
						'SQ_TOOL_MARKDOWN_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_MARKDOWNIFY_PATH'				=> Array('editable' => 1, 'default' => '/usr/bin/markdownify.php'),
						'SQ_TOOL_MARKDOWNIFY_ENABLED'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_GETID3_PATH'					=> Array('editable' => 1, 'default' => '/usr/share/php/getid3'),
						'SQ_TOOL_GETID3_ENABLED'				=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_VIRUS_CHECKER_PATH'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_VIRUS_CHECKER_ENABLED'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_VIRUS_CHECKER_TYPE'			=> Array('editable' => 1, 'default' => 'clam'),
						'SQ_TOOL_JS_PATH'						=> Array('editable' => 1, 'default' => '/usr/bin/js'),
						'SQ_TOOL_OGG_METADATA_EXTRACTION_PATH'	=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED'	=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_MOLLOM_PUBLIC_KEY'				=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_MOLLOM_PRIVATE_KEY'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_MOLLOM_TEST_MODE'				=> Array('editable' => 1, 'default' => '0'),
						'SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION'	=> Array('editable' => 1, 'default' => '1'),
						'SQ_TOOL_SQUID_CACHE_HOSTNAMES'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_SQUID_CACHE_PATH'			=> Array('editable' => 1, 'default' => '/usr/bin/squidclient'),
						'SQ_TOOL_SQUID_CACHE_PORT'			=> Array('editable' => 1, 'default' => '3128'),
						'SQ_TOOL_SQUID_CACHE_OPTION'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_SQUID_CACHE_SLASH'			=> Array('editable' => 1, 'default' => FALSE),
						'SQ_TOOL_SQUID_URL_PORT'				=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_SENDMAIL_PATH'				=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_SENDMAIL_ARGS'				=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_SIMPLESAMLPHP_PATH'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_RECAPTCHA_PUBLIC_KEY'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_RECAPTCHA_PRIVATE_KEY'			=> Array('editable' => 1, 'default' => ''),
				);


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/tools.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* return value indicates whether the file was written
	*
	* @param array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean	$send_message		whether to send a message using the messaging system
	*
	* @return boolean
	* @access public
	*/
	function save($vars, $backup_existing=FALSE, $send_message=TRUE)
	{
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, go away
			if (!$is_admin) {
				trigger_localised_error('SYS0267', E_USER_WARNING, $this->name);
				return FALSE;
			}
		}

		return parent::save($vars, $backup_existing, $send_message);

	}//end save()


	/**
	* Whether the current user can acquire the lock
	*
	* @return boolean
	* @access public
	* @see MySource::acquireLock()
	*/
	function canAcquireLock()
	{
		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end canAcquireLock()


	/**
	* Does the current user have writeAccess() to this config ?
	*
	* @return boolean
	* @access public
	*/
	function writeAccess()
	{
		if (!parent::writeAccess()) return FALSE;

		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return TRUE;
		}

		// need to be root or a sys admin
		return ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

	}//end writeAccess()


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	reference to the backend outputter
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

		$class = get_class_lower($this);

		$o->openSection(translate('active_external_tools'));

			$o->openField(translate('enable_html_tidy'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_HTML_TIDY_ENABLED);
			} else {
				echo (SQ_TOOL_HTML_TIDY_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('html_tidy_accessibility_level'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL]', Array('0', '1', '2', '3'), FALSE, SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL);
			} else {
				echo (SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL) ? SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL : 0;
			}

			$o->openField(translate('html_tidy_html_standard'), '', '');
			$html_standards = Array(
					'HTML_4.01_Transitional' =>'HTML 4.01 Transitional',
					'HTML_4.01_Strict' => 'HTML 4.01 Strict',
					'HTML_4.01_Frameset' => 'HTML 4.01 Frameset',
					'XHTML_1.0_Transitional' => 'XHTML 1.0 Transitional',
					'XHTML_1.0_Strict' => 'XHTML 1.0 Strict',
					'XHTML_1.0_Frameset' => 'XHTML 1.0 Frameset',
					'XHTML_1.1' => 'XHTML 1.1',
				);
			$html_standard_selected = (SQ_TOOL_HTML_TIDY_HTML_STANDARD) ? SQ_TOOL_HTML_TIDY_HTML_STANDARD : 'HTML_4.01_Transitional';
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_HTML_STANDARD]', $html_standards, FALSE, $html_standard_selected);
			} else {
				echo $html_standards[$html_standard_selected];
			}

			require_once SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
			$tidy_char_set = array_get_index($standards_lists_tidy_char_sets, SQ_CONF_DEFAULT_CHARACTER_SET);

			if (empty($tidy_char_set)) {
				$o->note('<span style="color: red;">'.translate('tidy_unsupported_char_set_warning').'</span>');
			}

			$o->openField(translate('enable_pdftohtml'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_PDFTOHTML_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_PDFTOHTML_ENABLED);
			} else {
				echo (SQ_TOOL_PDFTOHTML_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('enable_antiword'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_ANTIWORD_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_ANTIWORD_ENABLED);
			} else {
				echo (SQ_TOOL_ANTIWORD_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField('Enable Photoshop Metadata Extraction');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_JPEG_TOOLKIT_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_JPEG_TOOLKIT_ENABLED);
			} else {
				echo (SQ_TOOL_JPEG_TOOLKIT_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('enable_markdown'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_MARKDOWN_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_MARKDOWN_ENABLED);
			} else {
				echo (SQ_TOOL_MARKDOWN_ENABLED) ? translate('yes') : translate('no');
			}
			
			$o->openField(translate('enable_markdownify'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_MARKDOWNIFY_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_MARKDOWNIFY_ENABLED);
			} else {
				echo (SQ_TOOL_MARKDOWNIFY_ENABLED) ? translate('yes') : translate('no');
			}
			
			$o->openField(translate('enable_getid3_extraction'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_GETID3_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_GETID3_ENABLED);
			} else {
				echo (SQ_TOOL_GETID3_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('enable_ogg_metadata_extraction'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED);
			} else {
				echo (SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('enable_virus_checker'));
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_VIRUS_CHECKER_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_VIRUS_CHECKER_ENABLED);
			} else {
				echo (SQ_TOOL_VIRUS_CHECKER_ENABLED) ? translate('yes') : translate('no');
			}
			
		$o->closeSection();

		$o->openSection(translate('wysiwyg_editor_tools'));

			$o->openField(translate('path_to_html_tidy'));
				echo SQ_TOOL_HTML_TIDY_PATH;

				if (!is_file(SQ_TOOL_HTML_TIDY_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'HTML Tidy'); ?></span><?php
				} else if (!is_executable(SQ_TOOL_HTML_TIDY_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'HTML Tidy'); ?></span><?php
				}//end if

			$o->openField(translate('wysiwyg_spell_checker_pref_field'));
			if ($write_access) {
				// if spell checker is not installed, disable this pref
				text_box($class.'[SQ_TOOL_SPELL_CHECKER_LANG]', SQ_TOOL_SPELL_CHECKER_LANG, 6, 6);
				$o->note(translate('wysiwyg_spell_checker_pref_note'));
			} else {
				echo SQ_TOOL_SPELL_CHECKER_LANG;
			}

			if (!function_exists('pspell_suggest')) {
				?><br /><span class="sq-backend-warning"><?php echo translate('wysiwyg_pspell_not_installed'); ?></span><?php
			} else {
				$old_reporting = error_reporting(0);
					$status = pspell_new(SQ_TOOL_SPELL_CHECKER_LANG);
				error_reporting($old_reporting);
				if ($status === FALSE) {
					?><br /><span class="sq-backend-warning"><?php echo translate('wysiwyg_spell_checker_pref_error', SQ_TOOL_SPELL_CHECKER_LANG); ?></span><?php
				}
			}

		$o->closeSection();

		$o->openSection(translate('keyword_extraction_tools'));

			$o->openField(translate('path_to_pdftohtml'));
			echo SQ_TOOL_PDFTOHTML_PATH;


			if (!is_file(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'PDFTOHTML'); ?></span><?php
			} else if (!is_executable(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'PDFTOHTML'); ?></span><?php
			}

			$o->openField(translate('path_to_antiword'));
			echo SQ_TOOL_ANTIWORD_PATH;


			if (!is_file(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Antiword');?></span><?php
			} else if (!is_executable(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'Antiword'); ?></span><?php
			}

			$o->openField('Path to Antiword Mappings');
			echo SQ_TOOL_ANTIWORD_MAPPING_PATH;
			

			if (!is_dir(SQ_TOOL_ANTIWORD_MAPPING_PATH)) {
				?><br /><span class="sq-backend-warning">The path to Antiword Mappings is not valid</span><?php
			}

		$o->closeSection();


		$o->openSection('Photoshop Image Metadata Extraction');

			$o->openField('Path to PHP JPEG Metadata Toolkit');
			echo SQ_TOOL_JPEG_TOOLKIT_PATH;


			if (!is_dir(SQ_TOOL_JPEG_TOOLKIT_PATH)) {
				?><br /><span class="sq-backend-warning">Supplied path is not valid</span><?php
			}

		$o->closeSection();

		$o->openSection(translate('markdown_tools'));
		
			$o->openField(translate('path_to_markdown'));
			echo SQ_TOOL_MARKDOWN_PATH;


			if (!is_file(SQ_TOOL_MARKDOWN_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Markdown'); ?></span><?php
			} else {
				include_once SQ_TOOL_MARKDOWN_PATH;
				if (!function_exists('Markdown')) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_markdown_no_required_function'); ?></span><?php
				}
			}//end if

			$o->openField(translate('path_to_markdownify'));
			echo SQ_TOOL_MARKDOWNIFY_PATH;


			if (!is_file(SQ_TOOL_MARKDOWNIFY_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Markdownify'); ?></span><?php
			} else {
				include_once SQ_TOOL_MARKDOWNIFY_PATH;
				$class_name = basename(SQ_TOOL_MARKDOWNIFY_PATH, '.php');
				if (!method_exists($class_name, 'parseString')) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_markdownify_no_required_method', $class_name); ?></span><?php
				}
			}//end if
			
		$o->closeSection();
		
		$o->openSection(translate('getid3_extraction'));

			$o->openField(translate('path_to_getid3'));
			echo SQ_TOOL_GETID3_PATH;


			if (!is_dir(SQ_TOOL_GETID3_PATH)) {
				?><br /><span class="sq-backend-warning">Supplied path is not valid</span><?php
			}

		$o->closeSection();

		$o->openSection(translate('ogg_metadata_extraction'));

			$o->openField(translate('path_to_ogg_metadata_extraction'));
			echo SQ_TOOL_OGG_METADATA_EXTRACTION_PATH;


			$ogg_found = FALSE;
			if (is_file(SQ_TOOL_OGG_METADATA_EXTRACTION_PATH)) {
				include_once SQ_TOOL_OGG_METADATA_EXTRACTION_PATH;
            	$ogg_found = method_exists('Ogg', 'Ogg');
			}
			if (!$ogg_found) {
				?><br /><span class="sq-backend-warning">Supplied path is not valid</span><?php
			}

		$o->closeSection();

		$o->openSection(translate('virus_checker'));

			$o->openField(translate('virus_checker_type'));
				$av_types = Array(
								'clam'		=> translate('virus_checker_type_clam'),
								'f-prot'	=> translate('virus_checker_type_fprot'),
							);
				if ($write_access) {
					combo_box($class.'[SQ_TOOL_VIRUS_CHECKER_TYPE]', $av_types, FALSE, SQ_TOOL_VIRUS_CHECKER_TYPE);
				} else {
					switch (SQ_TOOL_VIRUS_CHECKER_TYPE) {
						case 'f-prot':
							echo translate('virus_checker_type_fprot');
							break;
						default:
							echo translate('virus_checker_type_clam');
					}//end switch
				}//end if

			$o->openField(translate('path_to_virus_checker'));
				echo SQ_TOOL_VIRUS_CHECKER_PATH;


				if (!is_file(SQ_TOOL_VIRUS_CHECKER_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Virus Checker'); ?></span><?php
				} else if (!is_executable(SQ_TOOL_VIRUS_CHECKER_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'Virus Checker'); ?></span><?php
				}//end if

		$o->closeSection();
		
		$o->openSection(translate('javascript_configuration'));
			$o->openField(translate('path_to_javascript'));
				echo SQ_TOOL_JS_PATH;

				?><br /><?php
				echo translate('javascript_command_note');
				if (!is_file(SQ_TOOL_JS_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'JavaScript'); ?></span><?php
				} else if (!is_executable(SQ_TOOL_JS_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'JavaScript'); ?></span><?php
				}//end if
		$o->closeSection();
		
		$o->openSection(translate('mollom_configuration'));

			$o->openField(translate('mollom_test_mode'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_TOOL_MOLLOM_TEST_MODE]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_MOLLOM_TEST_MODE);
				} else {
					echo (SQ_TOOL_MOLLOM_TEST_MODE) ? translate('yes') : translate('no');
				}
				?><br /><?php
				echo translate('mollom_test_mode_note');
			$o->openField(translate('mollom_public_key'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_MOLLOM_PUBLIC_KEY]', SQ_TOOL_MOLLOM_PUBLIC_KEY, 35);
				} else {
					echo SQ_TOOL_MOLLOM_PUBLIC_KEY;
				}//end if
				?><br /><?php
				echo translate('mollom_public_key_note');
		
			$o->openField(translate('mollom_private_key'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_MOLLOM_PRIVATE_KEY]', SQ_TOOL_MOLLOM_PRIVATE_KEY, 35);
				} else {
					echo SQ_TOOL_MOLLOM_PRIVATE_KEY;
				}//end if
				?><br /><?php
				echo translate('mollom_private_key_note');
			
			$o->openField(translate('mollom_valid_keys'));
				// Check that keys are valid
				if (SQ_TOOL_MOLLOM_PRIVATE_KEY && SQ_TOOL_MOLLOM_PUBLIC_KEY) {
					require_once SQ_FUDGE_PATH.'/mollom/mollom.inc';
					try {
						Mollom::setPublicKey(SQ_TOOL_MOLLOM_PUBLIC_KEY);
						Mollom::setPrivateKey(SQ_TOOL_MOLLOM_PRIVATE_KEY);
      					if (!empty($_SESSION['SQ_MOLLOM_SERVER_LIST'])) {
				            $server_list = unserialize($_SESSION['SQ_MOLLOM_SERVER_LIST']);
            				Mollom::setServerList($server_list);
			        	} else {
							$server_list = Mollom::getServerList();
	            			$_SESSION['SQ_MOLLOM_SERVER_LIST'] = serialize($server_list);
				        }
						$valid_keys = Mollom::verifyKey();
						echo $valid_keys ? '<span style="color: green"><b>'.translate('valid').'</b></span>' : '<span style="color: red"><b>'.translate('invalid').'</b></span>';
					} catch (Exception $e) {
						echo '<span style="color: black"><b>'.translate('unknown').'</b></span>';
					}
				}

			$o->openField(translate('mollom_accept_bypassed_submission_question'));
				if ($write_access && $is_root) {
					combo_box($class.'[SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION]', Array('1' => translate('yes'), '0' => translate('no')), FALSE, SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION);
				} else {
					echo (SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION) ? translate('yes') : translate('no');
				}
				?><br /><?php
				echo translate('mollom_accept_bypassed_submission_note');
		$o->closeSection();
		
		// Squid caching
		$o->openSection(translate('squid_cache_section'));

			$o->openField(translate('squid_tool_hostname'));
				if ($write_access) {
					text_area($class.'[SQ_TOOL_SQUID_CACHE_HOSTNAMES]', SQ_TOOL_SQUID_CACHE_HOSTNAMES, 80, 4);
				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_HOSTNAMES);
				}
				 echo $o->note(translate('squid_tool_pref_note'));
			$o->closeField();
			$o->openField(translate('squid_tool_path_field'));
				echo nl2br(SQ_TOOL_SQUID_CACHE_PATH);
			$o->closeField();
			$o->openField(translate('squid_tool_port_field'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_CACHE_PORT]', SQ_TOOL_SQUID_CACHE_PORT, 30);
				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_PORT);
				}
			$o->closeField();
			$o->openField(translate('squid_tool_option_field'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_CACHE_OPTION]', SQ_TOOL_SQUID_CACHE_OPTION, 30);
					$o->note(translate('squid_tool_option_note'));
				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_OPTION);
				}
			$o->closeField();
			$o->openField(translate('squid_tool_slash_field'));
				$val = SQ_TOOL_SQUID_CACHE_SLASH;
				if ($write_access) {
					$selection = Array(
								1	=> translate('true'),
								0	=> translate('false'),
							 );
				combo_box($class.'[SQ_TOOL_SQUID_CACHE_SLASH]', $selection, FALSE, $val);
				} else {
					?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $val ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
				}
			$o->closeField();
			$o->openField(translate('squid_tool_url_port_field'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_URL_PORT]', SQ_TOOL_SQUID_URL_PORT, 10);
					$o->note(translate('squid_tool_url_port_note'));
				} else {
					echo nl2br(SQ_TOOL_SQUID_URL_PORT);
				}
			$o->closeField();
		$o->closeSection();
		
		// Sendmail for bulkmail
		$o->openSection(translate('sendmail'));
			$o->openField(translate('sendmail_path'));
				echo SQ_TOOL_SENDMAIL_PATH;
				echo $o->note(translate('sendmail_path_note'));
			$o->closeField();
			$o->openField(translate('sendmail_arguments'));
				echo SQ_TOOL_SENDMAIL_ARGS;
				echo $o->note(translate('sendmail_arguments_note'));
			$o->closeField();
		$o->closeSection();	
				
		
		// simplesamlphp
		$o->openSection(translate('saml_authentication'));
		$o->openField(translate('simplesamlphp_path'));
		    echo SQ_TOOL_SIMPLESAMLPHP_PATH;
		    echo $o->note(translate('simplesamlphp_path_note'));
		$o->closeField();
		$o->closeSection();

		// Recaptcha
		$o->openSection(translate('recaptcha_configuration'));
			$o->openField(translate('recaptcha_public_key'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_RECAPTCHA_PUBLIC_KEY]', SQ_TOOL_RECAPTCHA_PUBLIC_KEY, 50);
				} else {
					echo SQ_TOOL_RECAPTCHA_PUBLIC_KEY;
				}//end if
				?><br /><?php
				echo translate('recaptcha_public_key_note');
			$o->closeField();

			$o->openField(translate('recaptcha_private_key'));
				if ($write_access) {
					text_box($class.'[SQ_TOOL_RECAPTCHA_PRIVATE_KEY]', SQ_TOOL_RECAPTCHA_PRIVATE_KEY, 50);
				} else {
					echo SQ_TOOL_RECAPTCHA_PRIVATE_KEY;
				}//end if
				?><br /><?php
				echo translate('recaptcha_private_key_note');
			$o->closeField();
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


}//end class

?>

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
						'SQ_TOOL_CLOSURE_COMPILER_PATH'			=> Array('editable' => 1, 'default' => ''),
						'SQ_TOOL_AUTHORISED_PATHS'				=> Array('editable' => 1, 'default' => ''),
				);


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->config_file = SQ_DATA_PATH.'/private/conf/tools.inc';
		$this->name = translate('External Tools Configuration');

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
				trigger_localised_error('SYS0267', sprintf(translate('You need to be either the Root User or a System Administrator to save the %s'), $this->name), E_USER_WARNING);
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

		$o->openSection(translate('Active External Tools'));


			$o->openField(translate('Enable HTML Tidy'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_HTML_TIDY_ENABLED);


			} else {
				echo (SQ_TOOL_HTML_TIDY_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('HTML Tidy Accessibility Check Level'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL]', Array('0', '1', '2', '3'), FALSE, SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL);
			} else {
				echo (SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL) ? SQ_TOOL_HTML_TIDY_ACCESSIBILITY_LEVEL : 0;
			}

			$o->openField(translate('HTML Standard to use for HTML Tidy'), '', '');

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
				$o->note('<span style="color: red;">'.translate('Your current System Default Character Set is not supported by HTML Tidy.').'</span>');

			}

			$o->openField(translate('Enable pdftohtml'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_PDFTOHTML_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_PDFTOHTML_ENABLED);


			} else {
				echo (SQ_TOOL_PDFTOHTML_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable Antiword'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_ANTIWORD_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_ANTIWORD_ENABLED);


			} else {
				echo (SQ_TOOL_ANTIWORD_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField('Enable Photoshop Metadata Extraction');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_JPEG_TOOLKIT_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_JPEG_TOOLKIT_ENABLED);


			} else {
				echo (SQ_TOOL_JPEG_TOOLKIT_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable Markdown'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_MARKDOWN_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_MARKDOWN_ENABLED);


			} else {
				echo (SQ_TOOL_MARKDOWN_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable Markdownify'), '', '');

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_MARKDOWNIFY_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_MARKDOWNIFY_ENABLED);


			} else {
				echo (SQ_TOOL_MARKDOWNIFY_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable getID3 Extraction'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_GETID3_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_GETID3_ENABLED);


			} else {
				echo (SQ_TOOL_GETID3_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable OGG Metadata Extraction'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED);


			} else {
				echo (SQ_TOOL_OGG_METADATA_EXTRACTION_ENABLED) ? translate('Yes') : translate('No');


			}

			$o->openField(translate('Enable Virus Checker'));

			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_VIRUS_CHECKER_ENABLED]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_VIRUS_CHECKER_ENABLED);


			} else {
				echo (SQ_TOOL_VIRUS_CHECKER_ENABLED) ? translate('Yes') : translate('No');


			}

		$o->closeSection();

		$o->openSection(translate('WYSIWYG Editor Tools'));


			$o->openField(translate('Path to HTML Tidy'));

				echo SQ_TOOL_HTML_TIDY_PATH;

				if (!is_file(SQ_TOOL_HTML_TIDY_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'HTML Tidy'); ?></span><?php

				} else if (!is_executable(SQ_TOOL_HTML_TIDY_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not executable'), 'HTML Tidy'); ?></span><?php

				}//end if

			$o->openField(translate('Spell Checker Language'));

			if ($write_access) {
				// if spell checker is not installed, disable this pref
				text_box($class.'[SQ_TOOL_SPELL_CHECKER_LANG]', SQ_TOOL_SPELL_CHECKER_LANG, 6, 6);
				$o->note(translate('Language code consists of the two letter ISO 639 language code and an optional two letter ISO 3166 country code after a dash or underscore'));

			} else {
				echo SQ_TOOL_SPELL_CHECKER_LANG;
			}

			if (!function_exists('pspell_suggest')) {
				?><br /><span class="sq-backend-warning"><?php echo translate('Pspell is not installed'); ?></span><?php

			} else {
				$old_reporting = error_reporting(0);
					$status = pspell_new(SQ_TOOL_SPELL_CHECKER_LANG);
				error_reporting($old_reporting);
				if ($status === FALSE) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('Pspell couldn\'t open the dictionary, no word lists can be found for the language "%s"'), SQ_TOOL_SPELL_CHECKER_LANG); ?></span><?php

				}
			}

		$o->closeSection();

		$o->openSection(translate('Keyword Extraction Tools'));


			$o->openField(translate('Path to pdftohtml'));

			echo SQ_TOOL_PDFTOHTML_PATH;


			if (!is_file(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'PDFTOHTML'); ?></span><?php

			} else if (!is_executable(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not executable'), 'PDFTOHTML'); ?></span><?php

			}

			$o->openField(translate('Path to Antiword'));

			echo SQ_TOOL_ANTIWORD_PATH;


			if (!is_file(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Antiword');?></span><?php

			} else if (!is_executable(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not executable'), 'Antiword'); ?></span><?php

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

		$o->openSection(translate('Markdown Tools'));


			$o->openField(translate('Path to Markdown'));

			echo SQ_TOOL_MARKDOWN_PATH;


			if (!is_file(SQ_TOOL_MARKDOWN_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Markdown'); ?></span><?php

			} else {
				include_once SQ_TOOL_MARKDOWN_PATH;
				if (!function_exists('Markdown')) {
					?><br /><span class="sq-backend-warning"><?php echo translate('The markdown PHP file does not have the required Markdown function'); ?></span><?php

				}
			}//end if

			$o->openField(translate('Path to Markdownify'));

			echo SQ_TOOL_MARKDOWNIFY_PATH;


			if (!is_file(SQ_TOOL_MARKDOWNIFY_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Markdownify'); ?></span><?php

			} else {
				include_once SQ_TOOL_MARKDOWNIFY_PATH;
				$class_name = basename(SQ_TOOL_MARKDOWNIFY_PATH, '.php');
				if (!method_exists($class_name, 'parseString')) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The markdownify PHP file does not have the required %s class (took from file name) or parseString method'), $class_name); ?></span><?php

				}
			}//end if

		$o->closeSection();

		$o->openSection(translate('getID3 Extraction'));


			$o->openField(translate('Path to getID3'));

			echo SQ_TOOL_GETID3_PATH;


			if (!is_dir(SQ_TOOL_GETID3_PATH)) {
				?><br /><span class="sq-backend-warning">Supplied path is not valid</span><?php
			}

		$o->closeSection();

		$o->openSection(translate('OGG Metadata Extraction'));


			$o->openField(translate('Path to OGG Metadata library'));

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

		$o->openSection(translate('Virus Checker'));


			$o->openField(translate('Virus Checker Type'));

				$av_types = Array(
								'clam'		=> translate('Clam Antivirus'),

								'f-prot'	=> translate('F-Prot Antivirus'),

							);
				if ($write_access) {
					combo_box($class.'[SQ_TOOL_VIRUS_CHECKER_TYPE]', $av_types, FALSE, SQ_TOOL_VIRUS_CHECKER_TYPE);
				} else {
					switch (SQ_TOOL_VIRUS_CHECKER_TYPE) {
						case 'f-prot':
							echo translate('F-Prot Antivirus');

							break;
						default:
							echo translate('Clam Antivirus');

					}//end switch
				}//end if

			$o->openField(translate('Path to Virus Checker'));

				echo SQ_TOOL_VIRUS_CHECKER_PATH;


				if (!is_file(SQ_TOOL_VIRUS_CHECKER_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Virus Checker'); ?></span><?php

				} else if (!is_executable(SQ_TOOL_VIRUS_CHECKER_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not executable'), 'Virus Checker'); ?></span><?php

				}//end if

		$o->closeSection();

		$o->openSection(translate('JavaScript Configuration'));

			$o->openField(translate('JavaScript Command'));

				echo SQ_TOOL_JS_PATH;

				?><br /><?php
				echo $o->note(translate('To include E4X support in Spidermonkey, use /usr/bin/js -x'));

				if (!is_file(SQ_TOOL_JS_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'JavaScript'); ?></span><?php

				} else if (!is_executable(SQ_TOOL_JS_PATH)) {
					?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not executable'), 'JavaScript'); ?></span><?php

				}//end if
		$o->closeSection();

		$o->openSection(translate('Mollom Spam Detector'));


			$o->openField(translate('Test Mode'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_TOOL_MOLLOM_TEST_MODE]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_MOLLOM_TEST_MODE);


				} else {
					echo (SQ_TOOL_MOLLOM_TEST_MODE) ? translate('Yes') : translate('No');


				}
				?><?php
				echo $o->note(translate('Test mode allows a user\'s posted content to be tagged as either "ham", "spam", or "unsure" by using the respective keywords in the form field without affecting the poster\'s "reputation". The content type of the field used for testing must be "Post Body".'));

			$o->openField(translate('Public Key'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_MOLLOM_PUBLIC_KEY]', SQ_TOOL_MOLLOM_PUBLIC_KEY, 35);
				} else {
					echo SQ_TOOL_MOLLOM_PUBLIC_KEY;
				}//end if
				?><?php
				echo $o->note(translate('Public key supplied by Mollom'));


			$o->openField(translate('Private Key'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_MOLLOM_PRIVATE_KEY]', SQ_TOOL_MOLLOM_PRIVATE_KEY, 35);
				} else {
					echo SQ_TOOL_MOLLOM_PRIVATE_KEY;
				}//end if
				?><?php
				echo $o->note(translate('Private key supplied by Mollom'));


			$o->openField(translate('Valid Keys?'));

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
						echo $valid_keys ? '<span style="color: green"><b>'.translate('Valid').'</b></span>' : '<span style="color: red"><b>'.translate('Invalid').'</b></span>';


					} catch (Exception $e) {
						echo '<span style="color: black"><b>'.translate('Unknown').'</b></span>';

					}
				}

			$o->openField(translate('Accept Bypassed Submission?'));

				if ($write_access && $is_root) {
					combo_box($class.'[SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION]', Array('1' => translate('Yes'), '0' => translate('No')), FALSE, SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION);


				} else {
					echo (SQ_TOOL_MOLLOM_ACCEPT_BYPASSED_SUBMISSION) ? translate('Yes') : translate('No');


				}
				?><?php
				echo $o->note(translate('If set to yes, the submission will be accepted if Mollom fails to respond as expected.'));

		$o->closeSection();

		// Squid caching
		$o->openSection(translate('Clear Squid Cache'));


			$o->openField(translate('Host Names'));

				if ($write_access) {
					text_area($class.'[SQ_TOOL_SQUID_CACHE_HOSTNAMES]', SQ_TOOL_SQUID_CACHE_HOSTNAMES, 80, 4);
				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_HOSTNAMES);
				}
				 echo $o->note(translate('Enter the Host Names separated by newlines, to be used in the command "squidclient -h hostname -m PURGE $url".'));

			$o->closeField();
			$o->openField(translate('Path'));

				echo nl2br(SQ_TOOL_SQUID_CACHE_PATH);
			$o->closeField();
			$o->openField(translate('Port Number'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_CACHE_PORT]', SQ_TOOL_SQUID_CACHE_PORT, 30);
				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_PORT);
				}
			$o->closeField();
			$o->openField(translate('Options'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_CACHE_OPTION]', SQ_TOOL_SQUID_CACHE_OPTION, 30);
					$o->note(translate('Additonal options to be included in the purge command.'));

				} else {
					echo nl2br(SQ_TOOL_SQUID_CACHE_OPTION);
				}
			$o->closeField();
			$o->openField(translate('Trailing Slash'));

				$val = SQ_TOOL_SQUID_CACHE_SLASH;
				if ($write_access) {
					$selection = Array(
								1	=> translate('True'),

								0	=> translate('False'),

							 );
				combo_box($class.'[SQ_TOOL_SQUID_CACHE_SLASH]', $selection, FALSE, $val);
				} else {
					?><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $val ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /><?php
				}
			$o->closeField();
			$o->openField(translate('Add Port to URL'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_SQUID_URL_PORT]', SQ_TOOL_SQUID_URL_PORT, 10);
					$o->note(translate('You can specify a port to add to each URL when submitted to Squid. If you are running Squiz Matrix on a non-standard port behind Squid, you may need to add the port back to the URL to clear the cache.'));

				} else {
					echo nl2br(SQ_TOOL_SQUID_URL_PORT);
				}
			$o->closeField();
		$o->closeSection();

		// Sendmail for bulkmail
		$o->openSection(translate('Sendmail'));

			$o->openField(translate('Path'));

				echo SQ_TOOL_SENDMAIL_PATH;
				echo $o->note(translate('The path to your sendmail binary. Eg. /usr/sbin/sendmail. Use config file data/private/conf/tools.inc.'));

			$o->closeField();
			$o->openField(translate('Arguments'));

				echo SQ_TOOL_SENDMAIL_ARGS;
				echo $o->note(translate('Eg. -t will run sendmail as \'sendmail -t\'. Use config file data/private/conf/tools.inc.'));

			$o->closeField();
		$o->closeSection();


		// simplesamlphp
		$o->openSection(translate('SAML Authentication'));

		$o->openField(translate('Simplesamlphp Path'));

		    echo SQ_TOOL_SIMPLESAMLPHP_PATH;
		    echo $o->note(translate('Install location of Simplesamlphp program'));

		$o->closeField();
		$o->closeSection();

		// Recaptcha
		$o->openSection(translate('Google reCAPTCHA'));

			$o->openField(translate('Public Key'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_RECAPTCHA_PUBLIC_KEY]', SQ_TOOL_RECAPTCHA_PUBLIC_KEY, 50);
				} else {
					echo SQ_TOOL_RECAPTCHA_PUBLIC_KEY;
				}//end if
				?><?php
				echo $o->note(translate('Global public key supplied by reCAPTCHA'));

			$o->closeField();

			$o->openField(translate('Private Key'));

				if ($write_access) {
					text_box($class.'[SQ_TOOL_RECAPTCHA_PRIVATE_KEY]', SQ_TOOL_RECAPTCHA_PRIVATE_KEY, 50);
				} else {
					echo SQ_TOOL_RECAPTCHA_PRIVATE_KEY;
				}//end if
				?><?php
				echo $o->note(translate('Global private key supplied by reCAPTCHA'));

			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('Closure Compiler'));

			$o->openField(translate('Path'));

			echo SQ_TOOL_CLOSURE_COMPILER_PATH;
			if (!is_file(SQ_TOOL_CLOSURE_COMPILER_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo sprintf(translate('The path to %s is not valid'), 'Closure compiler'); ?></span><?php

			}
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('Authorised File Paths'));

			$o->openField(translate('File Paths'));


			$paths = explode("\n", SQ_TOOL_AUTHORISED_PATHS);
			foreach($paths as $index => $val) {
				$paths[$index] = trim($paths[$index]);
				if (empty($paths[$index])) {
					unset($paths[$index]);
					continue;
				}
				if (!empty($paths[$index]) && !is_file($paths[$index])) {
					$paths[$index] = rtrim($paths[$index], '/').'/';
				}
			}//end foreach

			// Public data dir and temp asset import dir are always authorised by default
			$paths[] = SQ_DATA_PATH.'/public/';
			$paths[] = SQ_TEMP_PATH.'/import_assets/';

			$paths = array_unique($paths);
			foreach($paths as $path) {
				echo $path.'<br />';
			}

			echo $o->note(translate('Authorised locations to source the files from for backend operations, like \'Create File Asset\' trigger action.'));

			$o->closeField();
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


}//end class

?>

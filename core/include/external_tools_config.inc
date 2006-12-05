<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: external_tools_config.inc,v 1.18 2006/12/05 05:36:14 bcaldwell Exp $
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
* @version $Revision: 1.18 $
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
						'SQ_TOOL_HTML_TIDY_PATH'		=> Array('editable' => 1, 'default' => '/usr/bin/tidy'),
						'SQ_TOOL_HTML_TIDY_ENABLED'		=> Array('editable' => 1, 'default' => false),
						'SQ_TOOL_PDFTOHTML_PATH'		=> Array('editable' => 1, 'default' => '/usr/bin/pdftohtml'),
						'SQ_TOOL_PDFTOHTML_ENABLED'		=> Array('editable' => 1, 'default' => false),
						'SQ_TOOL_ANTIWORD_PATH'			=> Array('editable' => 1, 'default' => '/usr/bin/antiword'),
						'SQ_TOOL_ANTIWORD_MAPPING_PATH'	=> Array('editable' => 1, 'default' => '/usr/share/antiword'),
						'SQ_TOOL_ANTIWORD_ENABLED'		=> Array('editable' => 1, 'default' => false),
					   );


	/**
	* Constructor
	*
	*/
	function External_Tools_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/tools.inc';

	}//end constructor


	/**
	* Rewrites the conf file with current variables
	*
	* @param Array		$vars				the array with any new values (that are allowed to be edited)
	*										Array('[config_var_name]' => [scalar value])
	* @param boolean	$backup_existing	whether we should backup the existing config file first
	* @param boolean 	$send_message		whether to send a message using the messaging system
	*
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false, $send_message=true)
	{
		if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, go away
			if (!$is_admin) {
				trigger_localised_error('SYS0267', E_USER_WARNING, $this->name);
				return false;
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
		if (!parent::writeAccess()) return false;
		if (!$GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
			return true;
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
	* @param object Backend_Outputter	&$o		reference to the backend outputter
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

		$class = get_class($this);

		$o->openSection(translate('active_external_tools'));

			$o->openField(translate('enable_html_tidy'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_HTML_TIDY_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), false, SQ_TOOL_HTML_TIDY_ENABLED);
			} else {
				echo (SQ_TOOL_HTML_TIDY_ENABLED) ? translate('yes') : translate('no');
			}

			require_once SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';
			$tidy_char_set = array_get_index($standards_lists_tidy_char_sets, SQ_CONF_DEFAULT_CHARACTER_SET);

			if (empty($tidy_char_set)) {
				$o->note('<span style="color: red;">'.translate('tidy_unsupported_char_set_warning').'</span>');
			}

			$o->openField(translate('enable_pdftohtml'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_PDFTOHTML_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), false, SQ_TOOL_PDFTOHTML_ENABLED);
			} else {
				echo (SQ_TOOL_PDFTOHTML_ENABLED) ? translate('yes') : translate('no');
			}

			$o->openField(translate('enable_antiword'), '', '');
			if ($write_access && $is_root) {
				combo_box($class.'[SQ_TOOL_ANTIWORD_ENABLED]', Array('1' => translate('yes'), '0' => translate('no')), false, SQ_TOOL_ANTIWORD_ENABLED);
			} else {
				echo (SQ_TOOL_ANTIWORD_ENABLED) ? translate('yes') : translate('no');
			}

		$o->closeSection();

		$o->openSection(translate('wysiwyg_editor_tools'));

			$o->openField(translate('path_to_html_tidy'));
			if ($write_access) {
				text_box($class.'[SQ_TOOL_HTML_TIDY_PATH]', SQ_TOOL_HTML_TIDY_PATH, 30);
			} else {
				echo SQ_TOOL_HTML_TIDY_PATH;
			}

			if (!is_file(SQ_TOOL_HTML_TIDY_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'HTML Tidy'); ?></span><?php
			} else if (!is_executable(SQ_TOOL_HTML_TIDY_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'HTML Tidy'); ?></span><?php
			}

		$o->closeSection();

		$o->openSection(translate('keyword_extraction_tools'));

			$o->openField(translate('path_to_pdftohtml'));
			if ($write_access) {
				text_box($class.'[SQ_TOOL_PDFTOHTML_PATH]', SQ_TOOL_PDFTOHTML_PATH, 30);
			} else {
				echo SQ_TOOL_PDFTOHTML_PATH;
			}

			if (!is_file(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'PDFTOHTML'); ?></span><?php
			} else if (!is_executable(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'PDFTOHTML'); ?></span><?php
			}

			$o->openField(translate('path_to_antiword'));
			if ($write_access) {
				text_box($class.'[SQ_TOOL_ANTIWORD_PATH]', SQ_TOOL_ANTIWORD_PATH, 30);
			} else {
				echo SQ_TOOL_ANTIWORD_PATH;
			}

			if (!is_file(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_valid', 'Antiword');?></span><?php
			} else if (!is_executable(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning"><?php echo translate('path_to_not_executable', 'Antiword'); ?></span><?php
			}

			$o->openField('Path to Antiword Mappings');
			if ($write_access) {
				text_box($class.'[SQ_TOOL_ANTIWORD_MAPPING_PATH]', SQ_TOOL_ANTIWORD_MAPPING_PATH, 30);
			} else {
				echo SQ_TOOL_ANTIWORD_MAPPING_PATH;
			}

			if (!is_dir(SQ_TOOL_ANTIWORD_MAPPING_PATH)) {
				?><br /><span class="sq-backend-warning">The path to Antiword Mappings is not valid</span><?php
			}



		$o->closeSection();
		if ($write_access) $o->commitButton('', true);

	}//end paintBackend()


}//end class

?>

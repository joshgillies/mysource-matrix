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
* $Id: external_tools_config.inc,v 1.2 2004/09/08 23:39:56 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
		
		// HTML Tidy - Used by WYSIWYG Plugin
		'SQ_TOOL_HTML_TIDY_PATH'			=> Array('editable' => 1, 'default' => '/usr/bin/tidy'),
		
		// PDF to HTML - Used to extract keywords from PDF_File assets
		'SQ_TOOL_PDFTOHTML_PATH'			=> Array('editable' => 1, 'default' => '/usr/bin/pdftohtml'),
		
		// Antiword - Used to extract keywords from Word_Doc assets
		'SQ_TOOL_ANTIWORD_PATH'				=> Array('editable' => 1, 'default' => '/usr/bin/antiword'),
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
	
	* @return boolean	indicates whether the file was written
	* @access public
	*/
	function save($vars, $backup_existing=false, $send_message=true)
	{
		if (empty($GLOBALS['SQ_INSTALL'])) {
			$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
			$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

			// if we dont have access, go away
			if (!$is_admin) {
				trigger_error('You need to be either the Root User or a System Administrator to save the '.$this->name, E_USER_WARNING);
				return false;
			}
		}

		return parent::save($vars, $backup_existing, $send_message);

	}//end save()


	/**
	* Paint's out the code for the config file for the passed config
	* This allows extra things to be added for special cases
	*
	* @param string $name	the config var name
	* @param mixed  $value	the value that the config var has
	*
	* @return boolean	whether the value is valid or not
	*/
	function _paintConfigVar($var_name, $value)
	{
		return parent::_paintConfigVar($var_name, $value);

	}//end _paintConfigVar()


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
		if (!empty($GLOBALS['SQ_INSTALL'])) return true;
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

		$o->openSection('WYSIWYG Editor Tools');

			$o->openField('Path to HTML Tidy');
			if ($write_access) {
				text_box($class.'[SQ_TOOL_HTML_TIDY_PATH]', SQ_TOOL_HTML_TIDY_PATH, 30);
			} else {
				echo SQ_TOOL_HTML_TIDY_PATH;
			}

			if (!is_file(SQ_TOOL_HTML_TIDY_PATH)) {
				?><br /><span class="sq-backend-warning">The path to HTML Tidy is not valid</span><?php
			} else if (!is_executable(SQ_TOOL_HTML_TIDY_PATH)) {
				?><br /><span class="sq-backend-warning">The path to HTML Tidy is not executable</span><?php
			}
			
		$o->closeSection();
		
		$o->openSection('Keyword Extraction Tools');
			
			$o->openField('Path to pdftohtml');
			if ($write_access) {
				text_box($class.'[SQ_TOOL_PDFTOHTML_PATH]', SQ_TOOL_PDFTOHTML_PATH, 30);
			} else {
				echo SQ_TOOL_PDFTOHTML_PATH;
			}

			if (!is_file(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning">The path to PDFTOHTML is not valid</span><?php
			} else if (!is_executable(SQ_TOOL_PDFTOHTML_PATH)) {
				?><br /><span class="sq-backend-warning">The path to PDFTOHTML is not executable</span><?php
			}

			$o->openField('Path to Antiword');
			if ($write_access) {
				text_box($class.'[SQ_TOOL_ANTIWORD_PATH]', SQ_TOOL_ANTIWORD_PATH, 30);
			} else {
				echo SQ_TOOL_ANTIWORD_PATH;
			}

			if (!is_file(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning">The path to Antiword is not valid</span><?php
			} else if (!is_executable(SQ_TOOL_ANTIWORD_PATH)) {
				?><br /><span class="sq-backend-warning">The path to Antiword is not executable</span><?php
			}

		$o->closeSection();
		
		if ($write_access) $o->commitButton('Commit', true);

	}//end paintBackend()


}//end class

?>
